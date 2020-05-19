#pragma once

#include <algorithm>    // std::min
#include <cmath>        // std::exp
#include <type_traits>  // std::enable_if, std::is_base_of

#include "SimulatedAnnealingOptions.h"
#include "random_generator.h"

namespace simulated_annealing {
    template <class Solution>
    class SolutionBase {
    public:
        virtual ~SolutionBase() = default;

        // returns the cost of the current solution
        virtual double fitness() = 0;

        // manipulate the current solution to create a new feasible solution
        virtual Solution manipulate() = 0;

        // remove the solution from the pool of feasible solutions
        virtual void destroy() = 0;

        // mark the current solution as a good solution
        virtual void survives() = 0;
    };

    // The Solution template class must extend SolutionBase
    template <class Solution,
              typename = std::enable_if<std::is_base_of<SolutionBase<Solution>, Solution>::value>>
    class SimulatedAnnealing {
        double temperature;
        SimulatedAnnealingOptions options;
        random_generator::RealRandomGenerator rand;

        bool metropolis(double x, double y) {
            return rand() <= std::exp(-(x - y) / temperature);
        }

        void anneal() {
            temperature = temperature * options.lambda;
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
            temperature(options.init_temperature),
            options(std::forward<decltype(options)>(options)),
            rand(random_generator::RealRandomGenerator(0.0, 1.0)) {
        }

        // runs the Simulated Annealing optimization either for the specified amount of annealing
        // steps, or until the temperature reaches its minimum, whatever happens first.
        Solution solve(Solution initial_solution) {
            auto current_solution = initial_solution;
            auto best_solution = initial_solution;

            // variables used for debugging
            // TODO: remove
            size_t _iterations = 0;
            size_t _reheats = 0;
            double _final_temperature = 0;

            size_t i = 0;
            for (; i < options.annealing_steps && temperature > options.stop_temperature; ++i) {
                if (i % options.reheat_interval == 0) {
                    temperature *= std::min(temperature, options.init_temperature * temperature) *
                                   options.reheat_factor;

                    _reheats++;
                }

                for (size_t j = 0; j < options.steady_steps; ++j) {
                    // generate a new solution manipulating the current solution
                    auto new_solution = current_solution.manipulate();

                    // accept the new solution with a probability dependent on the metropolis policy
                    probably_accept(new_solution, current_solution, best_solution);
                }

                anneal();
                best_solution.survives();

                _iterations++;
            }

            _final_temperature = temperature;

            return best_solution;
        }
    };

}  // namespace simulated_annealing
