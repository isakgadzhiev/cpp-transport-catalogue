#include <iomanip>

#include "stat_reader.h"

namespace output {
    using namespace std;
    using namespace literals;
    using namespace input_reader;
    using namespace transport_catalogue;

    void GetOutputRequest(TransportCatalogue& catalogue, istream& input, ostream& output) {
        int request_count = detail_input::ReadLineWithNumber(input);
        string request;
        for(int i = 0; i < request_count; ++i) {
            getline(input, request);
            RequestType request_type = GetRequestType(std::move(request));
            if(request_type == RequestType::BUS) {
                GetBusInfoForOutput(catalogue, std::move(request), output);
            } else {
                GetStopInfoForOutput(catalogue, std::move(request), output);
            }
        }
    }

    void GetBusInfoForOutput(TransportCatalogue& catalogue, const string_view& request, ostream& output) {
        string bus_name = GetNameFromRequest(std::move(request));
        output << "Bus "s << bus_name << ": "s;
        const Bus* bus = catalogue.FindBus(std::move(bus_name));
        if (const auto bus_info = catalogue.GetBusInfo(bus); !bus_info.has_value()) {
            output << "not found"s << endl;
        } else {
            output << std::setprecision(6) <<
                   bus_info->stops_count << " stops on route, "s <<
                   bus_info->unique_stops_count << " unique stops, "s <<
                   bus_info->route_length << " route length, "s <<
                   bus_info->curvature << " curvature"s << endl;
        }
    }

    void GetStopInfoForOutput(TransportCatalogue& catalogue, const string_view& request, ostream& output) {
        string stop_name = GetNameFromRequest(std::move(request));
        output << "Stop "s << stop_name << ": "s;
        if (const auto stop_info = catalogue.GetStopInfo(stop_name); !stop_info.has_value()) {
            output << "not found"s;
        } else {
            if (stop_info->buses.empty()) {
                output << "no buses"s;
            } else {
                output << "buses";
                for (const auto bus : stop_info->buses) {
                    output << " " << bus;
                }
            }
        }
        output << endl;
    }
}