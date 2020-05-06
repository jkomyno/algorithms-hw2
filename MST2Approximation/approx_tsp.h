#pragma once

#include <vector>

#include "DFS.h"
#include "DistanceMatrix.h"
#include "prim_binary_heap_mst.h"
#include "sum_weights.h"

[[nodiscard]] inline double approx_tsp(DistanceMatrix<double>&& distance_matrix) noexcept {
    // compute the exact Minimum Spanning Tree of the input graph using Prim's algorithm
    auto mst(mst::prim_binary_heap_mst(distance_matrix));

    // we compute the weight of the mst
    const double mst_weight = sum_weights(mst.cbegin(), mst.cend());

    // use DFS to perform a preorder traversal of the MST
    DFS dfs(std::move(mst));
    const auto preorder_lst = dfs.preorder_traversal();

    // save the first node of the preorder traversal
    const size_t first_preorder = preorder_lst.front();

    // save last node of the preorder traversal, which is a leaf
    const size_t last_preorder = preorder_lst.back();

    // we use the original distance matrix to compute the distance between the first and the last
    // elements of the preorder traversal
    const double weight_first_last = distance_matrix.at(first_preorder, last_preorder);

    // return the 2-approximated TSP weight
    return mst_weight + weight_first_last;
}
