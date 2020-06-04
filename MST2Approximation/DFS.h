#pragma once

#include <unordered_set>  // std::unordered_set
#include <vector>         // std::vector

#include "AdjacencyMapGraph.h"
#include "Edge.h"

class DFS {
    // graph represented as an Adjacency Map
    const AdjacencyMapGraph adjacency_map;

    // recursive preorder traversal implementation
    void preorder_traversal_rec(size_t v, std::unordered_set<size_t>& visited,
                                std::vector<size_t>& preorder) const {
        visited.insert(v);
        preorder.push_back(v);

        for (const auto& [u, _] : adjacency_map.adjacent_vertexes(v)) {
            // if an adjacent node hasn't been visited, visit it recursively
            if (!visited.count(u)) {
                preorder_traversal_rec(u, visited, preorder);
            }
        }
    }

public:
    // create an Adjacency Map from a list of edges representing a Minimum Spanning Tree.
    explicit DFS(std::vector<Edge>&& mst) : adjacency_map(AdjacencyMapGraph(std::move(mst))) {
    }

    // perform a preorder traversal of the graph represented by an Adjacency Map.
    // The graph must not be empty.
    [[nodiscard]] std::vector<size_t> preorder_traversal() const {
        const auto n = adjacency_map.size();

        // set that keeps track of the visited nodes
        std::unordered_set<size_t> visited;
        visited.reserve(n);

        std::vector<size_t> preorder;
        preorder.reserve(n);

        preorder_traversal_rec(0, visited, preorder);

        return preorder;
    }
};
