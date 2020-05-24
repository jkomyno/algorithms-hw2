#pragma once

#include <algorithm>  // std::max
#include <limits>     // std::numeric_limits
#include <vector>     // std::vector

#include "DistanceMatrix.h"
#include "SimulatedAnnealing.h"
#include "SimulatedAnnealingOptions.h"
#include "TSPSolutionPool.h"
#include "nearest_neighbor_heuristic_tsp.h"
#include "random_generator.h"
#include "shared_utils.h"
#include "parallel.h"

[[nodiscard]] int simulated_annealing_tsp(DistanceMatrix<int>&& distance_matrix) {
    const size_t size = distance_matrix.size();
    random_generator::IntegerRandomGenerator rand_int(0, size - 1);

    const auto get_distance = [&](const size_t x, const size_t y) {
        return distance_matrix.at(x, y);
    };

    // generate the first feasible solution using the Nearest Neighbor heuristic
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

    const size_t sample_pair_size = (size / 20) + 1;
    const size_t sample_temperature_iterations = 5;

    // solves the TSP problem using Simulated Annealing starting from the solution returned by
    // initial_solution_factory
    auto solve_tsp = [&]() {
        TSPSolutionPool pool(
            std::forward<decltype(distance_matrix)>(distance_matrix),
            std::forward<decltype(initial_solution_factory)>(initial_solution_factory));

        simulated_annealing::SimulatedAnnealingOptions options{};

        // the initial feasible solution is returned. Some core annealing options are updated in the
        // process, such as the initial temperature and the annealing pressure
        const auto initial_solution =
            pool.init(options, sample_pair_size, sample_temperature_iterations);

        simulated_annealing::SimulatedAnnealing<TSPSolution> sa_optimizer(
            std::forward<decltype(options)>(options));

        return sa_optimizer.solve(initial_solution);
    };

    parallel::parallel_executor<decltype(solve_tsp)> executor(std::move(solve_tsp));
    std::vector<int> results = executor.get_results();

    auto min_it = std::min_element(results.cbegin(), results.cend());

    return *min_it;
}
