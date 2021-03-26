/**
    Three Wire Packet types (From BLUETOOTH SPECIFICATION V4.2 [Vol 4, Part D], 8.X

    |Type  | Name            | Byte pattern
    -------+-----------------+--------------------------------
    | 15   | SYNC MESSAGE    | 0x01 0x7e
    | 15   | SYNC RESPONSE   | 0x02 0x7d
    | 15   | CONFIG MESSAGE  | 0x03 0xfc CONFIGURATION_FIELD
    | 15   | CONFIG RESPONSE | 0x04 0x7b CONFIGURATION_FIELD
    | 15   | WAKEUP MESSAGE  | 0x05 0xfa
    | 15   | WOKEN MESSAGE   | 0x06 0xf9
    | 15   | SLEEP MESSAGE   | 0x07 0x78

*/

#include <iostream>

#include "h5_transport.h"

#include "nrf_error.h"
#include "sd_rpc_types.h"

#include "h5.h"
#include "slip.h"

#include "fmt_helper.h"
#include "log_helper.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <thread>
#include <uart_transport.h>

#include <fmt/format.h>

// Number of times to send reliable packets before giving in
constexpr uint8_t PACKET_RETRANSMISSIONS = 6;

// Other constants

// Duration to wait for state ACTIVE after open is called
const auto OPEN_WAIT_TIMEOUT = std::chrono::milliseconds(3000);
// Duration to wait before continuing UART communication after reset is sent to target
const auto RESET_WAIT_DURATION = std::chrono::milliseconds(300);

#pragma region Public methods
H5Transport::H5Transport(UartTransport *_nextTransportLayer, const uint32_t retransmission_interval)
    : nextTransportLayer(_nextTransportLayer)
    , seqNum(0)
    , ackNum(0)
    , c0Found(false)
    , retransmissionInterval(std::chrono::milliseconds(retransmission_interval))
    , incomingPacketCount(0)
    , outgoingPacketCount(0)
    , errorPacketCount(0)
    , currentState(h5_state::STATE_START)
    , stateMachineReady(false)
    , isOpen(false)
{}

H5Transport::~H5Transport() noexcept
{
    stopStateMachine();
    delete nextTransportLayer;
}

uint32_t H5Transport::open(const status_cb_t &status_callback, const data_cb_t &data_callback,
                           const log_cb_t &log_callback) noexcept
{
    std::lock_guard<std::recursive_mutex> openLck(isOpenMutex);

    if (isOpen)
    {
        return NRF_ERROR_SD_RPC_H5_TRANSPORT_ALREADY_OPEN;
    }

    isOpen = true;

    auto errorCode = Transport::open(status_callback, data_callback, log_callback);

    if (errorCode != NRF_SUCCESS)
    {
        return errorCode;
    }

    try
    {
        auto logger = getLogger();

        std::unique_lock<std::mutex> lck(currentStateMutex);

        if (!(currentState == h5_state::STATE_START || currentState == h5_state::STATE_CLOSED))
        {
            logger->critical("Not able to open, current state is not valid {}", currentState);
            return NRF_ERROR_SD_RPC_H5_TRANSPORT_STATE;
        }

        // State machine starts in a separate thread.
        // Wait for the state machine to be ready
        setupStateMachine();
        startStateMachine();

        lastPacket.clear();

        statusCallback = std::bind(&H5Transport::statusHandler, this, std::placeholders::_1,
                                   std::placeholders::_2);
        dataCallback   = std::bind(&H5Transport::dataHandler, this, std::placeholders::_1,
                                 std::placeholders::_2);

        errorCode = nextTransportLayer->open(statusCallback, dataCallback, upperLogCallback);

        std::unique_lock<std::mutex> stateMachineLock(stateMachineMutex);

        const auto currentExitCriteria = exitCriterias.at(currentState);
        const auto exitCriteria = dynamic_cast<StartExitCriterias *>(currentExitCriteria.get());

        if (exitCriteria == nullptr)
        {
            logger->warn(
                "h5_transport is in state {} but should be in state {}. This state is not valid.",
                currentState, h5_state::STATE_START);
            return NRF_ERROR_SD_RPC_H5_TRANSPORT_STATE;
        }

        if (errorCode != NRF_SUCCESS)
        {
            exitCriteria->ioResourceError = true;
        }
        else
        {
            exitCriteria->isOpened = true;
        }

        stateMachineLock.unlock();
        stateMachineChange.notify_all();
    }
    catch (const std::out_of_range &)
    {
        return NRF_ERROR_SD_RPC_H5_TRANSPORT_STATE;
    }
    catch (const std::exception &ex)
    {
        LogHelper::tryToLogException(spdlog::level::err, ex,
                                     "Unexpected exception when opening adapter");
        return NRF_ERROR_SD_RPC_H5_TRANSPORT_INTERNAL_ERROR;
    }

    if (waitForState(h5_state::STATE_ACTIVE, OPEN_WAIT_TIMEOUT))
    {
        return NRF_SUCCESS;
    }

    switch (state())
    {
        case h5_state::STATE_START:
        case h5_state::STATE_RESET:
        case h5_state::STATE_UNINITIALIZED:
        case h5_state::STATE_INITIALIZED:
        case h5_state::STATE_NO_RESPONSE:
            // There are two situations on can get timeout:
            // 1) there is no response from the device
            // 2) non failing state transitions from STATE_START to STATE_ACTIVE did not happen
            // in time period OPEN_WAIT_TIMEOUT
            return NRF_ERROR_TIMEOUT;
        case h5_state::STATE_FAILED:
        case h5_state::STATE_CLOSED:
            return NRF_ERROR_SD_RPC_H5_TRANSPORT_STATE;
        case h5_state::STATE_ACTIVE:
            return NRF_SUCCESS;
        default:
            return NRF_ERROR_SD_RPC_H5_TRANSPORT_STATE;
    }
}

