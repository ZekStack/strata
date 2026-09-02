#pragma once

#include "../Allocation.h"

#if !__has_include(<ArduinoJson.h>)
#error "Strata ArduinoJson integration requires ArduinoJson 7"
#endif

#include <ArduinoJson.h>

#if ARDUINOJSON_VERSION_MAJOR != 7
#error "Strata ArduinoJson integration requires ArduinoJson major version 7"
#endif

namespace Strata::ArduinoJson {

class Allocator final : public ::ArduinoJson::Allocator {
public:
	explicit Allocator(Placement placement = Placement::Default) noexcept : placement_(placement) {}

	void *allocate(std::size_t size) override {
		return Strata::allocate(size, placement_);
	}

	void deallocate(void *pointer) override {
		Strata::free(pointer);
	}

	void *reallocate(void *pointer, std::size_t newSize) override {
		return Strata::reallocate(pointer, newSize, placement_);
	}

	[[nodiscard]] Placement placement() const noexcept {
		return placement_;
	}

private:
	Placement placement_{Placement::Default};
};

} // namespace Strata::ArduinoJson
