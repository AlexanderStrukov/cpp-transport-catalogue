#include "transport_catalogue.h"

#include <algorithm>
#include <iostream>
#include <iterator>

namespace TS = Transport;

void TS::Catalogue::AddStop(const std::string& stop_name, geo::Coordinates coords) {
	Stop stop{ stop_name, coords };
	stops_.push_back(std::move(stop));

	const Stop* ptr = &stops_.back();
	stops_by_names_[stop_name] = ptr;
	if (buses_for_stop_.count(stop_name) == 0) {
		buses_for_stop_[stop_name] = {};
	}
}

void TS::Catalogue::AddBus(const std::string& bus_name, const std::vector<std::string_view>& stops_names, bool is_roundtrip) {
	Bus bus;
	bus.name = bus_name;
	bus.is_circle = is_roundtrip;

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

std::set<std::string> TS::Catalogue::GetBusesForStop(std::string_view stop_name) const {
	std::set<std::string> buses;

	for (const auto name : buses_for_stop_.at(std::string(stop_name))) {
		buses.emplace(std::string(name));
	}

	return buses;
}

double TS::Catalogue::GetDistanceBetweenStops(const std::pair<const TS::Stop*, const TS::Stop*> stops) const {
	auto iter = distances_.find(stops);

	if (iter == distances_.end()) {
		iter = distances_.find({ stops.second, stops.first });
	}

	return (iter != distances_.end()) ? iter->second : geo::ComputeDistance(stops.first->coords, stops.second->coords);
}

TS::BusData TS::Catalogue::GetBusData(const std::string_view bus_name) const {
	TS::BusData data;

	const TS::Bus* bus = GetBus(bus_name);
	if (!bus)
		throw std::invalid_argument("bus not found");

	data.count_stops = (bus->is_circle) ? static_cast<int>(bus->stops.size())
		: static_cast<int>(bus->stops.size()) * 2 - 1;
	data.unique_stops = static_cast<int>(std::unordered_set<const TS::Stop*>(bus->stops.begin(), bus->stops.end()).size());

	if (bus->stops.size() != 0) {
		for (size_t ind = 0; ind != bus->stops.size() - 1; ++ind) {
			if (bus->is_circle) {
				data.real_distance += GetDistanceBetweenStops({ bus->stops[ind], bus->stops[ind + 1] });
				data.geo_distance += geo::ComputeDistance(bus->stops[ind]->coords, bus->stops[ind + 1]->coords);
			}
			else
			{
				data.real_distance += GetDistanceBetweenStops({ bus->stops[ind], bus->stops[ind + 1] }) +
					GetDistanceBetweenStops({ bus->stops[ind + 1], bus->stops[ind] });
				data.geo_distance += geo::ComputeDistance(bus->stops[ind]->coords, bus->stops[ind + 1]->coords) * 2;
			}
		}
	}

	return data;
}

const std::map<std::string_view, const TS::Bus*> TS::Catalogue::GetBusesSortedByNames() const {
	std::map<std::string_view, const TS::Bus*> buses_by_names;
	for (const auto& [name, bus] : buses_by_names_) {
		buses_by_names[name] = bus;
	}
	return buses_by_names;
}

std::vector<const Transport::Stop*> Transport::Catalogue::GetStopsPtr() const {
	std::vector<const Transport::Stop*> stops_ptr;
	stops_ptr.reserve(stops_by_names_.size());

	for (const auto& [name, stop] : stops_by_names_) {
		stops_ptr.push_back(stop);
	}

	return stops_ptr;
}

std::vector<const Transport::Bus*> Transport::Catalogue::GetBusesPtr() const {
	std::vector<const Transport::Bus*> buses_ptr;
	buses_ptr.reserve(buses_by_names_.size());

	for (const auto& [name, bus] : buses_by_names_) {
		buses_ptr.push_back(bus);
	}

	return buses_ptr;
}