#pragma once

#include "json_reader.h"
#include "transport_catalogue.h"
#include "json_builder.h"

#include <sstream>

class RequestProcessor {
public:
    RequestProcessor(const Transport::Catalogue& catalogue, const renderer::MapRenderer& renderer)
        : catalogue_(catalogue), renderer_(renderer) { }

    void ProcessRequests(const json::Node& stat_requests) const;

    svg::Document RenderMap() const;

private:
    const json::Node ProcessStopData(const json::Dict& data_as_map) const;
    const json::Node ProcessBusData(const json::Dict& data_as_map) const;
    const json::Node ProcessMap(const json::Dict& data_as_map) const;

    const Transport::Catalogue& catalogue_;
    const renderer::MapRenderer& renderer_;
};