#include "virtual_uart.h"

#include "sd_rpc_types.h"
#include "transport.h"
#include "logger.h"

#include <algorithm>
#include <mutex>
#include <thread>
#include <vector>

VirtualUart::VirtualUart(const std::string &name)
    : Transport()
    , name(name)
    , isOpen(false)
    , peer(nullptr)
    , stopAtPktType(CONTROL_PKT_LAST)
    , stoppedProcessing(false)
{}

void VirtualUart::stopAt(control_pkt_type stopAtPktType_)
{
    stopAtPktType = stopAtPktType_;
}

uint32_t VirtualUart::open(const status_cb_t &status_callback, const data_cb_t &data_callback,
                           const log_cb_t &log_callback)
{
    Transport::open(status_callback, data_callback, log_callback);

    if (peer == nullptr)
    {
        get_logger()->debug("Peer port must be specified before calling open.");
        return NRF_ERROR_INTERNAL;
    }

    isOpen = true;

    outDataThread = std::thread([this]() {
        while (isOpen && stoppedProcessing == false)
        {
            std::unique_lock<std::mutex> lock(outDataMutex);

            if (outData.size() > 0)
            {
                std::for_each(outData.begin(), outData.end(), [&](std::vector<uint8_t> data) {
                    // TODO: do a proper SLIP decoding later on in case header hits SLIP encoding
                    // rules
                    if (H5Transport::isResetPacket(data, 2))
                    {
                        get_logger()->debug("[{}] Requested to send RESET, ignoring since a reset "
                                            "does not make sense in this case.",
                                            name);
                    }
                    else
                    {
                        try
                        {
                            if (peer->isOpen)
                            {
                                peer->injectInData(data);
                            }
                            else
                            {
                                // TODO: report error back
                            }
                        }
                        catch (std::exception &e)
                        {
                            get_logger()->debug("[{}] error sending {}", name, e.what());
                        }
                    }
                });
                outData.erase(outData.begin(), outData.end());
            }

            outDataAvailable.wait(lock, [&] {
                return !(isOpen == true && outData.size() == 0 && stoppedProcessing == false);
            });
        }
    });

    inDataThread = std::thread([this]() {
        while (isOpen)
        {
            std::unique_lock<std::mutex> lock(inDataMutex);

            if (inData.size() > 0)
            {
                std::for_each(inData.begin(), inData.end(), [&](std::vector<uint8_t> data) {
                    // TODO: do a proper SLIP decoding later on in case
                    // TODO: header hits SLIP encoding rules

                    if (H5Transport::isResetPacket(data, 2))
                    {
                        get_logger()->debug("[{}] Received RESET, ignoring", name);
                    }
                    else if (H5Transport::isSyncPacket(data, 5) &&
                             stopAtPktType <= CONTROL_PKT_SYNC)
                    {
                        get_logger()->debug("[{}] Received SYNC ignored.", name);
                        stoppedProcessing = true;
                        outDataAvailable.notify_all();
                    }
                    else if (H5Transport::isSyncResponsePacket(data, 5) &&
                             stopAtPktType <= CONTROL_PKT_SYNC_RESPONSE)
                    {
                        get_logger()->debug("[{}] Received SYNC RESPONSE ignored.", name);
                        stoppedProcessing = true;
                        outDataAvailable.notify_all();
                    }
                    else if (H5Transport::isSyncConfigPacket(data, 5) &&
                             stopAtPktType <= CONTROL_PKT_SYNC_CONFIG)
                    {
                        get_logger()->debug("[{}}] Received SYNC CONFIG ignored.", name);
                        stoppedProcessing = true;
                        outDataAvailable.notify_all();
                    }
                    else if (H5Transport::isSyncConfigResponsePacket(data, 5) &&
                             stopAtPktType <= CONTROL_PKT_SYNC_CONFIG_RESPONSE)
                    {
                        get_logger()->debug("[{}] Received SYNC CONFIG RESPONSE ignored.", name);
                        stoppedProcessing = true;
                        outDataAvailable.notify_all();
                    }
                    else
                    {
                        try
                        {
                            if (upperDataCallback != nullptr)
                            {
                                upperDataCallback(data.data(), data.size());
                            }
                        }
                        catch (std::exception &e)
                        {
                            get_logger()->debug("[{} error calling data callback: {}", name,
                                                e.what());
                        }
                    }
                });
                inData.erase(inData.begin(), inData.end());
            }

            inDataAvailable.wait(lock, [&] { return !(isOpen == true && inData.size() == 0); });
        }
    });

    return NRF_SUCCESS; // TODO: take into account other return codes
}

uint32_t VirtualUart::close()
{
    if (!isOpen)
        return NRF_ERROR_INTERNAL;

    isOpen = false;
    inDataAvailable.notify_all();
    outDataAvailable.notify_all();

    if (outDataThread.joinable())
    {
        outDataThread.join();
    }

    if (inDataThread.joinable())
    {
        inDataThread.join();
    }

    if (upperLogCallback != nullptr)
    {
        std::stringstream message;
        message << "serial port " << name << " closed.";
        upperLogCallback(SD_RPC_LOG_INFO, message.str());
    }

    return NRF_SUCCESS; // TODO: take into account other return codes
}

uint32_t VirtualUart::send(const std::vector<uint8_t> &data)
{
    std::unique_lock<std::mutex> lock(outDataMutex);

    if (!isOpen)
    {
        return NRF_ERROR_INTERNAL;
    }

    outData.push_back(data);
    lock.unlock();
    outDataAvailable.notify_all();

    return NRF_SUCCESS; // TODO: take into account other states
}

void VirtualUart::setPeer(VirtualUart *connectingPeer)
{
    peer = connectingPeer;
}

// Used by peer to inject data into its incoming data pipe
void VirtualUart::injectInData(const std::vector<uint8_t> data)
{
    std::unique_lock<std::mutex> lock(inDataMutex);
    inData.push_back(data);
    lock.unlock();
    inDataAvailable.notify_all();
}

VirtualUart::~VirtualUart()
{
    VirtualUart::close();
}
