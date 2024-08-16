#include "json_reader.h"
#include "transport_catalogue.h"
#include "request_handler.h"

int main() {

    Transport::Catalogue catalogue;

    JsonReader requests(std::cin);
    requests.FillCatalogue(catalogue);
    RequestProcessor(requests, catalogue);

}