uint32_t H5Transport::close() noexcept
{
    std::lock_guard<std::recursive_mutex> openLck(isOpenMutex);

    if (!isOpen)
    {
        return NRF_ERROR_SD_RPC_H5_TRANSPORT_ALREADY_CLOSED;
    }

    isOpen = false;

    {
        std::unique_lock<std::mutex> currentStateLck(currentStateMutex);
        auto logger = getLogger();

        try
        {
            const auto currentExitCriteria = exitCriterias.at(currentState);
            const auto exitCriteria        = currentExitCriteria.get();

            if (exitCriteria != nullptr)
            {
                std::unique_lock<std::mutex> stateMachineLock(stateMachineMutex);
                exitCriteria->close = true;
            }

            stateMachineChange.notify_all();
        }
        catch (const std::out_of_range &)
        {
            logger->warn("State {} does not have exit criteria associated with it. Will continue "
                         "to close the H5Transport.",
                         currentState);
        }
        catch (const std::exception &)
        {
            return NRF_ERROR_SD_RPC_H5_TRANSPORT_STATE;
        }
    }

    try
    {
        stopStateMachine();
    }
    catch (const std::exception &)
    {
        return NRF_ERROR_SD_RPC_H5_TRANSPORT_STATE;
    }

    return nextTransportLayer->close();
}

uint32_t H5Transport::send(const std::vector<uint8_t> &data) noexcept
{
    std::lock_guard<std::recursive_mutex> openLck(isOpenMutex);

    if (!isOpen)
    {
        return NRF_ERROR_SD_RPC_H5_TRANSPORT_STATE;
    }

    if (currentState != h5_state::STATE_ACTIVE)
    {
        return NRF_ERROR_SD_RPC_H5_TRANSPORT_STATE;
    }

    try
    {
        // max theoretical length of encoded packet, aditional 6 bytes h5 encoding and all bytes
        // escaped
        // + 2 packet encapsuling

        h5_payload_t h5EncodedPacket;

        {
            std::unique_lock<std::recursive_mutex> seqNumLck(seqNumMutex);
            std::unique_lock<std::recursive_mutex> ackNumLck(ackNumMutex);
            h5_encode(data, h5EncodedPacket, seqNum, ackNum, true, true,
                      h5_pkt_type::VENDOR_SPECIFIC_PACKET);
        }

        h5_payload_t encodedPacket;
        slip_encode(h5EncodedPacket, encodedPacket);

        auto remainingRetransmissions = PACKET_RETRANSMISSIONS;

        lastPacket.clear();
        lastPacket = encodedPacket;

        std::unique_lock<std::mutex> ackLock(ackMutex);

        while (remainingRetransmissions--)
        {
            logPacket(true, h5EncodedPacket);
            const auto err_code = nextTransportLayer->send(lastPacket);

            if (err_code != NRF_SUCCESS)
                return err_code;

            uint8_t seqNumBefore;

            {
                std::unique_lock<std::recursive_mutex> seqNumLck(seqNumMutex);
                seqNumBefore = seqNum;
            }

            // Checking for timeout. Also checking against spurios wakeup by making sure the
            // sequence number has actually increased. If the sequence number has not increased, we
            // have not received an ACK packet, and should not exit the loop (unless timeout). Ref.
            // spurious wakeup: http://en.cppreference.com/w/cpp/thread/condition_variable
            // https://en.wikipedia.org/wiki/Spurious_wakeup
            if (ackReceived.wait_for(
                    ackLock, std::chrono::milliseconds(retransmissionInterval), [&] {
                        std::unique_lock<std::recursive_mutex> seqNumLck(seqNumMutex);
                        return seqNum != seqNumBefore;
                    }))
            {
                lastPacket.clear();
                return NRF_SUCCESS;
            }
        }

        lastPacket.clear();
    }
    catch (const std::exception &)
    {
        return NRF_ERROR_SD_RPC_H5_TRANSPORT_INTERNAL_ERROR;
    }

    return NRF_ERROR_SD_RPC_H5_TRANSPORT_NO_RESPONSE;
}

