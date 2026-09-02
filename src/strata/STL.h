#pragma once

#include "Allocator.h"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Strata {

template <typename T>
using Vector = std::vector<T, Allocator<T>>;

using String = std::basic_string<char, std::char_traits<char>, Allocator<char>>;

template <typename T>
[[nodiscard]] Vector<T> makeVector(Placement placement = Placement::Default) {
    return Vector<T>{Allocator<T>{placement}};
}

[[nodiscard]] inline String makeString(Placement placement = Placement::Default) {
    return String{Allocator<char>{placement}};
}

template <typename Key, typename T, typename Compare = std::less<Key>>
[[nodiscard]] auto makeMap(
    Placement placement = Placement::Default,
    Compare compare = Compare{}) {
    using Value = std::pair<const Key, T>;
    using Map = std::map<Key, T, Compare, Allocator<Value>>;
    return Map{compare, Allocator<Value>{placement}};
}

template <typename T, typename... Args>
[[nodiscard]] std::shared_ptr<T> makeShared(Placement placement, Args &&...args) {
    return std::allocate_shared<T>(
        Allocator<T>{placement},
        std::forward<Args>(args)...);
}

} // namespace Strata
