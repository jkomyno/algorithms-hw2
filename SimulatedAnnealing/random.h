#pragma once

#include <random>

namespace random {
    class RealRandomGenerator {
        std::mt19937 engine{std::random_device()()};
        std::uniform_real_distribution<double> dist;

    public:
        RealRandomGenerator(double x, double y) : dist(x, y) {
        }

        void set_seed(unsigned int seed) {
            engine.seed(seed);
        }

        double operator()() {
            return dist(engine);
        }
    };

    class IntegerRandomGenerator {
        std::mt19937 engine{std::random_device()()};
        std::uniform_int_distribution<size_t> dist;

    public:
        IntegerRandomGenerator(size_t x, size_t y) : dist(x, y) {
        }

        void set_seed(unsigned int seed) {
            engine.seed(seed);
        }

        size_t operator()() {
            return dist(engine);
        }
    };
}  // namespace random
