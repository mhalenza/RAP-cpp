#pragma once
#include "Types.h"
#include "Configuration.h"
#define CRCPP_USE_NAMESPACE
#define CRCPP_BRANCHLESS
#define CRCPP_USE_CPP11
#include "CRCpp/inc/CRC.h"
#include <limits>
#include <span>
#include <variant>
#include <vector>
#include <assert.h>
#include <stdint.h>

namespace RAP::Serdes {

template <typename Cfg>
struct ReadSingleCommand
{
    uint8_t transaction_id;
    Cfg::AddressType addr;
    auto operator<=>(const ReadSingleCommand<Cfg>&) const = default;
};
template <typename Cfg>
struct WriteSingleCommand
{
    uint8_t transaction_id;
    bool posted;
    Cfg::AddressType addr;
    Cfg::DataType data;
    auto operator<=>(const WriteSingleCommand<Cfg>&) const = default;
};
template <typename Cfg>
struct ReadSeqCommand
{
    uint8_t transaction_id;
    Cfg::AddressType start_addr;
    Cfg::LengthType increment;
    Cfg::LengthType count;
    auto operator<=>(const ReadSeqCommand<Cfg>&) const = default;
};
template <typename Cfg>
struct WriteSeqCommand
{
    uint8_t transaction_id;
    bool posted;
    Cfg::AddressType start_addr;
    Cfg::LengthType increment;
    std::vector<typename Cfg::DataType> data;
    auto operator<=>(const WriteSeqCommand<Cfg>&) const = default;
};
template <typename Cfg>
struct ReadCompCommand
{
    uint8_t transaction_id;
    std::vector<typename Cfg::AddressType> addresses;
    auto operator<=>(const ReadCompCommand<Cfg>&) const = default;
};
template <typename Cfg>
struct WriteCompCommand
{
    uint8_t transaction_id;
    bool posted;
    std::vector<std::pair<typename Cfg::AddressType, typename Cfg::DataType>> addr_data;
    auto operator<=>(const WriteCompCommand<Cfg>&) const = default;
};
template <typename Cfg>
struct ReadModifyWriteCommand
{
    uint8_t transaction_id;
    bool posted;
    Cfg::AddressType addr;
    Cfg::DataType data;
    Cfg::DataType mask;
    auto operator<=>(const ReadModifyWriteCommand<Cfg>&) const = default;
};
template <typename Cfg>
using Command = std::variant<
    ReadSingleCommand<Cfg>,
    WriteSingleCommand<Cfg>,
    ReadSeqCommand<Cfg>,
    WriteSeqCommand<Cfg>,
    ReadCompCommand<Cfg>,
    WriteCompCommand<Cfg>,
    ReadModifyWriteCommand<Cfg>
>;

template <typename Cfg>
struct ReadSingleAckResponse
{
    uint8_t transaction_id;
    Cfg::DataType data;
    auto operator<=>(const ReadSingleAckResponse<Cfg>&) const = default;
};
template <typename Cfg>
struct WriteSingleAckResponse
{
    uint8_t transaction_id;
    auto operator<=>(const WriteSingleAckResponse<Cfg>&) const = default;
};
template <typename Cfg>
struct ReadSeqAckResponse
{
    uint8_t transaction_id;
    std::vector<typename Cfg::DataType> data;
    auto operator<=>(const ReadSeqAckResponse<Cfg>&) const = default;
};
template <typename Cfg>
struct WriteSeqAckResponse
{
    uint8_t transaction_id;
    auto operator<=>(const WriteSeqAckResponse<Cfg>&) const = default;
};
template <typename Cfg>
struct ReadCompAckResponse
{
    uint8_t transaction_id;
    std::vector<typename Cfg::DataType> data;
    auto operator<=>(const ReadCompAckResponse<Cfg>&) const = default;
};
template <typename Cfg>
struct WriteCompAckResponse
{
    uint8_t transaction_id;
    auto operator<=>(const WriteCompAckResponse<Cfg>&) const = default;
};
template <typename Cfg>
struct ReadSingleNakResponse
{
    uint8_t transaction_id;
    Cfg::DataType status;
    auto operator<=>(const ReadSingleNakResponse<Cfg>&) const = default;
};
template <typename Cfg>
struct WriteSingleNakResponse
{
    uint8_t transaction_id;
    Cfg::DataType status;
    auto operator<=>(const WriteSingleNakResponse<Cfg>&) const = default;
};
template <typename Cfg>
struct ReadSeqNakResponse
{
    uint8_t transaction_id;
    Cfg::DataType status;
    auto operator<=>(const ReadSeqNakResponse<Cfg>&) const = default;
};
template <typename Cfg>
struct WriteSeqNakResponse
{
    uint8_t transaction_id;
    Cfg::DataType status;
    auto operator<=>(const WriteSeqNakResponse<Cfg>&) const = default;
};
template <typename Cfg>
struct ReadCompNakResponse
{
    uint8_t transaction_id;
    Cfg::DataType status;
    auto operator<=>(const ReadCompNakResponse<Cfg>&) const = default;
};
template <typename Cfg>
struct WriteCompNakResponse
{
    uint8_t transaction_id;
    Cfg::DataType status;
    auto operator<=>(const WriteCompNakResponse<Cfg>&) const = default;
};
template <typename Cfg>
struct ReadmodifywriteSingleAckResponse
{
    uint8_t transaction_id;
    auto operator<=>(const ReadmodifywriteSingleAckResponse<Cfg>&) const = default;
};
template <typename Cfg>
struct ReadmodifywriteSingleNakResponse
{
    uint8_t transaction_id;
    Cfg::DataType status;
    auto operator<=>(const ReadmodifywriteSingleNakResponse<Cfg>&) const = default;
};
template <typename Cfg>
struct Interrupt
{
    uint8_t transaction_id;
    Cfg::DataType status;
    auto operator<=>(const Interrupt<Cfg>&) const = default;
};
template <typename Cfg>
using Response = std::variant<
    ReadSingleAckResponse<Cfg>,
    WriteSingleAckResponse<Cfg>,
    ReadSeqAckResponse<Cfg>,
    WriteSeqAckResponse<Cfg>,
    ReadCompAckResponse<Cfg>,
    WriteCompAckResponse<Cfg>,
    ReadSingleNakResponse<Cfg>,
    WriteSingleNakResponse<Cfg>,
    ReadSeqNakResponse<Cfg>,
    WriteSeqNakResponse<Cfg>,
    ReadCompNakResponse<Cfg>,
    WriteCompNakResponse<Cfg>,
    ReadmodifywriteSingleAckResponse<Cfg>,
    ReadmodifywriteSingleNakResponse<Cfg>,
    Interrupt<Cfg>
>;

template <typename CommandType>
struct CommandResponseRelationshipTrait {};

template <typename CmdType>
concept CommandResponseRelationship = requires()
{
    typename CommandResponseRelationshipTrait<CmdType>::CommandType;
    std::same_as<CmdType, typename CommandResponseRelationshipTrait<CmdType>::CommandType>;
    typename CommandResponseRelationshipTrait<CmdType>::AckResponseType;
    typename CommandResponseRelationshipTrait<CmdType>::NakResponseType;
};

#define RAP_DEFINE_CRRT(cmd, ack, nak)                  \
template <IsConfigurationType Cfg>                      \
struct CommandResponseRelationshipTrait<cmd<Cfg>> {     \
    using CommandType = cmd<Cfg>;                       \
    using AckResponseType = ack<Cfg>;                   \
    using NakResponseType = nak<Cfg>;                   \
};

RAP_DEFINE_CRRT(ReadSingleCommand, ReadSingleAckResponse, ReadSingleNakResponse);
RAP_DEFINE_CRRT(WriteSingleCommand, WriteSingleAckResponse, WriteSingleNakResponse);
RAP_DEFINE_CRRT(ReadSeqCommand, ReadSeqAckResponse, ReadSeqNakResponse);
RAP_DEFINE_CRRT(WriteSeqCommand, WriteSeqAckResponse, WriteSeqNakResponse);
RAP_DEFINE_CRRT(ReadCompCommand, ReadCompAckResponse, ReadCompNakResponse);
RAP_DEFINE_CRRT(WriteCompCommand, WriteCompAckResponse, WriteCompNakResponse);
RAP_DEFINE_CRRT(ReadModifyWriteCommand, ReadmodifywriteSingleAckResponse, ReadmodifywriteSingleNakResponse);

enum class MessageType : uint8_t {
    eCmdSingleRead = 0x01,
    eAckSingleRead = 0x80,
    eNakSingleRead = 0xC1,
    eCmdSingleWrite = 0x10,
    eCmdSingleWritePosted = 0x51,
    eAckSingleWrite = 0x91,
    eNakSingleWrite = 0xD0,
    eCmdSeqRead = 0x04,
    eAckSeqRead = 0x85,
    eNakSeqRead = 0xC4,
    eCmdSeqWrite = 0x15,
    eCmdSeqWritePosted = 0x54,
    eAckSeqWrite = 0x94,
    eNakSeqWrite = 0xD5,
    eCmdCompRead = 0x08,
    eAckCompRead = 0x89,
    eNakCompRead = 0xC8,
    eCmdCompWrite = 0x19,
    eCmdCompWritePosted = 0x58,
    eAckCompWrite = 0x98,
    eNakCompWrite = 0xD9,
    eCmdSingleRmw = 0x20,
    eCmdSingleRmwPosted = 0x61,
    eAckSingleRmw = 0xA1,
    eNakSingleRmw = 0xE0,
    eAckSingleInterrupt = 0xB0,
};

template <typename Cfg>
class Serdes {
public:
    static constexpr size_t minimum_max_message_size = 32;
    explicit Serdes(size_t max_message_size_)
        : max_message_size(max_message_size_)
    {
        if (this->max_message_size < minimum_max_message_size) {
            throw Exception("Serdes max_message_size must be at least Serdes::minimum_max_message_size!");
        }
    }

