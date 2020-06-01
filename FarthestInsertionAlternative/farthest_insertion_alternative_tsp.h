#pragma once

#include <algorithm>
#include <functional>     // std::less
#include <unordered_set>  // std::unordered_set
#include <vector>         // std::vector

#include "DistanceMatrix.h"
#include "shared_utils.h"

[[nodiscard]] inline int farthest_insertion_alt_tsp(DistanceMatrix<int>&& distance_matrix) noexcept {
    const size_t size = distance_matrix.size();

    // lambda function that returns the distance between any 2 nodes
    const auto get_distance = [&distance_matrix](const size_t x, const size_t y) {
        return distance_matrix.at(x, y);
    };

    // keep track of the nodes not in the partial Hamiltonian circuit.
    // Initially, none of them is in the circuit, so not_visited is initialized with
    // every vertex from 0 to size-1
    std::unordered_set<size_t> not_visited = utils::generate_range_set(size);

    // select the 2 farthest nodes and add them to the partial circuit
    const auto& [first_node, second_node] = distance_matrix.get_2_farthest_nodes();

    // keep track of the nodes in the partial Hamiltonian circuit
    std::vector<size_t> circuit{first_node, second_node};
    circuit.reserve(size);

    // remove the first 2 selected nodes from not_visited
    not_visited.erase(first_node);
    not_visited.erase(second_node);

    // Step 2: find a node k not in circuit that maximizes δ(k, circuit)
    const size_t k = utils::select_new_k_maximize(not_visited, circuit, get_distance);

    // Step 3: insert k in between the two consecutive tour cities i, j for which such an insertion
    // causes the minimum increase in total tour length.
    circuit.emplace_back(k);
    not_visited.erase(k);

    // Step 4: repeat the insertion from step 2 until all nodes have been inserted into the circuit
    while (!not_visited.empty()) {
        // select the not visited node k that maximizes δ(k, circuit)
        size_t new_k = utils::select_new_k_maximize(not_visited, circuit, get_distance);
        not_visited.erase(new_k);

        // find the arc (i, j) that minimizes the value of w(i, k) - w(k, j) - w(i, j)
        // and add k in between i and j in circuit
        utils::perform_best_circuit_insertion(new_k, circuit, get_distance);
    }

    // return the weights in the circuit
    return utils::sum_weights_in_circuit(circuit.cbegin(), circuit.cend(), get_distance);
}