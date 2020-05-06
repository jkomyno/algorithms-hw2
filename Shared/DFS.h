#pragma once

#include <stack>          // std::stack
#include <unordered_set>  // std::unordered_set
#include <vector>         // std::vector

#include "DistanceMatrix.h"
#include "Edge.h"

class DFS {
    // constant pointer to a non constant graph represented as an Adjacency Map
    const DistanceMatrix<double> distance_matrix;

public:
    /**
     * Create a partial Distance Matrix from a list of edges representing a Minimum Spanning Tree.
     * The unknown distances, i.e. the distances between points not adjacent in the mst, are set to
     * 0. They aren't used anyway.
     */
    DFS(std::vector<Edge>&& mst) : distance_matrix(DistanceMatrix<double>(std::move(mst))) {
    }

    // we should not deallocate adj_list_graph_ptr, as it resides on the stack, not the heap
    ~DFS() {
    }

    // perform a preorder traversal of the graph represented by a Distance Matrix.
    // The graph must not be empty.
    // TODO: (1) check if the recursive variant is faster
    // TODO: (2) check if an Adjacency List approach for storing the graph to be traversed by DFS is
    //       faster
    [[nodiscard]] std::vector<size_t> preorder_traversal() const {
        const auto n = distance_matrix.size();

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
            for (const auto& [u, _] : distance_matrix.adjacent_vertexes(s)) {
                if (!visited.count(u)) {
                    stack.push(u);
                }
            }
        }

        return preorder_vec;
    }
};
