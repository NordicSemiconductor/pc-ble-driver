#pragma once

#include "h5_transport.h"
#include "internal/log.h"
#include "transport.h"


#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>


class VirtualUart : public Transport
{
  private:
    std::string name;
    bool isOpen;
    VirtualUart *peer;

    std::mutex outDataMutex;
    std::vector<std::vector<uint8_t>> outData;
    std::condition_variable outDataAvailable;
    std::thread outDataThread;

    std::mutex inDataMutex;
    std::vector<std::vector<uint8_t>> inData;
    std::condition_variable inDataAvailable;
    std::thread inDataThread;

    control_pkt_type stopAtPktType;

    bool stoppedProcessing;

  public:
    VirtualUart() = delete;
    VirtualUart(const std::string &name);
    void stopAt(control_pkt_type stopAtPktType_);
    uint32_t open(const status_cb_t &status_callback, const data_cb_t &data_callback,
                  const log_cb_t &log_callback) noexcept override;
    uint32_t close() noexcept override;
    uint32_t send(const std::vector<uint8_t> &data) noexcept override;
    void setPeer(VirtualUart *connectingPeer);
    void injectInData(const std::vector<uint8_t> data);
    ~VirtualUart() override;
};
