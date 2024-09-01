#include "request_handler.h"

void RequestProcessor::ProcessRequests(const json::Node& stat_requests) const {
	using namespace std::literals;

	json::Array processed_data;

	for (const auto& request : stat_requests.AsArray()) {
		const auto& data_as_map = request.AsDict();
		const auto& type = data_as_map.at("type").AsString();

		if (type == "Stop"s) {
			processed_data.emplace_back(ProcessStopData(data_as_map).AsDict());
		}
		else if (type == "Bus"s)
		{
			processed_data.emplace_back(ProcessBusData(data_as_map).AsDict());
		}
		else if (type == "Map"s)
		{
			processed_data.emplace_back(ProcessMap(data_as_map).AsDict());
		}
	}

	json::Print(json::Document{processed_data}, std::cout);
}

const json::Node RequestProcessor::ProcessStopData(const json::Dict& data_as_map) const {
	using namespace std::literals;

	json::Builder builder;
	json::Node result_node;

	const std::string& stop_name = data_as_map.at("name"s).AsString();
	int request_id = data_as_map.at("id"s).AsInt();
	if (!catalogue_.GetStop(stop_name)) {
		builder.StartDict()
					.Key("request_id"s).Value(request_id)
					.Key("error_message"s).Value("not found"s)
				.EndDict();

		result_node = builder.Build();
	}
	else
	{
		builder.StartDict()
					.Key("request_id"s).Value(request_id)
					.Key("buses"s).StartArray();

		for (auto& bus : catalogue_.GetBusesForStop(stop_name)) {
			builder.Value(bus);
		}

		builder.EndArray().EndDict();
		result_node = builder.Build();
	}

	return result_node;
}

const json::Node RequestProcessor::ProcessBusData(const json::Dict& data_as_map) const {
	using namespace std::literals;

	json::Builder builder;
	json::Node result_node;

	const std::string& bus_id = data_as_map.at("name"s).AsString();
	int request_id = data_as_map.at("id"s).AsInt();
	if (!catalogue_.GetBus(bus_id)) {
		builder.StartDict()
					.Key("request_id"s).Value(request_id)
					.Key("error_message"s).Value("not found"s)
				.EndDict();

		result_node = builder.Build();
	}
	else
	{
		Transport::BusData bus_data = catalogue_.GetBusData(bus_id);
		builder.StartDict()
					.Key("request_id"s).Value(request_id)
					.Key("stop_count"s).Value(bus_data.count_stops)
					.Key("unique_stop_count"s).Value(bus_data.unique_stops)
					.Key("route_length"s).Value(bus_data.real_distance)
					.Key("curvature"s).Value(bus_data.real_distance / bus_data.geo_distance)
				.EndDict();

		result_node = builder.Build();
	}

	return result_node;
}

const json::Node RequestProcessor::ProcessMap(const json::Dict& request_map) const {
	using namespace std::literals;

	json::Builder builder;
	json::Node result_node;

	int request_id = request_map.at("id").AsInt();
	std::ostringstream out;
	svg::Document map = RenderMap();
	map.Render(out);

	builder.StartDict()
				.Key("request_id"s).Value(request_id)
				.Key("map"s).Value(out.str())
			.EndDict();

	result_node = builder.Build();

	return result_node;
}

svg::Document RequestProcessor::RenderMap() const {
	return renderer_.GetDataSVG(catalogue_.GetBusesSortedByNames());
}