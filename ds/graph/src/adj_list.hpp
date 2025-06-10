#pragma once

#include <algorithm>
#include <limits>
#include <unordered_map>
#include <queue>

#include "graph.hpp"

template <Vertex V, Weight W>
class AdjListGraph : public Graph<V, W> {
    std::unordered_map<V, std::vector<Edge<V, W>>> adj_list;
public:
    AdjListGraph() = default;
    ~AdjListGraph() = default;

    AdjListGraph(const std::vector<Edge<V, W>>& edges) {
        for (const auto& edge : edges) {
            add_vertex(edge.from);
            add_vertex(edge.to);
            add_edge(edge);
        }
    }

    bool add_vertex(const V& vtx) override {
        if (has_vertex(vtx)) {
            return false;
        }
        adj_list[vtx] = {};
        return true;
    }

    bool remove_vertex(const V& vtx) override {
        if (!has_vertex(vtx)) {
            return false;
        }
        adj_list.erase(vtx);
        return true;
    }

    bool has_vertex(const V& vtx) const override {
        return adj_list.contains(vtx);
    }

    size_t vertex_count() const override {
        return adj_list.size();
    }

    std::vector<V> get_vertices() const override {
        std::vector<V> vertices(adj_list.size());
        std::transform(adj_list.begin(), adj_list.end(), vertices.begin(), [](const auto& pair) {
            return pair.first;
        });
        return vertices;
    }

    bool add_edge(const Edge<V, W>& edge) override {
        if (!has_vertex(edge.from) || !has_vertex(edge.to)) {
            return false;
        }
        adj_list[edge.from].push_back(edge);
        return true;
    }

    bool remove_edge(const V& from, const V& to) override {
        if (!has_edge(from, to)) {
            return false;
        }

        adj_list[from].erase(
            std::remove_if(
                adj_list[from].begin(),
                adj_list[from].end(),
                [to](const Edge<V, W>& edge) {
                    return edge.to == to;
                }
            ),
            adj_list[from].end()
        );

        return true;
    }
    bool has_edge(const V& from, const V& to) const override {
        const auto it = adj_list.find(from);
        if (it == adj_list.end()) {
            return false;
        }
        return std::any_of(it->second.begin(), it->second.end(), [to](const Edge<V, W>& edge) {
            return edge.to == to;
        });
    }

    std::optional<Edge<V, W>> get_edge(const V& from, const V& to) const override {
        const auto it = adj_list.find(from);
        if (it == adj_list.end()) {
            return std::nullopt;
        }
        const auto edge = std::find_if(it->second.begin(), it->second.end(), [to](const Edge<V, W>& edge) {
            return edge.to == to;
        });
        if (edge == it->second.end()) {
            return std::nullopt;
        }
        return *edge;
    }

    std::optional<W> get_weight(const V& from, const V& to) const override {
        const auto it = adj_list.find(from);
        if (it == adj_list.end()) {
            return std::nullopt;
        }
        for (const auto& edge : it->second) {
            if (edge.to == to) {
                return edge.weight;
            }
        }
        return std::nullopt;
    }

    std::vector<Edge<V, W>> get_edges() const override {
        std::vector<Edge<V, W>> result;
        for (const auto& [from, edge_list] : adj_list) {
            result.insert(result.end(), edge_list.begin(), edge_list.end());
        }
        return result;
    }

    std::optional<std::vector<Edge<V, W>>> get_edges(const V& vtx) const override {
        const auto it = adj_list.find(vtx);
        if (it == adj_list.end()) {
            return std::nullopt; // no neighbors
        }
        return std::make_optional(it->second);
    }
};