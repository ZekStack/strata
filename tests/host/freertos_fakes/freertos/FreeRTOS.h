#pragma once

#include <cstdint>

using StackType_t = std::uint32_t;
using BaseType_t = int;
using UBaseType_t = unsigned int;
using configSTACK_DEPTH_TYPE = std::uint32_t;

struct StaticTask_t {
    std::uint32_t marker{0};
};

#define configSUPPORT_STATIC_ALLOCATION 1
#define tskIDLE_PRIORITY 0U
