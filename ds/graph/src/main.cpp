#include <chrono>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "graph.hpp"
#include "bench.hpp"

#include "adj_list.hpp"
#include "adj_matrix.hpp"
#include "edge_list.hpp"

#if defined(_MSC_VER)
#include <intrin.h>
#endif

template <typename T>
void black_box(T&& value) {
#if defined(__clang__) || defined(__GNUC__)
    asm volatile("" : : "r,m"(value) : "memory");
#elif defined(_MSC_VER)
    _ReadWriteBarrier();
#endif
}

template <typename T, typename U>
struct PairHash {
    std::size_t operator()(const std::pair<T, U>& p) const {
        auto hash1 = std::hash<T>{}(p.first);
        auto hash2 = std::hash<U>{}(p.second);
        return hash1 ^ (hash2 << 1); 
    }
};

template <Vertex V, Weight W>
void print_graphviz(const std::vector<Edge<V, W>>& edges) {
    std::cout << "digraph G {" << '\n';
    std::cout << "  rankdir=LR;" << '\n';
    std::cout << "  node [shape=circle];" << '\n';
    for (const auto& edge : edges) {
        std::cout << "  " << edge.from << " -> " << edge.to << " [label=" << edge.weight << "];" << '\n';
    }
    std::cout << "}" << '\n';
}

std::vector<Edge<int, int>> gen_random_directed_graph(size_t n, double density, int seed = 280131) {
    if (n == 0) {
        return {};
    }

    const auto total_possible = n > 1 ? n * (n - 1) : 0;
    auto edge_count = static_cast<size_t>(total_possible * density);

    // ensure the graph is weakly connected
    if (n > 1) {
        const auto min_edges_for_connectivity = n - 1;
        if (edge_count < min_edges_for_connectivity) {
            edge_count = min_edges_for_connectivity;
        }
    }
    if (edge_count > total_possible) {
        edge_count = total_possible;
    }

    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> weight_dist(1, 100);
    
    std::vector<Edge<int, int>> edges;
    edges.reserve(edge_count);

    std::vector<bool> used_edges_matrix(n * n, false);
    auto mark_used = [&](int u, int v) { used_edges_matrix[u * n + v] = true; };
    auto is_used = [&](int u, int v) { return used_edges_matrix[u * n + v]; };

    // ensure weak connectivity by creating a random spanning tree
    if (n > 1) {
        std::vector<int> vertices(n);
        std::iota(vertices.begin(), vertices.end(), 0);
        std::shuffle(vertices.begin(), vertices.end(), gen);
        
        // add edges for a random tree structure
        for (size_t i = 1; i < n; ++i) {
            // connect vertex i to a random vertex in 0..i-1
            std::uniform_int_distribution<size_t> parent_dist(0, i - 1);
            const size_t parent_idx = parent_dist(gen);
            const int u = vertices[parent_idx];
            const int v = vertices[i];
            
            // randomly choose direction
            const int from = (gen() % 2 == 0) ? u : v;
            const int to = (from == u) ? v : u;
            
            if (!is_used(from, to)) {
                const int weight = weight_dist(gen);
                edges.push_back({from, to, weight});
                mark_used(from, to);
            }
        }
    }
    
    // add remaining edges using efficient rejection sampling
    std::uniform_int_distribution<int> vertex_dist(0, static_cast<int>(n) - 1);
    
    while (edges.size() < edge_count) {
        const int from = vertex_dist(gen);
        const int to = vertex_dist(gen);
        
        if (from != to && !is_used(from, to)) {
            const int weight = weight_dist(gen);
            edges.push_back({from, to, weight});
            mark_used(from, to);
        }
    }
    
    return edges;
}

