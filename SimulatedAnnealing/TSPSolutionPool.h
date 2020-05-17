#pragma once

#include <cmath>
#include <functional>
#include <numeric>
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
    const DistanceMatrix<int>& distance_matrix;
    SolutionFactory solution_factory;

    random_generator::RealRandomGenerator rand;
    std::vector<std::vector<size_t>> holder;
    std::vector<size_t> unused;

    TSPSolution create();
    void reclaim(const TSPSolution& solution);
    std::vector<size_t>& get(size_t index);
    double random();
    size_t random_index(size_t start, size_t end);
    double compute_distance(const std::vector<size_t>& circuit);

public:
    TSPSolutionPool(const DistanceMatrix<int>& distance_matrix, SolutionFactory&& solution_factory) :
        circuit_size(distance_matrix.size()),
        distance_matrix(distance_matrix),
        solution_factory(std::move(solution_factory)),
        rand(0.0, 1.0) {
    }

    size_t size() const;
    TSPSolution init();
};

class TSPSolution : public simulated_annealing::SolutionBase<TSPSolution> {
    using super = simulated_annealing::SolutionBase<TSPSolution>;
    friend class TSPSolutionPool;

    TSPSolutionPool* pool;
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

    double fitness() override;

    TSPSolution neighbor() override;

    void destroy() override;
};

inline std::vector<size_t>& TSPSolutionPool::get(size_t index) {
    return holder[index];
}

inline void TSPSolutionPool::reclaim(const TSPSolution& solution) {
    if (solution.pool == this) {
        unused.push_back(solution.pool_index);
    }
}

inline double TSPSolutionPool::random() {
    return rand();
}

inline size_t TSPSolutionPool::size() const {
    return circuit_size;
}

inline size_t TSPSolutionPool::random_index(size_t start, size_t end) {
    const double stride = std::floor(random() * static_cast<double>(end - start));
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
    if (unused.empty()) {
        holder.emplace_back(std::vector<size_t>(circuit_size));
        unused.push_back(0);
    }

    auto to_use = unused.back();
    unused.pop_back();

    return TSPSolution(*this, to_use);
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

    for (; i < x; i++, j++) {
        new_path[j] = path[i];
    }
    for (i = y - 1; i >= x; i--, j++) {
        new_path[j] = path[i];
    }
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

    for (; i < x; i++, j++) {
        new_path[j] = path[i];
    }
    new_path[j++] = path[y - 1];
    for (i = x; i < y - 1; i++, j++) {
        new_path[j] = path[i];
    }
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

    for (; i < x; i++, j++) {
        new_path[j] = path[i];
    }
    new_path[j++] = path[y - 1];
    for (i = x + 1; i < y - 1; i++, j++) {
        new_path[j] = path[i];
    }
    new_path[j++] = path[x];
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

inline TSPSolution TSPSolution::neighbor() {
    // ensure that endpoints aren't involved in the manipulation
    // we assume length of the path > 4 and k >= 2

    size_t k = 2;
    size_t i = pool->random_index(1, pool->size() - k);
    size_t j = i + pool->random_index(2, pool->size() - i);

    const double dice = pool->random();

    /**
     * - 40% of probability of performing a 2-opt step;
     * - 40% of probability of performing a translate step;
     * - 20% of probability of performing a switching step
     */

    if (dice < 0.4) {
        return two_opt(i, j);
    }
    if (dice < 0.8) {
        return translate(i, j);
    }

    return switching(i, j);
}

inline void TSPSolution::destroy() {
    pool->reclaim(*this);
}
