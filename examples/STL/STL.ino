#include <Strata.h>

struct State {
    explicit State(int value) : value(value) {}
    int value;
};

void setup() {
    Serial.begin(115200);

    auto samples = Strata::makeVector<int>(Strata::Placement::PreferExternal);
    samples.reserve(16);
    samples.push_back(10);
    samples.push_back(20);

    auto label = Strata::makeString(Strata::Placement::PreferExternal);
    label = "Strata STL";

    auto lookup = Strata::makeMap<int, int>(Strata::Placement::PreferExternal);
    lookup.emplace(1, 100);

    auto state = Strata::makeShared<State>(Strata::Placement::PreferExternal, 42);

    Serial.printf("%s: %d samples, state=%d\n", label.c_str(), static_cast<int>(samples.size()), state->value);
}

void loop() {}
