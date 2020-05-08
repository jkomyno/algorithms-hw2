#pragma once

#include <stack>          // std::stack
#include <unordered_set>  // std::unordered_set
#include <vector>         // std::vector

#include "AdjacencyMapGraph.h"
#include "Edge.h"

class DFS {
    // constant pointer to a non constant graph represented as an Adjacency Map
    const AdjacencyMapGraph adjacency_map;

    // perform a preorder traversal of the graph represented by an Adjacency Map.
    // The graph must not be empty.
    void preorder_traversal_rec_helper(size_t v, std::unordered_set<size_t>& visited,
                                       std::vector<size_t>& circuit) const {
        visited.insert(v);
        circuit.push_back(v);

        // if an adjacent node hasn't been visited, push it to the stack
        for (const auto& [u, _] : adjacency_map.adjacent_vertexes(v)) {
            if (!visited.count(u)) {
                preorder_traversal_rec_helper(u, visited, circuit);
            }
        }
    }

public:
    /**
     * Create a partial Distance Matrix from a list of edges representing a Minimum Spanning Tree.
     * The unknown distances, i.e. the distances between points not adjacent in the mst, are set to
     * 0. They aren't used anyway.
     */
    explicit DFS(std::vector<Edge>&& mst) : adjacency_map(AdjacencyMapGraph(std::move(mst))) {
    }

    [[nodiscard]] std::vector<size_t> preorder_traversal_rec() const {
        const auto n = adjacency_map.size();

        // set that keeps track of the visited nodes
        std::unordered_set<size_t> visited;
        visited.reserve(n);

        std::vector<size_t> circuit;
        circuit.reserve(n);

        preorder_traversal_rec_helper(0, visited, circuit);

        return circuit;
    }

    // perform a preorder traversal of the graph represented by an Adjacency Map.
    // The graph must not be empty.
    // TODO: (1) check if the recursive variant is faster
    [[nodiscard]] std::vector<size_t> preorder_traversal() const {
        const auto n = adjacency_map.size();

        // set that keeps track of the visited nodes
        std::unordered_set<size_t> visited;
        visited.reserve(n);

        // create a stack for DFS
        std::stack<size_t> stack;

        // push the current source node.
        stack.push(0);

        // list of nodes traversed in a preorder fashion
        std::vector<size_t> preorder_vec;
        preorder_vec.reserve(n);

        while (!stack.empty()) {
            size_t s = stack.top();
            stack.pop();

            // Stack may contain same vertex twice. So
            // we need to print the popped item only
            // if it is not visited.
            if (!visited.count(s)) {
                preorder_vec.emplace_back(s);
                visited.insert(s);
            }

            // if an adjacent node hasn't been visited, push it to the stack
            for (const auto& [u, _] : adjacency_map.adjacent_vertexes(s)) {
                if (!visited.count(u)) {
                    stack.push(u);
                }
            }
        }

        return preorder_vec;
    }
};
