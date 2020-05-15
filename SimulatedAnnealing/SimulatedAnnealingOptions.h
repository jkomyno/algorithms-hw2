#pragma once

namespace simulated_annealing {
    struct SimulatedAnnealingOptions {
        // number of iterations
        const size_t iterations;

        // initial temperature
        const double init_temperature;

        // final temperature
        const double stop_temperature;

        // annealing coefficient
        const double lambda;

        // periodic reheating interval
        const size_t reheat;

        explicit SimulatedAnnealingOptions(size_t iterations = 100000,
                                           double init_temperature = 1000,
                                           double stop_temperature = 1e-16, double lambda = 0.995,
                                           size_t reheat = 10000) :
            iterations(iterations),
            init_temperature(init_temperature),
            stop_temperature(stop_temperature),
            lambda(lambda),
            reheat(reheat) {
        }
    };
}  // namespace simulated_annealing
