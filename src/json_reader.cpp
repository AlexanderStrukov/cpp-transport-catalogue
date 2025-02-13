#include "json_reader.h"

const json::Node& JsonReader::ReadBaseRequests() const {
	return input_.GetRoot().AsDict().at("base_requests");
}

const json::Node& JsonReader::ReadStatRequests() const {
	return input_.GetRoot().AsDict().at("stat_requests");
}

const json::Node& JsonReader::ReadRenderSettings() const {
	return input_.GetRoot().AsDict().at("render_settings");
}

const json::Node& JsonReader::ReadRoutingSettings() const {
	return input_.GetRoot().AsDict().at("routing_settings");
}

svg::Color JsonReader::GetUnderlayerColor(const json::Dict& data_as_map) const {
	svg::Color color;

	if (data_as_map.at("underlayer_color").IsString())
		color = data_as_map.at("underlayer_color").AsString();
	else if (data_as_map.at("underlayer_color").IsArray())
	{
		const json::Array& underlayer_color = data_as_map.at("underlayer_color").AsArray();
		if (underlayer_color.size() == 3)
			color = svg::Rgb(underlayer_color[0].AsInt(),
				underlayer_color[1].AsInt(),
				underlayer_color[2].AsInt());
		else if (underlayer_color.size() == 4)
			color = svg::Rgba(underlayer_color[0].AsInt(),
				underlayer_color[1].AsInt(),
				underlayer_color[2].AsInt(),
				underlayer_color[3].AsDouble());
		else
			throw json::ParsingError("invalid color type");
	}
	else
		throw json::ParsingError("invalid color");

	return color;
}

std::vector<svg::Color> JsonReader::GetColorPalette(const json::Dict& data_as_map) const {
	std::vector<svg::Color> palette;

	const json::Array color_palette = data_as_map.at("color_palette").AsArray();
	for (const auto& color : color_palette)
	{
		if (color.IsString())
			palette.push_back(color.AsString());
		else if (color.IsArray())
		{
			const json::Array& type_color = color.AsArray();
			if (type_color.size() == 3)
				palette.emplace_back(svg::Rgb(type_color[0].AsInt(), type_color[1].AsInt(), type_color[2].AsInt()));
			else if (type_color.size() == 4)
				palette.emplace_back(svg::Rgba(type_color[0].AsInt(), type_color[1].AsInt(), type_color[2].AsInt(), type_color[3].AsDouble()));
			else
				throw json::ParsingError("invalid color type");
		}
		else
			throw json::ParsingError("invalid color");
	}

	return palette;
}

renderer::MapRenderer JsonReader::GetRenderSettings(const json::Dict& data_as_map) const {
	renderer::RenderSettings settings;

	settings.width = data_as_map.at("width").AsDouble();
	settings.height = data_as_map.at("height").AsDouble();
	settings.padding = data_as_map.at("padding").AsDouble();
	settings.line_width = data_as_map.at("line_width").AsDouble();
	settings.stop_radius = data_as_map.at("stop_radius").AsDouble();
	settings.bus_label_font_size = data_as_map.at("bus_label_font_size").AsInt();
	settings.stop_label_font_size = data_as_map.at("stop_label_font_size").AsInt();
	settings.underlayer_width = data_as_map.at("underlayer_width").AsDouble();

	const json::Array& bus_label_offset = data_as_map.at("bus_label_offset").AsArray();
	settings.bus_label_offset = { bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble() };
	const json::Array& stop_label_offset = data_as_map.at("stop_label_offset").AsArray();
	settings.stop_label_offset = { stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble() };

	settings.underlayer_color = GetUnderlayerColor(data_as_map);
	settings.color_palette = GetColorPalette(data_as_map);

	return settings;
}

Transport::RoutingSettings JsonReader::GetRoutingSettings(const json::Dict& data_as_map) const {
	Transport::RoutingSettings settings;

	settings.bus_wait_time = data_as_map.at("bus_wait_time").AsDouble();
	settings.bus_velocity = data_as_map.at("bus_velocity").AsDouble();

	return settings;
}

void JsonReader::FillStopsData(Transport::Catalogue& catalogue, const json::Array& input_data) {
	using namespace std::literals;

	for (auto& stop_request : input_data) {
		const auto& data_as_map = stop_request.AsDict();
		const auto& type = data_as_map.at("type").AsString();

		if (type == "Stop"s) {
			const std::string& stop_name = data_as_map.at("name").AsString();
			geo::Coordinates coords = { data_as_map.at("latitude").AsDouble(), data_as_map.at("longitude").AsDouble() };

			catalogue.AddStop(stop_name, coords);
		}
	}
}

void JsonReader::FillDistancesData(Transport::Catalogue& catalogue, const json::Array& input_data) {
	using namespace std::literals;

	for (auto& stop_request : input_data) {
		const auto& data_as_map = stop_request.AsDict();
		const auto& type = data_as_map.at("type").AsString();

		if (type == "Stop"s) {
			const std::string& stop_from = data_as_map.at("name").AsString();

			for (auto& [stop_to, distance] : data_as_map.at("road_distances").AsDict()) {
				catalogue.AddDistance({ stop_from, stop_to }, distance.AsInt());
			}
		}
	}
}

void JsonReader::FillBusesData(Transport::Catalogue& catalogue, const json::Array& input_data) {
	using namespace std::literals;

	for (auto& bus_request : input_data) {
		const auto& data_as_map = bus_request.AsDict();
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

void JsonReader::FillCatalogue(Transport::Catalogue& catalogue) {

	const json::Array& input_data = ReadBaseRequests().AsArray();

	// обработка данных об остановках
	FillStopsData(catalogue, input_data);

	// обработка данных о расстояниях
	FillDistancesData(catalogue, input_data);

	// обработка данных о маршрутах
	FillBusesData(catalogue, input_data);
}