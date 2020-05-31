#pragma once

#include <random>

namespace random_generator {
    /**
     * Generic random generator abstract class.
     */
    template <typename T>
    class RandomGenerator {
    protected:
        std::mt19937 engine{std::random_device()()};

    public:
        // virtual destructor because this is a base class
        virtual ~RandomGenerator() noexcept = default;

        // set the seed of the random generator
        void set_seed(unsigned int seed) {
            engine.seed(seed);
        }

        // return a random value of generic type T
        virtual T operator()() = 0;
    };

    /**
     * Real number random generator that returns values in range [first, last]
     */
    class RealRandomGenerator : public RandomGenerator<double> {
        std::uniform_real_distribution<double> dist;

    public:
        RealRandomGenerator(double first, double last) noexcept : dist(first, last) {
        }

        double operator()() override {
            return dist(this->engine);
        }
    };

    /**
     * Unsigned integer random generator that returns values in range [first, last]
     */
    class IntegerRandomGenerator : public RandomGenerator<size_t> {
        std::uniform_int_distribution<size_t> dist;

    public:
        IntegerRandomGenerator(size_t first, size_t last) noexcept : dist(first, last) {
        }

        size_t operator()() override {
            return dist(this->engine);
        }
    };

    /**
     * Mock generator that implements RandomGenerator but returns a fixed value
     */
    template <typename T>
    class FixedGenerator : public RandomGenerator<T> {
        T fixed_value;

    public:
        explicit FixedGenerator(T fixed_value) noexcept : fixed_value(fixed_value) {
        }

        T operator()() override {
            return fixed_value;
        };
    };
}  // namespace random_generator
