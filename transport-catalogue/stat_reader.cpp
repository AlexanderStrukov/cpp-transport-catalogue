#include "stat_reader.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <iostream>

using Transport::Bus, Transport::Stop;

void OutputBusData(const Transport::Catalogue& catalogue, std::string_view request,
                    std::ostream& output) {
    auto* bus = catalogue.GetBus(std::string(request).substr(request.find_first_of(' ') + 1));

    if (!bus) {
        output << request << ": not found" << std::endl;
    }
    else {
        output << std::setprecision(6);
        output << request << ": " << bus->stops.size() << " stops on route, "
            << (catalogue.GetUniqueStops(*bus)).size() << " unique stops, "
            << catalogue.GetDistance(*bus) << " route length" << std::endl;
    }
}

void ParseAndPrintStat(const Transport::Catalogue& transport_catalogue, std::string_view request,
    std::ostream& output) {
    std::string_view type_of_request = request.substr(0, request.find_first_of(' '));
    if (type_of_request == "Bus") {
        OutputBusData(transport_catalogue, request, output);
    }
    else {
        auto* stop = transport_catalogue.GetStop(std::string(request).substr(request.find_first_of(' ') + 1));

        if (!stop) {
            output << request << ": not found" << std::endl;
        }
        else {
            std::vector<std::string> buses = transport_catalogue.GetBusesForStop(stop->name);

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

void ProccessingRequestsInfo(std::istream& input, Transport::Catalogue& catalogue, std::ostream& output) {
    int stat_request_count;
    input >> stat_request_count >> std::ws;

    for (int i = 0; i < stat_request_count; ++i) {
        std::string line;
        std::getline(input, line);
        ParseAndPrintStat(catalogue, line, output);
    }
}