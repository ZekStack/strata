#include <Strata.h>

#include <cassert>
#include <type_traits>

static_assert(std::is_trivially_copyable_v<Strata::MemoryPolicy>);
static_assert(std::is_copy_constructible_v<Strata::MemoryPolicy>);
static_assert(std::is_copy_assignable_v<Strata::MemoryPolicy>);
static_assert(Strata::validMemoryPolicy(Strata::MemoryPolicy{}));

int main() {
	using Strata::MemoryPolicy;
	using Strata::Placement;

	const MemoryPolicy defaults{};
	assert(defaults.allocation == Placement::Default);
	assert(defaults.taskStack == Placement::Internal);
	assert(Strata::validMemoryPolicy(defaults));

	const MemoryPolicy explicitPolicy{
		.allocation = Placement::PreferExternal,
		.taskStack = Placement::RequireExternal,
	};
	assert(Strata::validMemoryPolicy(explicitPolicy));

	const MemoryPolicy invalidAllocation{
		.allocation = static_cast<Placement>(0xFF),
		.taskStack = Placement::Internal,
	};
	assert(!Strata::validMemoryPolicy(invalidAllocation));

	const MemoryPolicy invalidTaskStack{
		.allocation = Placement::Default,
		.taskStack = static_cast<Placement>(0xFF),
	};
	assert(!Strata::validMemoryPolicy(invalidTaskStack));
}
