#pragma once

#include <cstdint>

namespace Strata {

enum class Placement : std::uint8_t {
    Default,
    Internal,
    PreferExternal,
    RequireExternal,
};

enum class Region : std::uint8_t {
    Unknown,
    Internal,
    External,
};

[[nodiscard]] constexpr const char *toString(Placement placement) noexcept {
    switch (placement) {
        case Placement::Default:
            return "default";
        case Placement::Internal:
            return "internal";
        case Placement::PreferExternal:
            return "prefer-external";
        case Placement::RequireExternal:
            return "require-external";
    }

    return "unknown";
}

[[nodiscard]] constexpr const char *toString(Region region) noexcept {
    switch (region) {
        case Region::Unknown:
            return "unknown";
        case Region::Internal:
            return "internal";
        case Region::External:
            return "external";
    }

    return "unknown";
}

} // namespace Strata
