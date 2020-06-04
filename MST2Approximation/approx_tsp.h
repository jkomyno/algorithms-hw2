#pragma once

#include <vector> // std::vector

#include "DFS.h"
#include "DistanceMatrix.h"
#include "prim_binary_heap_mst.h"
#include "shared_utils.h"
#include "random_generator.h"

[[nodiscard]] inline int approx_tsp(DistanceMatrix<int>&& distance_matrix) noexcept {
    // compute the exact Minimum Spanning Tree of the input graph using Prim's algorithm
    std::vector<Edge> mst(mst::prim_binary_heap_mst(distance_matrix));

    // use DFS to perform a preorder traversal of the MST
    DFS dfs(std::move(mst));
    const auto circuit = dfs.preorder_traversal();

    const auto get_distance = [&distance_matrix](const size_t x, const size_t y) {
        return distance_matrix.at(x, y);
    };

    // return the weights in the circuit
    return utils::sum_weights_in_circuit(circuit.cbegin(), circuit.cend(), get_distance);
}
