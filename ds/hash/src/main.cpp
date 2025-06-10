#include <algorithm>
#include <format>
#include <iostream>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>

#include "schashmap.hpp"
#include "ckhashmap.hpp"
#include "lphashmap.hpp"
#include "bench.hpp"

const std::vector<size_t> element_counts = {1000, 5000, 10000, 50000, 100000, 500000, 1000000};

int main() {
    constexpr size_t WARMUP_ITERATIONS = 50;
    constexpr size_t TEST_ITERATIONS = 300;
    constexpr size_t BATCH_SIZE = 10;
    BenchmarkSuite bench(WARMUP_ITERATIONS, TEST_ITERATIONS, BATCH_SIZE);

    for (const auto sz : element_counts) {
        BenchmarkTest<ScHashMap<int, int>> insert_optimistic_test(
            std::format("ScHashMap::insert - {} elements [optimistic]", sz),
            sz,
            [sz](size_t) {
                ScHashMap<int, int> hashmap(sz * 2);
                return hashmap;
            },
            [sz](auto& hashmap, size_t iteration) {
                hashmap.insert(static_cast<int>(sz + iteration), static_cast<int>(sz + iteration));
            }
        );
        
        BenchmarkTest<ScHashMap<int, int>> insert_average_test(
            std::format("ScHashMap::insert - {} elements [average]", sz),
            sz,
            [sz](size_t) {
                ScHashMap<int, int> hashmap(sz);
                for (size_t i = 0; i < sz / 2 - BATCH_SIZE / 2; i++) {
                    hashmap.insert(static_cast<int>(i), static_cast<int>(i));
                }
                return hashmap;
            },
            [sz](auto& hashmap, size_t iteration) {
                hashmap.insert(static_cast<int>(sz + iteration), static_cast<int>(sz + iteration));
            }
        );
        
        BenchmarkTest<ScHashMap<int, int>> insert_pessimistic_test(
            std::format("ScHashMap::insert - {} elements [pessimistic]", sz),
            sz,
            [sz](size_t) {
                ScHashMap<int, int> hashmap(sz);
                for (size_t i = 0; i < static_cast<size_t>(sz * 0.7 - BATCH_SIZE / 2); i++) {
                    hashmap.insert(static_cast<int>(i), static_cast<int>(i));
                }
                return hashmap;
            },
            [sz](auto& hashmap, size_t iteration) {
                hashmap.insert(static_cast<int>(sz + iteration), static_cast<int>(sz + iteration));
            }
        );

        BenchmarkTest<ScHashMap<int, int>> remove_optimistic_test(
            std::format("ScHashMap::remove - {} elements [optimistic]", sz),
            sz,
            [sz](size_t) {
                ScHashMap<int, int> hashmap(sz * 2);
                for (size_t i = 0; i < sz - BATCH_SIZE / 2; i++) {
                    hashmap.insert(static_cast<int>(i), static_cast<int>(i));
                }
                return hashmap;
            },
            [sz](auto& hashmap, size_t iteration) {
                hashmap.remove(static_cast<int>(iteration));
            }
        );
        
        BenchmarkTest<ScHashMap<int, int>> remove_average_test(
            std::format("ScHashMap::remove - {} elements [average]", sz),
            sz,
            [sz](size_t) {
                ScHashMap<int, int> hashmap(sz);
                for (size_t i = 0; i < sz - BATCH_SIZE / 2; i++) {
                    hashmap.insert(static_cast<int>(i), static_cast<int>(i));
                }
                return hashmap;
            },
            [sz](auto& hashmap, size_t iteration) {
                hashmap.remove(static_cast<int>(iteration));
            }
        );
        
        BenchmarkTest<ScHashMap<int, int>> remove_pessimistic_test(
            std::format("ScHashMap::remove - {} elements [pessimistic]", sz),
            sz,
            [sz](size_t) {
                ScHashMap<int, int> hashmap(sz / 2);
                for (size_t i = 0; i < sz - BATCH_SIZE / 2; i++) {
                    hashmap.insert(static_cast<int>(i), static_cast<int>(i));
                }
                return hashmap;
            },
            [sz](auto& hashmap, size_t iteration) {
                if (iteration % 3 == 0) {
                    hashmap.remove(static_cast<int>(sz + iteration));
                } else {
                    hashmap.remove(static_cast<int>((sz - 1) - (iteration % sz)));
                }
            }
        );

        bench.run_test(insert_optimistic_test);
        bench.run_test(insert_average_test);
        bench.run_test(insert_pessimistic_test);
        bench.run_test(remove_optimistic_test);
        bench.run_test(remove_average_test);
        bench.run_test(remove_pessimistic_test);

        BenchmarkTest<CkHashMap<int, int>> cuckoo_insert_optimistic_test(
            std::format("CkHashMap::insert - {} elements [optimistic]", sz),
            sz,
            [sz](size_t) {
                CkHashMap<int, int> cuckoo_map(sz * 4);
                return cuckoo_map;
            },
            [sz](auto& cuckoo_map, size_t iteration) {
                cuckoo_map.insert(static_cast<int>(sz + iteration), static_cast<int>(sz + iteration));
            }
        );
        
        BenchmarkTest<CkHashMap<int, int>> cuckoo_insert_average_test(
            std::format("CkHashMap::insert - {} elements [average]", sz),
            sz,
            [sz](size_t) {
                CkHashMap<int, int> cuckoo_map(sz * 2);
                for (size_t i = 0; i < sz * 3 / 10 - BATCH_SIZE / 2; i++) {
                    cuckoo_map.insert(static_cast<int>(i), static_cast<int>(i));
                }
                return cuckoo_map;
            },
            [sz](auto& cuckoo_map, size_t iteration) {
                cuckoo_map.insert(static_cast<int>(sz + iteration), static_cast<int>(sz + iteration));
            }
        );

        BenchmarkTest<CkHashMap<int, int>> cuckoo_insert_pessimistic_test(
            std::format("CkHashMap::insert - {} elements [pessimistic]", sz),
            sz,
            [sz](size_t) {
                CkHashMap<int, int> cuckoo_map(sz);
                for (size_t i = 0; i < static_cast<size_t>(sz * 0.45 - BATCH_SIZE / 2); i++) {
                    cuckoo_map.insert(static_cast<int>(i), static_cast<int>(i));
                }
                return cuckoo_map;
            },
            [sz](auto& cuckoo_map, size_t iteration) {
                cuckoo_map.insert(static_cast<int>(sz + iteration), static_cast<int>(sz + iteration));
            }
        );

        BenchmarkTest<CkHashMap<int, int>> cuckoo_remove_optimistic_test(
            std::format("CkHashMap::remove - {} elements [optimistic]", sz),
            sz,
            [sz](size_t) {
                CkHashMap<int, int> cuckoo_map(sz * 2);
                for (size_t i = 0; i < sz - BATCH_SIZE / 2; i++) {
                    cuckoo_map.insert(static_cast<int>(i), static_cast<int>(i));
                }
                return cuckoo_map;
            },
            [sz](auto& cuckoo_map, size_t iteration) {
                cuckoo_map.remove(static_cast<int>(iteration));
            }
        );
        
        BenchmarkTest<CkHashMap<int, int>> cuckoo_remove_average_test(
            std::format("CkHashMap::remove - {} elements [average]", sz),
            sz,
            [sz](size_t) {
                CkHashMap<int, int> cuckoo_map(sz);
                for (size_t i = 0; i < sz - BATCH_SIZE / 2; i++) {
                    cuckoo_map.insert(static_cast<int>(i * 7 + 13), static_cast<int>(i));
                }
                return cuckoo_map;
            },
            [sz](auto& cuckoo_map, size_t iteration) {
                cuckoo_map.remove(static_cast<int>((iteration) * 7 + 13));
            }
        );
        
        BenchmarkTest<CkHashMap<int, int>> cuckoo_remove_pessimistic_test(
            std::format("CkHashMap::remove - {} elements [pessimistic]", sz),
            sz,
            [sz](size_t) {
                CkHashMap<int, int> cuckoo_map(sz / 2);
                for (size_t i = 0; i < sz - BATCH_SIZE / 2; i++) {
                    cuckoo_map.insert(static_cast<int>(i * 2), static_cast<int>(i));
                }
                return cuckoo_map;
            },
            [sz](auto& cuckoo_map, size_t iteration) {
                cuckoo_map.remove(static_cast<int>((iteration) * 2 + 1));
            }
        );

        bench.run_test(cuckoo_insert_optimistic_test);
        bench.run_test(cuckoo_insert_average_test);
        bench.run_test(cuckoo_insert_pessimistic_test);
        bench.run_test(cuckoo_remove_optimistic_test);
        bench.run_test(cuckoo_remove_average_test);
        bench.run_test(cuckoo_remove_pessimistic_test);

        BenchmarkTest<LpHashMap<int, int>> lp_insert_optimistic_test(
            std::format("LpHashMap::insert - {} elements [optimistic]", sz),
            sz,
            [sz](size_t) {
                LpHashMap<int, int> lp_map(sz * 4);
                return lp_map;
            },
            [sz](auto& lp_map, size_t iteration) {
                lp_map.insert(static_cast<int>(sz + iteration), static_cast<int>(sz + iteration));
            }
        );
        
        BenchmarkTest<LpHashMap<int, int>> lp_insert_average_test(
            std::format("LpHashMap::insert - {} elements [average]", sz),
            sz,
            [sz](size_t) {
                LpHashMap<int, int> lp_map(sz * 2);
                for (size_t i = 0; i < sz * 2 / 5 - BATCH_SIZE / 2; i++) {
                    lp_map.insert(static_cast<int>(i), static_cast<int>(i));
                }
                return lp_map;
            },
            [sz](auto& lp_map, size_t iteration) {
                lp_map.insert(static_cast<int>(sz + iteration), static_cast<int>(sz + iteration));
            }
        );
        
        BenchmarkTest<LpHashMap<int, int>> lp_insert_pessimistic_test(
            std::format("LpHashMap::insert - {} elements [pessimistic]", sz),
            sz,
            [sz](size_t) {
                LpHashMap<int, int> lp_map(sz);
                for (size_t i = 0; i < static_cast<size_t>(sz * 0.65 - BATCH_SIZE / 2); i++) {
                    lp_map.insert(static_cast<int>(i), static_cast<int>(i));
                }
                return lp_map;
            },
            [sz](auto& lp_map, size_t iteration) {
                lp_map.insert(static_cast<int>(sz + iteration), static_cast<int>(sz + iteration));
            }
        );

        BenchmarkTest<LpHashMap<int, int>> lp_remove_optimistic_test(
            std::format("LpHashMap::remove - {} elements [optimistic]", sz),
            sz,
            [sz](size_t) {
                LpHashMap<int, int> lp_map(sz * 3);
                for (size_t i = 0; i < sz - BATCH_SIZE / 2; i++) {
                    lp_map.insert(static_cast<int>(i * 3), static_cast<int>(i));
                }
                return lp_map;
            },
            [sz](auto& lp_map, size_t iteration) {
                lp_map.remove(static_cast<int>((iteration) * 3));
            }
        );
        
        BenchmarkTest<LpHashMap<int, int>> lp_remove_average_test(
            std::format("LpHashMap::remove - {} elements [average]", sz),
            sz,
            [sz](size_t) {
                LpHashMap<int, int> lp_map(sz);
                for (size_t i = 0; i < sz - BATCH_SIZE / 2; i++) {
                    lp_map.insert(static_cast<int>(i), static_cast<int>(i));
                }
                for (size_t i = 0; i < sz / 10; i++) {
                    lp_map.remove(static_cast<int>(i * 10));
                }
                return lp_map;
            },
            [sz](auto& lp_map, size_t iteration) {
                lp_map.remove(static_cast<int>(iteration + 1));
            }
        );
        
        BenchmarkTest<LpHashMap<int, int>> lp_remove_pessimistic_test(
            std::format("LpHashMap::remove - {} elements [pessimistic]", sz),
            sz,
            [sz](size_t) {
                LpHashMap<int, int> lp_map(sz / 2);
                for (size_t i = 0; i < sz - BATCH_SIZE / 2; i++) {
                    lp_map.insert(static_cast<int>(i), static_cast<int>(i));
                }
                for (size_t i = 0; i < sz / 4; i++) {
                    lp_map.remove(static_cast<int>(i * 4 + 1));
                }
                return lp_map;
            },
            [sz](auto& lp_map, size_t iteration) {
                if ((iteration) % 2 == 0) {
                    lp_map.remove(static_cast<int>(-(static_cast<int>(iteration + 1))));
                } else {
                    lp_map.remove(static_cast<int>((sz - 1) - (iteration) % (sz / 2)));
                }
            }
        );

        bench.run_test(lp_insert_optimistic_test);
        bench.run_test(lp_insert_average_test);
        bench.run_test(lp_insert_pessimistic_test);
        bench.run_test(lp_remove_optimistic_test);
        bench.run_test(lp_remove_average_test);
        bench.run_test(lp_remove_pessimistic_test);
    }

    bench.write_results("results.csv");

    return 0;
}