    Buffer encodeCommand(Command<Cfg> const& cmd) const
    {
        return std::visit([&](auto&& cmd) -> Buffer {
            return this->encode(cmd);
        }, cmd);
    }
    Response<Cfg> decodeResponse(BufferView buff) const
    {
        if (buff.size() < 2 + Cfg::CrcBytes)
            throw MalformedPacketException();
        auto const wire_crc = extractCrc(buff);
        auto const expected_crc = calculateCrc(buff);
        if (wire_crc != expected_crc)
            throw CrcMismatchException(expected_crc, wire_crc);

        auto const transaction_id = extractByte(buff);
        auto const msg_type = static_cast<MessageType>(extractByte(buff));

        switch (msg_type) {
            case MessageType::eAckSingleRead:  return decode<ReadSingleAckResponse<Cfg>>(buff, transaction_id, msg_type);
            case MessageType::eNakSingleRead:  return decode<ReadSingleNakResponse<Cfg>>(buff, transaction_id, msg_type);
            case MessageType::eAckSingleWrite: return decode<WriteSingleAckResponse<Cfg>>(buff, transaction_id, msg_type);
            case MessageType::eNakSingleWrite: return decode<WriteSingleNakResponse<Cfg>>(buff, transaction_id, msg_type);
            case MessageType::eAckSeqRead: return decode<ReadSeqAckResponse<Cfg>>(buff, transaction_id, msg_type);
            case MessageType::eNakSeqRead: return decode<ReadSeqNakResponse<Cfg>>(buff, transaction_id, msg_type);
            case MessageType::eAckSeqWrite: return decode<WriteSeqAckResponse<Cfg>>(buff, transaction_id, msg_type);
            case MessageType::eNakSeqWrite: return decode<WriteSeqNakResponse<Cfg>>(buff, transaction_id, msg_type);
            case MessageType::eAckCompRead: return decode<ReadCompAckResponse<Cfg>>(buff, transaction_id, msg_type);
            case MessageType::eNakCompRead: return decode<ReadCompNakResponse<Cfg>>(buff, transaction_id, msg_type);
            case MessageType::eAckCompWrite: return decode<WriteCompAckResponse<Cfg>>(buff, transaction_id, msg_type);
            case MessageType::eNakCompWrite: return decode<WriteCompNakResponse<Cfg>>(buff, transaction_id, msg_type);
            case MessageType::eAckSingleRmw: return decode<ReadmodifywriteSingleAckResponse<Cfg>>(buff, transaction_id, msg_type);
            case MessageType::eNakSingleRmw: return decode<ReadmodifywriteSingleNakResponse<Cfg>>(buff, transaction_id, msg_type);
            case MessageType::eAckSingleInterrupt: return decode<Interrupt<Cfg>>(buff, transaction_id, msg_type);
            // ----
            case MessageType::eCmdSingleRead:
            case MessageType::eCmdSingleWrite:
            case MessageType::eCmdSingleWritePosted:
            case MessageType::eCmdSeqRead:
            case MessageType::eCmdSeqWrite:
            case MessageType::eCmdSeqWritePosted:
            case MessageType::eCmdCompRead:
            case MessageType::eCmdCompWrite:
            case MessageType::eCmdCompWritePosted:
            case MessageType::eCmdSingleRmw:
            case MessageType::eCmdSingleRmwPosted:
            default:
                throw UnexpectedMessageTypeException();
        }
    }

