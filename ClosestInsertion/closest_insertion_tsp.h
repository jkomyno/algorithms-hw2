#pragma once

#include <iostream>
#include <iterator>
#include <unordered_set>
#include <vector>
#include <functional>  // std::greater

#include "DistanceMatrix.h"
#include "shared_utils.h"

[[nodiscard]] inline int closest_insertion_tsp(DistanceMatrix<int>&& distance_matrix) noexcept {
    const size_t size = distance_matrix.size();
    const auto get_distance = [&distance_matrix](const size_t x, const size_t y) {
        return distance_matrix.at(x, y);
    };
    const auto comparator = std::greater<double>();

    // keep track of the nodes not in the partial Hamiltonian circuit.
    // Initially, none of them is in the circuit, so not_visited is initialized with
    // every vertex from 0 to size-1
    std::unordered_set<size_t> not_visited = utils::generate_range_set(size);

    // select the 2 farthest nodes and add them to the partial circuit
    const auto& [u, v] = distance_matrix.get_2_farthest_nodes();

    // keep track of the nodes in the partial Hamiltonian circuit
    std::vector<size_t> circuit{u, v};
    circuit.reserve(size);

    // remove the farthest nodes from not_visited
    not_visited.erase(u);
    not_visited.erase(v);

    // select the 3rd vertex of the cycle
    const size_t k = utils::select_new_k(not_visited, circuit, get_distance, comparator);

    // we insert k between i and j
    circuit.emplace_back(k);
    not_visited.erase(k);

    // perform farthest insertion until all vertexes are in the circuit
    while (!not_visited.empty()) {
        // select the new k vertex that maximizes δ(k, circuit)
        size_t new_k = utils::select_new_k(not_visited, circuit, get_distance, comparator);
        not_visited.erase(new_k);

        // find the arc (i, j) that minimizes the value of w(i, k) - w(k, j) - w(i, j)
        // and add k between i and j in circuit
        utils::perform_best_circuit_insertion(new_k, circuit, get_distance);
    }

    // return the weights in the circuit
    return utils::sum_weights_in_circuit(circuit.cbegin(), circuit.cend(), get_distance);
}
