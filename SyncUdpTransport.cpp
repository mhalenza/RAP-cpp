#include "Transports.h"
#include <YALF/YALF.h>
#include <asio.hpp>
#include <format>
#include <memory>

namespace RAP::Transport {

class UdpTransport : public ISyncWireTransport
{
public:
    UdpTransport(std::string_view remote_host, uint16_t remote_port, std::string_view local_host, uint16_t local_port, size_t mtu, bool log_)
        : timeout(std::chrono::seconds(1))
        , io_ctx()
        , io_remote_ep(this->resolveEndpoint(remote_host, remote_port))
        , io_local_ep(this->resolveEndpoint(local_host, local_port))
        , max_message_size(mtu - /*IPv6*/40 - /*UDP*/8)
        , io_socket(this->io_ctx, this->io_local_ep)
        , log(log_)
    {
        this->io_socket.connect(this->io_remote_ep);
    }
    static std::string_view getDomain() { return "UdpTransport"; }

    virtual void send(BufferView buffer) override
    {
        if (log) {
            std::string data_str;
            for (auto const d : buffer)
                std::format_to(std::back_inserter(data_str), "{:02x} ", d);
            LOG_NOISE(this, "send >>> [ {}]", data_str);
        }
        this->io_socket.send(asio::buffer(buffer.data(), buffer.size()));
    }
    virtual Buffer recv() override
    {
        Buffer buffer(this->max_message_size);
        auto const received_bytes = this->io_socket.receive(asio::buffer(buffer));
        buffer.resize(received_bytes);
        if (log) {
            std::string data_str;
            for (auto const d : buffer)
                std::format_to(std::back_inserter(data_str), "{:02x} ", d);
            LOG_NOISE(this, "recv <<< [ {}]", data_str);
        }
        return buffer;
    }
    virtual Buffer recv(std::stop_token stoken) override
    {
        Buffer buffer(this->max_message_size);
        size_t received_bytes = 0;
        this->io_socket.async_receive(asio::buffer(buffer), [&](std::error_code const& ec, size_t recvd_bytes) {
            if (ec)
                throw ec;
            received_bytes = recvd_bytes;
        });
        this->run(this->timeout, stoken);
        buffer.resize(received_bytes);
        if (log) {
            std::string data_str;
            for (auto const d : buffer)
                std::format_to(std::back_inserter(data_str), "{:02x} ", d);
            LOG_NOISE(this, "recv <<< [ {}]", data_str);
        }
        return buffer;
    }
    virtual uint16_t getMaxMessageSize() const override
    {
        return this->max_message_size;
    }
    virtual void setTimeout(std::chrono::microseconds new_timeout) override
    {
        this->timeout = new_timeout;
    }

private:
    asio::ip::udp::endpoint resolveEndpoint(std::string_view host, uint16_t port)
    {
        asio::ip::udp::resolver resolver{this->io_ctx};
        auto const endpoints = resolver.resolve(host, std::format("{}", port));
        return *endpoints.begin();
    }
    void run(std::chrono::microseconds timeout, std::stop_token stoken)
    {
        this->io_ctx.restart();
        auto stop_callback = std::stop_callback(stoken, [&] {
            io_socket.cancel();
            io_ctx.stop();
        });
        this->io_ctx.run_for(timeout);
        if (!this->io_ctx.stopped()) {
            this->io_socket.cancel();
            this->io_ctx.run();
        }
    }
private:
    std::chrono::microseconds timeout;
    asio::io_context io_ctx;
    asio::ip::udp::endpoint io_remote_ep;
    asio::ip::udp::endpoint io_local_ep;
    size_t max_message_size;
    asio::ip::udp::socket io_socket;
    bool log;
};

std::unique_ptr<ISyncWireTransport> makeSyncUdpTransport(std::string_view remote_host, uint16_t remote_port, std::string_view local_host, uint16_t local_port, bool log)
{
    return std::make_unique<UdpTransport>(remote_host, remote_port, local_host, local_port, 1500, log);
}

}
