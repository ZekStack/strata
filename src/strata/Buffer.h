#pragma once

#include "Allocation.h"
#include "Diagnostics.h"

#include <cstddef>
#include <type_traits>
#include <utility>

namespace Strata {

class Buffer {
public:
    Buffer() noexcept = default;

    explicit Buffer(
        std::size_t sizeBytes,
        Placement placement = Placement::Default) noexcept
        : placement_(placement) {
        if (sizeBytes == 0) {
            return;
        }

        data_ = Strata::allocate(sizeBytes, placement_);
        if (data_ != nullptr) {
            sizeBytes_ = sizeBytes;
        }
    }

    ~Buffer() noexcept {
        reset();
    }

    Buffer(const Buffer &) = delete;
    Buffer &operator=(const Buffer &) = delete;

    Buffer(Buffer &&other) noexcept
        : data_(std::exchange(other.data_, nullptr)),
          sizeBytes_(std::exchange(other.sizeBytes_, 0)),
          placement_(std::exchange(other.placement_, Placement::Default)) {}

    Buffer &operator=(Buffer &&other) noexcept {
        if (this == &other) {
            return *this;
        }

        reset();
        data_ = std::exchange(other.data_, nullptr);
        sizeBytes_ = std::exchange(other.sizeBytes_, 0);
        placement_ = std::exchange(other.placement_, Placement::Default);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return sizeBytes_;
    }

    [[nodiscard]] bool empty() const noexcept {
        return sizeBytes_ == 0;
    }

    [[nodiscard]] void *data() noexcept {
        return data_;
    }

    [[nodiscard]] const void *data() const noexcept {
        return data_;
    }

    template <typename T>
    requires (std::is_object_v<T> && !std::is_void_v<T>)
    [[nodiscard]] T *data() noexcept {
        return static_cast<T *>(data_);
    }

    template <typename T>
    requires (std::is_object_v<T> && !std::is_void_v<T>)
    [[nodiscard]] const T *data() const noexcept {
        return static_cast<const T *>(data_);
    }

    [[nodiscard]] Placement placement() const noexcept {
        return placement_;
    }

    [[nodiscard]] Region region() const noexcept {
        return Strata::regionOf(data_);
    }

    [[nodiscard]] bool resize(std::size_t newSizeBytes) noexcept {
        if (newSizeBytes == sizeBytes_) {
            return true;
        }

        if (newSizeBytes == 0) {
            reset();
            return true;
        }

        void *resized = data_ == nullptr
            ? Strata::allocate(newSizeBytes, placement_)
            : Strata::reallocate(data_, newSizeBytes, placement_);
        if (resized == nullptr) {
            return false;
        }

        data_ = resized;
        sizeBytes_ = newSizeBytes;
        return true;
    }

    void reset() noexcept {
        Strata::free(data_);
        data_ = nullptr;
        sizeBytes_ = 0;
    }

    [[nodiscard]] void *release() noexcept {
        sizeBytes_ = 0;
        return std::exchange(data_, nullptr);
    }

private:
    void *data_{nullptr};
    std::size_t sizeBytes_{0};
    Placement placement_{Placement::Default};
};

} // namespace Strata
