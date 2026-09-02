#include "Platform.h"

#if defined(ESP32)

namespace Strata::Internal {

PlatformKind platformKind() noexcept {
    return PlatformKind::Esp32;
}

const char *platformName() noexcept {
    return "esp32";
}

} // namespace Strata::Internal

#endif
