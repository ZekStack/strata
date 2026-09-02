#pragma once

#include <cstdint>

namespace Strata {

enum class Capability : std::uint8_t {
    None = 0,
    Dma = 1U << 0,
    Executable = 1U << 1,
};

[[nodiscard]] constexpr Capability operator|(Capability lhs, Capability rhs) noexcept {
    return static_cast<Capability>(
        static_cast<std::uint8_t>(lhs) | static_cast<std::uint8_t>(rhs));
}

[[nodiscard]] constexpr Capability operator&(Capability lhs, Capability rhs) noexcept {
    return static_cast<Capability>(
        static_cast<std::uint8_t>(lhs) & static_cast<std::uint8_t>(rhs));
}

constexpr Capability &operator|=(Capability &lhs, Capability rhs) noexcept {
    lhs = lhs | rhs;
    return lhs;
}

[[nodiscard]] constexpr bool hasCapability(Capability set, Capability capability) noexcept {
    return (set & capability) == capability;
}

inline constexpr std::uint8_t KnownCapabilityBits =
    static_cast<std::uint8_t>(Capability::Dma) |
    static_cast<std::uint8_t>(Capability::Executable);

[[nodiscard]] constexpr bool validCapabilities(Capability capabilities) noexcept {
    return (static_cast<std::uint8_t>(capabilities) & ~KnownCapabilityBits) == 0;
}

} // namespace Strata
