# ArduinoJson integration

Strata provides an optional ArduinoJson 7 allocator adapter through:

```cpp
#include <ArduinoJson.h>
#include <strata/arduinojson/Allocator.h>
```

The adapter is not included by `Strata.h`, and ArduinoJson is not a required Strata dependency.

## Supported version

The integration targets ArduinoJson major version 7 and is tested against ArduinoJson 7.4.3 in CI. Other ArduinoJson 7.x releases are expected to use the same allocator interface, but Strata rejects other major versions at compile time.

## Basic usage

```cpp
Strata::ArduinoJson::Allocator allocator{Strata::Placement::PreferExternal};
ArduinoJson::JsonDocument document{&allocator};

document["name"] = "Strata";
```

The allocator object must outlive every `ArduinoJson::JsonDocument` that uses it. Strata deliberately does not hide that ownership relationship behind a document wrapper.

## Placement semantics

The adapter forwards ArduinoJson's allocation lifecycle to the normal Strata allocation APIs:

- `Placement::Internal` requires internal memory;
- `Placement::PreferExternal` prefers external memory and may fall back to internal memory;
- `Placement::RequireExternal` fails when external memory is unavailable;
- `Placement::Default` follows the platform default allocator policy.

Reallocation preserves the same requested placement policy. A failed reallocation leaves the original ArduinoJson allocation valid according to Strata's normal reallocation contract.

## Failure behavior

ArduinoJson 7 grows `JsonDocument` dynamically. If Strata cannot satisfy an allocation or reallocation, ArduinoJson reports document overflow through `JsonDocument::overflowed()`.

For strict external placement, an unavailable PSRAM region therefore results in ArduinoJson allocation failure rather than silently moving the document to internal RAM.

## ESP32 and PSRAM

Using `PreferExternal` or `RequireExternal` routes ArduinoJson memory through Strata's ESP32 heap-capability backend. No ArduinoJson-specific ESP-IDF allocator code is required in application code.

External RAM remains subject to normal ESP32 cache and flash-operation restrictions. Do not assume a JSON allocation in PSRAM is safe to access while external memory is unavailable because caches are disabled.

## Lifetime

ArduinoJson stores a pointer to the custom allocator; it does not own it. Create the allocator before the document and destroy it after the document.

```cpp
Strata::ArduinoJson::Allocator allocator{Strata::Placement::PreferExternal};
{
	ArduinoJson::JsonDocument document{&allocator};
	// use document
}
```
