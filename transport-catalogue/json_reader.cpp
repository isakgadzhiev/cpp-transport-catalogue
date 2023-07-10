#include "json_reader.h"

#include <set>
#include <sstream>

namespace json_reader {
    using namespace std;
    using namespace std::literals;

    void GetMapRequest(const json::Dict& request,
                              request_handler::RequestHandler& request_handler) {
        json::Dict result;
        result["request_id"s] = request.at("id"s).AsInt();
        std::ostringstream out;
        svg::Document svg_map = request_handler.RenderMap();
        svg_map.Render(out);

        result["map"s] = out.str();
        request_handler.AddOutputRequest(result);
    }

    void GetStopInfoForOutput(const json::Dict& request,
                              request_handler::RequestHandler& request_handler) {
        std::optional<StopInfo> stop_info = request_handler.GetBusesByStop(request.at("name"s).AsString());
        json::Dict result;
        std::set<std::string_view> sort_buses;
        json::Array buses;
        result["request_id"s] = request.at("id"s).AsInt();
        if (stop_info) {
            for (const auto& route : stop_info->buses) {
                sort_buses.insert(route);
            }
            for (const auto& bus : sort_buses) {
                buses.emplace_back(static_cast<string>(bus));
            }
            result["buses"s] = buses;
        } else {
            result["error_message"s] = R"(not found)"s;
        }
        request_handler.AddOutputRequest(result);
    }

    void GetBusInfoForOutput(const json::Dict& request,
                             request_handler::RequestHandler& request_handler) {
        std::optional<BusInfo> bus_info = request_handler.GetBusStat(request.at("name"s).AsString());
        json::Dict result;
        result["request_id"s] = request.at("id"s).AsInt();
        if (bus_info) {
            result["curvature"s] = bus_info->curvature;
            result["route_length"s] = bus_info->route_length;
            result["stop_count"s] = bus_info->stops_count;
            result["unique_stop_count"s] = bus_info->unique_stops_count;
        } else {
            result["error_message"s] = R"(not found)"s;
        }
        request_handler.AddOutputRequest(result);
    }

    std::vector<std::string_view> GetStopsFromBusInfo(const json::Dict& bus_info) {
        std::vector<std::string_view> stops;
        for (const auto& stop : bus_info.at("stops"s).AsArray()) {
            stops.push_back(stop.AsString());
        }
        return stops;
    }

    Bus SetBusFromJsonRequest(transport_catalogue::TransportCatalogue& catalogue,
                              const json::Dict& bus_info) {
        Bus bus;
        bus.name = bus_info.at("name"s).AsString();
        bus.is_round_route = bus_info.at("is_roundtrip"s).AsBool();
        for (const auto& stop : GetStopsFromBusInfo(bus_info)) {
            bus.stops.push_back(catalogue.FindStop(stop));
        }
        if (!bus.is_round_route) {
            for (int i = (static_cast<int>(bus.stops.size()) - 2); i >= 0; --i) {
                bus.stops.push_back(bus.stops[i]);
            }
        }
        return bus;
    }

    Stop SetStopFromJsonRequest(const json::Dict& stop_info) {
        Stop stop;
        stop.name = stop_info.at("name"s).AsString();
        stop.coordinates.lng = stop_info.at("longitude"s).AsDouble();
        stop.coordinates.lat = stop_info.at("latitude"s).AsDouble();
        return stop;
    }

    void SetDistanceBetweenStops(transport_catalogue::TransportCatalogue& catalogue,
                                 const json::Dict& stop_info) {
        const Stop* source = catalogue.FindStop(stop_info.at("name"s).AsString());
        const json::Dict road_distances = stop_info.at("road_distances"s).AsMap();
        for (const auto& [destination, distance] : road_distances) {
            catalogue.SetDistance(source, catalogue.FindStop(destination), distance.AsInt());
        }
    }

    svg::Color GetColorFromRequest(const json::Node& color_request) {
        if (color_request.IsString()) {
            return color_request.AsString();
        } else if (color_request.IsArray()) {
            json::Array color_settings = color_request.AsArray();
            svg::Color result;
            if (color_settings.size() == 3) {
                return result = svg::Rgb{static_cast<uint8_t>(color_settings[0].AsInt()),
                                  static_cast<uint8_t>(color_settings[1].AsInt()),
                                  static_cast<uint8_t>(color_settings[2].AsInt())};
            } else if (color_settings.size() == 4) {
                return result = svg::Rgba{static_cast<uint8_t>(color_settings[0].AsInt()),
                                  static_cast<uint8_t>(color_settings[1].AsInt()),
                                  static_cast<uint8_t>(color_settings[2].AsInt()),
                                  color_settings[3].AsDouble()};
            } else {
                throw std::invalid_argument("Incorrect color type settings in JSON render request"s);
            }
        } else {
            throw std::invalid_argument("Incorrect color type settings in JSON render request"s);
        }
    }


