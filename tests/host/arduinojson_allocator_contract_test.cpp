#include <strata/arduinojson/Allocator.h>

#include <ArduinoJson.h>

#include <cassert>
#include <cstddef>
#include <type_traits>

int main() {
	using Strata::ArduinoJson::Allocator;
	using Strata::Placement;

	static_assert(std::is_base_of_v<::ArduinoJson::Allocator, Allocator>);

	Allocator internal{Placement::Internal};
	assert(internal.placement() == Placement::Internal);

	{
		::ArduinoJson::JsonDocument document{&internal};
		assert(document.reserve(128 * 1024));
		assert(document.size() == 128 * 1024);
		assert(document.reserve(256 * 1024));
		assert(document.size() == 256 * 1024);
	}

	Allocator strictExternal{Placement::RequireExternal};
	::ArduinoJson::JsonDocument unsupported{&strictExternal};
	assert(!unsupported.reserve(4096));
}
