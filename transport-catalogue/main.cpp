#include <fstream>
#include <iostream>
#include <string_view>

#include "json_reader.h"
#include "serialization.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        // make base here
        transport_catalogue::TransportCatalogue catalogue;
        renderer::MapRenderer map_renderer;
        transport_router::TransportRouter router(catalogue);
        serialize::Serializer serializer(catalogue, map_renderer, router);
        std::ifstream input_file("input_make.txt");
        json_reader::MakeBaseRequest(catalogue, map_renderer, router, serializer, input_file);
        input_file.close();
        std::cerr << "Success make base" << std::endl;
    } else if (mode == "process_requests"sv) {
        // process requests here
        transport_catalogue::TransportCatalogue catalogue;
        renderer::MapRenderer map_renderer;
        transport_router::TransportRouter router(catalogue);
        serialize::Serializer serializer(catalogue, map_renderer, router);
        std::ifstream input_file("input_process.txt");
        std::ofstream output_file("output.txt");
        json_reader::ProcessRequest(catalogue, map_renderer, router, serializer, input_file, output_file);
        output_file.close();
        input_file.close();
        std::cerr << "Success process requests" << std::endl;
    } else {
        PrintUsage();
        return 1;
    }
}