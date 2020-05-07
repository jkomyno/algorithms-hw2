#pragma once

namespace utils {
    // determine length of string at compile time
    template <size_t N>
    constexpr auto string_length(char const (&)[N]) {
        return N;
    }

    // fun fact, the constant value of π is only available in C++20, so we have to define it
    // manually in C++17
    template <typename T>
    constexpr T pi = T(3.141592);

    // converts x to radians 
    [[nodiscard]] inline double to_radians(const double x) noexcept {
        const int deg = static_cast<int>(x);
        const double min = x - deg;
        return pi<double> * (deg + 5.0 * min / 3.0) / 180.0;
    }
}
