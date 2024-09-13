#pragma once
#include <format>
#include <span>
#include <stdexcept>
#include <vector>
#include <stdint.h>

namespace RAP {

using BufferView = std::span<uint8_t const>;
using Buffer = std::vector<uint8_t>;

class Exception : public std::runtime_error
{
public:
    Exception(char const* msg) : runtime_error(msg) {}
    Exception(std::string const& msg) : runtime_error(msg) {}
};

class CrcMismatchException : public Exception
{
public:
    CrcMismatchException(uint32_t expected_, uint32_t actual_)
        : Exception(std::format("CRC mismatch on packet. got:0x{} want:0x{}", expected_, actual_))
        , expected(expected_)
        , actual(actual_)
    {}

    uint32_t expected;
    uint32_t actual;
};

class OperationNakException : public Exception
{
public:
    OperationNakException(uint32_t status_)
        : Exception(std::format("NAK received from Device. status:0x{}", status_))
        , status(status_)
    {}

    uint32_t status;
};

class UnexpectedMessageTypeException : public Exception
{
public:
    UnexpectedMessageTypeException() : Exception("Received an unexpected (but valid) message type.") {}
};

class MalformedPacketException : public Exception
{
public:
    MalformedPacketException() : Exception("Packet passed CRC but is otherwise malformed.") {}
};

class RapProtocolException : public Exception
{
public:
    RapProtocolException() : Exception("Response Transaction ID does not match Command Transaction ID.") {}
};

class MessageSizeException : public Exception
{
public:
    MessageSizeException() : Exception("Message size exceeds some limit.") {}
    MessageSizeException(char const* msg) : Exception(msg) {}
    MessageSizeException(std::string const& msg) : Exception(msg) {}
};

}
