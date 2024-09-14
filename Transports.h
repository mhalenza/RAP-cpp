#pragma once
#include "Types.h"
#include <chrono>
#include <stdexcept>
#include <stop_token>

namespace RAP::Transport {

class TransportTimeoutException : public RAP::Exception
{
public:
    TransportTimeoutException() : Exception("Transport operation timed out!") {}
};
class ISyncWireTransport {
public:
    virtual ~ISyncWireTransport() = default;
    virtual void send(BufferView buffer) = 0;
    virtual Buffer recv() = 0;
    virtual Buffer recv(std::stop_token stoken) = 0;
    virtual uint16_t getMaxMessageSize() const = 0;
    virtual void setTimeout(std::chrono::microseconds timeout) = 0;
};

std::pair<std::unique_ptr<ISyncWireTransport>, std::unique_ptr<ISyncWireTransport>> makeSyncPairedIpcTransport(size_t max_message_size = 512);
std::unique_ptr<ISyncWireTransport> makeSyncUdpTransport(std::string_view remote_host, uint16_t remote_port, std::string_view local_host = "", uint16_t local_port = 0, bool log = false);

}
