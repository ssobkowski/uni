#pragma once

#include <unordered_set>

#include "graph.hpp"

template <Vertex V, Weight W>
class EdgeListGraph : public Graph<V, W> {
    std::vector<Edge<V, W>> edges;

    // map to keep track of the vertex use count
    std::unordered_map<V, std::size_t> vertices;
public:
    EdgeListGraph() = default;
    ~EdgeListGraph() = default;

    EdgeListGraph(const std::vector<Edge<V, W>>& edges)
        : edges(edges)
    {
        for (const auto& edge : edges) {
            vertices[edge.from]++;
            vertices[edge.to]++;
        }
    }

    bool add_vertex(const V&) override {
        throw std::runtime_error("EdgeListGraph does not support adding individual vertices");
    }

    bool remove_vertex(const V&) override {
        throw std::runtime_error("EdgeListGraph does not support removing individual vertices");
    }

    bool has_vertex(const V& vtx) const override {
        return vertices.contains(vtx);
    }

    size_t vertex_count() const override {
        return vertices.size();
    }

    std::vector<V> get_vertices() const override {
        std::vector<V> result;
        result.reserve(vertices.size());
        std::transform(vertices.begin(), vertices.end(), std::back_inserter(result), [](const auto& pair) {
            return pair.first;
        });
        return result;
    }

    bool add_edge(const Edge<V, W>& edge) override {
        // check if the edge is already in the graph
        if (has_edge(edge.from, edge.to)) {
            return false;
        }
        edges.push_back(edge);
        vertices[edge.from]++;
        vertices[edge.to]++;
        return true;
    }

    bool remove_edge(const V& from, const V& to) override {
        if (!has_edge(from, to)) {
            return false;
        }
        
        edges.erase(std::remove_if(edges.begin(), edges.end(), [from, to](const auto& edge) {
            return edge.from == from && edge.to == to;
        }), edges.end());
        if (--vertices[from] == 0) {
            vertices.erase(from);
        }
        if (--vertices[to] == 0) {
            vertices.erase(to);
        }
        return true;
    }

    bool has_edge(const V& from, const V& to) const override {
        return std::any_of(edges.begin(), edges.end(), [from, to](const auto& edge) {
            return edge.from == from && edge.to == to;
        });
    }

    std::optional<Edge<V, W>> get_edge(const V& from, const V& to) const override {
        const auto it = std::find_if(edges.begin(), edges.end(), [from, to](const Edge<V, W>& edge) {
            if (edge.from == from && edge.to == to) {
                return true;
            }
            return false;
        });
        if (it == edges.end()) {
            return std::nullopt;
        }
        return *it;
    }

    std::optional<W> get_weight(const V& from, const V& to) const override {
        const auto edge = get_edge(from, to);
        if (!edge) {
            return std::nullopt;
        }
        return edge->weight;
    }
    
    std::vector<Edge<V, W>> get_edges() const override {
        return edges;
    }

    std::optional<std::vector<Edge<V, W>>> get_edges(const V& vtx) const override {
        std::vector<Edge<V, W>> vertex_edges;
        std::copy_if(edges.begin(), edges.end(), std::back_inserter(vertex_edges), [vtx](const auto& edge) {
            return edge.from == vtx;
        });
        return std::make_optional(vertex_edges);
    }
};

// utility type trait to check if a type is an EdgeListGraph
template<typename T>
struct is_edge_list_graph : std::false_type {};

template<typename V, typename W>
struct is_edge_list_graph<EdgeListGraph<V, W>> : std::true_type {};

template<typename T>
inline constexpr bool is_edge_list_graph_v = is_edge_list_graph<T>::value;

