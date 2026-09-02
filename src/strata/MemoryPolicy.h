#pragma once

#include "Placement.h"

namespace Strata {

struct MemoryPolicy {
	Placement allocation{Placement::Default};
	Placement taskStack{Placement::Internal};
};

[[nodiscard]] constexpr bool validMemoryPolicy(const MemoryPolicy &policy) noexcept {
	return validPlacement(policy.allocation) && validPlacement(policy.taskStack);
}

} // namespace Strata
