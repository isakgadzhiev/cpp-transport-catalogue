#include "request_handler.h"

namespace request_handler {
    std::optional<BusInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
        const Bus* bus = catalogue_.FindBus(bus_name);
        if (bus) {
            return catalogue_.GetBusInfo(bus);
        } else {
            return {};
        }
    }

    std::optional<StopInfo> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
        return catalogue_.GetStopInfo(stop_name);
    }

    void RequestHandler::AddOutputRequest(const json::Node& output_request) {
        output_request_handler_.emplace_back(output_request);
    }

    void RequestHandler::PrintOutputRequests(std::ostream& output) {
        if (!output_request_handler_.empty()) {
            json::Print(json::Document{output_request_handler_}, output);
            output << std::endl;
        } else {
            return;
        }
    }

    svg::Document RequestHandler::RenderMap() const {
        return renderer_.AddRoutesOnMap(catalogue_.GetRouteNames());
    }

    RequestHandler::RouteInfo RequestHandler::GetRouteInfo(const std::string_view from,
                                                           const std::string_view to) const {
        return router_.GetRouteInfo(catalogue_.FindStop(from), catalogue_.FindStop(to));
    }
}