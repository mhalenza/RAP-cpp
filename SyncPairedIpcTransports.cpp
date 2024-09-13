#include "Transports.h"
#include <condition_variable>
#include <mutex>
#include <queue>

namespace RAP::Transport {



class IpcTransportQueue
{
public:
    void push(BufferView buffer)
    {
        {
            std::lock_guard lg{ this->mtx };
            this->queue.push(Buffer{ buffer.begin(), buffer.end() });
        }
        this->cv.notify_one();
    }
    Buffer pop(std::chrono::microseconds timeout, std::stop_token stoken)
    {
        std::unique_lock lk{ this->mtx };
        this->cv.wait_for(lk, stoken, timeout, [&] {
            return this->queue.size() > 0;
        });
        if (stoken.stop_requested())
            return Buffer{};
        if (this->queue.size() == 0)
            throw TransportTimeoutException();
        Buffer buffer = std::move(this->queue.front());
        this->queue.pop();
        lk.unlock();
        return buffer;
    }
    Buffer pop(std::chrono::microseconds timeout)
    {
        std::unique_lock lk{ this->mtx };
        this->cv.wait_for(lk, timeout, [&] {
            return this->queue.size() > 0;
        });
        if (this->queue.size() == 0)
            throw TransportTimeoutException();
        Buffer buffer = std::move(this->queue.front());
        this->queue.pop();
        lk.unlock();
        return buffer;
    }

private:
    std::mutex mtx;
    std::condition_variable_any cv;
    std::queue<Buffer> queue;
};

class PairedIpcTransport : public ISyncWireTransport
{
public:
    PairedIpcTransport(std::shared_ptr<IpcTransportQueue> tx_queue_, std::shared_ptr<IpcTransportQueue> rx_queue_, size_t max_message_size_)
        : tx_queue(std::move(tx_queue_))
        , rx_queue(std::move(rx_queue_))
        , max_message_size(max_message_size_)
        , timeout(std::chrono::years(1))
    {}
    virtual void send(BufferView buffer) override
    {
        this->tx_queue->push(buffer);
    }
    virtual Buffer recv() override
    {
        return this->rx_queue->pop(this->timeout);
    }
    virtual Buffer recv(std::stop_token stoken) override
    {
        return this->rx_queue->pop(this->timeout, stoken);
    }
    virtual uint16_t getMaxMessageSize() const override
    {
        return max_message_size;
    }
    virtual void setTimeout(std::chrono::microseconds new_timeout) override
    {
        this->timeout = new_timeout;
    }

private:
    std::shared_ptr<IpcTransportQueue> tx_queue;
    std::shared_ptr<IpcTransportQueue> rx_queue;
    size_t max_message_size;
    std::chrono::microseconds timeout;
};

std::pair<std::unique_ptr<ISyncWireTransport>, std::unique_ptr<ISyncWireTransport>> makeSyncPairedIpcTransport(size_t max_message_size)
{
    auto q1 = std::make_shared<IpcTransportQueue>();
    auto q2 = std::make_shared<IpcTransportQueue>();
    return {
        std::make_unique<PairedIpcTransport>(q1, q2, max_message_size),
        std::make_unique<PairedIpcTransport>(q2, q1, max_message_size)
    };
}


}
