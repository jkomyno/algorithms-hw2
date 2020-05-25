#pragma once

#include <future>    // std::future, std::packaged_task
#include <optional>  // std:optional
#include <thread>    // std::thread
#include <vector>    // std::vector

#include "executor.h"

namespace executor {
    /**
     * Utility class to run
     */
    template <typename Function, class... Args>
    class parallel_executor : public executor<Function, Args...> {
        using Result = typename executor<Function, Args...>::Result;

        // fallback on a single core if the system isn't able to detect the number of CPU cores
        const size_t n_cores = std::max(std::thread::hardware_concurrency(), 1U);

        // vector used to store the threads to join
        mutable std::vector<std::thread> threads;

        // vector used to store the results wrapped in std::future
        mutable std::vector<std::future<Result>> futures;

        // return either the amount of CPU cores to use. If n_cores_opt isn't valid, the total
        // number of CPU cores is used.
        [[nodiscard]] static size_t get_n_cores(std::optional<size_t>&& n_cores_opt) noexcept {
            // fallback on a single core if the system isn't able to detect the number of CPU cores
            const auto fallback = std::max(std::thread::hardware_concurrency(), 1U);

            if (n_cores_opt.has_value() && n_cores_opt.value() > 0) {
                return n_cores_opt.value();
            }

            return fallback;
        }

    public:
        /**
         * @param n_cores_opt optional number of cores to use
         * @param f function to execute in parallel
         * @param args variadic arguments to pass to the function f
         */
        explicit parallel_executor(std::optional<size_t>&& n_cores_opt, Function&& f,
                                   Args&&... args) noexcept :
            n_cores(get_n_cores(std::move(n_cores_opt))) {

            threads.reserve(n_cores);
            futures.reserve(n_cores);

            // run the function on n_core threads
            for (size_t core = 0; core < n_cores; ++core) {
                std::packaged_task<Result(Args...)> task(f);
                std::future<Result> future = task.get_future();
                futures.push_back(std::move(future));

                std::thread thread(std::move(task), std::forward<Args>(args)...);
                threads.push_back(std::move(thread));
            }
        }

        /**
         * Returns the results of the parallel execution of the function.
         * This method blocks the main thread until all futures have completed their execution.
         * Futures must not throw any exception.
         */
        [[nodiscard]] std::vector<Result> get_results() const override {
            for (auto& thread : threads) {
                thread.join();
            }

            std::vector<Result> results;
            results.reserve(n_cores);

            for (auto& future : futures) {
                results.push_back(future.get());
            }

            return results;
        }
    };
}  // namespace executor
