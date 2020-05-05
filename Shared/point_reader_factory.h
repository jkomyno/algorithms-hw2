#pragma once

#include <fstream>
#include "EdgeWeightType.h"

#include "PointReader.h"
#include "EuclideanPointReader.h"
#include "GeodesicPointReader.h"

namespace point_reader {
    std::unique_ptr<PointReader> point_reader_factory(EdgeWeightType edge_weight_type,
                                                      std::fstream& file, size_t dimension) {
        switch (edge_weight_type) {
        case EdgeWeightType::EUC_2D:
            return std::make_unique<EuclideanPointReader>(file, dimension);
        case EdgeWeightType::GEO:
            return std::make_unique<GeodesicPointReader>(file, dimension);
        }
    }
}
