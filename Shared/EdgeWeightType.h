#pragma once

#include <iostream>

// enumeration of the supported metric distances
enum class EdgeWeightType {
    EUC_2D,  // euclidean 2-dimensional distance
    GEO      // geodesic distance
};

// read a EdgeWeightType object in input
std::istream& operator>>(std::istream& is, EdgeWeightType& edge_weight_type) {
    std::string name;
    is >> name;

    if (name == "EUC_2D") {
        edge_weight_type = EdgeWeightType::EUC_2D;
    } else if (name == "GEO") {
        edge_weight_type = EdgeWeightType::GEO;
    } else {
        throw std::exception("Unsupported EDGE_WEIGHT_TYPE");
    }

    return is;
}
