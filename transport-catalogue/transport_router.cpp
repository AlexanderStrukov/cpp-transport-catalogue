#include "transport_router.h"

namespace Transport {

	void RouteProcessor::BuildRouter() {
		std::vector<const Stop*> stops_ptr = catalogue_.GetStopsPtr();
		std::vector<const Bus*> buses_ptr = catalogue_.GetBusesPtr();

		graph_ = std::make_unique<Graph>(stops_ptr.size() * 2);

		AddStopEdgesToGraph(stops_ptr);

		for (auto bus_ptr : buses_ptr) {
			
			AddBusEdgesToGraph(bus_ptr->stops, bus_ptr);

		}

		router_ptr_ = std::make_unique<graph::Router<double>>(*graph_);
	}

	std::optional<RouteData> RouteProcessor::BuildRouteBetweenStops(const std::string& from, const std::string& to) const {

		if (!stops_by_id_.count(from) || !stops_by_id_.count(to)) {
			return std::nullopt;
		}

		const auto& route = router_ptr_->BuildRoute(stops_by_id_.at(from).start, stops_by_id_.at(to).start);
		if (!route) {
			return std::nullopt;
		}

		RouteData route_data;
		for (const auto item : route->edges) {
			route_data.route_items.emplace_back(items_of_graph_.at(item));
		}
		route_data.total_time = route->weight;

		return route_data;
	}

	void RouteProcessor::AddStopEdgesToGraph(const std::vector<const Stop*> stops_ptr) {
		graph::VertexId vertex_id = 0;
		for (auto stop_ptr : stops_ptr) {

			StopVertexIds stop_ids = { vertex_id, ++vertex_id };
			stops_by_id_[stop_ptr->name] = stop_ids;

			graph::EdgeId id = graph_->AddEdge({ stop_ids.start, stop_ids.finish, settings_.bus_wait_time });
			items_of_graph_[id] = WaitingOnStop{ stop_ptr->name, settings_.bus_wait_time };
			++vertex_id;
		}
	}

}