    Command<Cfg> decodeCommand(BufferView buff) const
    {
        if (buff.size() < 2 + Cfg::CrcBytes)
            throw MalformedPacketException();
        auto const wire_crc = extractCrc(buff);
        auto const expected_crc = calculateCrc(buff);
        if (wire_crc != expected_crc)
            throw CrcMismatchException(expected_crc, wire_crc);

        auto const transaction_id = extractByte(buff);
        auto const msg_type = static_cast<MessageType>(extractByte(buff));
        
        switch (msg_type) {
            case MessageType::eCmdSingleRead: return decode<ReadSingleCommand<Cfg>>(buff, transaction_id, msg_type);
            case MessageType::eCmdSingleWrite: return decode<WriteSingleCommand<Cfg>>(buff, transaction_id, msg_type);
            case MessageType::eCmdSingleWritePosted: return decode<WriteSingleCommand<Cfg>>(buff, transaction_id, msg_type);
            case MessageType::eCmdSeqRead: return decode<ReadSeqCommand<Cfg>>(buff, transaction_id, msg_type);
            case MessageType::eCmdSeqWrite: return decode<WriteSeqCommand<Cfg>>(buff, transaction_id, msg_type);
            case MessageType::eCmdSeqWritePosted: return decode<WriteSeqCommand<Cfg>>(buff, transaction_id, msg_type);
            case MessageType::eCmdCompRead: return decode<ReadCompCommand<Cfg>>(buff, transaction_id, msg_type);
            case MessageType::eCmdCompWrite: return decode<WriteCompCommand<Cfg>>(buff, transaction_id, msg_type);
            case MessageType::eCmdCompWritePosted: return decode<WriteCompCommand<Cfg>>(buff, transaction_id, msg_type);
            case MessageType::eCmdSingleRmw: return decode<ReadModifyWriteCommand<Cfg>>(buff, transaction_id, msg_type);
            case MessageType::eCmdSingleRmwPosted: return decode<ReadModifyWriteCommand<Cfg>>(buff, transaction_id, msg_type);
            // ----
            case MessageType::eAckSingleRead:
            case MessageType::eNakSingleRead:
            case MessageType::eAckSingleWrite:
            case MessageType::eNakSingleWrite:
            case MessageType::eAckSeqRead:
            case MessageType::eNakSeqRead:
            case MessageType::eAckSeqWrite:
            case MessageType::eNakSeqWrite:
            case MessageType::eAckCompRead:
            case MessageType::eNakCompRead:
            case MessageType::eAckCompWrite:
            case MessageType::eNakCompWrite:
            case MessageType::eAckSingleRmw:
            case MessageType::eNakSingleRmw:
            case MessageType::eAckSingleInterrupt:
            default:
                throw UnexpectedMessageTypeException();
        }
    }
    Buffer encodeResponse(Response<Cfg> const& resp) const
    {
        return std::visit([&](auto&& resp) -> Buffer {
            return this->encode(resp);
        }, resp);
    }

