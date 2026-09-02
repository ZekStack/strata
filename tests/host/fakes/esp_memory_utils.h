#pragma once

#include "esp_heap_caps.h"

inline bool esp_ptr_external_ram(const void *ptr) {
    const auto it = fake_heap_caps_allocations.find(ptr);
    return it != fake_heap_caps_allocations.end() && fake_heap_caps_is_external(it->second);
}

inline bool esp_ptr_internal(const void *ptr) {
    const auto it = fake_heap_caps_allocations.find(ptr);
    return it != fake_heap_caps_allocations.end() && fake_heap_caps_is_internal(it->second);
}
