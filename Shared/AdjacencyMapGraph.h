#pragma once

#include <unordered_map>
#include <vector>  // std::vector

#include "Edge.h"

class AdjacencyMapGraph {
    using vertex_weight_map_t = std::unordered_map<size_t, double>;
    using adj_map_t = std::unordered_map<size_t, vertex_weight_map_t>;
    adj_map_t adj_map;

    void add_edge(const Edge& edge) noexcept;

    /**
     * Initializes the graph starting from a vector of edges. The given number of vertexes is used
     * to efficiently preallocate memory.
     */
    void init(const std::vector<Edge>& edge_list, size_t n_vertex) noexcept;

public:
    AdjacencyMapGraph(std::vector<Edge>&& edge_list, const size_t n_vertex = 0) noexcept {
        init(edge_list, n_vertex);
    }

    /**
     * Return the number of nodes stored.
     * Time:  O(1)
     * Space: O(1)
     */
    [[nodiscard]] size_t size() const noexcept;

    /**
     * Return the set of vertexes adjacent to the given vertex.
     * Time:  O(1)
     * Space: O(1)
     */
    [[nodiscard]] const vertex_weight_map_t& adjacent_vertexes(const size_t vertex) const noexcept;
};

inline void AdjacencyMapGraph::add_edge(const Edge& edge) noexcept {
    const auto& [from, to, weight] = edge;

    vertex_weight_map_t& adj_map_from = adj_map[from];
    vertex_weight_map_t& adj_map_to = adj_map[to];

    adj_map_from[to] = weight;
    adj_map_to[from] = weight;
}

inline void AdjacencyMapGraph::init(const std::vector<Edge>& edge_list, size_t n_vertex) noexcept {
    // preallocate memory
    adj_map.reserve(n_vertex);

    for (size_t v = 0; v < n_vertex; ++v) {
        adj_map[v];
    }

    for (const auto& edge : edge_list) {
        add_edge(edge);
    }
}

[[nodiscard]] inline size_t AdjacencyMapGraph::size() const noexcept {
    return adj_map.size();
}

[[nodiscard]] inline const std::unordered_map<size_t, double>& AdjacencyMapGraph::adjacent_vertexes(
    const size_t vertex) const noexcept {
    return adj_map.at(vertex);
}
