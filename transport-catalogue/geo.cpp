#define _USE_MATH_DEFINES
#include "geo.h"

#include <cmath>

namespace geo {

    double ComputeDistance(Coordinates from, Coordinates to) {
        using namespace std;

        if (from == to) {
            return 0;
        }

        return acos(sin(from.lat * GRAD_2_RAD) * sin(to.lat * GRAD_2_RAD)
            + cos(from.lat * GRAD_2_RAD) * cos(to.lat * GRAD_2_RAD) * cos(abs(from.lng - to.lng) * GRAD_2_RAD))
            * RAD_EARTH;
    }

}  // namespace geo