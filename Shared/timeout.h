#pragma once

#include <chrono>              // std::milliseconds, std::chrono_literals
#include <future>              // std::future, std::promise
#include <thread>              // std::thread
#include <type_traits>         // std::invoke_result_t

using namespace std::chrono_literals;

namespace timeout {
    namespace detail {
        // returns true iff the given future was terminated by a timeout
        template <typename R>
        bool is_expired(std::future<R> const& future) noexcept {
            return future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
        }
    }  // namespace detail

    // future<void> wrapper used to signal expired timeout signals
    struct timeout_signal {
        timeout_signal() = delete;

        explicit timeout_signal(std::future<void>&& signal_future) :
            signal_future(std::move(signal_future)) {
        }

        // returns true iff the timeout expired
        [[nodiscard]] bool is_expired() const {
            return detail::is_expired(signal_future);
        }

        // create a new timeout_signal from a promise
        static timeout_signal from_promise(std::promise<void>& promise) {
            return timeout_signal{promise.get_future()};
        }

    private:
        std::future<void> signal_future;
    };

    // execute a function f with the given arguments with a timeout guard of the specified duration.
    // The first argument of f must be a timeout_signal&& object.
    // The function is executed in a separate thread. The caller thread is blocked until either
    // the function returns or the timeout expires, whatever of the two conditions happens earlier.
    // If the function concludes before the timeout expires, the result is immediately returned.
    // If the timeout expires while the function is still in execution, its timeout signal object is
    // marked as expired. The function itself must check whether the timeout signal was triggered or
    // not with the timeout_signal::is_expired() mmethod.
    template <typename Duration, typename Function, class... Args>
    std::invoke_result_t<Function, timeout_signal&&, Args...> with_timeout(Duration duration,
                                                                           Function&& f,
                                                                           Args&&... args) {
        using R = std::invoke_result_t<Function, timeout_signal&&, Args...>;

        // create promise and timeout for signaling the timeout expiration
        std::promise<void> promise_signal;
        timeout_signal signal{timeout_signal::from_promise(promise_signal)};

        // we create a task to run the function f in a separate thread
        std::packaged_task<R(timeout_signal&&, Args...)> task(f);
        std::future<R> future = task.get_future();

        // start the function in a new thread. The timeout signal object is the first argument of
        // the function.
        std::thread thread(std::move(task), std::move(signal), std::forward<Args>(args)...);

        // if the packaged function returns before the timeout expired, return its
        // result.
        if (future.wait_for(duration) == std::future_status::timeout) {
            // the allotted timeout expired.
            // Tell the function thread to exit the function immediately
            promise_signal.set_value();
        }

        // wait for thread to join
        thread.join();

        // return the function result
        return future.get();
    }
}  // namespace timeout