    Cfg::LengthType getMaxSeqReadCount() const
    {
        // Size is determined by Ack Response AND Length field
        auto const max_by_cmd = [&] {
            auto const overhead_sz = calcSize(0, /*count*/0, 1);
            auto const read_data_sz_available = this->max_message_size - overhead_sz;
            return read_data_sz_available / Cfg::DataBytes;
        }();
        auto const max_by_length = std::numeric_limits<Cfg::LengthType>::max();
        return std::min<size_t>(max_by_cmd, max_by_length);
    }
    Cfg::LengthType getMaxSeqWriteCount() const
    {
        // Size is determined by Command AND Length field
        auto const max_by_cmd = [&] {
            auto const overhead_sz = calcSize(1, /*count*/0, 2);
            auto const sz_available = this->max_message_size - overhead_sz;
            return sz_available / Cfg::DataBytes;
        }();
        auto const max_by_length = std::numeric_limits<Cfg::LengthType>::max();
        return std::min<size_t>(max_by_cmd, max_by_length);
    }
    Cfg::LengthType getMaxCompReadCount() const
    {
        // Size is determined by either Command or Ack Response AND Length field
        auto const max_by_cmd = [&] {
            auto const overhead_sz = calcSize(/*count*/0, 0, 1);
            auto const sz_available = this->max_message_size - overhead_sz;
            return sz_available / Cfg::AddressBytes;
        }();
        auto const max_by_ack = [&] {
            auto const overhead_sz = calcSize(0, /*count*/0, 1);
            auto const sz_available = this->max_message_size - overhead_sz;
            return sz_available / Cfg::DataBytes;
        }();
        auto const max_by_length = std::numeric_limits<Cfg::LengthType>::max();
        return std::min<size_t>({max_by_cmd, max_by_ack, max_by_length});
    }
    Cfg::LengthType getMaxCompWriteCount() const
    {
        // Size is determined by Command AND Length field
        auto const max_by_cmd = [&] {
            auto const overhead_sz = calcSize(/*count*/0, /*count*/0, 1);
            auto const sz_available = this->max_message_size - overhead_sz;
            return sz_available / (Cfg::AddressBytes + Cfg::DataBytes);
        }();
        auto const max_by_length = std::numeric_limits<Cfg::LengthType>::max();
        return std::min<size_t>({max_by_cmd, max_by_length});
    }

private:
    size_t calcSize(size_t address_cnt, size_t data_cnt, size_t length_cnt) const
    {
        size_t const sz = 2 +
            (address_cnt * Cfg::AddressBytes) +
            (data_cnt * Cfg::DataBytes) +
            (length_cnt * Cfg::LengthBytes) +
            (Cfg::CrcBytes)
            ;
        if (sz > this->max_message_size)
            throw MessageSizeException("Serialized message too large to fit in Transport limits");
        return sz;
    }
    Buffer mkBuffer(size_t sz, uint8_t txn_id, MessageType msg_type) const
    {
        Buffer buf{};
        buf.reserve(sz);
        appendByte(buf, txn_id);
        appendByte(buf, static_cast<uint8_t>(msg_type));
        return buf;
    }
    void appendByte(Buffer& buf, uint8_t v) const
    {
        buf.push_back(v);
    }
    void appendAddress(Buffer& buf, Cfg::AddressType v) const
    {
        for (size_t i = 0; i < Cfg::AddressBytes; i++) {
            buf.push_back(v & 0xFF);
            v >>= 8;
        }
    }
    void appendData(Buffer& buf, Cfg::DataType v) const
    {
        for (size_t i = 0; i < Cfg::DataBytes; i++) {
            buf.push_back(v & 0xFF);
            v >>= 8;
        }
    }
    void appendLength(Buffer& buf, Cfg::LengthType v) const
    {
        for (size_t i = 0; i < Cfg::LengthBytes; i++) {
            buf.push_back(v & 0xFF);
            v >>= 8;
        }
    }
    void appendCrc(Buffer& buf) const
    {
        auto const crc = calculateCrc(buf);
        for (size_t i = 0; i < Cfg::CrcBytes; i++) {
            buf.push_back(crc >> (i*8));
        }
    }
    uint8_t extractByte(BufferView& buf) const
    {
        if (buf.size() < 1)
            throw MalformedPacketException();
        auto const v = buf[0];
        buf = buf.subspan(1);
        return v;
    }
    Cfg::AddressType extractAddress(BufferView& buf) const
    {
        if (buf.size() < Cfg::AddressBytes)
            throw MalformedPacketException();
        typename Cfg::AddressType v{};
        for (size_t i = 0; i < Cfg::AddressBytes; i++) {
            v |= typename Cfg::AddressType{ buf[i] } << (i * 8);
        }
        buf = buf.subspan(Cfg::AddressBytes);
        return v;
    }
    std::vector<typename Cfg::AddressType> extractAddressArray(BufferView& buf, size_t count) const
    {
        std::vector<Cfg::AddressType> addrs;
        addrs.reserve(count);
        for (auto i = 0; i < count; i++) {
            addrs.push_back(extractAddress(buf));
        }
        return addrs;
    }
    Cfg::DataType extractData(BufferView& buf) const
    {
        if (buf.size() < Cfg::DataBytes)
            throw MalformedPacketException();
        typename Cfg::DataType v{};
        for (size_t i = 0; i < Cfg::DataBytes; i++) {
            v |= typename Cfg::DataType{ buf[i] } << (i * 8);
        }
        buf = buf.subspan(Cfg::DataBytes);
        return v;
    }
    std::vector<typename Cfg::DataType> extractDataArray(BufferView& buf, size_t count) const
    {
        std::vector<Cfg::DataType> data;
        data.reserve(count);
        for (auto i = 0; i < count; i++) {
            data.push_back(extractData(buf));
        }
        return data;
    }
    std::vector<std::pair<typename Cfg::AddressType, typename Cfg::DataType>> extractAddressDataArray(BufferView& buf, size_t count) const
    {
        std::vector<std::pair<typename Cfg::AddressType, typename Cfg::DataType>> addr_data;
        addr_data.reserve(count);
        for (auto i = 0; i < count; i++) {
            auto const a = extractAddress(buf);
            auto const d = extractData(buf);
            addr_data.push_back(std::make_pair(a, d));
        }
        return addr_data;
    }
    Cfg::LengthType extractLength(BufferView& buf) const
    {
        if (buf.size() < Cfg::LengthBytes)
            throw MalformedPacketException();
        typename Cfg::LengthType v{};
        for (size_t i = 0; i < Cfg::LengthBytes; i++) {
            v |= typename Cfg::LengthType{ buf[i] } << (i * 8);
        }
        buf = buf.subspan(Cfg::LengthBytes);
        return v;
    }
    Cfg::CrcType extractCrc(BufferView& buf) const
    {
        if (buf.size() < Cfg::CrcBytes)
            throw MalformedPacketException();
        auto const crc_buf = buf.subspan(buf.size() - Cfg::CrcBytes);
        typename Cfg::CrcType v{};
        for (size_t i = 0; i < Cfg::CrcBytes; i++) {
            v |= typename Cfg::CrcType{ crc_buf[i] } << (i * 8);
        }
        buf = buf.subspan(0, buf.size() - Cfg::CrcBytes);
        return v;
    }
    Cfg::CrcType calculateCrc(BufferView buf) const
    {
        if constexpr (Cfg::CrcBytes == 1) {
            static auto const table = CRCPP::CRC::Parameters<Cfg::CrcType, 8>{ // CRC-8/DVB-S2
                .polynomial = 0xd5,
                .initialValue = 0x00,
                .finalXOR = 0x00,
                .reflectInput = false,
                .reflectOutput = false,
            }.MakeTable();
            return CRCPP::CRC::Calculate(buf.data(), buf.size(), table);
        }
        else if constexpr (Cfg::CrcBytes == 2) {
            static auto const table = CRCPP::CRC::Parameters<Cfg::CrcType, 16>{ // CRC-16/XMODEM
                .polynomial = 0x1021,
                .initialValue = 0x0000,
                .finalXOR = 0x0000,
                .reflectInput = false,
                .reflectOutput = false,
            }.MakeTable();
            return CRCPP::CRC::Calculate(buf.data(), buf.size(), table);
        }
        else if constexpr (Cfg::CrcBytes == 3) {
            static auto const table = CRCPP::CRC::Parameters<Cfg::CrcType, 24>{ // CRC-24/INTERLAKEN
                .polynomial = 0x328b63,
                .initialValue = 0xffffff,
                .finalXOR = 0xffffff,
                .reflectInput = false,
                .reflectOutput = false,
            }.MakeTable();
            return CRCPP::CRC::Calculate(buf.data(), buf.size(), table);
        }
        else if constexpr (Cfg::CrcBytes == 4) {
            static auto const table = CRCPP::CRC::Parameters<Cfg::CrcType, 32>{ // CRC-32/INTERLAKEN
                .polynomial = 0x1edc6f41,
                .initialValue = 0xffffffff,
                .finalXOR = 0xffffffff,
                .reflectInput = true,
                .reflectOutput = true,
            }.MakeTable();
            return CRCPP::CRC::Calculate(buf.data(), buf.size(), table);
        }
    }
private:
    template <typename T>
    T decode(BufferView buf, uint8_t txn_id, MessageType msg_type) const { static_assert(false); }
    Buffer encode(ReadSingleCommand<Cfg> const& cmd) const
    {
        auto const sz = calcSize(1, 0, 0);
        auto buf = mkBuffer(sz, cmd.transaction_id, MessageType::eCmdSingleRead);
        appendAddress(buf, cmd.addr);
        appendCrc(buf);
        assert(buf.size() == sz);
        return buf;
    }
    template <> ReadSingleCommand<Cfg> decode<ReadSingleCommand<Cfg>>(BufferView buf, uint8_t txn_id, MessageType msg_type) const
    {
        auto const addr = extractAddress(buf);
        if (buf.size() != 0) throw MalformedPacketException();
        return ReadSingleCommand<Cfg>{
            .transaction_id = txn_id,
            .addr = addr,
        };
    }
    Buffer encode(WriteSingleCommand<Cfg> const& cmd) const
    {
        auto const sz = calcSize(1, 1, 0);
        auto buf = mkBuffer(sz, cmd.transaction_id, cmd.posted ? MessageType::eCmdSingleWritePosted : MessageType::eCmdSingleWrite);
        appendAddress(buf, cmd.addr);
        appendData(buf, cmd.data);
        appendCrc(buf);
        assert(buf.size() == sz);
        return buf;
    }
    template <> WriteSingleCommand<Cfg> decode<WriteSingleCommand<Cfg>>(BufferView buf, uint8_t txn_id, MessageType msg_type) const
    {
        auto const addr = extractAddress(buf);
        auto const data = extractData(buf);
        if (buf.size() != 0) throw MalformedPacketException();
        return WriteSingleCommand<Cfg>{
            .transaction_id = txn_id,
            .posted = msg_type == MessageType::eCmdSingleWritePosted,
            .addr = addr,
            .data = data,
        };
    }
    Buffer encode(ReadSeqCommand<Cfg> const& cmd) const
    {
        if (cmd.count > this->getMaxSeqReadCount())
            throw MessageSizeException("ReadSeqCommand count exceeded transport-imposed limit");
        auto const sz = calcSize(1, 0, 2);
        auto buf = mkBuffer(sz, cmd.transaction_id, MessageType::eCmdSeqRead);
        appendAddress(buf, cmd.start_addr);
        appendLength(buf, cmd.increment);
        appendLength(buf, cmd.count);
        appendCrc(buf);
        assert(buf.size() == sz);
        return buf;
    }
    template <> ReadSeqCommand<Cfg> decode<ReadSeqCommand<Cfg>>(BufferView buf, uint8_t txn_id, MessageType msg_type) const
    {
        auto const start_addr = extractAddress(buf);
        auto const increment = extractLength(buf);
        auto const count = extractLength(buf);
        if (buf.size() != 0) throw MalformedPacketException();
        return ReadSeqCommand<Cfg>{
            .transaction_id = txn_id,
            .start_addr = start_addr,
            .increment = increment,
            .count = count,
        };
    }
    Buffer encode(WriteSeqCommand<Cfg> const& cmd) const
    {
        if (cmd.data.size() > this->getMaxSeqWriteCount())
            throw MessageSizeException("WriteSeqCommand count exceeded transport-imposed limit");
        auto const sz = calcSize(1, cmd.data.size(), 2);
        auto buf = mkBuffer(sz, cmd.transaction_id, cmd.posted ? MessageType::eCmdSeqWritePosted : MessageType::eCmdSeqWrite);
        appendAddress(buf, cmd.start_addr);
        appendLength(buf, cmd.increment);
        appendLength(buf, cmd.data.size());
        for (auto const d : cmd.data)
            appendData(buf, d);
        appendCrc(buf);
        assert(buf.size() == sz);
        return buf;
    }
    template <> WriteSeqCommand<Cfg> decode<WriteSeqCommand<Cfg>>(BufferView buf, uint8_t txn_id, MessageType msg_type) const
    {
        auto const start_addr = extractAddress(buf);
        auto const increment = extractLength(buf);
        auto const count = extractLength(buf);
        if (buf.size() != count * Cfg::DataBytes)
            throw Exception("Buffer size error");
        auto data = extractDataArray(buf, count);
        if (buf.size() != 0) throw MalformedPacketException();
        return WriteSeqCommand<Cfg>{
            .transaction_id = txn_id,
            .posted = msg_type == MessageType::eCmdSeqWritePosted,
            .start_addr = start_addr,
            .increment = increment,
            .data = std::move(data)
        };
    }
    Buffer encode(ReadCompCommand<Cfg> const& cmd) const
    {
        if (cmd.addresses.size() > this->getMaxCompReadCount())
            throw MessageSizeException("ReadCompCommand count exceeded transport-imposed limit");
        auto const sz = calcSize(cmd.addresses.size(), 0, 1);
        auto buf = mkBuffer(sz, cmd.transaction_id, MessageType::eCmdCompRead);
        appendLength(buf, cmd.addresses.size());
        for (auto const a : cmd.addresses)
            appendAddress(buf, a);
        appendCrc(buf);
        assert(buf.size() == sz);
        return buf;
    }
    template <> ReadCompCommand<Cfg> decode<ReadCompCommand<Cfg>>(BufferView buf, uint8_t txn_id, MessageType msg_type) const
    {
        auto const count = extractLength(buf);
        if (buf.size() != count * Cfg::AddressBytes)
            throw Exception("Buffer size error");
        auto addrs = extractAddressArray(buf, count);
        if (buf.size() != 0) throw MalformedPacketException();
        return ReadCompCommand<Cfg>{
            .transaction_id = txn_id,
            .addresses = std::move(addrs),
        };
    }
    Buffer encode(WriteCompCommand<Cfg> const& cmd) const
    {
        if (cmd.addr_data.size() > this->getMaxCompWriteCount())
            throw MessageSizeException("WriteCompCommand count exceeded transport-imposed limit");
        auto const sz = calcSize(cmd.addr_data.size(), cmd.addr_data.size(), 1);
        auto buf = mkBuffer(sz, cmd.transaction_id, cmd.posted ? MessageType::eCmdCompWritePosted : MessageType::eCmdCompWrite);
        appendLength(buf, cmd.addr_data.size());
        for (auto const ad : cmd.addr_data) {
            appendAddress(buf, ad.first);
            appendData(buf, ad.second);
        }
        appendCrc(buf);
        assert(buf.size() == sz);
        return buf;
    }
    template <> WriteCompCommand<Cfg> decode<WriteCompCommand<Cfg>>(BufferView buf, uint8_t txn_id, MessageType msg_type) const
    {
        auto const count = extractLength(buf);
        if (buf.size() != count * (Cfg::AddressBytes + Cfg::DataBytes))
            throw Exception("Buffer size error");
        auto addr_data = extractAddressDataArray(buf, count);
        if (buf.size() != 0) throw MalformedPacketException();
        return WriteCompCommand<Cfg>{
            .transaction_id = txn_id,
            .posted = msg_type == MessageType::eCmdCompWritePosted,
            .addr_data = std::move(addr_data),
        };
    }
    Buffer encode(ReadModifyWriteCommand<Cfg> const& cmd) const
    {
        auto const sz = calcSize(1, 2, 0);
        auto buf = mkBuffer(sz, cmd.transaction_id, cmd.posted ? MessageType::eCmdSingleRmwPosted : MessageType::eCmdSingleRmw);
        appendAddress(buf, cmd.addr);
        appendData(buf, cmd.data);
        appendData(buf, cmd.mask);
        appendCrc(buf);
        assert(buf.size() == sz);
        return buf;
    }
    template <> ReadModifyWriteCommand<Cfg> decode<ReadModifyWriteCommand<Cfg>>(BufferView buf, uint8_t txn_id, MessageType msg_type) const
    {
        auto const addr = extractAddress(buf);
        auto const data = extractData(buf);
        auto const mask = extractData(buf);
        if (buf.size() != 0) throw MalformedPacketException();
        return ReadModifyWriteCommand<Cfg>{
            .transaction_id = txn_id,
            .posted = msg_type == MessageType::eCmdSingleRmwPosted,
            .addr = addr,
            .data = data,
            .mask = mask,
        };
    }
    Buffer encode(ReadSingleAckResponse<Cfg> const& resp) const
    {
        auto const sz = calcSize(0, 1, 0);
        auto buf = mkBuffer(sz, resp.transaction_id, MessageType::eAckSingleRead);
        appendData(buf, resp.data);
        appendCrc(buf);
        assert(buf.size() == sz);
        return buf;
    }
    template <> ReadSingleAckResponse<Cfg> decode<ReadSingleAckResponse<Cfg>>(BufferView buf, uint8_t txn_id, MessageType msg_type) const
    {
        auto const data = extractData(buf);
        if (buf.size() != 0) throw MalformedPacketException();
        return ReadSingleAckResponse<Cfg>{
            .transaction_id = txn_id,
            .data = data,
        };
    }
    Buffer encode(WriteSingleAckResponse<Cfg> const& resp) const
    {
        auto const sz = calcSize(0, 0, 0);
        auto buf = mkBuffer(sz, resp.transaction_id, MessageType::eAckSingleWrite);
        appendCrc(buf);
        assert(buf.size() == sz);
        return buf;
    }
    template <> WriteSingleAckResponse<Cfg> decode<WriteSingleAckResponse<Cfg>>(BufferView buf, uint8_t txn_id, MessageType msg_type) const
    {
        if (buf.size() != 0) throw MalformedPacketException();
        return WriteSingleAckResponse<Cfg>{
            .transaction_id = txn_id,
        };
    }
    Buffer encode(ReadSeqAckResponse<Cfg> const& resp) const
    {
        if (resp.data.size() > this->getMaxSeqReadCount())
            throw MessageSizeException("ReadSeqAckResponse count exceeded transport-imposed limit");
        auto const sz = calcSize(0, resp.data.size(), 1);
        auto buf = mkBuffer(sz, resp.transaction_id, MessageType::eAckSeqRead);
        appendLength(buf, resp.data.size());
        for (auto const d : resp.data)
            appendData(buf, d);
        appendCrc(buf);
        assert(buf.size() == sz);
        return buf;
    }
    template <> ReadSeqAckResponse<Cfg> decode<ReadSeqAckResponse<Cfg>>(BufferView buf, uint8_t txn_id, MessageType msg_type) const
    {
        auto const count = extractLength(buf);
        auto data = extractDataArray(buf, count);
        if (buf.size() != 0) throw MalformedPacketException();
        return ReadSeqAckResponse<Cfg>{
            .transaction_id = txn_id,
            .data = std::move(data),
        };
    }
    Buffer encode(WriteSeqAckResponse<Cfg> const& resp) const
    {
        auto const sz = calcSize(0, 0, 0);
        auto buf = mkBuffer(sz, resp.transaction_id, MessageType::eAckSeqWrite);
        appendCrc(buf);
        assert(buf.size() == sz);
        return buf;
    }
    template <> WriteSeqAckResponse<Cfg> decode<WriteSeqAckResponse<Cfg>>(BufferView buf, uint8_t txn_id, MessageType msg_type) const
    {
        if (buf.size() != 0) throw MalformedPacketException();
        return WriteSeqAckResponse<Cfg>{
            .transaction_id = txn_id,
        };
    }
    Buffer encode(ReadCompAckResponse<Cfg> const& resp) const
    {
        if (resp.data.size() > this->getMaxCompReadCount())
            throw MessageSizeException("ReadCompAckResponse count exceeded transport-imposed limit");
        auto const sz = calcSize(0, resp.data.size(), 1);
        auto buf = mkBuffer(sz, resp.transaction_id, MessageType::eAckCompRead);
        appendLength(buf, resp.data.size());
        for (auto const d : resp.data)
            appendData(buf, d);
        appendCrc(buf);
        assert(buf.size() == sz);
        return buf;
    }
    template <> ReadCompAckResponse<Cfg> decode<ReadCompAckResponse<Cfg>>(BufferView buf, uint8_t txn_id, MessageType msg_type) const
    {
        auto const count = extractLength(buf);
        auto data = extractDataArray(buf, count);
        if (buf.size() != 0) throw MalformedPacketException();
        return ReadCompAckResponse<Cfg>{
            .transaction_id = txn_id,
            .data = std::move(data),
        };
    }
    Buffer encode(WriteCompAckResponse<Cfg> const& resp) const
    {
        auto const sz = calcSize(0, 0, 0);
        auto buf = mkBuffer(sz, resp.transaction_id, MessageType::eAckCompWrite);
        appendCrc(buf);
        assert(buf.size() == sz);
        return buf;
    }
    template <> WriteCompAckResponse<Cfg> decode<WriteCompAckResponse<Cfg>>(BufferView buf, uint8_t txn_id, MessageType msg_type) const
    {
        if (buf.size() != 0) throw MalformedPacketException();
        return WriteCompAckResponse<Cfg>{
            .transaction_id = txn_id,
        };
    }
    Buffer encode(ReadSingleNakResponse<Cfg> const& resp) const
    {
        auto const sz = calcSize(0, 1, 0);
        auto buf = mkBuffer(sz, resp.transaction_id, MessageType::eNakSingleRead);
        appendData(buf, resp.status);
        appendCrc(buf);
        assert(buf.size() == sz);
        return buf;
    }
    template <> ReadSingleNakResponse<Cfg> decode<ReadSingleNakResponse<Cfg>>(BufferView buf, uint8_t txn_id, MessageType msg_type) const
    {
        auto const status = extractData(buf);
        if (buf.size() != 0) throw MalformedPacketException();
        return ReadSingleNakResponse<Cfg>{
            .transaction_id = txn_id,
            .status = status,
        };
    }
    Buffer encode(WriteSingleNakResponse<Cfg> const& resp) const
    {
        auto const sz = calcSize(0, 1, 0);
        auto buf = mkBuffer(sz, resp.transaction_id, MessageType::eNakSingleWrite);
        appendData(buf, resp.status);
        appendCrc(buf);
        assert(buf.size() == sz);
        return buf;
    }
    template <> WriteSingleNakResponse<Cfg> decode<WriteSingleNakResponse<Cfg>>(BufferView buf, uint8_t txn_id, MessageType msg_type) const
    {
        auto const status = extractData(buf);
        if (buf.size() != 0) throw MalformedPacketException();
        return WriteSingleNakResponse<Cfg>{
            .transaction_id = txn_id,
            .status = status,
        };
    }
    Buffer encode(ReadSeqNakResponse<Cfg> const& resp) const
    {
        auto const sz = calcSize(0, 1, 0);
        auto buf = mkBuffer(sz, resp.transaction_id, MessageType::eNakSeqRead);
        appendData(buf, resp.status);
        appendCrc(buf);
        assert(buf.size() == sz);
        return buf;
    }
    template <> ReadSeqNakResponse<Cfg> decode<ReadSeqNakResponse<Cfg>>(BufferView buf, uint8_t txn_id, MessageType msg_type) const
    {
        auto const status = extractData(buf);
        if (buf.size() != 0) throw MalformedPacketException();
        return ReadSeqNakResponse<Cfg>{
            .transaction_id = txn_id,
            .status = status,
        };
    }
    Buffer encode(WriteSeqNakResponse<Cfg> const& resp) const
    {
        auto const sz = calcSize(0, 1, 0);
        auto buf = mkBuffer(sz, resp.transaction_id, MessageType::eNakSeqWrite);
        appendData(buf, resp.status);
        appendCrc(buf);
        assert(buf.size() == sz);
        return buf;
    }
    template <> WriteSeqNakResponse<Cfg> decode<WriteSeqNakResponse<Cfg>>(BufferView buf, uint8_t txn_id, MessageType msg_type) const
    {
        auto const status = extractData(buf);
        if (buf.size() != 0) throw MalformedPacketException();
        return WriteSeqNakResponse<Cfg>{
            .transaction_id = txn_id,
            .status = status,
        };
    }
    Buffer encode(ReadCompNakResponse<Cfg> const& resp) const
    {
        auto const sz = calcSize(0, 1, 0);
        auto buf = mkBuffer(sz, resp.transaction_id, MessageType::eNakCompRead);
        appendData(buf, resp.status);
        appendCrc(buf);
        assert(buf.size() == sz);
        return buf;
    }
    template <> ReadCompNakResponse<Cfg> decode<ReadCompNakResponse<Cfg>>(BufferView buf, uint8_t txn_id, MessageType msg_type) const
    {
        auto const status = extractData(buf);
        if (buf.size() != 0) throw MalformedPacketException();
        return ReadCompNakResponse<Cfg>{
            .transaction_id = txn_id,
            .status = status,
        };
    }
    Buffer encode(WriteCompNakResponse<Cfg> const& resp) const
    {
        auto const sz = calcSize(0, 1, 0);
        auto buf = mkBuffer(sz, resp.transaction_id, MessageType::eNakCompWrite);
        appendData(buf, resp.status);
        appendCrc(buf);
        assert(buf.size() == sz);
        return buf;
    }
    template <> WriteCompNakResponse<Cfg> decode<WriteCompNakResponse<Cfg>>(BufferView buf, uint8_t txn_id, MessageType msg_type) const
    {
        auto const status = extractData(buf);
        if (buf.size() != 0) throw MalformedPacketException();
        return WriteCompNakResponse<Cfg>{
            .transaction_id = txn_id,
            .status = status,
        };
    }
    Buffer encode(ReadmodifywriteSingleAckResponse<Cfg> const& resp) const
    {
        auto const sz = calcSize(0, 0, 0);
        auto buf = mkBuffer(sz, resp.transaction_id, MessageType::eAckSingleRmw);
        appendCrc(buf);
        assert(buf.size() == sz);
        return buf;
    }
    template <> ReadmodifywriteSingleAckResponse<Cfg> decode<ReadmodifywriteSingleAckResponse<Cfg>>(BufferView buf, uint8_t txn_id, MessageType msg_type) const
    {
        if (buf.size() != 0) throw MalformedPacketException();
        return ReadmodifywriteSingleAckResponse<Cfg>{
            .transaction_id = txn_id,
        };
    }
    Buffer encode(ReadmodifywriteSingleNakResponse<Cfg> const& resp) const
    {
        auto const sz = calcSize(0, 0, 0);
        auto buf = mkBuffer(sz, resp.transaction_id, MessageType::eNakSingleRmw);
        appendCrc(buf);
        assert(buf.size() == sz);
        return buf;
    }
    template <> ReadmodifywriteSingleNakResponse<Cfg> decode<ReadmodifywriteSingleNakResponse<Cfg>>(BufferView buf, uint8_t txn_id, MessageType msg_type) const
    {
        auto const status = extractData(buf);
        if (buf.size() != 0) throw MalformedPacketException();
        return ReadmodifywriteSingleNakResponse<Cfg>{
            .transaction_id = txn_id,
            .status = status,
        };
    }
    Buffer encode(Interrupt<Cfg> const& resp) const
    {
        auto const sz = calcSize(0, 1, 0);
        auto buf = mkBuffer(sz, resp.transaction_id, MessageType::eAckSingleInterrupt);
        appendData(buf, resp.status);
        appendCrc(buf);
        assert(buf.size() == sz);
        return buf;
    }
    template <> Interrupt<Cfg> decode<Interrupt<Cfg>>(BufferView buf, uint8_t txn_id, MessageType msg_type) const
    {
        auto const status = extractData(buf);
        if (buf.size() != 0) throw MalformedPacketException();
        return Interrupt<Cfg>{
            .transaction_id = txn_id,
            .status = status,
        };
    }
private:
    size_t max_message_size;
};

}
