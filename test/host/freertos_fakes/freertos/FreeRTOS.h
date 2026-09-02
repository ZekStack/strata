#pragma once

#include <cstdint>

using StackType_t = std::uint32_t;
using BaseType_t = int;
using UBaseType_t = unsigned int;

struct StaticTask_t {
    std::uint32_t marker{0};
};

#define configSUPPORT_STATIC_ALLOCATION 1