h5_state H5Transport::state() const
{
    return currentState;
}

#pragma endregion Public methods

#pragma region Processing incoming data from UART
void H5Transport::processPacket(const h5_payload_t &packet)
{
    uint8_t decodedSeqNum;
    uint8_t decodedAckNum;
    bool decodedReliablePacket;
    h5_pkt_type decodedPacketType;

    h5_payload_t slipPayload;
    auto err_code = slip_decode(packet, slipPayload);

    auto logger = getLogger();

    if (err_code != NRF_SUCCESS)
    {
        ++errorPacketCount;

        logger->error("slip_decode error, code {:#04x}, H5 error count: {}, raw packet: {}",
                      err_code, errorPacketCount.load(), packet);

        return;
    }

    logPacket(false, slipPayload);

    h5_payload_t h5Payload;

    err_code = h5_decode(slipPayload, h5Payload, &decodedSeqNum, &decodedAckNum, nullptr, nullptr,
                         nullptr, &decodedReliablePacket, &decodedPacketType);

    if (err_code != NRF_SUCCESS)
    {
        ++errorPacketCount;
        logger->error("h5_decode error, code: {:#04x}, H5 error count: {}. raw packet: {}",
                      err_code, errorPacketCount, packet);

        return;
    }

    std::unique_lock<std::mutex> currentStateLock(currentStateMutex);

    if (currentState == h5_state::STATE_RESET)
    {
        // Ignore packets packets received in this state.
        stateMachineChange.notify_all();
        return;
    }

    std::unique_lock<std::mutex> stateMachineLock(stateMachineMutex);

    if (decodedPacketType == h5_pkt_type::LINK_CONTROL_PACKET)
    {
        if (currentState == h5_state::STATE_UNINITIALIZED)
        {
            if (H5Transport::isSyncResponsePacket(h5Payload))
            {
                const auto stateCriteria =
                    dynamic_cast<UninitializedExitCriterias *>(exitCriterias[currentState].get());

                if (stateCriteria != nullptr)
                {
                    stateCriteria->syncRspReceived = true;
                }
            }
            else if (H5Transport::isSyncPacket(h5Payload))
            {
                sendControlPacket(control_pkt_type::SYNC_RESPONSE);
            }
        }
        else if (currentState == h5_state::STATE_INITIALIZED)
        {
            const auto exit =
                dynamic_cast<InitializedExitCriterias *>(exitCriterias[currentState].get());

            if (H5Transport::isSyncConfigResponsePacket(h5Payload))
            {
                exit->syncConfigRspReceived = true;
            }
            else if (H5Transport::isSyncConfigPacket(h5Payload))
            {
                sendControlPacket(control_pkt_type::SYNC_CONFIG_RESPONSE);
            }
            else if (H5Transport::isSyncPacket(h5Payload))
            {
                sendControlPacket(control_pkt_type::SYNC_RESPONSE);
            }
        }
        else if (currentState == h5_state::STATE_ACTIVE)
        {
            const auto exit =
                dynamic_cast<ActiveExitCriterias *>(exitCriterias[currentState].get());

            if (H5Transport::isSyncPacket(h5Payload))
            {
                exit->syncReceived = true;
            }
            else if (H5Transport::isSyncConfigPacket(h5Payload))
            {
                sendControlPacket(control_pkt_type::SYNC_CONFIG_RESPONSE);
            }
        }
    }
    else if (decodedPacketType == h5_pkt_type::VENDOR_SPECIFIC_PACKET)
    {
        if (currentState == h5_state::STATE_ACTIVE)
        {
            if (decodedReliablePacket)
            {
                std::lock_guard<std::recursive_mutex> lck(ackNumMutex);

                if (decodedSeqNum == ackNum)
                {
                    incrementAckNum();
                    sendControlPacket(control_pkt_type::ACK, ackNum);
                    upperDataCallback(h5Payload.data(), h5Payload.size());
                }
                else
                {
                    sendControlPacket(control_pkt_type::ACK, ackNum);
                }
            }
        }
    }
    else if (decodedPacketType == h5_pkt_type::ACK_PACKET)
    {
        std::lock_guard<std::recursive_mutex> lck(seqNumMutex);

        if (decodedAckNum == ((seqNum + 1) & 0x07))
        {
            // Received a packet with valid ack_num, inform threads that wait the command is
            // received on the other end
            incrementSeqNum();
            ackReceived.notify_all();
        }
        else if (decodedAckNum == seqNum)
        {
            // Discard packet, we assume that we have received a reply from a previous packet
        }
        else
        {
            if (currentState == h5_state::STATE_ACTIVE)
            {
                const auto exit =
                    dynamic_cast<ActiveExitCriterias *>(exitCriterias[currentState].get());

                if (exit != nullptr)
                {
                    exit->irrecoverableSyncError = true;
                }
            }
            else
            {
                logger->warn("h5_transport received ack packet in state {}. ack_num is: {} seq_num "
                             "is: {}. Ignoring the packet.",
                             currentState, decodedAckNum, decodedSeqNum);
            }
        }
    }

    stateMachineLock.unlock();
    stateMachineChange.notify_all();
}

