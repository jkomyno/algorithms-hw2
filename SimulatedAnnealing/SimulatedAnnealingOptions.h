#pragma once

namespace simulated_annealing {
    struct SimulatedAnnealingOptions {
        // number of annealing iterations
        const size_t annealing_steps;

        // number of iterations in an annealing step where the temperature doesn't change
        const size_t steady_steps;

        // initial temperature
        const double init_temperature;

        // temperature that, when reached, stops the optimization process
        const double stop_temperature;

        // annealing multiplicative coefficient
        const double lambda;

        // the temperature will be re-heated every reheat_interval annealing steps
        const size_t reheat_interval;

        // factor by which the temperature will be multiplied during reheating
        const double reheat_factor;

        /**
         * These options seem to work best when the graphs have less than 300 nodes.
         */
        explicit SimulatedAnnealingOptions(size_t annealing_steps = 100000, size_t steady_steps = 5,
                                           double init_temperature = 10000,
                                           double stop_temperature = 1e-16, double lambda = 0.992,
                                           size_t reheat_interval = 2500,
                                           double reheat_factor = 10e3) :
            annealing_steps(annealing_steps),
            steady_steps(steady_steps),
            init_temperature(init_temperature),
            stop_temperature(stop_temperature),
            lambda(lambda),
            reheat_interval(reheat_interval),
            reheat_factor(reheat_factor) {
        }
    };
}  // namespace simulated_annealing
