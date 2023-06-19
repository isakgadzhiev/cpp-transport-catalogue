#include <iostream>

#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

using namespace transport_catalogue;

[[maybe_unused]] void Tests(TransportCatalogue& catalogue) {
    using namespace transport_catalogue::tests;
    TestGetStopNames(catalogue);
    TestGetBusNames(catalogue);
    TestGetDistancesBetweenStops(catalogue);
}

int main() {
    TransportCatalogue transport_catalogue;
    input_reader::GetInputRequest(transport_catalogue, std::cin);
    output::GetOutputRequest(transport_catalogue, std::cin);
    //Tests(transport_catalogue);
}