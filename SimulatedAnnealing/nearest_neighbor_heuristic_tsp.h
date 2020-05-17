#pragma once

#include <algorithm>
#include <unordered_set>
#include <vector>

#include "DistanceMatrix.h"
#include "random_generator.h"
#include "shared_utils.h"

[[nodiscard]] std::vector<size_t> nearest_neighbor_heuristic_tsp(
    DistanceMatrix<int>& distance_matrix, random_generator::IntegerRandomGenerator& rand) {
    const size_t size = distance_matrix.size();

    size_t curr_node = rand();

    std::vector<size_t> circuit{curr_node};
    circuit.reserve(size);

    // keep track of the nodes not in the partial Hamiltonian circuit.
    // Initially, none of them is in the circuit, so not_visited is initialized with
    // every vertex from 0 to size-1
    std::unordered_set<size_t> not_visited = utils::generate_range_set(size);
    not_visited.erase(curr_node);

    size_t next_node = distance_matrix.get_closest_node(curr_node);
    circuit.emplace_back(next_node);
    not_visited.erase(next_node);
    curr_node = next_node;

    while (!not_visited.empty()) {
        auto it_next_node = std::min_element(
            not_visited.cbegin(), not_visited.cend(), [&](const auto& x, const auto& y) {
                return distance_matrix.at(curr_node, x) < distance_matrix.at(curr_node, y);
            });
        circuit.emplace_back(*it_next_node);
        not_visited.erase(it_next_node);
        curr_node = next_node;
    }

    return circuit;
}