void H5Transport::statusHandler(const sd_rpc_app_status_t code, const std::string &message) noexcept
{
    auto logger = getLogger();

    if (code == IO_RESOURCES_UNAVAILABLE)
    {
        try
        {
            std::unique_lock<std::mutex> stateMachineLock(stateMachineMutex);
            const auto currentExitCriteria = exitCriterias.at(currentState);
            const auto exitCriteria        = currentExitCriteria.get();

            if (exitCriteria != nullptr)
            {
                exitCriteria->ioResourceError = true;
            }

            stateMachineLock.unlock();
            stateMachineChange.notify_all();
        }
        catch (std::out_of_range &)
        {
            try
            {
                logger->warn("State {} does not have criteria associated with it.", currentState);
            }
            catch (const std::exception &)
            {
                logger->error("Error creating string describing error status");
            }
        }
        catch (std::exception &e)
        {
            logger->error("Unexpected exception received in state {}, {}", currentState, e.what());
        }
    }

    status(code, message);
}

void H5Transport::dataHandler(const uint8_t *data, const size_t length) noexcept
{
    h5_payload_t packet;

    try
    {
        auto logger = getLogger();

        // Check if we have any data from before that has not been processed.
        // If so add the remaining data from previous callback(s) to this packet
        if (!unprocessedData.empty())
        {
            packet.insert(packet.begin(), unprocessedData.begin(), unprocessedData.end());
        }

        for (size_t i = 0; i < length; i++)
        {
            packet.push_back(data[i]);

            if (data[i] == 0xC0)
            {
                if (c0Found)
                {
                    // End of packet found

                    // If we have two 0xC0 after another we assume it is the beginning of a new
                    // packet, and not the end
                    if (packet.size() == 2)
                    {
                        packet.clear();
                        packet.push_back(0xc0);
                        continue;
                    }

                    processPacket(packet);

                    packet.clear();
                    unprocessedData.clear();
                    c0Found = false;
                }
                else
                {
                    // Start of packet found
                    c0Found = true;

                    // Clear previous data from packet since data before the start of packet is
                    // irrelevant.
                    packet.clear();
                    packet.push_back(0xC0);
                }
            }
        }

        if (!packet.empty())
        {
            unprocessedData.clear();
            unprocessedData.insert(unprocessedData.begin(), packet.begin(), packet.end());
        }
    }
    catch (const std::exception &e)
    {
        getLogger()->error("Error processing incoming packet, {}", e.what());
    }
}

void H5Transport::incrementSeqNum()
{
    std::unique_lock<std::recursive_mutex> lck(seqNumMutex);
    seqNum++;
    seqNum = seqNum & 0x07;
}

void H5Transport::incrementAckNum()
{
    std::unique_lock<std::recursive_mutex> lck(ackNumMutex);
    ackNum++;
    ackNum = ackNum & 0x07;
}

#pragma endregion Processing of incoming packets from UART

#pragma region State machine

h5_state H5Transport::stateActionStart()
{
    std::unique_lock<std::mutex> stateMachineLock(stateMachineMutex);
    auto exit = dynamic_cast<StartExitCriterias *>(exitCriterias[h5_state::STATE_START].get());

    stateMachineReady = true;

    // Notify other threads that the state machine is ready
    stateMachineLock.unlock();
    stateMachineChange.notify_all();
    stateMachineLock.lock();

    // Wait for notification of a stateMachineChange that exits the state
    stateMachineChange.wait(stateMachineLock, [&exit] { return exit->isFullfilled(); });

    // Order is of importance when returning state
    if (exit->ioResourceError)
    {
        return h5_state::STATE_FAILED;
    }

    if (exit->close)
    {
        return h5_state::STATE_CLOSED;
    }

    if (exit->isOpened)
    {
        return h5_state::STATE_RESET;
    }

    return h5_state::STATE_FAILED;
}

