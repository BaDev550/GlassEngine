#pragma once

#include <type_traits>

namespace ge {
    template <typename FlagBitsType>
    struct FlagTraits {
        static constexpr bool isBitmask = false;
    };

    template<typename T>
    class Flags {
    public:
        using Type = std::underlying_type_t<T>;
        constexpr Flags() noexcept : _value(0) {}
        constexpr Flags(T bit) noexcept : _value(static_cast<Type>(bit)) {}
        constexpr Flags(Type bits) noexcept : _value(bits) {}
        constexpr bool operator==(Type bits) const noexcept = default;
        constexpr bool operator==(Flags bits) const noexcept = default;
        constexpr Flags operator|(Flags o) const noexcept { return Flags(_value | o._value); }
        constexpr Flags operator&(Flags o) const noexcept { return Flags(_value & o._value); }
        constexpr Flags operator^(Flags o) const noexcept { return Flags(_value ^ o._value); }
        constexpr Flags operator~() const noexcept { return Flags(~_value); }
        constexpr Flags& operator|=(Flags o) noexcept { _value |= o._value; return *this; }
        constexpr Flags& operator&=(Flags o) noexcept { _value &= o._value; return *this; }
        constexpr Flags& operator^=(Flags o) noexcept { _value ^= o._value; return *this; }
        constexpr bool Any()  const noexcept { return _value != 0; }
        constexpr bool None() const noexcept { return _value == 0; }
        constexpr bool Has(T bit) const noexcept { return (_value & static_cast<Type>(bit)) != 0; }
        constexpr explicit operator Type() const noexcept { return _value; }
        constexpr explicit operator bool() const noexcept { return Any(); }
    private:
        Type _value;
    };

    template <typename BitType, typename std::enable_if<FlagTraits<BitType>::isBitmask, bool>::type = true>
    constexpr Flags<BitType> operator&(BitType lhs, BitType rhs) noexcept { return Flags<BitType>(lhs) & rhs; }

    template <typename BitType, typename std::enable_if<FlagTraits<BitType>::isBitmask, bool>::type = true>
    constexpr Flags<BitType> operator|(BitType lhs, BitType rhs) noexcept { return Flags<BitType>(lhs) | rhs; }

    template <typename BitType, typename std::enable_if<FlagTraits<BitType>::isBitmask, bool>::type = true>
    constexpr Flags<BitType> operator^(BitType lhs, BitType rhs) noexcept { return Flags<BitType>(lhs) ^ rhs; }

    template <typename BitType, typename std::enable_if<FlagTraits<BitType>::isBitmask, bool>::type = true>
    constexpr Flags<BitType> operator~(BitType bit) noexcept { return ~(Flags<BitType>(bit)); }
}