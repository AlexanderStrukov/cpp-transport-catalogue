#include "map_renderer.h"

namespace renderer {

	bool IsZero(double value) {
		return std::abs(value) < EPSILON;
	}
	
	std::vector<svg::Polyline> MapRenderer::GetBusRouteLines(const std::map<std::string_view, const Transport::Bus*>& buses_by_names, const SphereProjector& projector) const {
		std::vector<svg::Polyline> lines_route;

		size_t color_index = 0;
		for (const auto& [name, bus] : buses_by_names) {
			if (bus->stops.empty())
				continue;

			std::vector<const Transport::Stop*> stops{ bus->stops.begin(), bus->stops.end() };
			if (!bus->is_circle) {
				stops.insert(stops.end(), std::next(bus->stops.rbegin()), bus->stops.rend());
			}

			svg::Polyline line;
			for (const auto& stop : stops) {
				line.AddPoint(projector(stop->coords));
			}
			line.SetStrokeWidth(render_settings_.line_width);
			line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			line.SetFillColor("none");
			line.SetStrokeColor(render_settings_.color_palette[color_index]);

			if (color_index < render_settings_.color_palette.size() - 1)
				++color_index;
			else
				color_index = 0;

			lines_route.push_back(line);
		}

		return lines_route;
	}

	std::vector<svg::Text> MapRenderer::GetBusLabels(const std::map<std::string_view, const Transport::Bus*>& buses_by_names, const SphereProjector& projector) const {
		std::vector<svg::Text> bus_labels;

		size_t color_index = 0;
		for (const auto& [name, bus] : buses_by_names) {
			if (bus->stops.empty())
				continue;

			svg::Text text;
			text.SetPosition(projector(bus->stops[0]->coords));
			text.SetOffset(render_settings_.bus_label_offset);
			text.SetFontSize(render_settings_.bus_label_font_size);
			text.SetFontFamily("Verdana");
			text.SetFontWeight("bold");
			text.SetData(bus->name);
			text.SetFillColor(render_settings_.color_palette[color_index]);

			if (color_index < render_settings_.color_palette.size() - 1)
				++color_index;
			else
				color_index = 0;

			svg::Text underlayer;
			underlayer.SetPosition(projector(bus->stops[0]->coords));
			underlayer.SetOffset(render_settings_.bus_label_offset);
			underlayer.SetFontSize(render_settings_.bus_label_font_size);
			underlayer.SetFontFamily("Verdana");
			underlayer.SetFontWeight("bold");
			underlayer.SetData(bus->name);
			underlayer.SetFillColor(render_settings_.underlayer_color);
			underlayer.SetStrokeColor(render_settings_.underlayer_color);
			underlayer.SetStrokeWidth(render_settings_.underlayer_width);
			underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

			bus_labels.push_back(underlayer);
			bus_labels.push_back(text);

			if (!bus->is_circle && bus->stops[0] != bus->stops[bus->stops.size() - 1]) {
				svg::Text text_opt { text };
				text_opt.SetPosition(projector(bus->stops[bus->stops.size() - 1]->coords));
				
				svg::Text underlayer_opt { underlayer };
				underlayer_opt.SetPosition(projector(bus->stops[bus->stops.size() - 1]->coords));

				bus_labels.push_back(underlayer_opt);
				bus_labels.push_back(text_opt);
			}
		}

		return bus_labels;
	}

	std::vector<svg::Circle> MapRenderer::GetStopsSymbols(const std::map<std::string_view, const Transport::Stop*>& stops_by_names, const SphereProjector& projector) const {
		std::vector<svg::Circle> stops_symbols;

		for (const auto& [name, stop] : stops_by_names) {
			svg::Circle symbol;
			symbol.SetCenter(projector(stop->coords));
			symbol.SetRadius(render_settings_.stop_radius);
			symbol.SetFillColor("white");

			stops_symbols.push_back(std::move(symbol));
		}

		return stops_symbols;
	}

	std::vector<svg::Text> MapRenderer::GetStopsLabels(const std::map<std::string_view, const Transport::Stop*>& stops_by_names, const SphereProjector& projector) const {
		std::vector<svg::Text> stops_labels;

		for (const auto& [name, stop] : stops_by_names) {
			svg::Text text;
			text.SetPosition(projector(stop->coords));
			text.SetOffset(render_settings_.stop_label_offset);
			text.SetFontSize(render_settings_.stop_label_font_size);
			text.SetFontFamily("Verdana");
			text.SetData(stop->name);
			text.SetFillColor("black");

			svg::Text underlayer;
			underlayer.SetPosition(projector(stop->coords));
			underlayer.SetOffset(render_settings_.stop_label_offset);
			underlayer.SetFontSize(render_settings_.stop_label_font_size);
			underlayer.SetFontFamily("Verdana");
			underlayer.SetData(stop->name);
			underlayer.SetFillColor(render_settings_.underlayer_color);
			underlayer.SetStrokeColor(render_settings_.underlayer_color);
			underlayer.SetStrokeWidth(render_settings_.underlayer_width);
			underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

			stops_labels.push_back(std::move(underlayer));
			stops_labels.push_back(std::move(text));
		}

		return stops_labels;
	}

	svg::Document MapRenderer::GetDataSVG(const std::map<std::string_view, const Transport::Bus*>& buses_by_names) const {
		svg::Document dataSVG;

		std::vector<geo::Coordinates> bus_stops_coords;
		std::map<std::string_view, const Transport::Stop*> stops_by_names;
		for (const auto& [name, bus] : buses_by_names) {
			for (const auto& stop : bus->stops) {
				bus_stops_coords.push_back(stop->coords);
				stops_by_names[stop->name] = stop;
			}
		}

		SphereProjector projector(bus_stops_coords.begin(), bus_stops_coords.end(),
								render_settings_.width, render_settings_.height, render_settings_.padding);
		for (const auto& line : GetBusRouteLines(buses_by_names, projector)) {
			dataSVG.Add(std::move(line));
		}

		for (const auto& text : GetBusLabels(buses_by_names, projector)) {
			dataSVG.Add(std::move(text));
		}

		for (const auto& circle : GetStopsSymbols(stops_by_names, projector)) {
			dataSVG.Add(std::move(circle));
		}

		for (const auto& text : GetStopsLabels(stops_by_names, projector)) {
			dataSVG.Add(std::move(text));
		}

		return dataSVG;
	}

} // пространство имен renderer