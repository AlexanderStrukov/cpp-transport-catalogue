#pragma once
#include <unordered_map>
#include <deque>
#include <string>
#include <vector>
#include "geo.h"
#include <unordered_set>

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
        double distance = 0.0;
    };

    class Catalogue {

    public:

        void AddStop(const std::string&, Coordinates);

        void AddBus(const std::string&, const std::vector<std::string_view>);

        const Stop* GetStop(std::string) const;

        const Bus* GetBus(std::string) const;

        BusData GetBusData(const Bus&) const;

        std::vector<std::string> GetBusesForStop(std::string stop_name) const;

    private:

        std::deque<Stop> stops_;
        std::deque<Bus> buses_;

        std::unordered_map<std::string, const Stop*> stops_by_names_;
        std::unordered_map<std::string, const Bus*> buses_by_names_;

        std::unordered_map<std::string, std::vector<const Bus*>> buses_for_stop_;
    };

}