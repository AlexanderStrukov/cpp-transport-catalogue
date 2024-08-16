#pragma once

#include <unordered_map>
#include <deque>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_set>
#include <set>
#include <utility>
#include <optional>

#include "geo.h"

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

    struct Hasher {
        static const size_t coef = 67;

        static auto StopHash(const Stop* stop) {
            return std::hash<std::string>{}(stop->name) +
                std::hash<double>{}(stop->coords.lat)* coef +
                std::hash<double>{}(stop->coords.lng)* coef* coef;
        }

        size_t operator()(const std::pair<const Stop*, const Stop*>& stops) const {
            return StopHash(stops.first) * coef + StopHash(stops.second);
        }

    };

    class Catalogue {

    public:

        void AddStop(const std::string& stop_name, geo::Coordinates coords);

        void AddBus(const std::string& bus_name, const std::vector<std::string_view> stops_names, bool is_roundtrip);

        void AddDistance(const std::pair<std::string_view, std::string_view> stops, int distance);

        const Stop* GetStop(std::string_view stop_name) const;

        const Bus* GetBus(std::string_view bus_name) const;

        std::vector<std::string> GetBusesForStop(std::string_view stop_name) const;

        BusData GetBusData(const std::string_view bus_name) const;

        double GetDistanceBetweenStops(const std::pair<const Stop*, const Stop*> stops) const;

    private:

        std::deque<Stop> stops_;
        std::deque<Bus> buses_;

        std::unordered_map<std::string, const Stop*> stops_by_names_;
        std::unordered_map<std::string, const Bus*> buses_by_names_;

        std::unordered_map<std::string, std::set<std::string_view>> buses_for_stop_;
        std::unordered_map<std::pair<const Stop*, const Stop*>, int, Hasher> distances_;
    };

}