#pragma once
#include <concepts>
#include <type_traits>
#include <stdint.h>

namespace RAP {

template <typename T>
concept ValidAdlcType = std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>;

/*
struct Configuration {
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
*/

template <typename CfgType>
concept IsConfigurationType = requires()
{
    typename CfgType::AddressType;
    ValidAdlcType<typename CfgType::AddressType> == true;
    std::same_as<decltype(CfgType::AddressBits), uint8_t>;
    std::same_as<decltype(CfgType::AddressBytes), uint8_t>;
    (CfgType::AddressBits + 7) / 8 <= CfgType::AddressBytes;
    CfgType::AddressBytes <= sizeof(CfgType::AddressType);

    typename CfgType::DataType;
    ValidAdlcType<typename CfgType::DataType> == true;
    std::same_as<decltype(CfgType::DataBits), uint8_t>;
    std::same_as<decltype(CfgType::DataBytes), uint8_t>;
    (CfgType::DataBits + 7) / 8 <= CfgType::DataBytes;
    CfgType::DataBytes <= sizeof(CfgType::DataType);

    typename CfgType::LengthType;
    ValidAdlcType<typename CfgType::LengthType> == true;
    std::same_as<decltype(CfgType::LengthBytes), uint8_t>;
    CfgType::LengthBytes <= sizeof(CfgType::LengthType);

    typename CfgType::CrcType;
    ValidAdlcType<typename CfgType::CrcType> == true;
    std::same_as<decltype(CfgType::CrcBytes), uint8_t>;
    CfgType::CrcBytes <= sizeof(CfgType::CrcType);

    std::same_as<decltype(CfgType::FeatureSequential), bool>;
    std::same_as<decltype(CfgType::FeatureFifo), bool>;
    std::same_as<decltype(CfgType::FeatureIncrement), bool>;
    std::same_as<decltype(CfgType::FeatureCompressed), bool>;
    std::same_as<decltype(CfgType::FeatureInterrupt), bool>;
    std::same_as<decltype(CfgType::FeatureReadModifyWrite), bool>;
};

struct ExampleRapCfg {
    using AddressType = uint32_t;
    static constexpr uint8_t AddressBits = 24;
    static constexpr uint8_t AddressBytes = 3;
    using DataType = uint32_t;
    static constexpr uint8_t DataBits = 32;
    static constexpr uint8_t DataBytes = 4;
    using LengthType = uint16_t;
    static constexpr uint8_t LengthBytes = 2;
    using CrcType = uint16_t;
    static constexpr uint8_t CrcBytes = 2;
    static constexpr bool FeatureSequential = true;
    static constexpr bool FeatureFifo = true;
    static constexpr bool FeatureIncrement = false;
    static constexpr bool FeatureCompressed = true;
    static constexpr bool FeatureInterrupt = false;
    static constexpr bool FeatureReadModifyWrite = false;
};
static_assert(IsConfigurationType<ExampleRapCfg>);

}
