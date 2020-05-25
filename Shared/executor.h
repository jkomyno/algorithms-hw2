#pragma once

#include <type_traits>  // std::invoke_result_t
#include <vector>       // std::vector

namespace executor {
    /**
     * Utility class to run
     */
    template <typename Function, class... Args>
    class executor {
    protected:
        using Result = std::invoke_result_t<Function, Args...>;

    public:
        virtual ~executor() = default;

        /**
         * Returns the results of the execution of the function.
         * The function should not throw any exceptions.
         */
        [[nodiscard]] virtual std::vector<Result> get_results() const = 0;

        /**
         * Return the best result found according to the to_best higher-order function.
         *
         * @param to_best function that receives the first and last iterators to the results and
         *                selects the iterator of the best solution according to some policy
         */
        template <typename ToBest>
        [[nodiscard]] Result get_best_result(ToBest&& to_best) const {
            const auto results = get_results();
            const auto best_it = to_best(results.cbegin(), results.cend());
            return *best_it;
        }
    };
}  // namespace parallel
