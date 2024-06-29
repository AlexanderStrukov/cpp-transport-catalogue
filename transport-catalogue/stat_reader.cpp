#include "stat_reader.h"

#include <iomanip>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iostream>

using Transport::Bus, Transport::Stop;

void ParseAndPrintStat(const Transport::Catalogue& transport_catalogue, std::string_view request,
    std::ostream& output) {
    std::string_view type_of_request = request.substr(0, request.find_first_of(' '));
    if (type_of_request == "Bus") {
        auto* bus = transport_catalogue.GetBus(std::string(request).substr(request.find_first_of(' ') + 1));
        if (!bus) {
            output << request << ": not found" << std::endl;
        }
        else {
            output << std::setprecision(6);
            output << request << ": " << bus->stops.size() << " stops on route, "
                << (transport_catalogue.GetUniqueStops(*bus)).size() << " unique stops, " << transport_catalogue.GetDistance(*bus) << " route length" << std::endl;
        }
    }
    else {
        std::string name = std::string(request).substr(request.find_first_of(' ') + 1);
        std::unordered_map<std::string, std::vector<const Bus*>> buses_from_stop = transport_catalogue.GetBusesFromStop();

        if (!buses_from_stop.count(name)) {
            output << request << ": not found" << std::endl;
        }
        else {
            std::vector<std::string> buses;
            for (auto bus : buses_from_stop[name]) {
                buses.push_back(std::move(bus->name));
            }
            auto last = std::unique(buses.begin(), buses.end());
            buses.erase(last, buses.end());
            std::sort(buses.begin(), buses.end());

            if (buses.size() == 0) {
                output << request << ": no buses" << std::endl;
            }
            else {
                output << request << ": buses ";
                for (auto& bus : buses) {
                    output << bus << " ";
                }
                output << std::endl;
            }
        }
    }
}