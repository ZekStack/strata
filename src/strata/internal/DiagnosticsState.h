#pragma once

#include "../Diagnostics.h"

#include <cstddef>

namespace Strata::Internal {

#if STRATA_ENABLE_ADVANCED_DIAGNOSTICS
void recordAllocationAttempt(std::size_t sizeBytes, Placement placement) noexcept;
void recordAllocationResult(
	std::size_t sizeBytes,
	Placement placement,
	bool success,
	bool invalidRequest,
	Region region) noexcept;
#endif

} // namespace Strata::Internal
