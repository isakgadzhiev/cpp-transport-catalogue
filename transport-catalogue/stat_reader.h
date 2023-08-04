#pragma once

#include "input_reader.h"
#include "transport_catalogue.h"

namespace output {
    void GetOutputRequest(transport_catalogue::TransportCatalogue& catalogue, std::istream& input, std::ostream& output);
    void GetBusInfoForOutput(transport_catalogue::TransportCatalogue& catalogue, const std::string_view& request, std::ostream& output);
    void GetStopInfoForOutput(transport_catalogue::TransportCatalogue& catalogue, const std::string_view& request, std::ostream& output);
}