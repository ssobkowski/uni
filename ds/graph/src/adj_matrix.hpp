#pragma once

#include "graph.hpp"

#include <unordered_map>

template <Vertex V, Weight W>
class AdjMatrixGraph : public Graph<V, W> {
    std::unordered_map<V, std::unordered_map<V, W>> adj_matrix;
public:
    AdjMatrixGraph() = default;
    ~AdjMatrixGraph() = default;

    AdjMatrixGraph(const std::vector<Edge<V, W>>& edges) {
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
        adj_matrix.insert({vtx, {}});
        return true;
    }

    bool remove_vertex(const V& vtx) override {
        if (!has_vertex(vtx)) {
            return false;
        }
        adj_matrix.erase(vtx);
        return true;
    }

    bool has_vertex(const V& vtx) const override {
        return adj_matrix.contains(vtx);
    }

    size_t vertex_count() const override {
        return adj_matrix.size();
    }

    std::vector<V> get_vertices() const override {
        std::vector<V> vertices(adj_matrix.size());
        std::transform(adj_matrix.begin(), adj_matrix.end(), vertices.begin(), [](const auto& pair) {
            return pair.first;
        });
        return vertices;
    }

    bool add_edge(const Edge<V, W>& edge) override {
        if (!has_vertex(edge.from) || !has_vertex(edge.to)) {
            return false;
        }
        adj_matrix[edge.from][edge.to] = edge.weight;
        return true;
    }
    
    bool remove_edge(const V& from, const V& to) override {
        if (!has_edge(from, to)) {
            return false;
        }
        adj_matrix[from].erase(to);
        return true;
    }
    
    bool has_edge(const V& from, const V& to) const override {
        const auto row_it = adj_matrix.find(from);
        return row_it != adj_matrix.end() && row_it->second.contains(to);
    }

    std::optional<Edge<V, W>> get_edge(const V& from, const V& to) const override {
        const auto row_it = adj_matrix.find(from);
        if (row_it == adj_matrix.end()) {
            return std::nullopt;
        }
        const auto weight_it = row_it->second.find(to);
        if (weight_it == row_it->second.end()) {
            return std::nullopt;
        }
        return Edge<V, W>{from, to, weight_it->second};
    }

    std::optional<W> get_weight(const V& from, const V& to) const override {
        const auto row_it = adj_matrix.find(from);
        if (row_it == adj_matrix.end()) {
            return std::nullopt;
        }
        const auto weight_it = row_it->second.find(to);
        if (weight_it == row_it->second.end()) {
            return std::nullopt;
        }
        return weight_it->second;
    }

    std::vector<Edge<V, W>> get_edges() const override {
        std::vector<Edge<V, W>> edges;
        for (const auto& [from, row] : adj_matrix) {
            for (const auto& [to, weight] : row) {
                edges.push_back({from, to, weight});
            }
        }
        return edges;
    }

    std::optional<std::vector<Edge<V, W>>> get_edges(const V& vtx) const override {
        const auto row_it = adj_matrix.find(vtx);
        if (row_it == adj_matrix.end()) {
            return std::nullopt;
        }
        std::vector<Edge<V, W>> neighbors;
        for (const auto& [to, weight] : row_it->second) {
            neighbors.push_back({vtx, to, weight});
        }
        return neighbors;
    }
};