    void GetRenderJsonRequest(request_handler::RequestHandler& request_handler,
                              renderer::MapRenderer& map_renderer,
                              const json::Node& request_info) {
        renderer::MapRendererSettings renderer_settings;
        for (const auto& [key, value] : request_info.AsMap()) {
            if (key == "width") {
                renderer_settings.width = value.AsDouble();
            } else if (key == "height"s) {
                renderer_settings.height = value.AsDouble();
            } else if (key == "padding"s) {
                renderer_settings.padding = value.AsDouble();
            } else if (key == "line_width"s) {
                renderer_settings.line_width = value.AsDouble();
            } else if (key == "stop_radius"s) {
                renderer_settings.stop_radius = value.AsDouble();
            } else if (key == "bus_label_font_size"s) {
                renderer_settings.bus_label_font_size = value.AsInt();
            } else if (key == "bus_label_offset"s) {
                renderer_settings.bus_label_offset.x = value.AsArray()[0].AsDouble();
                renderer_settings.bus_label_offset.y = value.AsArray()[1].AsDouble();
            } else if (key == "stop_label_font_size"s) {
                renderer_settings.stop_label_font_size = value.AsInt();
            } else if (key == "stop_label_offset"s) {
                renderer_settings.stop_label_offset.x = value.AsArray()[0].AsDouble();
                renderer_settings.stop_label_offset.y = value.AsArray()[1].AsDouble();
            } else if (key == "underlayer_color"s) {
                renderer_settings.underlayer_color = GetColorFromRequest(value);
            } else if (key == "underlayer_width"s) {
                renderer_settings.underlayer_width = value.AsDouble();
            } else if (key == "color_palette"s) {
                for (const auto& item : value.AsArray()) {
                    renderer_settings.color_palette.push_back(GetColorFromRequest(item));
                }
            } else {
                throw std::invalid_argument("Incorrect render settings in JSON request"s);
            }
        }
        map_renderer.SetRenderSettings(renderer_settings);
        request_handler.RenderMap();
    }

    void GetInputJsonRequest(transport_catalogue::TransportCatalogue& catalogue,
                             const json::Array& request_info) {
        Requests requests{};
        for (const auto& item : request_info) {
            json::Dict input_request = item.AsMap();
            if (input_request.at("type"s).AsString() == "Stop"s) {
                //Добавляем остановки в каталог
                catalogue.AddStop(SetStopFromJsonRequest(input_request));
                requests.stops.emplace_back(std::move(input_request));
            } else if (input_request.at("type"s).AsString() == "Bus"s) {
                requests.buses.emplace_back(std::move(input_request));
            } else {
                throw std::invalid_argument("Incorrect type of input request"s);
            }
        }
        for (const auto& stop : requests.stops) {
            SetDistanceBetweenStops(catalogue, stop.AsMap());
        }
        for (const auto& bus : requests.buses) {
            catalogue.AddBus(SetBusFromJsonRequest(catalogue, bus.AsMap()));
        }
    }

    void GetOutputJsonRequest(request_handler::RequestHandler& request_handler, const json::Node& request_info,
                              [[maybe_unused]] ostream& output) {
        for (const auto& output_requests : request_info.AsArray()) {
            json::Dict request = output_requests.AsMap();
            if (request.at("type"s).AsString() == "Stop"s) {
                GetStopInfoForOutput(request, request_handler) ;
            } else if (request.at("type"s).AsString() == "Bus"s) {
                GetBusInfoForOutput(request, request_handler);
            } else if (request.at("type"s).AsString() == "Map"s) {
                GetMapRequest(request, request_handler);
            }
        }
        request_handler.PrintOutputRequests(output);
    }

    void GetJsonRequest(transport_catalogue::TransportCatalogue& catalogue,
                        renderer::MapRenderer& map_renderer,
                        request_handler::RequestHandler& request_handler,
                        istream& input, ostream& output) {
        json::Document requests = json::Load(input);
        for (const auto& [request_type, request_info] : requests.GetRoot().AsMap()) {
            if (request_type == "base_requests"s) {
                GetInputJsonRequest(catalogue, request_info.AsArray());
            } else if (request_type == "render_settings"s) {
                GetRenderJsonRequest(request_handler, map_renderer, request_info);
            } else if (request_type == "stat_requests"s) {
                GetOutputJsonRequest(request_handler, request_info.AsArray(), output);
            } else {
                throw std::invalid_argument("Incorrect JSON request"s);
            }
        }
    }
}