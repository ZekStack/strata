#include "Platform.h"

#if !defined(ESP32)

namespace Strata::Internal {

PlatformKind platformKind() noexcept {
    return PlatformKind::Generic;
}

const char *platformName() noexcept {
    return "generic";
}

} // namespace Strata::Internal

#endif
