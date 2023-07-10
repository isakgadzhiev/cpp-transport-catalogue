#include <unordered_set>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>

#include "transport_catalogue.h"

using namespace std::literals;

namespace transport_catalogue {
    void TransportCatalogue::AddStop(const Stop& stop) {
        stops_.push_back(stop);
        stop_names_[stops_.back().name] = &stops_.back();
    }
    void TransportCatalogue::AddBus(const Bus& bus) {
        buses_.push_back(bus);
        route_names_[buses_.back().name] = &buses_.back();
    }

    const Stop* TransportCatalogue::FindStop(const std::string_view& stop_name) const {
        if (stop_names_.count(stop_name) != 0) {
            return stop_names_.at(stop_name);
        } else {
            return nullptr;
        }
    }

    const Bus* TransportCatalogue::FindBus(std::string_view bus_name) const {
        if (route_names_.count(bus_name) != 0) {
            return route_names_.at(bus_name);
        } else {
            return nullptr;
        }
    }

    void TransportCatalogue::SetDistance(const Stop* from, const Stop* to, int distance) {
        real_distances_[{from, to}] = distance;
    }

    int TransportCatalogue::GetRealDistance(const Stop* from, const Stop* to) const {
        if (real_distances_.count({from, to})) {
            return real_distances_.at({from, to});
        } else {
            return real_distances_.at({to, from});
        }
    }

    std::optional<BusInfo> TransportCatalogue::GetBusInfo(const Bus* bus) const {
        if (bus != nullptr) {
            BusInfo bus_info;
            bus_info.route_length = 0;
            bus_info.curvature = 0.0;
            double common_direct_distance = 0.0;
            bus_info.stops_count = static_cast<int>(bus->stops.size());
            std::unordered_set unique_stops(bus->stops.begin(), bus->stops.end());
            bus_info.unique_stops_count = static_cast<int>(unique_stops.size());
            for (size_t i = 1; i < bus->stops.size(); ++i) {
                bus_info.route_length += GetRealDistance(bus->stops[i-1], bus->stops[i]);
                common_direct_distance += geo::ComputeDistance(bus->stops[i-1]->coordinates, bus->stops[i]->coordinates);
            }
            bus_info.curvature = 1.0 * bus_info.route_length / common_direct_distance;
            return bus_info;
        } else {
            return std::nullopt;
        }
    }

    std::optional<StopInfo> TransportCatalogue::GetStopInfo(const std::string_view& stop_name) const {
        if (FindStop(stop_name) == nullptr) {
            return std::nullopt;
        } else {
            StopInfo stop_info{};
            std::for_each(route_names_.begin(), route_names_.end(),
                          [stop_name, &stop_info] (const auto bus_info) {
                              for (auto stop_on_route : bus_info.second->stops) {
                                  if (stop_on_route->name == stop_name) {
                                      stop_info.buses.insert(bus_info.second->name);
                                      break;
                                  }
                              }
                          });
            return stop_info;
        }
    }

    const RealDistanceTable& TransportCatalogue::GetAllDistances() const {
        return real_distances_;
    }

    std::unordered_map<std::string_view, const Stop*> TransportCatalogue::GetStopNames() const {
        return stop_names_;
    }
    std::map<std::string_view, const Bus*> TransportCatalogue::GetRouteNames() const {
        return route_names_;
    }

    void TransportCatalogue::TestGetStopNames() {
        for (const auto& [name, name_link] : stop_names_) {
            std::cout << "Name: " << name << " <> " << " Link name: " << name_link->name << std::endl;
            std::cout << std::setprecision(8) << name_link->coordinates.lat << "___" << name_link->coordinates.lng << std::endl;
        }
    }

    void TransportCatalogue::TestGetBusNames() {
        for (const auto& [bus, bus_link] : route_names_) {
            std::cout << "Name: " << bus << " <> " << " Link name: " << bus_link->name << std::endl;
            for (const auto& stop : bus_link->stops) {
                std::cout << stop->name << std::endl;
            }
        }
    }

    void TransportCatalogue::TestGetDistancesBetweenStops() {
        for (const auto& [pair_stops, dist] : real_distances_) {
            std::cout << "1: " << pair_stops.first->name << " | 2: " << pair_stops.second->name << " ----- " << dist << std::endl;
        }
    }
}