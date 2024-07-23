#include "transport_catalogue.h"
#include "stat_reader.h"

#include <algorithm>
#include <iostream>
#include <iterator>

namespace TS = Transport;

void TS::Catalogue::AddStop(const std::string& stop_name, Coordinates coords) {
	Stop stop{ stop_name, coords };
	stops_.push_back(std::move(stop));

	const Stop* ptr = &stops_.back();
	stops_by_names_[stop_name] = ptr;
	if (buses_for_stop_.count(stop_name) == 0) {
		buses_for_stop_[stop_name] = {};
	}
}

void TS::Catalogue::AddBus(const std::string& bus_name, const std::vector<std::string_view> stops_names) {
	Bus bus;
	bus.name = bus_name;

	for (const auto& stop_name : stops_names) {
		auto iter = stops_by_names_.find(std::string(stop_name));
		if (iter != stops_by_names_.end()) {
			bus.stops.push_back(iter->second);
		}
	}
	buses_.push_back(std::move(bus));

	const Bus* ptr = &buses_.back();
	buses_by_names_[bus_name] = ptr;
	for (const auto stop : ptr->stops) {
		buses_for_stop_[stop->name].insert(ptr->name);
	}
}

void TS::Catalogue::AddDistance(const std::pair<std::string_view, std::string_view> stops, int distance) {
	const auto stop_from = GetStop(stops.first);
	const auto stop_to = GetStop(stops.second);

	if (stop_from != nullptr && stop_to != nullptr)
		distances_[std::pair(stop_from, stop_to)] = distance;
}

const TS::Stop* TS::Catalogue::GetStop(std::string_view stop_name) const {
	auto iter = stops_by_names_.find(std::string(stop_name));
	return (iter != stops_by_names_.end()) ? iter->second : nullptr;
}

const TS::Bus* TS::Catalogue::GetBus(std::string_view bus_name) const {
	auto iter = buses_by_names_.find(std::string(bus_name));
	return (iter != buses_by_names_.end()) ? iter->second : nullptr;
}

double TS::Catalogue::GetDistanceBetweenStops(const std::pair<const Stop*, const Stop*> stops) const {
	auto iter = distances_.find(stops);

	if (iter == distances_.end()) {
		iter = distances_.find({ stops.second, stops.first });
	}

	return (iter != distances_.end()) ? iter->second : ComputeDistance(stops.first->coords, stops.second->coords);
}

TS::BusData TS::Catalogue::GetBusData(const TS::Bus& bus) const {

	BusData data;
	data.count_stops = static_cast<int>(bus.stops.size());
	data.unique_stops = static_cast<int>(std::unordered_set<const Stop*>(bus.stops.begin(), bus.stops.end()).size());

	if (bus.stops.size() != 0) {
		for (size_t ind = 0; ind != bus.stops.size() - 1; ++ind) {
			data.real_distance += GetDistanceBetweenStops({ bus.stops[ind], bus.stops[ind + 1] });
			data.geo_distance += ComputeDistance(bus.stops[ind]->coords, bus.stops[ind + 1]->coords);
		}
	}

	return data;
}

std::vector<std::string> TS::Catalogue::GetBusesForStop(std::string_view stop_name) const {
	std::vector<std::string> buses;

	for (auto name : buses_for_stop_.at(std::string(stop_name))) {
		buses.push_back(std::move(std::string(name)));
	}
	std::sort(buses.begin(), buses.end());

	return buses;
}