#pragma once

#include <cstddef>

#define ARDUINOJSON_VERSION "7.4.3"
#define ARDUINOJSON_VERSION_MAJOR 7
#define ARDUINOJSON_VERSION_MINOR 4
#define ARDUINOJSON_VERSION_REVISION 3

namespace ArduinoJson {

class Allocator {
public:
	virtual ~Allocator() = default;
	virtual void *allocate(std::size_t size) = 0;
	virtual void deallocate(void *pointer) = 0;
	virtual void *reallocate(void *pointer, std::size_t newSize) = 0;
};

class JsonDocument {
public:
	explicit JsonDocument(Allocator *allocator) noexcept : allocator_(allocator) {}

	~JsonDocument() {
		if (storage_ != nullptr) {
			allocator_->deallocate(storage_);
		}
	}

	JsonDocument(const JsonDocument &) = delete;
	JsonDocument &operator=(const JsonDocument &) = delete;

	[[nodiscard]] bool reserve(std::size_t size) {
		void *next = storage_ == nullptr ? allocator_->allocate(size) : allocator_->reallocate(storage_, size);
		if (next == nullptr) {
			return false;
		}
		storage_ = next;
		size_ = size;
		return true;
	}

	[[nodiscard]] void *storage() const noexcept {
		return storage_;
	}

	[[nodiscard]] std::size_t size() const noexcept {
		return size_;
	}

private:
	Allocator *allocator_{nullptr};
	void *storage_{nullptr};
	std::size_t size_{0};
};

} // namespace ArduinoJson
