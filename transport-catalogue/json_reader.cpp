#include "json_reader.h"

#include <set>
#include <sstream>

namespace json_reader {
    using namespace std;
    using namespace std::literals;

    void GetRouteRequest(const json::Dict& request, request_handler::RequestHandler& request_handler,
                         json::Builder& builder) {
        using namespace transport_router;
        using RouteInfo = std::optional<TransportRouter::RouteInfo>;
        RouteInfo route_info = request_handler.GetRouteInfo(request.at("from").AsString(),
                                                            request.at("to").AsString());

        if (route_info) {
            builder.StartDict()
                    .Key("request_id"s).Value(request.at("id"s).AsInt())
                    .Key("total_time"s).Value(route_info.value().time)
                    .Key("items"s).StartArray();
            for (const auto& item : route_info.value().items) {
                if (item.type == TransportRouter::ItemType::WAIT) {
                    builder.StartDict().Key("type"s).Value("Wait"s)
                        .Key("stop_name"s).Value(std::string(item.route_name))
                        .Key("time"s).Value(item.time)
                    .EndDict();
                } else if (item.type == TransportRouter::ItemType::BUS) {
                    builder.StartDict().Key("type"s).Value("Bus"s)
                        .Key("bus"s).Value(std::string(item.route_name))
                        .Key("span_count"s).Value(item.span_count)
                        .Key("time"s).Value(item.time)
                    .EndDict();
                }
            }
            builder.EndArray().EndDict();
        } else {
            builder.StartDict().Key("request_id"s).Value(request.at("id"s).AsInt())
                    .Key("error_message"s).Value("not found"s)
                    .EndDict();
        }
    }

    void GetMapRequest(const json::Dict& request, request_handler::RequestHandler& request_handler,
                       json::Builder& builder) {
        std::ostringstream out;
        svg::Document svg_map = request_handler.RenderMap();
        svg_map.Render(out);
        builder.StartDict()
                .Key("request_id"s).Value(request.at("id"s).AsInt())
                .Key("map"s).Value(out.str())
            .EndDict();
    }

    void GetStopInfoForOutput(const json::Dict& request, request_handler::RequestHandler& request_handler,
                              json::Builder& builder) {
        std::optional<StopInfo> stop_info = request_handler.GetBusesByStop(request.at("name"s).AsString());
        builder.StartDict()
            .Key("request_id"s).Value(request.at("id"s).AsInt());
        std::set<std::string_view> sort_buses;
        if (stop_info) {
            builder.Key("buses"s).StartArray();
            for (const auto& route : stop_info->buses) {
                sort_buses.insert(route);
            }
            for (const auto& bus : sort_buses) {
                builder.Value(static_cast<string>(bus));
            }
            builder.EndArray();
        } else {
            builder.Key("error_message"s).Value(R"(not found)"s);
        }
        builder.EndDict();
    }

    void GetBusInfoForOutput(const json::Dict& request, request_handler::RequestHandler& request_handler,
                             json::Builder& builder) {
        std::optional<BusInfo> bus_info = request_handler.GetBusStat(request.at("name"s).AsString());
        builder.StartDict()
                .Key("request_id"s).Value(request.at("id"s).AsInt());
        if (bus_info) {
            builder.Key("curvature"s).Value(bus_info->curvature)
                .Key("route_length"s).Value(bus_info->route_length)
                .Key("stop_count"s).Value(bus_info->stops_count)
                .Key("unique_stop_count"s).Value(bus_info->unique_stops_count);
        } else {
            builder.Key("error_message"s).Value(R"(not found)"s);
        }
        builder.EndDict();
    }

    std::vector<std::string_view> GetStopsFromBusInfo(const json::Dict& bus_info) {
        std::vector<std::string_view> stops;
        for (const auto& stop : bus_info.at("stops"s).AsArray()) {
            stops.push_back(stop.AsString());
        }
        return stops;
    }

