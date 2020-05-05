#pragma once

#include <vector>

#include "sum_weights.h"
#include "DistanceMatrix.h"
#include "prim_binary_heap_mst.h"

[[nodiscard]] inline double approx_tsp(DistanceMatrix<double>&& distance_matrix) noexcept {
    auto mst(mst::prim_binary_heap_mst(distance_matrix));

    // TODO: if we wanted to return the complete path, we would have to perform a preorder visit
    // of mst. Since we just want the weights, that is not necessary. We just need to close the cycle,
    // adding the arc from the last element of mst to the first element of mst.

    const size_t n = mst.size();
    constexpr size_t first = 0;
    const size_t last = n - 1;

    const double weight_first_last = distance_matrix.at(first, last);
    mst.emplace_back(first, last, weight_first_last);

    return sum_weights(mst.cbegin(), mst.cend());
}
