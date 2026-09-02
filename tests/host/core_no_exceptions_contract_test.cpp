#include <Strata.h>

struct Value {
	explicit Value(int value) noexcept : value(value) {}
	~Value() noexcept = default;

	int value;
};

int main() {
	auto *raw = Strata::allocate(128, Strata::Placement::Internal);
	if (raw == nullptr) {
		return 1;
	}
	Strata::free(raw);

	Strata::Buffer buffer{256, Strata::Placement::Internal};
	if (buffer.empty() || buffer.data() == nullptr || buffer.size() != 256) {
		return 2;
	}

	auto value = Strata::makeUnique<Value>(Strata::Placement::Internal, 42);
	if (!value || value->value != 42) {
		return 3;
	}

	return 0;
}
