#include <strata/arduinojson/Allocator.h>

#include <ArduinoJson.h>
#include <esp_heap_caps.h>

#include <cassert>

int main() {
	using Strata::ArduinoJson::Allocator;
	using Strata::Placement;
	using Strata::Region;

	fake_heap_caps_reset();
	{
		Allocator preferred{Placement::PreferExternal};
		::ArduinoJson::JsonDocument document{&preferred};
		assert(document.reserve(64 * 1024));
		assert(Strata::regionOf(document.storage()) == Region::External);
		assert(document.reserve(128 * 1024));
		assert(Strata::regionOf(document.storage()) == Region::External);
	}
	assert(fake_heap_caps_allocations.empty());

	fake_heap_caps_reset();
	fake_heap_caps_fail_external = true;
	{
		Allocator fallback{Placement::PreferExternal};
		::ArduinoJson::JsonDocument document{&fallback};
		assert(document.reserve(64 * 1024));
		assert(Strata::regionOf(document.storage()) == Region::Internal);
	}
	assert(fake_heap_caps_allocations.empty());

	fake_heap_caps_reset();
	fake_heap_caps_external_total = 0;
	Allocator strict{Placement::RequireExternal};
	::ArduinoJson::JsonDocument unavailable{&strict};
	assert(!unavailable.reserve(64 * 1024));

	fake_heap_caps_reset();
	Allocator internal{Placement::Internal};
	::ArduinoJson::JsonDocument internalDocument{&internal};
	assert(internalDocument.reserve(64 * 1024));
	assert(Strata::regionOf(internalDocument.storage()) == Region::Internal);
}
