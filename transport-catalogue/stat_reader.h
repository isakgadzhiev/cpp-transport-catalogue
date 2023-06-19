#pragma once

#include "input_reader.h"
#include "transport_catalogue.h"

namespace output {
    void GetOutputRequest(transport_catalogue::TransportCatalogue& catalogue, std::istream& input);
    void GetBusInfoForOutput(transport_catalogue::TransportCatalogue& catalogue, const std::string_view& request);
    void GetStopInfoForOutput(transport_catalogue::TransportCatalogue& catalogue, const std::string_view& request);
}