h5_state H5Transport::stateActionReset()
{
    std::unique_lock<std::mutex> stateMachineLock(stateMachineMutex);
    auto exit = dynamic_cast<ResetExitCriterias *>(exitCriterias[h5_state::STATE_RESET].get());

    // Send the reset packet, and wait for the device to reboot and ready for receiving commands
    sendControlPacket(control_pkt_type::RESET);

    if (statusCallback)
    {
        statusCallback(RESET_PERFORMED, "Target Reset performed");
    }

    exit->resetSent = true;
    stateMachineChange.wait_for(stateMachineLock, RESET_WAIT_DURATION,
                                [&exit] { return exit->isFullfilled(); });
    exit->resetWait = true;

    // Order is of importance when returning state
    if (exit->ioResourceError)
    {
        return h5_state::STATE_FAILED;
    }

    if (exit->close)
    {
        return h5_state::STATE_CLOSED;
    }

    if (exit->resetSent && exit->resetWait)
    {
        return h5_state::STATE_UNINITIALIZED;
    }

    return h5_state::STATE_FAILED;
};

h5_state H5Transport::stateActionUninitialized()
{
    std::unique_lock<std::mutex> stateMachineLock(stateMachineMutex);
    auto exit = dynamic_cast<UninitializedExitCriterias *>(
        exitCriterias[h5_state::STATE_UNINITIALIZED].get());
    auto syncRetransmission = PACKET_RETRANSMISSIONS;

    while (!exit->isFullfilled() && syncRetransmission > 0)
    {
        sendControlPacket(control_pkt_type::SYNC);
        exit->syncSent = true;
        stateMachineChange.wait_for(stateMachineLock, retransmissionInterval,
                                    [&exit] { return exit->isFullfilled(); });
        syncRetransmission--;
    }

    // Order is of importance when returning state
    if (exit->ioResourceError)
    {
        return h5_state::STATE_FAILED;
    }

    if (exit->close)
    {
        return h5_state::STATE_CLOSED;
    }

    if (exit->syncSent && exit->syncRspReceived)
    {
        return h5_state::STATE_INITIALIZED;
    }

    if (syncRetransmission == 0)
    {
        std::stringstream status;
        status << fmt::format("No response from device. Tried to send packet {} times.",
                              PACKET_RETRANSMISSIONS)
                      .c_str();
        statusHandler(PKT_SEND_MAX_RETRIES_REACHED, status.str());
        return h5_state::STATE_NO_RESPONSE;
    }

    return h5_state::STATE_FAILED;
};

h5_state H5Transport::stateActionInitialized()
{
    std::unique_lock<std::mutex> stateMachineLock(stateMachineMutex);
    auto exit =
        dynamic_cast<InitializedExitCriterias *>(exitCriterias[h5_state::STATE_INITIALIZED].get());
    auto syncRetransmission = PACKET_RETRANSMISSIONS;

    // Send a package immediately
    while (!exit->isFullfilled() && syncRetransmission > 0)
    {
        sendControlPacket(control_pkt_type::SYNC_CONFIG);
        exit->syncConfigSent = true;

        stateMachineChange.wait_for(stateMachineLock, retransmissionInterval,
                                    [&exit] { return exit->isFullfilled(); });

        syncRetransmission--;
    }

    // Order is of importance when returning state
    if (exit->ioResourceError)
    {
        return h5_state::STATE_FAILED;
    }

    if (exit->close)
    {
        return h5_state::STATE_CLOSED;
    }

    if (exit->syncConfigSent && exit->syncConfigRspReceived)
    {
        return h5_state::STATE_ACTIVE;
    }

    if (syncRetransmission == 0)
    {
        statusHandler(PKT_SEND_MAX_RETRIES_REACHED,
                      fmt::format("No response from device. Tried to send packet {} times.",
                                  PACKET_RETRANSMISSIONS)
                          .c_str());
        return h5_state::STATE_NO_RESPONSE;
    }

    return h5_state::STATE_FAILED;
};
h5_state H5Transport::stateActionActive()
{
    std::unique_lock<std::mutex> stateMachineLock(stateMachineMutex);
    auto exit = dynamic_cast<ActiveExitCriterias *>(exitCriterias[h5_state::STATE_ACTIVE].get());

    {
        std::unique_lock<std::recursive_mutex> seqNumLck(seqNumMutex);
        std::unique_lock<std::recursive_mutex> ackNumLck(ackNumMutex);

        seqNum = 0;
        ackNum = 0;
    }

    statusHandler(CONNECTION_ACTIVE, "Connection active");
    stateMachineChange.wait(stateMachineLock, [&exit] { return exit->isFullfilled(); }); // T#2

    if (exit->ioResourceError)
    {
        return h5_state::STATE_FAILED;
    }

    if (exit->close)
    {
        return h5_state::STATE_CLOSED;
    }

    if (exit->syncReceived || exit->irrecoverableSyncError)
    {
        return h5_state::STATE_RESET;
    }

    return h5_state::STATE_FAILED;
};

