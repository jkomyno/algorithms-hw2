#pragma once

#include <future>       // std::future, std::packaged_task
#include <thread>       // std::thread
#include <type_traits>  // std::invoke_result_t
#include <vector>       // std::vector

namespace parallel {
    template <typename Function, class... Args>
    class parallel_executor {
        using Result = std::invoke_result_t<Function, Args...>;

        const size_t n_cores = std::max(std::thread::hardware_concurrency(), 1U);
        std::vector<std::thread> threads;
        std::vector<std::future<int>> futures;

    public:
        explicit parallel_executor(Function&& f, Args&&... args) {
            threads.reserve(n_cores);
            futures.reserve(n_cores);

            for (size_t core = 0; core < n_cores; ++core) {
                std::packaged_task<Result(Args...)> task(f);
                std::future<int> future = task.get_future();
                futures.push_back(std::move(future));

                std::thread thread(std::move(task), std::forward<Args>(args)...);
                threads.push_back(std::move(thread));
            }
        }

        [[nodiscard]] std::vector<Result> get_results() {
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
}  // namespace parallel
