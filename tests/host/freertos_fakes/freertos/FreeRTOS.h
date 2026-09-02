#pragma once

#include <cstddef>
#include <cstdint>

using StackType_t = std::uint32_t;
using BaseType_t = int;
using UBaseType_t = unsigned int;
using TickType_t = std::uint32_t;
using configSTACK_DEPTH_TYPE = std::uint32_t;

inline constexpr BaseType_t pdFALSE = 0;
inline constexpr BaseType_t pdTRUE = 1;
inline constexpr TickType_t portMAX_DELAY = 0xFFFFFFFFU;

struct StaticTask_t {
	std::uint32_t marker{0};
};

struct StaticQueue_t {
	std::uint8_t *storage{nullptr};
	std::size_t length{0};
	std::size_t itemSize{0};
	std::size_t head{0};
	std::size_t tail{0};
	std::size_t count{0};
	bool deleted{false};
};

#define configSUPPORT_STATIC_ALLOCATION 1
#define configUSE_MUTEXES 1
#define configUSE_RECURSIVE_MUTEXES 1
#define INCLUDE_vTaskDelete 1
#define INCLUDE_uxTaskGetStackHighWaterMark 1
#define tskIDLE_PRIORITY 0U