h5_state H5Transport::stateActionFailed()
{
    std::lock_guard<std::mutex> stateMachineLock(stateMachineMutex);
    getLogger()->critical("Entered state failed. No exit exists from this state.");
    return h5_state::STATE_FAILED;
};

h5_state H5Transport::stateActionClosed()
{
    std::lock_guard<std::mutex> stateMachineLock(stateMachineMutex);
    getLogger()->debug("Entered state closed.");
    return h5_state::STATE_CLOSED;
};
h5_state H5Transport::stateActionNoResponse()
{
    std::lock_guard<std::mutex> stateMachineLock(stateMachineMutex);
    getLogger()->debug("No response to data sent to device.");
    return h5_state::STATE_NO_RESPONSE;
};

void H5Transport::setupStateMachine()
{
    // Setup state actions
    //
    // The reason for using lambdas calling data member functions is to be able to easier debug when
    // compiling the project as a release build Doing it this way allows us to see the symbol name
    // when debugging
    stateActions[h5_state::STATE_START]         = [this] { return stateActionStart(); };
    stateActions[h5_state::STATE_RESET]         = [this] { return stateActionReset(); };
    stateActions[h5_state::STATE_UNINITIALIZED] = [this] { return stateActionUninitialized(); };
    stateActions[h5_state::STATE_INITIALIZED]   = [this] { return stateActionInitialized(); };
    stateActions[h5_state::STATE_ACTIVE]        = [this] { return stateActionActive(); };
    stateActions[h5_state::STATE_FAILED]        = [this] { return stateActionFailed(); };
    stateActions[h5_state::STATE_CLOSED]        = [this] { return stateActionClosed(); };
    stateActions[h5_state::STATE_NO_RESPONSE]   = [this] { return stateActionNoResponse(); };

    // Setup exit criteria
    exitCriterias[h5_state::STATE_START] = std::shared_ptr<ExitCriterias>(new StartExitCriterias());
    exitCriterias[h5_state::STATE_RESET] = std::shared_ptr<ExitCriterias>(new ResetExitCriterias());
    exitCriterias[h5_state::STATE_UNINITIALIZED] =
        std::shared_ptr<ExitCriterias>(new UninitializedExitCriterias());
    exitCriterias[h5_state::STATE_INITIALIZED] =
        std::shared_ptr<ExitCriterias>(new InitializedExitCriterias());
    exitCriterias[h5_state::STATE_ACTIVE] =
        std::shared_ptr<ExitCriterias>(new ActiveExitCriterias());
}

void H5Transport::startStateMachine()
{
    if (!stateMachineThread.joinable())
    {
        currentState = h5_state::STATE_START;

        // Lock the stateMachineMutex and let stateMachineThread notify
        // when the state machine is ready to process states
        std::unique_lock<std::mutex> stateMachineLock(stateMachineMutex);
        stateMachineThread = std::thread([this] { stateMachineWorker(); });

        // Wait for the state machine to be ready
        stateMachineChange.wait(stateMachineLock, [this] { return stateMachineReady; });
    }
    else
    {
        // Terminate if the state machine thread is joinable (already runs), this should not happen.
        std::cerr << __FILE__ << ":" << __LINE__
                  << " stateMachineThread exists, this should not happen. Terminating."
                  << std::endl;
        std::terminate();
    }
}

void H5Transport::stopStateMachine()
{
    if (stateMachineThread.joinable())
    {
        stateMachineThread.join();
    }
}

