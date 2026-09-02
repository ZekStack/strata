# Examples

Strata ships focused Arduino sketches under `examples/`.

| Example | What it demonstrates |
| --- | --- |
| `Basic` | Minimal allocation, region inspection, and cleanup. |
| `Placement` | Placement and region terminology. |
| `Allocation` | Raw allocation and explicit allocation requests. |
| `Diagnostics` | Region support and heap statistics. |
| `TypedOwnership` | Typed storage, construction, destruction, and ownership helpers. |
| `STL` | Placement-aware standard-library allocators and containers. |
| `Buffer` | Move-only owned byte storage and resize behavior. |
| `Capabilities` | Required DMA/executable constraints. |
| `FreeRTOSTask` | Optional FreeRTOS stack placement and task diagnostics. |
| `FreeRTOSQueue` | Optional typed FreeRTOS queue storage placement and task-only queue use. |
| `FreeRTOSMutex` | Optional statically created mutex and recursive-mutex ownership with internal control storage. |
| `ArduinoJson` | Optional ArduinoJson 7 document allocation through Strata placement. |

Start with `examples/Basic`. Move to the specialized example that matches the API you need rather than combining all Strata features into one sketch.

The CI matrix builds every example for ESP32, ESP32-S3, ESP32-C3, and ESP32-P4 targets. ArduinoJson 7.4.3 is supplied only to the example build environment so it remains an optional Strata integration.
