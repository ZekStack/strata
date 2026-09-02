#include <Strata.h>

#include <cstddef>
#include <type_traits>

static_assert(std::is_enum_v<Strata::Placement>);
static_assert(std::is_enum_v<Strata::Region>);
static_assert(std::is_enum_v<Strata::Capability>);
static_assert(Strata::validPlacement(Strata::Placement::Default));
static_assert(Strata::validPlacement(Strata::Placement::Internal));
static_assert(Strata::validPlacement(Strata::Placement::PreferExternal));
static_assert(Strata::validPlacement(Strata::Placement::RequireExternal));
static_assert(!Strata::validPlacement(static_cast<Strata::Placement>(0xFF)));
static_assert(std::is_same_v<decltype(Strata::regionOf(nullptr)), Strata::Region>);
static_assert(noexcept(Strata::allocate(std::size_t{1}, Strata::Placement::Default)));
static_assert(noexcept(Strata::free(nullptr)));

int main() {
	using Strata::Capability;
	using Strata::Placement;
	using Strata::Region;

	Strata::AllocationRequest request{
		.sizeBytes = 64,
		.placement = Placement::Internal,
		.alignment = alignof(std::max_align_t),
		.capabilities = Capability::None,
	};
	if (request.placement != Placement::Internal) {
		return 1;
	}
	if (Strata::toString(Placement::PreferExternal) == nullptr) {
		return 2;
	}
	if (Strata::toString(Region::External) == nullptr) {
		return 3;
	}
	if (!Strata::hasCapability(Capability::Dma | Capability::Executable, Capability::Dma)) {
		return 4;
	}
	return 0;
}
