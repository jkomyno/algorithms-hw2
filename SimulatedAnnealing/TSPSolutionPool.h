#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <vector>

#include "DistanceMatrix.h"
#include "SimulatedAnnealing.h"
#include "SimulatedAnnealingOptions.h"
#include "random_generator.h"
#include "shared_utils.h"
#include "utils.h"

class TSPSolution;

class TSPSolutionPool {
    friend class TSPSolution;
    using Path = std::vector<size_t>;
    using Cost = int;
    using SolutionFactory = std::function<std::pair<Path, Cost>()>;

    // size of the graph
    size_t circuit_size;

    // graph represented as a distance matrix
    const DistanceMatrix<int>& distance_matrix;

    // function used to initialize a new solution with a heuristic provided by the caller
    SolutionFactory solution_factory;

    // maximum number of feasible solutions considered after every Simulated Annealing iterations
    const size_t solutions_prune_size;

    // if true, the oldest feasible solutions are discarded during pruning
    const bool prefer_new_solutions;

    // real number generator in the range [0, 1)
    random_generator::RealRandomGenerator random;

public:
    // list of all feasible solutions generated up to now
    std::vector<std::vector<size_t>> feasible_solutions;

private:
    // list of indexes of unused solutions
    std::vector<size_t> unused_solution_indexes;

    // create a new initial solution starting from the heuristic initialization returned by
    // solution_factory
    [[nodiscard]] TSPSolution create();

    // add the pool index of solution to the unused vector
    void reclaim(const TSPSolution& solution);

    // return the path at index
    std::vector<size_t>& get(size_t index);

    // return a random index in [start, end]
    [[nodiscard]] size_t random_index(size_t start, size_t end);

    // return the total distance of the circuit
    int compute_distance(const std::vector<size_t>& circuit);

    // prunes every solution but the best and current solution, which are stored respectively in
    // index 0 and 1
    void remove_solutions_except(size_t& best_index, size_t& current_index);

public:
    TSPSolutionPool(const DistanceMatrix<int>& distance_matrix, SolutionFactory&& solution_factory,
                    size_t solutions_prune_size = 32, bool prefer_new_solutions = true) :
        circuit_size(distance_matrix.size()),
        distance_matrix(distance_matrix),
        solution_factory(std::move(solution_factory)),
        solutions_prune_size(solutions_prune_size),
        prefer_new_solutions(prefer_new_solutions),
        random(0.0, 1.0) {
        feasible_solutions.reserve(solutions_prune_size);
    }

    // return the size of the graph
    [[nodiscard]] size_t size() const;

    TSPSolution init(simulated_annealing::SimulatedAnnealingOptions& options,
                     size_t sample_pair_size, size_t sample_temperature_iterations);
};

class TSPSolution : public simulated_annealing::SolutionBase<TSPSolution> {
    using super = simulated_annealing::SolutionBase<TSPSolution>;
    friend class TSPSolutionPool;

public:
    TSPSolutionPool* pool;

private:
    size_t pool_index;

    // initial fitness value of this solution. Since computing the distance of a solution path can
    // take some time, it is computed lazily and then memoized for future TSPSolution::fitness()
    // calls.
    int NOT_INITIALIZED = std::numeric_limits<int>::max();

    // distance is mutable so that TSPSolution::fitness() is a const method.
    mutable int distance = NOT_INITIALIZED;

    // manipulate the path of a solution to a feasible neighbor path called new_path.
    // The new path found is returned.
    std::vector<size_t> manipulate_raw(const std::vector<size_t>& path,
                                       std::vector<size_t>& new_path) const noexcept;

public:
    TSPSolution(TSPSolutionPool& pool, size_t pool_index) :
        super(), pool(&pool), pool_index(pool_index) {
    }

    TSPSolution(const TSPSolution&) = default;

    TSPSolution& operator=(const TSPSolution&) = default;

    bool operator==(const TSPSolution& rhs) const {
        return pool == rhs.pool && pool_index == rhs.pool_index;
    }

    bool operator!=(const TSPSolution& rhs) const {
        return !(rhs == *this);
    }

    // TODO: remove
    size_t feasible_size() const {
        return this->pool->feasible_solutions.size();
    }

    // return the TSP path associated with this solution
    std::vector<size_t>& circuit() const;

    // manipulate the path of a solution to create a feasible neighbor path for a new solution.
    // The new path found is returned.
    std::vector<size_t> manipulate_raw(const std::vector<size_t>& path) const noexcept;

