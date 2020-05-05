#pragma once

#include <vector>

#include "sum_weights.h"
#include "DistanceMatrix.h"
#include "prim_binary_heap_mst.h"

[[nodiscard]] inline double approx_tsp(DistanceMatrix<double>&& distance_matrix) noexcept {
    const auto& mst(mst::prim_binary_heap_mst(std::move(distance_matrix)));

    // TODO: if we wanted to return the complete path, we would have to perform a preorder visit
    // of mst. Since we just want the weights, that is not necessary.

    return sum_weights(mst.cbegin(), mst.cend());
}
