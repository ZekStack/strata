#include <Strata.h>
#include <strata/internal/Platform.h>

#include <cassert>
#include <cstring>
#include <type_traits>

int main() {
    static_assert(std::is_enum_v<Strata::Placement>);
    static_assert(std::is_enum_v<Strata::Region>);

    static_assert(Strata::Placement::Default != Strata::Placement::Internal);
    static_assert(Strata::Placement::PreferExternal != Strata::Placement::RequireExternal);
    static_assert(Strata::Region::Unknown != Strata::Region::Internal);
    static_assert(Strata::Region::Internal != Strata::Region::External);

    assert(std::strcmp(Strata::toString(Strata::Placement::Default), "default") == 0);
    assert(std::strcmp(Strata::toString(Strata::Placement::Internal), "internal") == 0);
    assert(std::strcmp(Strata::toString(Strata::Placement::PreferExternal), "prefer-external") == 0);
    assert(std::strcmp(Strata::toString(Strata::Placement::RequireExternal), "require-external") == 0);

    assert(std::strcmp(Strata::toString(Strata::Region::Unknown), "unknown") == 0);
    assert(std::strcmp(Strata::toString(Strata::Region::Internal), "internal") == 0);
    assert(std::strcmp(Strata::toString(Strata::Region::External), "external") == 0);

#if defined(ESP32)
    assert(Strata::Internal::platformKind() == Strata::Internal::PlatformKind::Esp32);
    assert(std::strcmp(Strata::Internal::platformName(), "esp32") == 0);
#else
    assert(Strata::Internal::platformKind() == Strata::Internal::PlatformKind::Generic);
    assert(std::strcmp(Strata::Internal::platformName(), "generic") == 0);
#endif

    return 0;
}