    // lazily computes the cost of the current solution.
    // Time: O(n) the first time, O(1) the next times
    [[nodiscard]] int fitness() const override;

    // manipulate the current solution to create a new feasible neighbor solution
    [[nodiscard]] TSPSolution manipulate() override;

    // remove the solution from the pool
    void destroy() override;

    // this solution is marked as the best solution, and all the other temporary solutions except
    // the current solution are pruned away to free the memory
    void survives(TSPSolution& current) override;
};

inline std::vector<size_t>& TSPSolutionPool::get(size_t index) {
    return feasible_solutions[index];
}

inline void TSPSolutionPool::reclaim(const TSPSolution& solution) {
    unused_solution_indexes.push_back(solution.pool_index);
}

inline size_t TSPSolutionPool::size() const {
    return circuit_size;
}

inline size_t TSPSolutionPool::random_index(size_t start, size_t end) {
    const double stride = std::floor(random() * (end - start));
    return start + static_cast<size_t>(stride);
}

inline int TSPSolutionPool::compute_distance(const std::vector<size_t>& circuit) {
    const auto get_distance = [this](const size_t x, const size_t y) {
        return distance_matrix.at(x, y);
    };

    return utils::sum_weights_in_circuit(circuit.cbegin(), circuit.cend(), get_distance);
}

// @param sample_size size n of R, a sample of 2n randomly generated solutions

/**
 * Initialize the first TSP solution and some core Simulated Annealing options.
 * The first TSP solution is initialized via solution_factory (which uses the Nearest Neighbor
 * heuristic).
 * The initial annealing temperature τ_0 is determined using the approach suggested by Ben-Ameur,
 * @see
 * https://www.researchgate.net/publication/227061666_Computing_the_Initial_Temperature_of_Simulated_Annealing.
 *
 * The reheating interval ρ is determined by max{τ_0 / 4000, 100}.
 */
inline TSPSolution TSPSolutionPool::init(simulated_annealing::SimulatedAnnealingOptions& options,
                                         size_t sample_pair_size,
                                         size_t sample_temperature_iterations) {
    // initial solution and its cost using the user-provided heuristic
    // (in our case, Nearest Neighbor)
    const auto [initial_path, initial_cost] = solution_factory();

    TSPSolution solution = create();
    solution.circuit() = initial_path;
    solution.distance = initial_cost;

    // percentage of proposed uphill transitions that must be accepted at τ_0
    // (0.8 <= χ_0 <= 0.99)
    const double init_acceptance_ratio = options.init_acceptance_ratio;

    // average absolute difference in objective function over the n sample transitions composing a
    // sample of 2n solutions
    double delta_v = 0;

    const auto trials = 2 * sample_pair_size;
    for (size_t r = 0; r < trials; ++r) {
        auto n1 = solution.manipulate_raw(initial_path);
        auto n2 = solution.manipulate_raw(n1);

        const double c1 = compute_distance(n1);
        const double c2 = compute_distance(n2);

        delta_v += std::abs(c1 - c2);
    }

    // initial temperature τ_0
    double init_temperature = delta_v / std::log(1.0 / init_acceptance_ratio);

    for (size_t accepted = 0;; accepted = 0, init_temperature *= 1.5) {
        auto current_path = initial_path;
        double current_cost = initial_cost;

        for (size_t i = 0; i < sample_temperature_iterations; ++i) {
            auto next = solution.manipulate_raw(current_path);
            const double next_cost = compute_distance(next);
            const double current_e = current_cost;
            const double next_e = next_cost;

            if (options.metropolis(init_temperature, current_e, next_e)) {
                current_path = next;
                current_cost = next_cost;
                ++accepted;
            }
        }

        const auto acc = static_cast<double>(accepted);
        const auto sample_it = static_cast<double>(sample_temperature_iterations);

        if ((acc / sample_it) >= init_acceptance_ratio) {
            break;
        }
    }

    // set the values of τ_0 and ρ for the current instance
    options.set_init_temperature(init_temperature);
    options.set_reheat_interval(std::max(static_cast<int>(init_temperature / 4000), 100));

    return solution;
}

inline TSPSolution TSPSolutionPool::create() {
    if (unused_solution_indexes.empty()) {
        feasible_solutions.emplace_back(std::vector<size_t>(circuit_size));
        unused_solution_indexes.push_back(feasible_solutions.size() - 1);
    }

    const auto pool_index_to_use = unused_solution_indexes.back();
    unused_solution_indexes.pop_back();

    return TSPSolution(*this, pool_index_to_use);
}

