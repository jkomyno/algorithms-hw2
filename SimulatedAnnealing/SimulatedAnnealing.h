#pragma once

#include <algorithm>    // std::min
#include <cmath>        // std::exp
#include <type_traits>  // std::enable_if, std::is_base_of

#include "SimulatedAnnealingOptions.h"

namespace simulated_annealing {
    template <class Solution>
    class SolutionBase {
    public:
        virtual ~SolutionBase() = default;

        virtual size_t feasible_size() const = 0;

        // returns the cost of the current solution
        [[nodiscard]] virtual int fitness() const = 0;

        // manipulate the current solution to create a new feasible solution
        [[nodiscard]] virtual Solution manipulate() = 0;

        // remove the solution from the pool of feasible solutions
        virtual void destroy() = 0;

        // mark the current solution as a good solution
        virtual void survives(Solution& current) = 0;
    };

    // The Solution template class must extend SolutionBase
    template <class Solution,
              typename = std::enable_if<std::is_base_of<SolutionBase<Solution>, Solution>::value>>
    class SimulatedAnnealing {
        double temperature;
        SimulatedAnnealingOptions options;

        bool metropolis(double x, double y) {
            return options.metropolis(temperature, x, y);
        }

        void anneal() {
            temperature *= options.cooling;
        }

        // accept with probability 1 if the candidate solution is better than the current one.
        // Accept with probability metropolis if the candidate solution is worse.
        void probably_accept(Solution& new_solution, Solution& current_solution,
                             Solution& best_solution) {
            double candidate_fitness = new_solution.fitness();

            if (candidate_fitness < current_solution.fitness()) {
                if (current_solution != best_solution) {
                    current_solution.destroy();
                }

                current_solution = new_solution;
                best_solution = new_solution;
            } else if (metropolis(new_solution.fitness(), current_solution.fitness())) {
                if (current_solution != best_solution) {
                    current_solution.destroy();
                }

                current_solution = new_solution;
            } else {
                new_solution.destroy();
            }
        }

    public:
        SimulatedAnnealing(SimulatedAnnealingOptions&& options) :
            temperature(options.get_init_temperature()),
            options(std::forward<decltype(options)>(options)) {
        }

        // runs the Simulated Annealing optimization either for the specified amount of annealing
        // steps, or until the temperature reaches its minimum, whatever happens first.
        int solve(Solution initial_solution) {
            auto current_solution = initial_solution;
            auto best_solution = initial_solution;

            int best_cost = best_solution.fitness();
            size_t same_best_solution_times = 1;

            size_t max_feasible_size = 0;

            for (size_t r = 0; r < options.restarts; ++r) {
                // variables used for debugging
                // TODO: remove
                size_t _iterations = 0;
                size_t _reheats = 0;
                double _final_temperature = 0;

                size_t i = 0;
                for (; i < options.annealing_steps &&
                       (temperature > options.stop_temperature &&
                        same_best_solution_times < options.max_same_best_solution_times);
                     ++i) {
                    if ((i + 1) % options.get_reheat_interval() == 0) {
                        temperature =
                            options.get_init_temperature() * options.reheat_factor / (10 * (i + 1));
                        _reheats++;
                    }

                    for (size_t j = 0; j < options.steady_steps; ++j) {
                        // generate a new solution manipulating the current solution
                        auto new_solution = current_solution.manipulate();

                        // accept the new solution with a probability dependent on the metropolis
                        // policy
                        probably_accept(new_solution, current_solution, best_solution);
                    }

                    anneal();

                    // prune the feasible solutions while keeping the best solution and the current
                    // solution
                    best_solution.survives(current_solution);

                    // keeps track of the best solution cost found up to now, regardless of what
                    // best_solution is
                    if (best_solution.fitness() == best_cost) {
                        ++same_best_solution_times;
                    } else {
                        best_cost = std::min({best_cost, best_solution.fitness()});
                        same_best_solution_times = 1;
                    }

                    max_feasible_size = std::max(max_feasible_size, best_solution.feasible_size());

                    _iterations++;
                }

                _final_temperature = temperature;

                temperature = options.get_init_temperature();
            }

            return best_solution.fitness();
        }
    };

}  // namespace simulated_annealing
