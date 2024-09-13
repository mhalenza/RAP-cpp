# RAP-cpp

This repository contains the reference C++ implementation of the Register Access Protocol.

[Main RAP Specification](https://github.com/mhalenza/RAP-spec)

## Table of Contents
- [Configuration](#configuration)
- [Serdes](#serdes)
- [Transports](#transports)
- [RapRegisterTarget](#rapregistertarget)
- [RapServerAdapter](#rapserveradapter)
- [Example!](#pure-software-example)

## Implementation Notes/TODO
- CRCs are not implemented yet.  A placeholder `0xFE..` is used for now.
- Only the in-process paired transport is implemented.
- RapRegisterTarget needs to implement chunking for large messages.
- Interrupt handling is not thought out yet.

## Configuration
The implementation aims to be as configurable as the RAP spec itself.
This is accomplished using templates everywhere where the spec is configurable.

Configuraton is handled by defining a "traits" struct with the parameters of the configuration:
```c++
struct SomeRapImplementationConfiguration {
    using AddressType = ...;
    static constexpr uint8_t AddressBits = ...;
    static constexpr uint8_t AddressBytes = ...;
    using DataType = ...;
    static constexpr uint8_t DataBits = ...;
    static constexpr uint8_t DataBytes = ...;
    using LengthType = ...;
    static constexpr uint8_t LengthBytes = ...;
    using CrcType = ...;
    static constexpr uint8_t CrcBytes = ...;
    static constexpr bool FeatureSequential = ...;
    static constexpr bool FeatureFifo = ...;
    static constexpr bool FeatureIncrement = ...;
    static constexpr bool FeatureCompressed = ...;
    static constexpr bool FeatureInterrupt = ...;
    static constexpr bool FeatureReadModifyWrite = ...;
};
static_assert(RAP::IsConfigurationType<SomeRapImplementationConfiguration>);
```

Configuration Items
- `AddressType` the C++ type used for Address fields.
- `AddressBits` sets the number of bits actually used from Address field.
- `AddressBytes` sets the number of bytes used in Address fields on the wire.
- `DataType` the C++ type used for Data fields.    
- `DataBits` sets the number of bits actually used from Data field.
- `DataBytes` sets the number of bytes used in Data fields on the wire.
- `LengthType` the C++ type used for Length fields.
- `LengthBytes` ets the number of bytes used in Length fields on the wire.
- `CrcType` the C++ type used for CRC fields.
- `CrcBytes` ets the number of bytes used in CRC fields on the wire.
- `FeatureSequential` Feature flag corresponding to `S` in the RAP spec.
- `FeatureFifo` Feature flag corresponding to `F` in the RAP spec.
- `FeatureIncrement` Feature flag corresponding to `I` in the RAP spec.
- `FeatureCompressed` Feature flag corresponding to `C` in the RAP spec.
- `FeatureInterrupt` Feature flag corresponding to `Q` in the RAP spec.
- `FeatureReadModifyWrite` Feature flag corresponding to `M` in the RAP spec.

The validity of the configuration struct should be asserted using the `RAP::IsConfigurationType` concept.
This will ensure requirements (especially between `*Type`, `*Bits`, and `*Bytes` members) are correct.

## Serdes
The namespace `RAP::Serdes` and the class `RAP::Serdes::Serdes` contain an implementation of Serialization and Parsing routines.

The constructor takes a parameter that indicates the maximum message size allowed by an associated Transport.
If a message serialization would result in a buffer larger than this maximum, an exception (`MessageSizeException`) will be thrown.

The main interface to this class is as follows:
```c++
    Buffer encodeCommand(Command<Cfg> const& cmd); // Encode a command into a byte buffer
    Response<Cfg> decodeResponse(BufferView buff); // Decode a byte buffer into a response
    Command<Cfg> decodeCommand(BufferView buff); // Decode a byte buffer into a command
    Buffer encodeResponse(Response<Cfg> const& resp); // Encode a response into a byte buffer
```

The pair `encodeCommand()` and `decodeResponse()` are expected to be used in client-side implementations,
while the pair `decodeCommand()` and `encodeResponse()` are expected to be used in server-side implementations.

These functions will throw exceptions to indicate serialization or parsing errors.
A few `assert()`s are included to ensure integrity of the serialization and parsing routines.

## Transports
Currently only Synchronous transports have been defined.

### ISyncWireTransport
This interface defines the API contract for all synchronous tranports.
Timeouts are signalled by throwing an exception (`TransportTimeoutException`).

- `void send(BufferView buffer)` Sends a serialized message out the transport.
- `Buffer recv()` Blocks until a serialized message is received by the transport or a timeout occurrs.
- `Buffer recv(std::stop_token stoken)` Blocks until a serialized message is received by the transport, or a timeout occurs, or the stop_token is signalled._
- `uint16_t getMaxMessageSize() const` Returns the maximum message size supported by the transport.
- `void setTimeout(std::chrono::microseconds timeout)` Sets the timeout to be used by the transport.

#### Sync Paired IPC Transport
`std::pair<std::unique_ptr<ISyncWireTransport>, std::unique_ptr<ISyncWireTransport>> makeSyncPairedIpcTransport(size_t max_message_size);`

This transport is meant for demonstration / testing purposes.
It is handled entirely within a single process by using a pair of queues.
A `Buffer` that is `send()` by one transport can be `recv()`'d by the other and the pair is bidirectional.

#### Sync UDP Transport
A UDP-based Transport is planned to be implemented next.

#### Sync Serial Transport
A UART-based Transport is planned to be implemented eventually.

#### Sync SpW Transport
A SpaceWire-based Transport is planned to be implemented eventually.

## RapRegisterTarget
`RapRegisterTarget` is a subclass of `RTF::IRegisterTarget` aimed for use in applications using the [Register Target Framework](https://github.com/mhalenza/RTF) (RTF).

The constructor takes a `name` (as all `IRegisterTarget`'s) and a `std::unique_ptr<RAP::Transport::ISyncWireTransport>` for the transport to use.

The class will automatically adjust itself based on the feature flags and other configuration items in the Configuration struct.

## RapServerAdapter
`RapServerAdapter` provides a "server side" implemenatation that forwards commands to an `RTF::IRegisterTarget`.
As "server side" implementations are expected to primarily be implemented in hardware, this class is not very robust.
The main purpose is to "close the loop" and allow for unit testing.

The constructor takes a transport and an `IRegisterTarget` to which commands will be forwarded.

## Pure Software Example
Closing the loop entirely in software is extremely simple.
An example using the Sync Paired IPC Transport is as follows:
```c++
    struct CFG {
        // ...
    };
    static_assert(RAP::IsConfigurationType<CFG>);
    auto [client_xport, server_xport] = RAP::Transport::makeSyncPairedIpcTransport(512);
    client_xport->setTimeout(std::chrono::seconds(1));

    auto rap_target = RAP::RTF::RapRegisterTarget<CFG>("Rap Target", std::move(client_xport));

    auto simple_target = std::make_shared<RTF::SimpleDummyRegisterTarget<CFG::AddressType, CFG::DataType>>("Simple Dummy");
    auto rap_server_adapter = RAP::RTF::RapServerAdapter<CFG>(std::move(server_xport), simple_target);

    auto fluent_target = RTF::FluentRegisterTarget{ rap_target };
```

At this point `fluent_target` can be used like any RTF::FluentRegisterTarget.
Operations will be sent to the `rap_target` which will serialize the messages and push them into the `client_xport`.
On a separate thread, `rap_server_adapter` will receive those commands from the `server_xport_`, deserialize them, and pass them on to the `simple_target`.
Responses (read data and exceptions) will be handled by the worker thread, converted into ACK or NAK messages, serialized, and send back through the transports.
Finally, the `rap_target` will receive the response messages, deserialze them, and handle them appropriately.