    Bus SetBusFromJsonRequest(transport_catalogue::TransportCatalogue& catalogue, const json::Dict& bus_info) {
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

    void SetDistanceBetweenStops(transport_catalogue::TransportCatalogue& catalogue, const json::Dict& stop_info) {
        const Stop* source = catalogue.FindStop(stop_info.at("name"s).AsString());
        const json::Dict road_distances = stop_info.at("road_distances"s).AsDict();
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


    void GetRenderJsonRequest(renderer::MapRenderer& map_renderer, const json::Dict& request_info) {
        renderer::MapRendererSettings renderer_settings;
        for (const auto& [key, value] : request_info) {
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
        //request_handler.RenderMap();
    }

    void GetInputJsonRequest(transport_catalogue::TransportCatalogue& catalogue,
                             const json::Array& request_info) {
        Requests requests{};
        for (const auto& item : request_info) {
            json::Dict input_request = item.AsDict();
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
            SetDistanceBetweenStops(catalogue, stop.AsDict());
        }
        for (const auto& bus : requests.buses) {
            catalogue.AddBus(SetBusFromJsonRequest(catalogue, bus.AsDict()));
        }
    }

    void GetOutputJsonRequest(request_handler::RequestHandler& request_handler, const json::Array& request_info,
                              [[maybe_unused]] ostream& output) {
        json::Builder builder;
        builder.StartArray();
        for (const auto& output_requests : request_info) {
            json::Dict request = output_requests.AsDict();
            if (request.at("type"s).AsString() == "Stop"s) {
                GetStopInfoForOutput(request, request_handler, builder) ;
            } else if (request.at("type"s).AsString() == "Bus"s) {
                GetBusInfoForOutput(request, request_handler, builder);
            } else if (request.at("type"s).AsString() == "Map"s) {
                GetMapRequest(request, request_handler, builder);
            } else if (request.at("type"s).AsString() == "Route"s) {
                GetRouteRequest(request, request_handler, builder);
            }
        }
        builder.EndArray();
        json::Print(json::Document{builder.Build()}, output);
    }

    void GetRouteJsonRequest(transport_router::TransportRouter& router, const json::Dict &request_info) {
        using namespace transport_router;
        transport_router::TransportRouter::RouteSettings r_settings{};
        for (const auto& [setting, value] : request_info) {
            if (setting == "bus_velocity"s) {
                r_settings.velocity = value.AsDouble();
            } else if (setting == "bus_wait_time"s) {
                r_settings.wait_time = value.AsInt();
            } else {
                throw std::invalid_argument("Incorrect types of routing settings"s);
            }
        }
        router.SetSettingsAndBuildGraph(r_settings);
    }

    void GetJsonRequest(transport_catalogue::TransportCatalogue& catalogue,
                        renderer::MapRenderer& map_renderer,
                        transport_router::TransportRouter& router,
                        istream& input, ostream& output) {
        request_handler::RequestHandler request_handler(catalogue, map_renderer, router);
        json::Document requests = json::Load(input);
        for (const auto& [request_type, request_info] : requests.GetRoot().AsDict()) {
            if (request_type == "base_requests"s) {
                GetInputJsonRequest(catalogue, request_info.AsArray());
            } else if (request_type == "render_settings"s) {
                GetRenderJsonRequest(map_renderer, request_info.AsDict());
            } else if (request_type == "routing_settings"s) {
                GetRouteJsonRequest(router, request_info.AsDict());
            } else if (request_type == "stat_requests"s) {
                GetOutputJsonRequest(request_handler, request_info.AsArray(), output);
            } else {
                throw std::invalid_argument("Incorrect JSON request"s);
            }
        }
    }

    void GetSerializeJsonRequest(serialize::Serializer& serializer, const json::Dict &request_info) {
        serializer.SetSetting(request_info.at("file").AsString());
    }

    void MakeBaseRequest(transport_catalogue::TransportCatalogue& catalogue,
                         renderer::MapRenderer& map_renderer,
                         transport_router::TransportRouter& router,
                         serialize::Serializer& serializer, istream& input) {
        json::Document requests = json::Load(input);
        for (const auto& [request_type, request_info] : requests.GetRoot().AsDict()) {
            if (request_type == "base_requests"s) {
                GetInputJsonRequest(catalogue, request_info.AsArray());
            } else if (request_type == "render_settings"s) {
                GetRenderJsonRequest(map_renderer, request_info.AsDict());
            } else if (request_type == "routing_settings"s) {
                GetRouteJsonRequest(router, request_info.AsDict());
            } else if (request_type == "serialization_settings"s) {
                GetSerializeJsonRequest(serializer, request_info.AsDict());
            } else {
                throw std::invalid_argument("Incorrect make base JSON request"s);
            }
        }
        serializer.SerializeToFile();
    }

    void ProcessRequest(transport_catalogue::TransportCatalogue& catalogue,
                        renderer::MapRenderer& map_renderer,
                        transport_router::TransportRouter& router,
                        serialize::Serializer& serializer, istream& input, ostream& output) {
        json::Document requests = json::Load(input);
        request_handler::RequestHandler request_handler(catalogue, map_renderer, router);
        for (const auto& [request_type, request_info] : requests.GetRoot().AsDict()) {
            if (request_type == "serialization_settings"s) {
                GetSerializeJsonRequest(serializer, request_info.AsDict());
                serializer.DeserializeFromFile();
            } else if (request_type == "stat_requests"s) {
                GetOutputJsonRequest(request_handler, request_info.AsArray(), output);
            } else {
                throw std::invalid_argument("Incorrect process JSON request"s);
            }
        }
    }
}