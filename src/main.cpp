#include "json_reader.h"
#include "transport_catalogue.h"
#include "request_handler.h"

int main() {

    Transport::Catalogue catalogue;

    JsonReader json_reader(std::cin);
    json_reader.FillCatalogue(catalogue);

    const auto& stat_requests = json_reader.ReadStatRequests();

    const auto& render_settings = json_reader.ReadRenderSettings().AsDict();
    const auto& renderer = json_reader.GetRenderSettings(render_settings);

    const auto& routing_settings = json_reader.ReadRoutingSettings().AsDict();
    const auto& settings = json_reader.GetRoutingSettings(routing_settings);

    RequestProcessor processor(catalogue, renderer, settings);
    processor.ProcessRequests(stat_requests);
}