#pragma once

#include <cstdint>

namespace Strata::Internal {

enum class PlatformKind : std::uint8_t {
    Generic,
    Esp32,
};

[[nodiscard]] PlatformKind platformKind() noexcept;
[[nodiscard]] const char *platformName() noexcept;

} // namespace Strata::Internal
