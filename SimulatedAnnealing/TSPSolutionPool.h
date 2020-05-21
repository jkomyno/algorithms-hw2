#pragma once

#include <algorithm>
#include <cmath>
#include <functional>
#include <vector>

#include "DistanceMatrix.h"
#include "SimulatedAnnealing.h"
#include "random_generator.h"
#include "shared_utils.h"

class TSPSolution;

class TSPSolutionPool {
    friend class TSPSolution;
    using SolutionFactory = std::function<std::vector<size_t>()>;

    size_t circuit_size;

    // graph represented as a distance matrix
    const DistanceMatrix<int>& distance_matrix;

    // function used to initialize a new solution with a heuristic provided by the caller
    SolutionFactory solution_factory;

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
    TSPSolution create();

    // add the pool index of solution to the unused vector
    void reclaim(const TSPSolution& solution);

    // return the path at index
    std::vector<size_t>& get(size_t index);

    size_t random_index(size_t start, size_t end);

    double compute_distance(const std::vector<size_t>& circuit);

    // prunes some solutions at random mantaining the current best solution. Although it reduces the
    // simulated annealing solution space, it helps keeping the memory consumption low
    void remove_random_solutions_except(size_t best_index);

public:
    TSPSolutionPool(const DistanceMatrix<int>& distance_matrix,
                    SolutionFactory&& solution_factory) :
        circuit_size(distance_matrix.size()),
        distance_matrix(distance_matrix),
        solution_factory(std::move(solution_factory)),
        random(0.0, 1.0) {
    }

    size_t size() const;
    TSPSolution init();
};

class TSPSolution : public simulated_annealing::SolutionBase<TSPSolution> {
    using super = simulated_annealing::SolutionBase<TSPSolution>;
    friend class TSPSolutionPool;

public:
    TSPSolutionPool* pool;

private:
    size_t pool_index;

    double NOT_INITIALIZED = std::numeric_limits<double>::max();
    mutable double distance = NOT_INITIALIZED;

    TSPSolution two_opt(size_t x, size_t y);
    TSPSolution translate(size_t x, size_t y);
    TSPSolution switching(size_t x, size_t y);

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

    std::vector<size_t>& circuit() const;

    // lazily computes the cost of the current solution
    double fitness() override;

    // manipulate the current solution to create a new feasible solution
    TSPSolution manipulate() override;

    // remove the solution from the pool
    void destroy() override;

    // mark the current solution as a good solution. A portion of the other solutions may be pruned
    // away to spare memory
    void survives() override;
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

inline double TSPSolutionPool::compute_distance(const std::vector<size_t>& circuit) {
    const auto get_distance = [this](const size_t x, const size_t y) {
        return distance_matrix.at(x, y);
    };

    return utils::sum_weights_in_circuit(circuit.cbegin(), circuit.cend(), get_distance);
}

inline TSPSolution TSPSolutionPool::init() {
    auto solution = create();
    solution.circuit() = solution_factory();
    return solution;
}

inline TSPSolution TSPSolutionPool::create() {
    if (unused_solution_indexes.empty()) {
        feasible_solutions.emplace_back(std::vector<size_t>(circuit_size));
        unused_solution_indexes.push_back(feasible_solutions.size() - 1);
    }

    auto pool_index_to_use = unused_solution_indexes.back();
    unused_solution_indexes.pop_back();

    return TSPSolution(*this, pool_index_to_use);
}

 // TODO: find a way to efficiently reindex the spared solutions
inline void TSPSolutionPool::remove_random_solutions_except(size_t best_index) {
    size_t size = feasible_solutions.size();
    size_t n_to_remove = std::min(size * 0.3, 10.0);

    /*
    size_t n = size;

    // The element at the index of the current best solution is never moved.
    // since deletion from a vector is inefficient (linear time complexity), we simply
    // swap the items from the indexes to be removed to the end of the vector.
    for (size_t i = 1; i <= n_to_remove; ++i) {
        size_t index_to_remove = static_cast<size_t>(std::trunc(random() * n));

        if (index_to_remove != best_index) {
            --n;
            std::swap(holder[index_to_remove], holder[n]);
        } else {
            ++i;
        }
    }

    // remove the last size - n elements from holder
    holder.resize(n);
    */
}

inline std::vector<size_t>& TSPSolution::circuit() const {
    return pool->get(pool_index);
}

inline TSPSolution TSPSolution::two_opt(size_t x, size_t y) {
    auto new_solution = pool->create();
    auto& path = circuit();
    auto& new_path = new_solution.circuit();

    size_t i = 0;
    size_t j = 0;

    // copy the first x items
    for (; i < x; i++, j++) {
        new_path[j] = path[i];
    }

    // copy the reversed version of the next x items
    for (i = y - 1; i >= x; i--, j++) {
        new_path[j] = path[i];
    }

    // copy the last part
    for (i = y; i < path.size(); i++, j++) {
        new_path[j] = path[i];
    }

    return new_solution;
}

inline TSPSolution TSPSolution::translate(size_t x, size_t y) {
    auto new_solution = pool->create();
    auto& path = circuit();
    auto& new_path = new_solution.circuit();

    size_t i = 0;
    size_t j = 0;

    // copy the first x items
    for (; i < x; i++, j++) {
        new_path[j] = path[i];
    }

    // copy the item at position y-1
    new_path[j++] = path[y - 1];

    // copy the next y-x-1 items
    for (i = x; i < y - 1; i++, j++) {
        new_path[j] = path[i];
    }

    // copy the last part
    for (i = y; i < path.size(); i++, j++) {
        new_path[j] = path[i];
    }

    return new_solution;
}

inline TSPSolution TSPSolution::switching(size_t x, size_t y) {
    auto new_solution = pool->create();
    auto& path = circuit();
    auto& new_path = new_solution.circuit();

    size_t i = 0;
    size_t j = 0;

    // copy the first x items
    for (; i < x; i++, j++) {
        new_path[j] = path[i];
    }

    // copy the item at position y-1
    new_path[j++] = path[y - 1];

    // skip the x-th item, copy the next y-x-2 items
    for (i = x + 1; i < y - 1; i++, j++) {
        new_path[j] = path[i];
    }

    // copy the x-th item
    new_path[j++] = path[x];

    // copy the last part
    for (i = y; i < path.size(); i++, j++) {
        new_path[j] = path[i];
    }

    return new_solution;
}

inline double TSPSolution::fitness() {
    if (distance == NOT_INITIALIZED) {
        distance = pool->compute_distance(circuit());
    }

    return distance;
}

inline TSPSolution TSPSolution::manipulate() {
    // ensure that endpoints aren't involved in the manipulation
    // we assume length of the path > 4 and k >= 2
    size_t k = 2;

    // x and j are the selected endpoints used for manipulating the previous solution to create a
    // new solution.
    // The first x items and the items at position >= y will be always copied as they are.
    size_t x = pool->random_index(1, pool->size() - k);
    size_t y = x + pool->random_index(2, pool->size() - x);

    const double dice = pool->random();

    /**
     * - 40% of probability of performing a 2-opt step;
     * - 40% of probability of performing a translate step;
     * - 20% of probability of performing a switching step
     */

    if (dice < 0.4) {
        return two_opt(x, y);
    }
    if (dice < 0.8) {
        return translate(x, y);
    }

    return switching(x, y);
}

inline void TSPSolution::destroy() {
    pool->reclaim(*this);
}

inline void TSPSolution::survives() {
    pool->remove_random_solutions_except(this->pool_index);
}
