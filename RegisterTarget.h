#pragma once
#include "Types.h"
#include "Configuration.h"
#include "Transports.h"
#include "Serdes.h"
#include <RTF/RTF.h>

namespace RAP::RTF {

template <IsConfigurationType Cfg>
class RapRegisterTarget : public ::RTF::IRegisterTarget<typename Cfg::AddressType, typename Cfg::DataType>
{
public:
    using AddressType = typename Cfg::AddressType;
    using DataType = typename Cfg::DataType;
public:
    RapRegisterTarget(std::string_view name, std::unique_ptr<RAP::Transport::ISyncWireTransport> transport)
        : ::RTF::IRegisterTarget<typename Cfg::AddressType, typename Cfg::DataType>(name)
        , transport(std::move(transport))
        , serdes(this->transport->getMaxMessageSize())
    {}
    virtual std::string_view getDomain() const { return "RapRegisterTarget"; }

    virtual void write(AddressType addr, DataType data) override
    {
        auto const cmd = RAP::Serdes::WriteSingleCommand<Cfg>{
            .transaction_id = this->getNextTxnId(),
            .posted = false,
            .addr = addr,
            .data = data,
        };
        this->doCmdResp(cmd);
    }
    [[nodiscard]] virtual DataType read(AddressType addr) override
    {
        auto const cmd = RAP::Serdes::ReadSingleCommand<Cfg>{
            .transaction_id = this->getNextTxnId(),
            .addr = addr,
        };
        auto const resp = this->doCmdResp(cmd);
        return resp.data;
    }

    virtual void readModifyWrite(AddressType addr, DataType new_data, DataType mask) override
    {
        if (!Cfg::FeatureReadModifyWrite) {
            return this->IRegisterTarget::readModifyWrite(addr, new_data, mask);
        }
        auto const cmd = RAP::Serdes::ReadModifyWriteCommand<Cfg>{
            .transaction_id = this->getNextTxnId(),
            .posted = false,
            .addr = addr,
            .data = new_data,
            .mask = mask,
        };
        this->doCmdResp(cmd);
    }

    virtual void seqWrite(AddressType start_addr, std::span<DataType const> data, size_t increment = sizeof(DataType)) override
    {
        if (!this->checkIFS(increment))
             return this->IRegisterTarget::seqWrite(start_addr, data, increment);

        auto const cmd = RAP::Serdes::WriteSeqCommand<Cfg>{
            .transaction_id = this->getNextTxnId(),
            .posted = false,
            .start_addr = start_addr,
            .increment = static_cast<Cfg::LengthType>(increment),
            .data = std::vector<DataType>{ data.begin(), data.end() },
        };
        this->doCmdResp(cmd);
    }
    virtual void seqRead(AddressType start_addr, std::span<DataType> out_data, size_t increment = sizeof(DataType)) override
    {
        if (!this->checkIFS(increment))
            return this->IRegisterTarget::seqRead(start_addr, out_data, increment);

        auto const cmd = RAP::Serdes::ReadSeqCommand<Cfg>{
            .transaction_id = this->getNextTxnId(),
            .start_addr = start_addr,
            .increment = static_cast<Cfg::LengthType>(increment),
            .count = static_cast<Cfg::LengthType>(out_data.size()),
        };
        auto const resp = this->doCmdResp(cmd);
        std::copy(resp.data.begin(), resp.data.end(), out_data.begin());
    }

    virtual void fifoWrite(AddressType fifo_addr, std::span<DataType const> data) override
    {
        if (!Cfg::FeatureFifo)
            return this->IRegisterTarget::fifoWrite(fifo_addr, data);
        auto const cmd = RAP::Serdes::WriteSeqCommand<Cfg>{
            .transaction_id = this->getNextTxnId(),
            .posted = false,
            .start_addr = fifo_addr,
            .increment = 0,
            .data = std::vector<DataType>{ data.begin(), data.end() },
        };
        this->doCmdResp(cmd);
    }
    virtual void fifoRead(AddressType fifo_addr, std::span<DataType> out_data) override
    {
        auto const cmd = RAP::Serdes::ReadSeqCommand<Cfg>{
            .transaction_id = this->getNextTxnId(),
            .start_addr = fifo_addr,
            .increment = 0,
            .count = static_cast<Cfg::LengthType>(out_data.size()),
        };
        auto const resp = this->doCmdResp(cmd);
        std::copy(resp.data.begin(), resp.data.end(), out_data.begin());
    }

    virtual void compWrite(std::span<std::pair<AddressType, DataType> const> addr_data) override
    {
        if (!Cfg::FeatureCompressed)
            return this->IRegisterTarget::compWrite(addr_data);
        auto const cmd = RAP::Serdes::WriteCompCommand<Cfg>{
            .transaction_id = this->getNextTxnId(),
            .posted = false,
            .addr_data = std::vector<std::pair<AddressType, DataType>>{ addr_data.begin(), addr_data.end() },
        };
        this->doCmdResp(cmd);
    }
    virtual void compRead(std::span<AddressType const> const addresses, std::span<DataType> out_data) override
    {
        assert(addresses.size() == out_data.size());
        if (!Cfg::FeatureCompressed)
            return this->IRegisterTarget::compRead(addresses, out_data);
        auto const cmd = RAP::Serdes::ReadCompCommand<Cfg>{
            .transaction_id = this->getNextTxnId(),
            .addresses = std::vector<AddressType>{ addresses.begin(), addresses.end() },
        };
        auto const resp = this->doCmdResp(cmd);
        std::copy(resp.data.begin(), resp.data.end(), out_data.begin());
    }
private:
    template <typename CmdType>
    RAP::Serdes::CommandResponseRelationshipTrait<CmdType>::AckResponseType doCmdResp(CmdType const& cmd)
    {
        this->transport->send(this->serdes.encodeCommand(cmd));
        auto const resp = this->serdes.decodeResponse(this->transport->recv());
        return std::visit([&](auto&& resp) -> RAP::Serdes::CommandResponseRelationshipTrait<CmdType>::AckResponseType  {
            if (cmd.transaction_id != resp.transaction_id)
                throw RapProtocolException();
            using T = std::decay_t<decltype(resp)>;
            if constexpr (std::is_same_v<T, RAP::Serdes::CommandResponseRelationshipTrait<CmdType>::AckResponseType>) {
                return resp;
            }
            else if constexpr (std::is_same_v<T, RAP::Serdes::CommandResponseRelationshipTrait<CmdType>::NakResponseType>) {
                throw OperationNakException(resp.status);
            }
            else {
                throw UnexpectedMessageTypeException();
            }
        }, resp);
    }
    uint8_t getNextTxnId()
    {
        return this->next_txn_id.fetch_add(1);
    }
    bool checkIFS(size_t increment) const
    {
        if (Cfg::FeatureIncrement)
            return true;
        if (increment == 0 && Cfg::FeatureFifo)
            return true;
        if (increment == sizeof(Cfg::DataType) && Cfg::FeatureSequential)
            return true;
        return false;
    }
private:
    std::unique_ptr<RAP::Transport::ISyncWireTransport> transport;
    RAP::Serdes::Serdes<Cfg> serdes;
    std::atomic<uint8_t> next_txn_id;
};
}
