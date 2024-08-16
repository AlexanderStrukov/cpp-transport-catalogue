#include "json_reader.h"
#include "transport_catalogue.h"
#include "request_handler.h"

int main() {

    Transport::Catalogue catalogue;

    JsonReader json_reader(std::cin);
    json_reader.FillCatalogue(catalogue);

    const auto& stat_requests = json_reader.ReadStatRequests();
    const auto& render_settings = json_reader.ReadRenderSettings().AsMap();
    const auto& renderer = json_reader.GetRenderSettings(render_settings);

    RequestProcessor processor(catalogue, renderer);
    //processor.RenderMap().Render(std::cout);
    processor.ProcessRequests(stat_requests);
}