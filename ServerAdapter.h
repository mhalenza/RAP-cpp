#pragma once
#include "Configuration.h"
#include "Types.h"
#include "Transports.h"
#include "Serdes.h"
#include <RTF/RTF.h>
#include <memory>
#include <thread>

namespace RAP::RTF {

    template <RAP::IsConfigurationType Cfg>
    class RapServerAdapter
    {
    public:
        RapServerAdapter(std::unique_ptr<RAP::Transport::ISyncWireTransport> transport_, std::shared_ptr<::RTF::IRegisterTarget<typename Cfg::AddressType, typename Cfg::DataType>> target_)
            : transport(std::move(transport_))
            , target(std::move(target_))
            , serdes(this->transport->getMaxMessageSize())
            , worker([this] { this->backgroundWork(); })
        {}

    private:
        Serdes::ReadSingleAckResponse<Cfg> handleCmd(Serdes::ReadSingleCommand<Cfg> const& cmd)
        {
            auto const data = this->target->read(cmd.addr);
            return Serdes::ReadSingleAckResponse<Cfg>{
                .transaction_id = cmd.transaction_id,
                    .data = data,
            };
        }
        Serdes::WriteSingleAckResponse<Cfg> handleCmd(Serdes::WriteSingleCommand<Cfg> const& cmd)
        {
            this->target->write(cmd.addr, cmd.data);
            return Serdes::WriteSingleAckResponse<Cfg>{
                .transaction_id = cmd.transaction_id,
            };
        }
        Serdes::ReadSeqAckResponse<Cfg> handleCmd(Serdes::ReadSeqCommand<Cfg> const& cmd)
        {
            std::vector<Cfg::DataType> out_data(cmd.count);
            this->target->seqRead(cmd.start_addr, out_data, cmd.increment);
            return Serdes::ReadSeqAckResponse<Cfg>{
                .transaction_id = cmd.transaction_id,
                    .data = std::move(out_data),
            };
        }
        Serdes::WriteSeqAckResponse<Cfg> handleCmd(Serdes::WriteSeqCommand<Cfg> const& cmd)
        {
            this->target->seqWrite(cmd.start_addr, cmd.data, cmd.increment);
            return Serdes::WriteSeqAckResponse<Cfg>{
                .transaction_id = cmd.transaction_id,
            };
        }
        Serdes::ReadCompAckResponse<Cfg> handleCmd(Serdes::ReadCompCommand<Cfg> const& cmd)
        {
            std::vector<Cfg::DataType> out_data(cmd.addresses.size());
            this->target->compRead(cmd.addresses, out_data);
            return Serdes::ReadCompAckResponse<Cfg>{
                .transaction_id = cmd.transaction_id,
                    .data = std::move(out_data),
            };
        }
        Serdes::WriteCompAckResponse<Cfg> handleCmd(Serdes::WriteCompCommand<Cfg> const& cmd)
        {
            this->target->compWrite(cmd.addr_data);
            return Serdes::WriteCompAckResponse<Cfg>{
                .transaction_id = cmd.transaction_id,
            };
        }
        Serdes::ReadmodifywriteSingleAckResponse<Cfg> handleCmd(Serdes::ReadModifyWriteCommand<Cfg> const& cmd)
        {
            this->target->readModifyWrite(cmd.addr, cmd.data, cmd.mask);
            return Serdes::ReadmodifywriteSingleAckResponse<Cfg>{
                .transaction_id = cmd.transaction_id,
            };
        }
        void backgroundWork()
        {
            while (!this->worker.get_stop_token().stop_requested()) {
                auto const cmd_buf = this->transport->recv(this->worker.get_stop_token());
                if (this->worker.get_stop_token().stop_requested())
                    return;
                auto const cmd = this->serdes.decodeCommand(cmd_buf);
                auto const resp = std::visit([&](auto&& cmd) -> Serdes::Response<Cfg> {
                    using T = std::decay_t<decltype(cmd)>;
                    try {
                        return this->handleCmd(cmd);
                    }
                    catch (std::exception const& ex) {
                        //LOG_ERROR(this, "Error while processing command: {}", ex.what());
                        return typename RAP::Serdes::CommandResponseRelationshipTrait<T>::NakResponseType{
                            .transaction_id = cmd.transaction_id,
                            .status = 0xFD,
                        };
                    }
                    catch (...) {
                        return typename RAP::Serdes::CommandResponseRelationshipTrait<T>::NakResponseType{
                            .transaction_id = cmd.transaction_id,
                            .status = 0xFD,
                        };
                    }
                }, cmd);
                auto const resp_buf = this->serdes.encodeResponse(resp);
                this->transport->send(resp_buf);
            }
        }

private:
    std::unique_ptr<Transport::ISyncWireTransport> transport;
    std::shared_ptr<::RTF::IRegisterTarget<typename Cfg::AddressType, typename Cfg::DataType>> target;
    Serdes::Serdes<Cfg> serdes;
    std::jthread worker;
};

}
