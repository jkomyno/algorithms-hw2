#pragma once

#include "random_generator.h"

namespace simulated_annealing {
    class SimulatedAnnealingOptions {
        random_generator::RealRandomGenerator rand;

        // initial temperature τ_0
        double init_temperature = 0;

        // the temperature will be re-heated every reheat_interval annealing steps
        size_t reheat_interval = 0;

    public:
        bool metropolis(double temperature, double x, double y) {
            return rand() <= std::exp(-(x - y) / temperature);
        }

        // number of annealing iterations
        const size_t annealing_steps;

        // number of iterations in an annealing step where the temperature doesn't change
        const size_t steady_steps;

        const size_t restarts;

        // temperature that, when reached, stops the optimization process
        double stop_temperature;

        // stop when the best solution stays the same for the specified amount of times
        size_t max_same_best_solution_times;

        // percentage of proposed uphill transitions that must be accepted at τ_0
        // (0.8 <= χ_0 <= 0.99)
        const double init_acceptance_ratio;

        // cooling coefficient (0 <= β <= 1)
        const double cooling;

        // factor by which the temperature will be multiplied during reheating
        const double reheat_factor;

        void set_init_temperature(double init_temperature) noexcept {
            this->init_temperature = init_temperature;
        }

        void set_reheat_interval(size_t reheat_interval) noexcept {
            this->reheat_interval = reheat_interval;
        }

        [[nodiscard]] double get_init_temperature() const {
            return init_temperature;
        }

        [[nodiscard]] size_t get_reheat_interval() const {
            return reheat_interval;
        }

        /**
         * These options seem to work best when the graphs have less than 300 nodes.
         */
        explicit SimulatedAnnealingOptions(size_t annealing_steps = 25000, size_t steady_steps = 5,
                                           size_t restarts = 2,
                                           double stop_temperature = 1e-16,
                                           size_t max_same_best_solution_times = 150,
                                           double init_acceptance_ratio = 0.94,
                                           double cooling = 0.994,
                                           double reheat_factor = 0.8) noexcept :
            rand(random_generator::RealRandomGenerator(0.0, 1.0)),
            annealing_steps(annealing_steps),
            steady_steps(steady_steps),
            restarts(restarts),
            stop_temperature(stop_temperature),
            max_same_best_solution_times(max_same_best_solution_times),
            init_acceptance_ratio(init_acceptance_ratio),
            cooling(cooling),
            reheat_factor(reheat_factor) {
        }
    };
}  // namespace simulated_annealing
