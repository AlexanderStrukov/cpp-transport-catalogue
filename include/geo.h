#pragma once

#include <cmath>

namespace geo {

    const double RAD_EARTH = 6371000.0;
    const double GRAD_2_RAD = 3.1415926535 / 180.;

    struct Coordinates {
        double lat;
        double lng;

        bool operator==(const Coordinates& other) const {
            return lat == other.lat && lng == other.lng;
        }

        bool operator!=(const Coordinates& other) const {
            return !(*this == other);
        }
    };

    double ComputeDistance(Coordinates from, Coordinates to);

}