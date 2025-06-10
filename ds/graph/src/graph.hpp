#pragma once

#include <algorithm>
#include <compare>
#include <concepts>
#include <functional>
#include <optional>
#include <queue>
#include <stdexcept>
#include <random>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// vertex concept
template <typename T>
concept Vertex = std::equality_comparable<T>
    && std::totally_ordered<T>
    && std::copyable<T>
    && requires (T vtx) {
        { std::hash<T>{}(vtx) } -> std::convertible_to<std::size_t>;
    };

// weight concept
template <typename T>
concept Weight = std::totally_ordered<T>
    && std::copyable<T>
    && requires (T a, T b) {
        { a + b } -> std::convertible_to<T>;
        { a - b } -> std::convertible_to<T>;
        { a * b } -> std::convertible_to<T>;
    }
    && std::default_initializable<T>
    && requires {
        T {};
    };

template <Vertex V>
struct VertexPairHash {
    std::size_t operator()(const std::pair<V, V>& p) const {
        auto hash1 = std::hash<V>{}(p.first);
        auto hash2 = std::hash<V>{}(p.second);
        return hash1 ^ (hash2 << 1); 
    }
};

template <Vertex V, Weight W>
struct Edge {
    V from;
    V to;
    W weight;

    bool operator==(const Edge<V, W>& other) const {
        return from == other.from && to == other.to && weight == other.weight;
    }

    bool operator<(const Edge<V, W>& other) const {
        return std::tie(from, to, weight) < std::tie(other.from, other.to, other.weight);
    }

    bool operator>(const Edge<V, W>& other) const {
        return std::tie(from, to, weight) > std::tie(other.from, other.to, other.weight);
    }
};

template <Vertex V, Weight W>
class Graph {
public:
    virtual ~Graph() = default;

    virtual bool add_vertex(const V& vtx) = 0;
    virtual bool remove_vertex(const V& vtx) = 0;
    virtual bool has_vertex(const V& vtx) const = 0;

    virtual size_t vertex_count() const = 0;
    virtual std::vector<V> get_vertices() const = 0;

    virtual bool add_edge(const Edge<V, W>& edge) = 0;
    virtual bool remove_edge(const V& from, const V& to) = 0;
    virtual bool has_edge(const V& from, const V& to) const = 0;
    virtual std::optional<Edge<V, W>> get_edge(const V& from, const V& to) const = 0;
    virtual std::optional<W> get_weight(const V& from, const V& to) const = 0;

    virtual std::vector<Edge<V, W>> get_edges() const = 0;
    virtual std::optional<std::vector<Edge<V, W>>> get_edges(const V& vtx) const = 0;

    // path methods
    std::optional<std::vector<Edge<V, W>>> dijkstra(const V& start, const V& end) const {
        using Node = std::pair<W, V>;

        std::unordered_map<V, std::optional<W>> distances;
        std::unordered_map<V, V> prev;
        std::priority_queue<Node, std::vector<Node>, std::greater<>> pq;

        const auto vertices = get_vertices();

        // initialize distances
        for (const auto& vtx : vertices) {
            distances[vtx] = std::nullopt;
        }
        distances[start] = std::make_optional(W{0});
        pq.push({W{0}, start});

        while (!pq.empty()) {
            auto [current_distance_val, current_vertex] = pq.top();
            pq.pop();

            // skip if we've found a better path or current_vertex is unreachable initially (should not happen with proper start)
            if (!distances[current_vertex].has_value() || current_distance_val > *distances[current_vertex]) {
                continue;
            }

            if (current_vertex == end) {
                std::vector<Edge<V, W>> path;
                while (current_vertex != start) {
                    const auto edge_opt = get_edge(prev[current_vertex], current_vertex);
                    if (!edge_opt) {
                        return std::nullopt;
                    }
                    path.push_back(edge_opt.value());
                    current_vertex = prev[current_vertex];
                }
                std::reverse(path.begin(), path.end());
                return std::make_optional(path);
            }

            const auto edges_opt = get_edges(current_vertex);
            if (!edges_opt) { // check if get_edges returned a valid optional
                continue;
            }

            for (const auto& edge : *edges_opt) {
                // new_distance calculation assumes current_distance_val is finite (from PQ)
                const auto new_distance = current_distance_val + edge.weight;
                if (!distances[edge.to].has_value() || new_distance < *distances[edge.to]) {
                    distances[edge.to] = std::make_optional(new_distance);
                    prev[edge.to] = current_vertex;
                    pq.push({new_distance, edge.to});
                }
            }
        }
        return std::nullopt;
    }

    std::optional<std::vector<Edge<V, W>>> bellman_ford(const V& start, const V& end, bool cycle_check = true) const {
        std::unordered_map<V, std::optional<W>> distances;
        std::unordered_map<V, V> prev;

        const auto vertices = get_vertices();
        const auto edges = get_edges(); // Get all edges once!

        for (const auto& vtx : vertices) {
            distances[vtx] = std::nullopt;
        }
        distances[start] = std::make_optional(W{0});

        for (auto i = 0u; i < vertices.size() - 1; ++i) {
            bool updated = false;
            for (const auto& edge : edges) {
                if (distances[edge.from].has_value()) {
                    W new_dist_val = *distances[edge.from] + edge.weight;
                    if (!distances[edge.to].has_value() || new_dist_val < *distances[edge.to]) {
                        distances[edge.to] = std::make_optional(new_dist_val);
                        prev[edge.to] = edge.from;
                        updated = true;
                    }
                }
            }
            if (!updated) {
                break;
            }
        }

        // step 3: check for negative cycles
        if (cycle_check) {
            for (const auto& edge : edges) {
                if (distances[edge.from].has_value()) {
                    W new_dist_val = *distances[edge.from] + edge.weight;
                    if (!distances[edge.to].has_value() || new_dist_val < *distances[edge.to]) {
                        return std::nullopt;
                    }
                }
            }
        }

        if (!distances[end].has_value()) {
            return std::nullopt;
        }

        // reconstruct path
        std::vector<Edge<V, W>> path;
        V current = end;
        while (current != start) {
            const auto edge_opt = get_edge(prev[current], current);
            if (!edge_opt) {
                return std::nullopt;
            }
            path.push_back(edge_opt.value());
            current = prev[current];
        }
        std::reverse(path.begin(), path.end());
        return std::make_optional(path);
    }

protected:
    void check_no_loop(const Edge<V, W>& edge) const {
        if (std::equal_to<V>{}(edge.from, edge.to)) {
            throw std::invalid_argument("Self-loops are not allowed.");
        }
    }
};
