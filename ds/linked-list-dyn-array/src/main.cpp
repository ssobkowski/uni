#include "bench.hpp"

int main() {
    const std::vector<size_t> test_sizes = { 10'000, 20'000, 50'000, 100'000, 200'000 };

	Benchmark::run(test_sizes);
	return 0;
}
