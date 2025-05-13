#include <algorithm>
#include <format>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include "bench.hpp"
#include "heap.hpp"
#include "linked-list.hpp"
#include "sorted-array.hpp"

const std::vector<size_t> element_counts = { 500, 1'000, 2'000, 5'000, 10'000, 20'000 };

int main() {
    constexpr size_t WARMUP_ITERATIONS = 50;
    constexpr size_t TEST_ITERATIONS = 300;
    constexpr size_t BATCH_SIZE = 100;
    BenchmarkSuite bench(WARMUP_ITERATIONS, TEST_ITERATIONS, BATCH_SIZE);

    std::cout << "Starting Priority Queue Benchmarks..." << std::endl;

    for (const auto& sz : element_counts) {
#pragma region Heap
        {
            // Heap (push) - average
            BenchmarkTest<Heap<int, int>> test(
                std::format("Heap (push) - {} elements, average", sz),
                sz,
                [sz](size_t) {
                    Heap<int> heap;
                    for (size_t i = 0; i < sz - BATCH_SIZE / 2; i++) {
                        heap.push(i, 0);
                    }
                    return heap;
                },
                [sz](auto& heap, size_t) {
                    heap.push(sz, -1);
                }
            );
            bench.run_test(test);
        }
        
        {
            // Heap (push) - pessimistic
            BenchmarkTest<Heap<int, int>> test(
                std::format("Heap (push) - {} elements, pessimistic", sz),
                sz,
                [sz](size_t) {
                    Heap<int> heap;
                    for (size_t i = 0; i < sz - BATCH_SIZE / 2; i++) {
                        heap.push(i, 0);
                    }
                    return heap;
                },
                [](auto& heap, size_t) {
                    heap.push(INT_MAX, -1);
                }
            );
            bench.run_test(test);
        }
        
        {
            // Heap (push) - optimistic
            BenchmarkTest<Heap<int, int>> test(
                std::format("Heap (push) - {} elements, optimistic", sz),
                sz,
                [sz](size_t) {
                    Heap<int> heap;
                    for (size_t i = 0; i < sz - BATCH_SIZE / 2; i++) {
                        heap.push(i, 0);
                    }
                    return heap;
                },
                [](auto& heap, size_t) {
                    heap.push(INT_MIN, -1);
                }
            );
            bench.run_test(test);
        }

        {
            // Heap (pop) - average
            BenchmarkTest<Heap<int, int>> test(
                std::format("Heap (pop) - {} elements, average", sz),
                sz,
                [sz](size_t) {
                    Heap<int> heap;
                    for (size_t i = 0; i < sz + BATCH_SIZE / 2; i++) {
                        heap.push(util::random_int(0, INT_MAX), 0);
                    }
                    return heap;
                },
                [](auto& heap, size_t) {
                    heap.pop();
                }
            );
            bench.run_test(test);
        }

        {
            // Heap (pop) - optimistic
            BenchmarkTest<Heap<int, int>> test(
                std::format("Heap (pop) - {} elements, optimistic", sz),
                sz,
                [sz](size_t) {
                    Heap<int> heap;
                    for (size_t i = 0; i < sz + BATCH_SIZE / 2; i++) {
                        heap.push(i, 0);
                    }
                    return heap;
                },
                [](auto& heap, size_t) {
                    heap.pop();
                }
            );
            bench.run_test(test);
        }

        {
            // Heap (set_priority) - average
            BenchmarkTest<Heap<int, int>> test(
                std::format("Heap (set_priority) - {} elements, average", sz),
                sz,
                [sz](size_t) {
                    Heap<int> heap;
                    for (size_t i = 0; i < sz; i++) {
                        heap.push(util::random_int(0, INT_MAX), i);
                    }
                    return heap;
                },
                [](auto& heap, size_t j) {
                    heap.set_priority(j, INT_MAX);
                }
            );
            bench.run_test(test);
        }

        {
            // Heap (set_priority) - pessimistic
            BenchmarkTest<Heap<int, int>> test(
                std::format("Heap (set_priority) - {} elements, pessimistic", sz),
                sz,
                [sz](size_t) {
                    Heap<int> heap;
                    for (size_t i = 0; i < sz; i++) {
                        heap.push(util::random_int(0, INT_MAX), i);
                    }
                    return heap;
                },
                [sz](auto& heap, size_t j) {
                    heap.set_priority(sz - j, INT_MAX);
                }
            );
            bench.run_test(test);
        }
        
        {
            // Heap (set_priority) - optimistic
            BenchmarkTest<Heap<int, int>> test(
                std::format("Heap (set_priority) - {} elements, optimistic", sz),
                sz,
                [sz](size_t) {
                    Heap<int> heap;
                    for (size_t i = 0; i < sz; i++) {
                        heap.push(i, 0);
                    }
                    return heap;
                },
                [](auto& heap, size_t) {
                    heap.set_priority(0, INT_MAX);
                }
            );
            bench.run_test(test);
        }
#pragma endregion
#pragma region SortedArray
        {
            // SortedArray (push) - average
            BenchmarkTest<SortedArray<int, int>> test(
                std::format("SortedArray (push) - {} elements, average", sz),
                sz,
                [sz](size_t) {
                    SortedArray<int> array;
                    for (size_t i = 0; i < sz - BATCH_SIZE / 2; i++) {
                        array.push(i, 0);
                    }
                    return array;
                },
                [sz](auto& array, size_t) {
                    array.push(sz, -1);
                }
            );
            bench.run_test(test);
        }
        
        {
            // SortedArray (push) - pessimistic
            BenchmarkTest<SortedArray<int, int>> test(
                std::format("SortedArray (push) - {} elements, pessimistic", sz),
                sz,
                [sz](size_t) {
                    SortedArray<int> array;
                    for (size_t i = 0; i < sz - BATCH_SIZE / 2; i++) {
                        array.push(i, 0);
                    }
                    return array;
                },
                [](auto& array, size_t) {
                    array.push(INT_MIN, -1);
                }
            );
            bench.run_test(test);
        }
        
        {
            // SortedArray (push) - optimistic
            BenchmarkTest<SortedArray<int, int>> test(
                std::format("SortedArray (push) - {} elements, optimistic", sz),
                sz,
                [sz](size_t) {
                    SortedArray<int> array;
                    for (size_t i = 0; i < sz - BATCH_SIZE / 2; i++) {
                        array.push(i, 0);
                    }
                    return array;
                },
                [](auto& array, size_t) {
                    array.push(INT_MAX, -1);
                }
            );
            bench.run_test(test);
        }

        {
            // SortedArray (pop)
            BenchmarkTest<SortedArray<int, int>> test(
                std::format("SortedArray (pop) - {} elements, optimistic", sz),
                sz,
                [sz](size_t) {
                    SortedArray<int> array;
                    for (size_t i = 0; i < sz + BATCH_SIZE / 2; i++) {
                        array.push(i, 0);
                    }
                    return array;
                },
                [](auto& array, size_t) {
                    array.pop();
                }
            );
            bench.run_test(test);
        }

        {
            std::vector<int> priorities;
            for (size_t i = 0; i < sz; i++) {
                priorities.push_back(util::random_int(0, INT_MAX));
            }

            // SortedArray (set_priority) - average
            BenchmarkTest<SortedArray<int, int>> test(
                std::format("SortedArray (set_priority) - {} elements, average", sz),
                sz,
                [sz](size_t) {
                    SortedArray<int> array;
                    for (size_t i = 0; i < sz; i++) {
                        array.push(util::random_int(0, INT_MAX), i);
                    }
                    return array;
                },
                [priorities](auto& array, size_t j) {
                    array.set_priority(j, priorities[j]);
                }
            );
            bench.run_test(test);
        }

        {
            std::vector<int> values(sz);
            std::iota(values.begin(), values.end(), 0);
            std::vector<int> priorities;
            for (size_t i = 0; i < sz; i++) {
                priorities.push_back(util::random_int(0, INT_MAX));
            }

            // SortedArray (set_priority) - pessimistic
            BenchmarkTest<SortedArray<int, int>> test(
                std::format("SortedArray (set_priority) - {} elements, pessimistic", sz),
                sz,
                [sz](size_t) {
                    SortedArray<int> array;
                    for (size_t i = 0; i < sz; i++) {
                        array.push(util::random_int(0, INT_MAX), sz - i);
                    }
                    return array;
                },
                [priorities, values](auto& array, size_t j) {
                    array.set_priority(values[j], priorities[j]);
                }
            );
            bench.run_test(test);
        }
        
        {
            std::vector<int> priorities;
            for (size_t i = 0; i < sz; i++) {
                priorities.push_back(util::random_int(0, INT_MAX));
            }

            // SortedArray (set_priority) - optimistic
            BenchmarkTest<SortedArray<int, int>> test(
                std::format("SortedArray (set_priority) - {} elements, optimistic", sz),
                sz,
                [sz](size_t) {
                    SortedArray<int> array;
                    for (size_t i = 0; i < sz; i++) {
                        array.push(i, 0);
                    }
                    return array;
                },
                [priorities](auto& array, size_t j) {
                    array.set_priority(array.peek(), priorities[j]);
                }
            );
            bench.run_test(test);
        }
#pragma endregion
#pragma region LinkedList
        {
            // LinkedList (push) - average
            BenchmarkTest<LinkedList<int, int>> test(
                std::format("LinkedList (push) - {} elements, average", sz),
                sz,
                [sz](size_t) {
                    LinkedList<int> list;
                    for (size_t i = 0; i < sz - BATCH_SIZE / 2; i++) {
                        list.push(i, 0);
                    }
                    return list;
                },
                [sz](auto& list, size_t) {
                    list.push(sz / 2, -1);
                }
            );
            bench.run_test(test);
        }
        
        {
            // LinkedList (push) - pessimistic
            BenchmarkTest<LinkedList<int, int>> test(
                std::format("LinkedList (push) - {} elements, pessimistic", sz),
                sz,
                [sz](size_t) {
                    LinkedList<int> list;
                    for (size_t i = 0; i < sz - BATCH_SIZE / 2; i++) {
                        list.push(i, 0);
                    }
                    return list;
                },
                [](auto& list, size_t) {
                    list.push(INT_MIN, -1);
                }
            );
            bench.run_test(test);
        }
        
        {
            // LinkedList (push) - optimistic
            BenchmarkTest<LinkedList<int, int>> test(
                std::format("LinkedList (push) - {} elements, optimistic", sz),
                sz,
                [sz](size_t) {
                    LinkedList<int> list;
                    for (size_t i = 0; i < sz - BATCH_SIZE / 2; i++) {
                        list.push(i, 0);
                    }
                    return list;
                },
                [](auto& list, size_t) {
                    list.push(INT_MAX, -1);
                }
            );
            bench.run_test(test);
        }

        {
            // LinkedList (pop)
            BenchmarkTest<LinkedList<int, int>> test(
                std::format("LinkedList (pop) - {} elements", sz),
                sz,
                [sz](size_t) {
                    LinkedList<int> list;
                    for (size_t i = 0; i < sz + BATCH_SIZE / 2; i++) {
                        list.push(i, 0);
                    }
                    return list;
                },
                [](auto& list, size_t) {
                    list.pop();
                }
            );
            bench.run_test(test);
        }

        {
            // LinkedList (set_priority) - pessimistic
            BenchmarkTest<LinkedList<int, int>> test(
                std::format("LinkedList (set_priority) - {} elements, pessimistic", sz),
                sz,
                [sz](size_t) {
                    LinkedList<int> list;
                    for (size_t i = 0; i < sz; i++) {
                        list.push(util::random_int(0, INT_MAX), i);
                    }
                    return list;
                },
                [sz](auto& list, size_t j) {
                    list.set_priority(sz - j - 1, INT_MIN);
                }
            );
            bench.run_test(test);
        }

        {
            // LinkedList (set_priority) - optimistic
            BenchmarkTest<LinkedList<int, int>> test(
                std::format("LinkedList (set_priority) - {} elements, optimistic", sz),
                sz,
                [sz](size_t) {
                    LinkedList<int> list;
                    for (size_t i = 0; i < sz; i++) {
                        list.push(i, i);
                    }
                    return list;
                },
                [](auto& list, size_t) {
                    list.set_priority(list.peek(), INT_MAX);
                }
            );
            bench.run_test(test);
        }
        
        {
            std::vector<int> priorities;
            for (size_t i = 0; i < sz; i++) {
                priorities.push_back(util::random_int(0, INT_MAX));
            }

            // LinkedList (set_priority) - average
            BenchmarkTest<LinkedList<int, int>> test(
                std::format("LinkedList (set_priority) - {} elements, average", sz),
                sz,
                [sz](size_t) {
                    LinkedList<int> list;
                    for (size_t i = 0; i < sz; i++) {
                        list.push(i, i);
                    }
                    return list;
                },
                [priorities](auto& list, size_t j) {
                    list.set_priority(j, priorities[j]);
                }
            );
            bench.run_test(test);
        }
#pragma endregion
    }

    bench.write_results("results.csv");

    return 0;
}
