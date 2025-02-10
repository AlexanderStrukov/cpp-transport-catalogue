#pragma once

#include "domain.h"
#include "transport_catalogue.h"
#include "router.h"

#include <memory>
#include <optional>

namespace Transport {

	using Graph = graph::DirectedWeightedGraph<double>;

	const short KILOMETER_2_METER = 1000;
	const short HOUR_2_MIN = 60;

	struct StopVertexIds {
		graph::VertexId start = 0;
		graph::VertexId finish = 0;
	};

	class RouteProcessor {
	public:
		RouteProcessor() = default;

		RouteProcessor(const Transport::RoutingSettings& settings, const Transport::Catalogue& catalogue)
			: settings_(settings), catalogue_(catalogue) { BuildRouter(); }

		std::optional<RouteData> BuildRouteBetweenStops(const std::string& from, const std::string& to) const;

	private:

		void BuildRouter();

		void AddStopEdgesToGraph(const std::vector<const Stop*> stops_ptr);

		void AddBusEdgesToGraph(std::vector<const Stop*> stops_ptr, const Bus* bus_ptr) {

			for (size_t i = 0; i != stops_ptr.size(); ++i) {
				double distance = 0.0;
				double distance_reverse = 0.0;
				size_t span_count = 0;

				for (size_t j = i + 1; j != stops_ptr.size(); ++j) {
					distance += catalogue_.GetDistanceBetweenStops({ stops_ptr[j - 1], stops_ptr[j] });
					distance_reverse += catalogue_.GetDistanceBetweenStops({ stops_ptr[j], stops_ptr[j - 1] });
					++span_count;

					StopVertexIds from = stops_by_id_.at(stops_ptr[i]->name);
					StopVertexIds to = stops_by_id_.at(stops_ptr[j]->name);
					graph::EdgeId id = graph_->AddEdge({ from.finish, to.start, distance / (settings_.bus_velocity * KILOMETER_2_METER / HOUR_2_MIN) });

					items_of_graph_[id] = RidingOnBus{ bus_ptr->name, span_count, graph_->GetEdge(id).weight };

					if (!bus_ptr->is_circle) {
						graph::EdgeId id_new = graph_->AddEdge({ to.finish, from.start, distance_reverse / (settings_.bus_velocity * KILOMETER_2_METER / HOUR_2_MIN) });

						items_of_graph_[id_new] = RidingOnBus{ bus_ptr->name, span_count, graph_->GetEdge(id_new).weight };
					}
				}
			}
		}

		const Transport::RoutingSettings settings_;

		std::unique_ptr<Graph> graph_;
		std::unique_ptr<graph::Router<double>> router_ptr_;

		std::map<const std::string, StopVertexIds> stops_by_id_;
		std::map<graph::EdgeId, std::variant<WaitingOnStop, RidingOnBus>> items_of_graph_;

		const Transport::Catalogue& catalogue_;

	};

}