// State machine thread
void H5Transport::stateMachineWorker() noexcept
{
    try
    {
        auto doRun = true;

        while (doRun)
        {
            // Run next state actions
            const auto nextState = stateActions[currentState]();

            // After returning from current state action, lock the current state
            std::unique_lock<std::mutex> currentStateLck(currentStateMutex);

            // Make sure that state is not changed when assigning a new current state
            {
                std::unique_lock<std::mutex> stateMachineLock(stateMachineMutex);
                logStateTransition(currentState, nextState);

                // Reset the next states variables before starting to use them.
                switch (nextState)
                {
                    case h5_state::STATE_START:
                        dynamic_cast<StartExitCriterias *>(
                            exitCriterias[h5_state::STATE_START].get())
                            ->reset();
                        break;
                    case h5_state::STATE_RESET:
                        dynamic_cast<ResetExitCriterias *>(
                            exitCriterias[h5_state::STATE_RESET].get())
                            ->reset();
                        break;
                    case h5_state::STATE_UNINITIALIZED:
                        dynamic_cast<UninitializedExitCriterias *>(
                            exitCriterias[h5_state::STATE_UNINITIALIZED].get())
                            ->reset();
                        break;
                    case h5_state::STATE_INITIALIZED:
                        dynamic_cast<InitializedExitCriterias *>(
                            exitCriterias[h5_state::STATE_INITIALIZED].get())
                            ->reset();
                        break;
                    case h5_state::STATE_ACTIVE:
                        dynamic_cast<ActiveExitCriterias *>(
                            exitCriterias[h5_state::STATE_ACTIVE].get())
                            ->reset();
                        break;
                    case h5_state::STATE_FAILED:
                    case h5_state::STATE_CLOSED:
                    case h5_state::STATE_NO_RESPONSE:
                        // These are terminal states that do not have exit criteria associated with
                        // them
                        break;
                    case h5_state::STATE_UNKNOWN:
                        // Not used
                        break;
                }

                currentState = nextState;

                // Check if current state give any reason to continue running this thread
                if (currentState == h5_state::STATE_FAILED ||
                    currentState == h5_state::STATE_CLOSED ||
                    currentState == h5_state::STATE_NO_RESPONSE)
                {
                    doRun = false;
                }

                // Inform interested parties that new current state is set and ready
                currentStateChange.notify_all();
            }
        }

        stateMachineReady = false;
    }
    catch (const std::exception &e)
    {
        LogHelper::tryToLogException(spdlog::level::critical, e, "Error in state machine thread");
    }
}

bool H5Transport::waitForState(h5_state state, std::chrono::milliseconds timeout)
{
    std::unique_lock<std::mutex> lock(currentStateMutex);
    return currentStateChange.wait_for(lock, timeout,
                                       [&state, this] { return currentState == state; });
}

#pragma endregion State machine related methods

#pragma region Sending packet types

void H5Transport::sendControlPacket(const control_pkt_type type, const uint8_t ackNum)
{
    h5_pkt_type h5_packet;

    if (ackNum == 0xff && type == control_pkt_type::ACK)
    {
        throw std::invalid_argument("Argument ackNum must be set for ACK");
    }

    switch (type)
    {
        case control_pkt_type::RESET:
            h5_packet = h5_pkt_type::RESET_PACKET;
            break;
        case control_pkt_type::SYNC:
        case control_pkt_type::SYNC_RESPONSE:
        case control_pkt_type::SYNC_CONFIG:
        case control_pkt_type::SYNC_CONFIG_RESPONSE:
            h5_packet = h5_pkt_type::LINK_CONTROL_PACKET;
            break;
        case control_pkt_type::ACK:
            h5_packet = h5_pkt_type::ACK_PACKET;
            break;
        default:
            h5_packet = h5_pkt_type::LINK_CONTROL_PACKET;
    }

    h5_payload_t h5Packet;

    try
    {
        const auto payload = getPktPattern(type);
        h5_encode(payload, h5Packet, 0, type == control_pkt_type::ACK ? ackNum : 0, false, false,
                  h5_packet);
    }
    catch (const std::out_of_range &e)
    {
        LogHelper::tryToLogException(spdlog::level::critical, e,
                                     "Trying to send unknown control packet to device, aborting");
        std::terminate();
    }

    h5_payload_t slipPacket;
    slip_encode(h5Packet, slipPacket);

    logPacket(true, h5Packet);

    nextTransportLayer->send(slipPacket);
}

#pragma endregion Methods related to sending packet types defined in the Three Wire Standard

#pragma region Debugging

std::string H5Transport::hciPacketLinkControlToString(const h5_payload_t &payload)
{
    std::stringstream retval;

    const auto configToString = [](uint8_t config) {
        return fmt::format(" sliding-window: {} out-of-frame:{} data-integrity-check-type:{} "
                           "version-number:{} ",
                           config & 0x07, (config & 0x08) ? "1" : "0", (config & 0x0f) ? "1" : "0",
                           (config & 0x0e) >> 5)
            .c_str();
    };

    if (payload.size() >= 2)
    {
        retval << "[";

        if (isSyncPacket(payload))
        {
            retval << "SYNC";
        }
        else if (isSyncResponsePacket(payload))
        {
            retval << "SYNC_RESP";
        }
        else if (isSyncConfigPacket(payload))
        {
            retval << "CONFIG [" << configToString(payload[2]) << "]";
        }
        else if (isSyncConfigResponsePacket(payload))
        {
            retval << "CONFIG_RESP [" << configToString(payload[2]) << "]";
        }
        else
        {
            if (payload[0] == 0x05 && payload[1] == 0xfa)
            {
                retval << "WAKEUP";
            }

            if (payload[0] == 0x06 && payload[1] == 0xf9)
            {
                retval << "WOKEN";
            }

            if (payload[0] == 0x07 && payload[1] == 0x78)
            {
                retval << "SLEEP";
            }
        }

        retval << "]";
    }

    return retval.str();
}

