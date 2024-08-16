#include "request_handler.h"

void RequestProcessor::ProcessRequests(const json::Node& stat_requests) const {
	using namespace std::literals;

	json::Array processed_data;

	for (const auto& request : stat_requests.AsArray()) {
		const auto& data_as_map = request.AsMap();
		const auto& type = data_as_map.at("type").AsString();

		if (type == "Stop"s) {
			processed_data.emplace_back(ProcessStopData(data_as_map).AsMap());
		}
		else if (type == "Bus"s)
		{
			processed_data.emplace_back(ProcessBusData(data_as_map).AsMap());
		}
		else if (type == "Map"s)
		{
			processed_data.emplace_back(ProcessMap(data_as_map).AsMap());
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

const json::Node RequestProcessor::ProcessMap(const json::Dict& request_map) const {
	json::Dict result;

	result["request_id"] = request_map.at("id").AsInt();
	std::ostringstream out;
	svg::Document map = RenderMap();
	map.Render(out);
	result["map"] = out.str();

	return json::Node{ result };
}

svg::Document RequestProcessor::RenderMap() const {
	return renderer_.GetDataSVG(catalogue_.GetBusesSortedByNames());
}