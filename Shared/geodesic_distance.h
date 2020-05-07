#pragma once

#include <cmath>  // std::cos, std::acos

#include "point.h"  // point::point_geo

namespace distance {
    // return the geodesic distance between point i and point j.
    // See: http://comopt.ifi.uni-heidelberg.de/software/TSPLIB95/TSPFAQ.html
    [[nodiscard]] inline int geodesic_distance(const point::point_geo& i,
                                               const point::point_geo& j) noexcept {
        // radius of earth in km
        constexpr double RRR = 6378.388;

        const auto& [lat_i, long_i] = i;
        const auto& [lat_j, long_j] = j;

        const double q1 = std::cos(long_i - long_j);
        const double q2 = std::cos(lat_i - lat_j);
        const double q3 = std::cos(lat_i + lat_j);

        return static_cast<int>(RRR * std::acos(0.5 * ((1.0 + q1) * q2 - (1.0 - q1) * q3)) + 1.0);
    }
}  // namespace distance
