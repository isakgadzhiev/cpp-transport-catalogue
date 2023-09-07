#include <iostream>

#include "json_reader.h"
#include "map_renderer.h"
#include "transport_router.h"

[[maybe_unused]] void Tests(transport_catalogue::TransportCatalogue& catalogue) {
    using namespace transport_catalogue;
    catalogue.TestGetStopNames();
    catalogue.TestGetBusNames();
    catalogue.TestGetDistancesBetweenStops();
}

int main() {
    transport_catalogue::TransportCatalogue catalogue;
    renderer::MapRenderer map_renderer;
    transport_router::TransportRouter router(catalogue);
    json_reader::GetJsonRequest(catalogue, map_renderer, router, std::cin, std::cout);
}