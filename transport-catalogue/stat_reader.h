#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

void ParseAndPrintStat(const Transport::Catalogue& transport_catalogue, std::string_view request,
    std::ostream& output);

void ProcessRequestsInfo(std::istream& input, Transport::Catalogue& catalogue,
    std::ostream& output);