int main() {
    const std::vector<size_t> sizes{ 50, 100, 200, 500 };
    const std::vector<double> densities{ 0.1, 0.25, 0.5, 0.7, 0.9, 1.0 };

    std::unordered_map<std::pair<size_t, double>, std::vector<Edge<int, int>>, PairHash<size_t, double>> precomputed_graphs;
    for (const auto& size : sizes) {
        for (const auto& density : densities) {
            const auto edges = gen_random_directed_graph(size, density);
            precomputed_graphs[{size, density}] = edges;
        }
    }

    constexpr size_t WARMUP_ITERATIONS = 1;
    constexpr size_t TEST_ITERATIONS = 30;
    constexpr size_t BATCH_SIZE = 10;
    BenchmarkSuite bench(WARMUP_ITERATIONS, TEST_ITERATIONS, BATCH_SIZE);

    for (const auto& [params, edges] : precomputed_graphs) {
        const auto& [size, density] = params;
        const auto real_size = size * (size - 1);
        
        const auto& edge_list_graph = EdgeListGraph<int, int>(edges);
        const auto& adj_list_graph = AdjListGraph<int, int>(edges);
        const auto& adj_matrix_graph = AdjMatrixGraph<int, int>(edges);

        std::vector<int> random_vertices;
        for (const auto& edge : edges) {
            random_vertices.push_back(edge.from);
            random_vertices.push_back(edge.to);
        }
        std::sort(random_vertices.begin(), random_vertices.end());
        random_vertices.erase(std::unique(random_vertices.begin(), random_vertices.end()), random_vertices.end());
        std::mt19937 gen(280131);
        std::shuffle(random_vertices.begin(), random_vertices.end(), gen);

        // Dijkstra benchmarks for all graph types
        BenchmarkTest<EdgeListGraph<int, int>> edge_list_dijkstra_bench(
            std::format("Dijkstra EdgeList - {} edges [density: {}]", real_size, density),
            real_size,
            [edges](size_t) {
                return EdgeListGraph<int, int>(edges);
            },
            [random_vertices](auto& graph, size_t iteration) {
                const auto start = random_vertices[iteration % random_vertices.size()];
                const auto end = random_vertices[(iteration + 1) % random_vertices.size()];
                const auto path = graph.dijkstra(start, end);
                black_box(path);
            }
        );
        bench.run_test(edge_list_dijkstra_bench);

        BenchmarkTest<AdjListGraph<int, int>> adj_list_dijkstra_bench(
            std::format("Dijkstra AdjList - {} edges [density: {}]", real_size, density),
            real_size,
            [edges](size_t) {
                return AdjListGraph<int, int>(edges);
            },
            [random_vertices](auto& graph, size_t iteration) {
                const auto start = random_vertices[iteration % random_vertices.size()];
                const auto end = random_vertices[(iteration + 1) % random_vertices.size()];
                const auto path = graph.dijkstra(start, end);
                black_box(path);
            }
        );
        bench.run_test(adj_list_dijkstra_bench);

        BenchmarkTest<AdjMatrixGraph<int, int>> adj_matrix_dijkstra_bench(
            std::format("Dijkstra AdjMatrix - {} edges [density: {}]", real_size, density),
            real_size,
            [edges](size_t) {
                return AdjMatrixGraph<int, int>(edges);
            },
            [random_vertices](auto& graph, size_t iteration) {
                const auto start = random_vertices[iteration % random_vertices.size()];
                const auto end = random_vertices[(iteration + 1) % random_vertices.size()];
                const auto path = graph.dijkstra(start, end);
                black_box(path);
            }
        );
        bench.run_test(adj_matrix_dijkstra_bench);

        // Bellman-Ford benchmarks for all graph types
        BenchmarkTest<EdgeListGraph<int, int>> edge_list_bellman_bench(
            std::format("Bellman-Ford EdgeList - {} edges [density: {}]", real_size, density),
            real_size,
            [edges](size_t) {
                return EdgeListGraph<int, int>(edges);
            },
            [random_vertices](auto& graph, size_t iteration) {
                const auto start = random_vertices[iteration % random_vertices.size()];
                const auto end = random_vertices[(iteration + 1) % random_vertices.size()];
                const auto path = graph.bellman_ford(start, end, false);
                black_box(path);
            }
        );
        bench.run_test(edge_list_bellman_bench);

        BenchmarkTest<AdjListGraph<int, int>> adj_list_bellman_bench(
            std::format("Bellman-Ford AdjList - {} edges [density: {}]", real_size, density),
            real_size,
            [edges](size_t) {
                return AdjListGraph<int, int>(edges);
            },
            [random_vertices](auto& graph, size_t iteration) {
                const auto start = random_vertices[iteration % random_vertices.size()];
                const auto end = random_vertices[(iteration + 1) % random_vertices.size()];
                const auto path = graph.bellman_ford(start, end, false);
                black_box(path);
            }
        );
        bench.run_test(adj_list_bellman_bench);

        BenchmarkTest<AdjMatrixGraph<int, int>> adj_matrix_bellman_bench(
            std::format("Bellman-Ford AdjMatrix - {} edges [density: {}]", real_size, density),
            real_size,
            [edges](size_t) {
                return AdjMatrixGraph<int, int>(edges);
            },
            [random_vertices](auto& graph, size_t iteration) {
                const auto start = random_vertices[iteration % random_vertices.size()];
                const auto end = random_vertices[(iteration + 1) % random_vertices.size()];
                const auto path = graph.bellman_ford(start, end, false);
                black_box(path);
            }
        );
        bench.run_test(adj_matrix_bellman_bench);
    }

    bench.write_results("benchmark_results.csv");
    
    // try {
    //     const auto edges = gen_random_directed_graph(500, 0.5);

    //     std::vector<int> vertices;
    //     for (const auto& edge : edges) {
    //         vertices.push_back(edge.from);
    //         vertices.push_back(edge.to); 
    //     }
    //     std::sort(vertices.begin(), vertices.end());
    //     vertices.erase(std::unique(vertices.begin(), vertices.end()), vertices.end());
    //     EdgeListGraph<int, int> graph(edges);

    //     std::random_device rd;
    //     std::mt19937 gen(rd());
    //     std::uniform_int_distribution<> distr(0, vertices.size() - 1);

    //     const int num_trials = 30;
    //     long long dijkstra_total = 0, bellman_total = 0;
    //     int dijkstra_found = 0, bellman_found = 0;
        
    //     for (int trial = 0; trial < num_trials; ++trial) {
    //         const auto start = vertices[distr(gen)];
    //         const auto end = vertices[distr(gen)];
    //         if (start == end) {
    //             --trial;
    //             continue;
    //         }
    //         std::cout << "Trial " << trial << ": " << start << " -> " << end << '\n';
            
    //         auto start_time = std::chrono::high_resolution_clock::now();
    //         const auto dijkstra_path = graph.dijkstra(start, end);
    //         auto end_time = std::chrono::high_resolution_clock::now();
    //         dijkstra_total += std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    //         if (dijkstra_path.has_value()) {
    //             dijkstra_found++;
    //             std::cout << "Dijkstra path: " << dijkstra_path.value().size() << " edges" << '\n';
    //             for (const auto& edge : dijkstra_path.value()) {
    //                 std::cout << edge.from << " -> " << edge.to << " (" << edge.weight << ")" << '\n';
    //             }
    //         }
            
    //         start_time = std::chrono::high_resolution_clock::now();
    //         const auto bellman_path = graph.bellman_ford(start, end, false);
    //         end_time = std::chrono::high_resolution_clock::now();
    //         bellman_total += std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    //         if (bellman_path.has_value()) {
    //             bellman_found++;
    //             std::cout << "Bellman-Ford path: " << bellman_path.value().size() << " edges" << '\n';
    //             for (const auto& edge : bellman_path.value()) {
    //                 std::cout << edge.from << " -> " << edge.to << " (" << edge.weight << ")" << '\n';
    //             }
    //         }
    //     }
        
    //     std::cout << "Dijkstra avg: " << dijkstra_total / num_trials << "us, paths found: " << dijkstra_found << "/" << num_trials << '\n';
    //     std::cout << "Bellman-Ford avg: " << bellman_total / num_trials << "us, paths found: " << bellman_found << "/" << num_trials << '\n';
        
    // } catch (const std::exception& e) {
    //     std::cerr << "Error: " << e.what() << '\n';
    // }

    return 0;
}
