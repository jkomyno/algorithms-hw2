#pragma once

#include <vector>

#include "DistanceMatrix.h"
#include "SimulatedAnnealing.h"
#include "SimulatedAnnealingOptions.h"
#include "TSPSolutionPool.h"
#include "nearest_neighbor_heuristic_tsp.h"
#include "random_generator.h"
#include "shared_utils.h"

[[nodiscard]] int simulated_annealing_tsp(DistanceMatrix<int>&& distance_matrix) {
    const size_t size = distance_matrix.size();
    random_generator::IntegerRandomGenerator rand_int(0, size);

    const auto get_distance = [&](const size_t x, const size_t y) {
        return distance_matrix.at(x, y);
    };

    auto initial_solution_factory = [&]() -> std::vector<size_t> {
        size_t trials = 10;

        int best_cost = std::numeric_limits<int>::max();
        std::vector<size_t> best_init;
        best_init.reserve(distance_matrix.size());

        while (trials-- > 0) {
            auto result = nearest_neighbor_heuristic_tsp(distance_matrix, rand_int);
            auto result_cost =
                utils::sum_weights_in_circuit(result.cbegin(), result.cend(), get_distance);

            if (result_cost < best_cost) {
                best_cost = result_cost;
                best_init = result;
            }
        }

        return best_init;
    };

    TSPSolutionPool pool(
        std::forward<decltype(distance_matrix)>(distance_matrix),
        std::forward<decltype(initial_solution_factory)>(initial_solution_factory));

    simulated_annealing::SimulatedAnnealingOptions options{};

    simulated_annealing::SimulatedAnnealing<TSPSolution> sa_optimizer(
        std::forward<decltype(options)>(options));

    const auto initial_solution = pool.init();
    const auto best_solution = sa_optimizer.solve(initial_solution);

    std::vector<size_t>& circuit = best_solution.circuit();

    return utils::sum_weights_in_circuit(circuit.cbegin(), circuit.cend(), get_distance);
}
