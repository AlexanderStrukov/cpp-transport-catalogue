/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

#include "request_handler.h"

void RequestProcessor::ProcessRequests() const {
	using namespace std::literals;

	json::Array processed_data;

	const json::Array& stat_requests = reader_.ReadStatRequests().AsArray();
	for (const auto& request : stat_requests) {
		const auto& data_as_map = request.AsMap();
		const auto& type = data_as_map.at("type").AsString();

		if (type == "Stop"s) {
			processed_data.emplace_back(ProcessStopData(data_as_map).AsMap());
		}
		else if (type == "Bus"s)
		{
			processed_data.emplace_back(ProcessBusData(data_as_map).AsMap());
		}
	}

	json::Print(json::Document{processed_data}, std::cout);
}

const json::Node RequestProcessor::ProcessStopData(const json::Dict& data_as_map) const {
	using namespace std::literals;

	json::Dict request_data;

	const std::string& stop_name = data_as_map.at("name").AsString();
	request_data["request_id"] = data_as_map.at("id").AsInt();
	if (!catalogue_.GetStop(stop_name)) {
		request_data["error_message"] = json::Node{"not found"s};
	}
	else
	{
		json::Array buses;
		for (auto& bus : catalogue_.GetBusesForStop(stop_name)) {
			buses.push_back(bus);
		}
		request_data["buses"] = buses;
	}

	return json::Node{ request_data };
}

const json::Node RequestProcessor::ProcessBusData(const json::Dict& data_as_map) const {
	using namespace std::literals;

	json::Dict request_data;

	const std::string& bus_id = data_as_map.at("name").AsString();
	request_data["request_id"] = data_as_map.at("id").AsInt();
	if (!catalogue_.GetBus(bus_id)) {
		request_data["error_message"] = json::Node{"not found"s};
	}
	else
	{
		Transport::BusData bus_data = catalogue_.GetBusData(bus_id);
		request_data["stop_count"] = bus_data.count_stops;
		request_data["unique_stop_count"] = bus_data.unique_stops;
		request_data["route_length"] = bus_data.real_distance;
		request_data["curvature"] = bus_data.real_distance / bus_data.geo_distance;
	}

	return json::Node{ request_data };
}