#pragma once

#include "geo.h"

#include <vector>
#include <set>
#include <string>

namespace Transport {

    struct Stop {
        std::string name;
        geo::Coordinates coords;
    };

    struct Bus {
        std::string name;
        std::vector<const Stop*> stops;
        bool is_circle;
    };

    struct BusData {
        int count_stops = 0;
        int unique_stops = 0;
        double real_distance = 0.0;
        double geo_distance = 0.0;
    };

} // namespace Transport