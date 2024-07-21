#pragma once

#include <cmath>

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

inline double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;

    if (from == to) {
        return 0;
    }

    return acos(sin(from.lat * GRAD_2_RAD) * sin(to.lat * GRAD_2_RAD)
        + cos(from.lat * GRAD_2_RAD) * cos(to.lat * GRAD_2_RAD) * cos(abs(from.lng - to.lng) * GRAD_2_RAD))
        * RAD_EARTH;
}