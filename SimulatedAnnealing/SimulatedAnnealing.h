#pragma once

#include <cmath>        // std::exp
#include <type_traits>  // std::enable_if, std::is_base_of

#include "SimulatedAnnealingOptions.h"
#include "random.h"

namespace simulated_annealing {
    template <class Solution>
    class SolutionBase {
    public:
        virtual ~SolutionBase() = default;

        virtual double fitness() = 0;
        virtual Solution neighbor() = 0;

        virtual void destroy() {
        }
    };

    template <class Solution,
              typename = std::enable_if<std::is_base_of<SolutionBase<Solution>, Solution>::value>>
    class SimulatedAnnealing {
        double temperature;
        SimulatedAnnealingOptions options;
        random::RealRandomGenerator rand;

        bool metropolis(double x, double y) {
            return rand() <= std::exp(-(x - y) / temperature);
        }

        double anneal() {
            return temperature * options.lambda;
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
            rand(random::RealRandomGenerator(0.0, 1.0)) {
        }

        Solution solve(Solution initial_solution) {
            auto current_solution = initial_solution;
            auto best_solution = initial_solution;

            for (size_t i = 0; i < options.iterations && temperature > options.stop_temperature; ++i) {
                if (i % options.reheat == 0) {
                    temperature = options.init_temperature;
                }

                auto new_solution = current_solution.neighbor();
                probably_accept(new_solution, current_solution, best_solution);

                temperature = anneal();
            }

            return best_solution;
        }
    };

}  // namespace simulated_annealing
