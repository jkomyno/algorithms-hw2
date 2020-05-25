#pragma once

#include <vector>       // std::vector

#include "executor.h"

namespace executor {
    /**
     * Utility class to run
     */
    template <typename Function, class... Args>
    class sequential_executor : public executor<Function, Args...> {
        using Result = typename executor<Function, Args...>::Result;

        std::vector<Result> results;

    public:
        /**
         * @param n number of execution time
         * @param f function to execute in parallel
         * @param args variadic arguments to pass to the function f
         */
        explicit sequential_executor(size_t n, Function&& f,
                                   Args&&... args) noexcept {
            results.reserve(n);

            // run the function n times and capture its results
            for (size_t i = 0; i < n; ++i) {
                results.emplace_back(f(args...));
            }
        }

        /**
         * Returns the results of the sequential execution of the function.
         */
        [[nodiscard]] std::vector<Result> get_results() const override {
            return results;
        }
    };
}  // namespace executor
