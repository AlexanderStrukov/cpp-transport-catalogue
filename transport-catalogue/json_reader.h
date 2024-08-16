#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

#include <iostream>

class JsonReader {
public:
    JsonReader(std::istream& input)
        : input_(json::Load(input)) {}

    const json::Node& ReadBaseRequests() const;
    const json::Node& ReadStatRequests() const;
    const json::Node& ReadRenderSettings() const;

    renderer::MapRenderer GetRenderSettings(const json::Dict& data_as_map) const;

    void FillCatalogue(Transport::Catalogue& catalogue);

private:
    json::Document input_;
    json::Node null_ = nullptr;
};