inline void TSPSolutionPool::remove_solutions_except(size_t& best_index, size_t& current_index) {
    const auto size = feasible_solutions.size();

    // if the feasible solutions are more than the allotted number, first shift them to maintain the
    // newest solutions and discard the oldest solutions in the latest steps
    if (prefer_new_solutions && size > solutions_prune_size) {
        const size_t shift = size + 2 - solutions_prune_size;
        std::rotate(feasible_solutions.begin(), feasible_solutions.begin() + shift,
                    feasible_solutions.end());

        best_index = (size - shift + best_index) % size;
        current_index = (size - shift + current_index) % size;
    }

    // add the best and current circuits to the back of feasible_solutions. This means that there
    // are two copies of the best solution and the current solution in the vector.
    feasible_solutions.emplace_back(feasible_solutions[best_index]);
    feasible_solutions.emplace_back(feasible_solutions[current_index]);

    // shift feasible_solutions by 2 positions to the right, so that the best circuit is stored in
    // index 0 and the current circuit is stored in index 1.
    std::rotate(feasible_solutions.rbegin(), feasible_solutions.rbegin() + 2,
                feasible_solutions.rend());

    best_index += 2;
    current_index += 2;

    // index to the back of the feasible_solutions array
    size_t n = size + 1;  // size + 2 - 1

    // move the duplicates of the best and current circuit to the back of feasible_solutions
    for (size_t i = 2; i < feasible_solutions.size(); ++i) {
        if (i == best_index || i == current_index) {
            std::swap(feasible_solutions[i], feasible_solutions[n]);
            --n;
        }
    }

    // the index of the best solution now is at the front of the feasible_solutions vector
    best_index = 0;

    // the index of the current solution now is at after the front of the feasible_solutions vector
    current_index = 1;

    // if feasible_solutions is small enough, keep its original size, otherwise prune it to keep a
    // maximum of solutions_prune_size elements
    const size_t new_size = std::min(size, solutions_prune_size);
    feasible_solutions.resize(new_size);

    // updated unused_solution_indexes only if it has some elements
    if (!unused_solution_indexes.empty()) {
        // we shouldn't consider the first 2 indexes, since they are used
        const size_t unused_indexes_size = std::min(unused_solution_indexes.size(), new_size - 2);

        std::vector<size_t> new_unused_indexes(unused_indexes_size);
        std::iota(new_unused_indexes.begin(), new_unused_indexes.end(), 2);
        std::swap(new_unused_indexes, this->unused_solution_indexes);
    }
}

inline std::vector<size_t>& TSPSolution::circuit() const {
    return pool->get(pool_index);
}

inline int TSPSolution::fitness() const {
    if (distance == NOT_INITIALIZED) {
        distance = pool->compute_distance(circuit());
    }

    return distance;
}

inline std::vector<size_t> TSPSolution::manipulate_raw(const std::vector<size_t>& path) const
    noexcept {
    std::vector<size_t> new_path(pool->size());
    return manipulate_raw(path, new_path);
}

inline std::vector<size_t> TSPSolution::manipulate_raw(const std::vector<size_t>& path,
                                                       std::vector<size_t>& new_path) const
    noexcept {
    // ensure that endpoints aren't involved in the manipulation
    // we assume length of the path > 4 and k >= 2
    const size_t k = 2;

    // x and j are the selected endpoints used for manipulating the previous solution to create a
    // new solution.
    // The first x items and the items at position >= y will be always copied as they are.
    const size_t x = pool->random_index(1, pool->size() - k);
    const size_t y = x + pool->random_index(2, pool->size() - x);

    const double dice = pool->random();

    /**
     * - 40% of probability of performing a 2-opt step;
     * - 40% of probability of performing a translate step;
     * - 20% of probability of performing a switching step
     */

    if (dice < 0.4) {
        utils::two_opt(path, new_path, x, y);
    } else if (dice < 0.8) {
        utils::translate(path, new_path, x, y);
    } else {
        utils::switching(path, new_path, x, y);
    }

    return new_path;
}

inline TSPSolution TSPSolution::manipulate() {
    auto new_solution = pool->create();
    auto& path = circuit();
    auto& new_path = new_solution.circuit();

    manipulate_raw(path, new_path);
    return new_solution;
}

inline void TSPSolution::destroy() {
    pool->reclaim(*this);
}

inline void TSPSolution::survives(TSPSolution& current) {
    pool->remove_solutions_except(this->pool_index, current.pool_index);
}