std::string H5Transport::h5PktToString(const bool out, const h5_payload_t &h5Packet) const
{
    h5_payload_t payload;

    uint8_t seq_num;
    uint8_t decodedAckNum;
    bool decodedReliablePacket;
    h5_pkt_type decodedPacketType;
    bool data_integrity;
    uint16_t payload_length;
    uint8_t header_checksum;

    const auto err_code =
        h5_decode(h5Packet, payload, &seq_num, &decodedAckNum, &data_integrity, &payload_length,
                  &header_checksum, &decodedReliablePacket, &decodedPacketType);

    std::stringstream count;

    if (out)
    {
        count << std::setw(8) << outgoingPacketCount << " -> ";
    }
    else
    {
        count << std::setw(5) << incomingPacketCount << "/" << std::setw(2) << errorPacketCount
              << " <- ";
    }

    std::stringstream retval;

    retval << fmt::format(
                  "{} [{}] type:{:>20} reliable:{:>3} seq#:{:04x} ack#:{:04x} payload_length:{} "
                  "data_integrity:{}",
                  count.str(), payload, decodedPacketType, decodedReliablePacket ? "yes" : "no",
                  seq_num, decodedAckNum, payload_length, data_integrity)
                  .c_str();

    if (data_integrity)
    {
        retval << fmt::format(" header_checksum:{:04x}", header_checksum).c_str();
    }

    retval << fmt::format(" err_code:{:#04x}", err_code);

    if (decodedPacketType == h5_pkt_type::LINK_CONTROL_PACKET)
    {
        retval << " " << H5Transport::hciPacketLinkControlToString(payload);
    }

    return retval.str();
}

void H5Transport::logPacket(const bool outgoing, const h5_payload_t &packet)
{
    if (outgoing)
    {
        ++outgoingPacketCount;
    }
    else
    {
        ++incomingPacketCount;
    }

    const std::string logLine = H5Transport::h5PktToString(outgoing, packet);
    getLogger()->debug(logLine);
}

void H5Transport::logStateTransition(h5_state from, h5_state to) const
{
    getLogger()->debug("State change: {} -> {}", from, to);
}

#pragma endregion Debugging related methods

#pragma region Test related methods
bool H5Transport::checkPattern(const h5_payload_t &packet, const uint8_t offset,
                               const h5_payload_t &pattern)
{
    if (offset >= packet.size())
        return false;

    auto packetItr = packet.begin() + offset;

    for (auto patternEntry : pattern)
    {
        if (packetItr == packet.end())
        {
            return false;
        }

        if (*packetItr++ != patternEntry)
        {
            return false;
        }
    }

    return true;
}

h5_payload_t H5Transport::getPktPattern(const control_pkt_type type)
{
    switch (type)
    {
        case control_pkt_type::RESET:
            return {};
        case control_pkt_type::ACK:
            return {};
        case control_pkt_type::SYNC:
            return {SyncFirstByte, SyncSecondByte};
        case control_pkt_type::SYNC_RESPONSE:
            return {SyncRspFirstByte, SyncRspSecondByte};
        case control_pkt_type::SYNC_CONFIG:
            return {SyncConfigFirstByte, SyncConfigSecondByte, SyncConfigField};
        case control_pkt_type::SYNC_CONFIG_RESPONSE:
            return {SyncConfigRspFirstByte, SyncConfigRspSecondByte, SyncConfigField};
        case control_pkt_type::LAST:
            return {};
        default:
            std::stringstream ss;
            ss << "unknown CONTROL packet type 0x" << std::hex << static_cast<uint32_t>(type);
            throw std::out_of_range(ss.str());
    }
}

bool H5Transport::isSyncPacket(const h5_payload_t &packet, const uint8_t offset)
{
    return checkPattern(packet, offset, h5_payload_t{SyncFirstByte, SyncSecondByte});
}

bool H5Transport::isSyncResponsePacket(const h5_payload_t &packet, const uint8_t offset)
{
    return checkPattern(packet, offset, h5_payload_t{SyncRspFirstByte, SyncRspSecondByte});
}

bool H5Transport::isSyncConfigPacket(const h5_payload_t &packet, const uint8_t offset)
{
    return checkPattern(packet, offset, h5_payload_t{SyncConfigFirstByte, SyncConfigSecondByte});
}

bool H5Transport::isSyncConfigResponsePacket(const h5_payload_t &packet, const uint8_t offset)
{
    return checkPattern(packet, offset,
                        h5_payload_t{SyncConfigRspFirstByte, SyncConfigRspSecondByte});
}

bool H5Transport::isResetPacket(const h5_payload_t &packet, const uint8_t offset)
{
    return checkPattern(packet, offset, h5_payload_t{0x05});
}
#pragma endregion Test related methods
