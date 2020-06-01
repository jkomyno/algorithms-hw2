#pragma once

#include <algorithm>  // std::max
#include <vector>     // std::vector

#include "DistanceMatrix.h"
#include "SimulatedAnnealing.h"
#include "SimulatedAnnealingOptions.h"
#include "TSPSolutionPool.h"
#include "nearest_neighbor_heuristic_tsp.h"
#include "parallel_executor.h"
#include "random_generator.h"
#include "sequential_executor.h"
#include "shared_utils.h"

[[nodiscard]] int simulated_annealing_tsp(DistanceMatrix<int>&& distance_matrix) {
    const size_t size = distance_matrix.size();
    random_generator::IntegerRandomGenerator rand_int(0, size - 1);

    const auto get_distance = [&](const size_t x, const size_t y) {
        return distance_matrix.at(x, y);
    };

    // generate the first feasible solution using the Nearest Neighbor heuristic.
    // The heuristic is sequentially executed 10 times, but only the best solution and its cost are
    // returned.
    auto initial_solution_factory = [&]() -> std::pair<std::vector<size_t>, int> {
        const size_t trials = 10;

        const auto executor(executor::sequential_executor({trials}, [&]() {
            const auto result = nearest_neighbor_heuristic_tsp(distance_matrix, rand_int);
            const auto result_cost =
                utils::sum_weights_in_circuit(result.cbegin(), result.cend(), get_distance);

            return std::make_pair(result, result_cost);
        }));

        return executor.get_best_result(utils::select_best_result_cost_pair);
    };

    // solves the TSP problem using Simulated Annealing starting from the solution returned by
    // initial_solution_factory
    auto solve_tsp = [&]() {
        // maximum number of feasible solutions considered after every Simulated Annealing
        // iterations. If the generated feasible solutions are more than this number, they are
        // pruned away. The higher this parameter, the more RAM memory is required, but also the
        // bigger is the hypothesis space explored. In practice, it's better to have a relatively
        // high number for small inputs and a small number for large inputs.
        constexpr size_t pool_solutions_prune_size = 128;

        // if true, the oldest feasible solutions are discarded during pruning
        constexpr bool prefer_new_solutions = true;

        // instantiate the TSP solution pool, which is responsible of keeping track of the feasible
        // solutions and initializing the best initial temperature according to the input.
        TSPSolutionPool pool(
            distance_matrix,
            std::forward<decltype(initial_solution_factory)>(initial_solution_factory),
            pool_solutions_prune_size, prefer_new_solutions);

        // we use the default Simulated Annealing options. The initial temperature
        simulated_annealing::SimulatedAnnealingOptions options{};

        // Ben-Ameur initialization parameters. These values are arbitrary, but they resulted in
        // good experimental results.
        const size_t sample_pair_size = (size / 20) + 1;
        const size_t sample_temperature_iterations = 5;

        // the initial feasible solution is returned. The initial temperature and the reheating
        // interval is determined using Ben-Ameur initialization.
        const auto initial_solution =
            pool.init(options, sample_pair_size, sample_temperature_iterations);

        // initialize the Simulated Annealing solver object
        simulated_annealing::SimulatedAnnealing<TSPSolution> sa_optimizer(
            std::forward<decltype(options)>(options));

        // run the Simulated Annealing process starting from the initial solution, and return the
        // best solution found
        return sa_optimizer.solve(initial_solution);
    };

    // run Simulated Annealing as many times as the number of CPU cores
    const auto executor(executor::parallel_executor({}, std::move(solve_tsp)));

    // return the best cost found
    return executor.get_best_result(utils::min_element);
}
