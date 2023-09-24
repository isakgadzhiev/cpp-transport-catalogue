#include "transport_router.h"

namespace transport_router {
    TransportRouter::TransportRouter(const transport_catalogue::TransportCatalogue &catalogue)
            : catalogue_(catalogue) {
    }

    void TransportRouter::SetSettingsAndBuildGraph(const TransportRouter::RouteSettings &r_settings) {
        r_settings_ = r_settings;
        BuildGraph();
    }

    void TransportRouter::BuildGraph() {
        graph_ = std::make_unique<graph::DirectedWeightedGraph<double>>(catalogue_.GetStopsCount() * 2);
        AddVertexesAndWaitEdges();
        AddRouteEdges();
        router_ = std::make_unique<graph::Router<double>>(*graph_);
    }

    std::optional<TransportRouter::RouteInfo> TransportRouter::GetRouteInfo(const Stop *from, const Stop *to) const {
        RouteInfo route_info;
        std::optional<graph::Router<double>::RouteInfo> router_info = router_->BuildRoute(GetStopVertexID(from),
                                                                                          GetStopVertexID(to));
        if (router_info) {
            route_info.time = router_info->weight;
            for (const auto& edge : router_info->edges) {
                route_info.items.push_back(edges_.at(edge));
            }
            return route_info;
        } else {
            return {};
        }
    }

    TransportRouter::RouteSettings TransportRouter::GetSettings() const {
        return r_settings_;
    }

    graph::VertexId transport_router::TransportRouter::GetStopVertexID(const Stop *from) const {
        return vertexes_.at(from).first;
    }

    graph::VertexId transport_router::TransportRouter::GetStartVertexID(const Stop *to) const {
        return vertexes_.at(to).second;
    }

    void TransportRouter::CreateRouteEdge(graph::VertexId from, graph::VertexId to, const std::string_view bus_name,
                                          double length, int span_count) {
        const double route_time = length / (r_settings_.velocity * 1000 / 60);
        graph::Edge<double> edge = {from, to, route_time};
        edges_[graph_->AddEdge(edge)] = {ItemType::BUS,
                                         bus_name,
                                         route_time,
                                         span_count};
    }

    void TransportRouter::AddVertexesAndWaitEdges() {
        graph::VertexId vertex_id = 0;
        const auto wait_time = static_cast<double>(r_settings_.wait_time);
        for (const auto& [stop_name, stop_ptr] : catalogue_.GetStopNames()) {
            vertexes_[stop_ptr] = {vertex_id, vertex_id + 1};
            graph::Edge<double> edge = {vertex_id, vertex_id + 1, wait_time};
            edges_[graph_->AddEdge(edge)] = {ItemType::WAIT,
                                             stop_name,
                                             wait_time,
                                             1};
            vertex_id += 2;
        }
    }

    void TransportRouter::AddSimpleRouteEdge(const std::string_view bus_name, const Bus *bus_ptr) {
        for (int i = 0; i < bus_ptr->stops.size()/2; ++i) {
            double forward_length = 0.;
            double backward_length = 0.;
            for (int k = i; k < bus_ptr->stops.size()/2; ++k) {
                if (bus_ptr->stops[i] != bus_ptr->stops[k+1]) {
                    forward_length += catalogue_.GetRealDistance(bus_ptr->stops[k], bus_ptr->stops[k+1]);
                    CreateRouteEdge(GetStartVertexID(bus_ptr->stops[i]),
                                    GetStopVertexID(bus_ptr->stops[k+1]),
                                    bus_name,forward_length, k-i+1);
                    if (!bus_ptr->is_round_route) {
                        backward_length += catalogue_.GetRealDistance(bus_ptr->stops[k+1], bus_ptr->stops[k]);
                        CreateRouteEdge(GetStartVertexID(bus_ptr->stops[k+1]),
                                        GetStopVertexID(bus_ptr->stops[i]),
                                        bus_name, backward_length, k-i+1);
                    }
                }
            }
        }
    }

    void TransportRouter::AddRoundRouteEdge(const std::string_view bus_name, const Bus *bus_ptr) {
        for (int i = 1; i < bus_ptr->stops.size(); ++i) {
            double forward_length = 0.;
            for (int k = i; k < bus_ptr->stops.size(); ++k) {
                if (bus_ptr->stops[i-1] != bus_ptr->stops[k]) {
                    forward_length += catalogue_.GetRealDistance(bus_ptr->stops[k-1], bus_ptr->stops[k]);
                    CreateRouteEdge(GetStartVertexID(bus_ptr->stops[i-1]),
                                    GetStopVertexID(bus_ptr->stops[k]), bus_name,forward_length, k-i+1);
                }
            }
        }
    }

    void TransportRouter::AddRouteEdges() {
        for (const auto& [bus_name, bus_ptr] : catalogue_.GetRouteNames()) {
            if (bus_ptr->is_round_route) {
                AddRoundRouteEdge(bus_name, bus_ptr);
            } else {
                AddSimpleRouteEdge(bus_name, bus_ptr);
            }
        }
    }
}