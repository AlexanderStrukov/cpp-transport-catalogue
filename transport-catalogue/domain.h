#pragma once

#include "geo.h"

#include <vector>
#include <set>
#include <string>
#include <variant>

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

    struct RoutingSettings {
        double bus_wait_time = 0.0;
        double bus_velocity = 0.0;
    };

    struct WaitingOnStop {
        std::string stop_name;
        double wait_time = 0.0;
    };

    struct RidingOnBus {
        std::string bus_id;
        size_t span_count = 0;
        double time = 0.0;
    };

    struct RouteData {
        double total_time;
        std::vector<std::variant<WaitingOnStop, RidingOnBus>> route_items;
    };

} // namespace Transport