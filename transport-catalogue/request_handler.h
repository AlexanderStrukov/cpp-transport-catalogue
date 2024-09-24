#pragma once

#include "json_reader.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "json_builder.h"

#include <sstream>

class RequestProcessor {
public:

    struct RouteItemParser {

        json::Node operator()(const Transport::WaitingOnStop& route_item) {
            using namespace std::literals;

            return json::Builder{}.StartDict()
                                    .Key("type"s).Value("Wait"s)
                                    .Key("stop_name"s).Value(route_item.stop_name)
                                    .Key("time"s).Value(route_item.wait_time)
                                  .EndDict().Build();
        }

        json::Node operator()(const Transport::RidingOnBus& route_item) {
            using namespace std::literals;

            return json::Builder{}.StartDict()
                                    .Key("type"s).Value("Bus"s)
                                    .Key("bus"s).Value(route_item.bus_id)
                                    .Key("span_count"s).Value(static_cast<int>(route_item.span_count))
                                    .Key("time"s).Value(route_item.time)
                                  .EndDict().Build();
        }

    };

    RequestProcessor(const Transport::Catalogue& catalogue, const renderer::MapRenderer& renderer, const Transport::RoutingSettings& settings)
        : catalogue_(catalogue), renderer_(renderer), router_(Transport::RouteProcessor(settings, catalogue)) { }

    void ProcessRequests(const json::Node& stat_requests) const;

    svg::Document RenderMap() const;

private:
    const json::Node ProcessStopData(const json::Dict& data_as_map) const;
    const json::Node ProcessBusData(const json::Dict& data_as_map) const;
    const json::Node ProcessMap(const json::Dict& data_as_map) const;
    const json::Node ProcessRoute(const json::Dict& data_as_map) const;

    const Transport::Catalogue& catalogue_;
    const renderer::MapRenderer& renderer_;
    const Transport::RouteProcessor router_;
};