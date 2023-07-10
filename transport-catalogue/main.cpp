#include <iostream>

#include "json_reader.h"
#include "map_renderer.h"

[[maybe_unused]] void Tests(transport_catalogue::TransportCatalogue& catalogue) {
    using namespace transport_catalogue;
    catalogue.TestGetStopNames();
    catalogue.TestGetBusNames();
    catalogue.TestGetDistancesBetweenStops();
}

int main() {
    transport_catalogue::TransportCatalogue catalogue;
    renderer::MapRenderer map_renderer;
    request_handler::RequestHandler request_handler(catalogue, map_renderer);
    json_reader::GetJsonRequest(catalogue, map_renderer, request_handler, std::cin, std::cout);
}