/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

#include "json_reader.h"

const json::Node& JsonReader::ReadBaseRequests() const {
	return input_.GetRoot().AsMap().at("base_requests");
}

const json::Node& JsonReader::ReadStatRequests() const {
	return input_.GetRoot().AsMap().at("stat_requests");
}

void JsonReader::FillCatalogue(Transport::Catalogue& catalogue) {
	using namespace std::literals;

	const json::Array& input_data = ReadBaseRequests().AsArray();

	// массив для сохранения данных о расстояниях между остановками
	std::vector<std::tuple<std::string_view, std::string_view, int>> distances_data;

	// обработка данных об остановках
	for (auto& stop_request : input_data) {
		const auto& data_as_map = stop_request.AsMap();
		const auto& type = data_as_map.at("type").AsString();

		if (type == "Stop"s) {
			const std::string& stop_name = data_as_map.at("name").AsString();
			geo::Coordinates coords = { data_as_map.at("latitude").AsDouble(), data_as_map.at("longitude").AsDouble() };

			catalogue.AddStop(stop_name, coords);
			for (auto& [stop_to, distance] : data_as_map.at("road_distances").AsMap()) {
				distances_data.push_back({ stop_name, stop_to, distance.AsInt() });
			}
		}
	}

	// обработка данных о расстояниях
	for (auto& [stop_from, stop_to, distance] : distances_data) {
		catalogue.AddDistance({stop_from, stop_to}, distance);
	}

	// обработка данных о маршрутах
	for (auto& bus_request : input_data) {
		const auto& data_as_map = bus_request.AsMap();
		const auto& type = data_as_map.at("type").AsString();

		if (type == "Bus"s) {
			std::string bus_name = data_as_map.at("name").AsString();
			bool is_roundtrip = data_as_map.at("is_roundtrip").AsBool();

			std::vector<std::string_view> stops_names;
			for (auto& stop_name : data_as_map.at("stops").AsArray()) {
				stops_names.emplace_back(stop_name.AsString());
			}

			catalogue.AddBus(bus_name, stops_names, is_roundtrip);
		}
	}
}