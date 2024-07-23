#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <iostream>

using namespace std::literals;
using namespace InputData;

Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return { nan, nan };
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return { lat, lng };
}

std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return { std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1)) };
}

/*
 * Парсит строку вида "3700m to Narushkino" и возвращает вектор пар (расстояние, назначение)
 */
std::unordered_map<std::string, int> ParseDistance(std::string_view data) {
    using namespace std::string_view_literals;
    auto delim = "m to "sv;

    std::unordered_map<std::string, int> distances;
    auto parts = Split(data, ',');

    for (auto& part : parts) {
        auto delim_pos = part.find(delim);

        if (delim_pos == std::string::npos) {
            continue;
        }

        auto distance = std::stoi(std::string(part.substr(0, delim_pos)));
        auto dest = part.substr(delim_pos + delim.size());
        distances[std::string(dest)] = distance;
    }

    return distances;
}

void Reader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void Reader::ApplyCommands([[maybe_unused]] Transport::Catalogue& catalogue) const {
    for (const auto& command : commands_) {
        if (command.command == "Stop") {
            Coordinates coords = ParseCoordinates(command.description);
            catalogue.AddStop(command.id, coords);
        }
    }

    for (const auto& command : commands_) {
        std::unordered_map<std::string, int> distances = ParseDistance(command.description);
        for (auto [name, distance] : distances) {
            catalogue.AddDistance(std::pair(command.id, name), distance);
        }
    }

    for (const auto& command : commands_) {
        if (command.command == "Bus") {
            std::vector<std::string_view> stops = ParseRoute(command.description);
            catalogue.AddBus(command.id, stops);
        }
    }
}

void InputData::ProcessCommands(std::istream& input, Transport::Catalogue& catalogue) {
    int base_request_count;
    input >> base_request_count >> std::ws;

    Reader reader;
    for (int i = 0; i < base_request_count; ++i) {
        std::string line;
        std::getline(input, line);
        reader.ParseLine(line);
    }

    reader.ApplyCommands(catalogue);
}