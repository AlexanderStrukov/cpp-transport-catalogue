#pragma once
#include <unordered_map>
#include <deque>
#include <string>
#include <vector>
#include "geo.h"
#include <unordered_set>
#include <utility>

namespace Transport {

    struct Stop {
        std::string name;
        Coordinates coords;
    };

    struct Bus {
        std::string name;
        std::vector<const Stop*> stops;
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
                std::hash<double>{}(stop->coords.lat) * coef +
                std::hash<double>{}(stop->coords.lng) * coef * coef;
        }

        size_t operator()(const std::pair<const Stop*, const Stop*>& stops) const {
            return StopHash(stops.first) * coef + StopHash(stops.second);
        }

    };

    class Catalogue {

    public:

        void AddStop(const std::string&, Coordinates);

        void AddBus(const std::string&, const std::vector<std::string_view>);

        void AddDistance(const std::string&, std::unordered_map<std::string, int>);

        const Stop* GetStop(std::string) const;

        const Bus* GetBus(std::string) const;

        double GetDistanceBetweenStops(const std::pair<const Stop*, const Stop*>) const;

        BusData GetBusData(const Bus&) const;

        std::vector<std::string> GetBusesForStop(std::string stop_name) const;

        double GetDistanceOfRoute(const std::string&) const;

    private:

        std::deque<Stop> stops_;
        std::deque<Bus> buses_;

        std::unordered_map<std::string, const Stop*> stops_by_names_;
        std::unordered_map<std::string, const Bus*> buses_by_names_;

        std::unordered_map<std::string, std::vector<const Bus*>> buses_for_stop_;
        std::unordered_map<std::pair<const Stop*, const Stop*>, int, Hasher> distances_;
    };

}