#include <iostream>

#include "serialization.h"
#include "domain.h"

using namespace serialize;

void Serializer::SetSetting(const std::string &file_name) {
    file_name_ = file_name;
}

void Serializer::SerializeToFile() {
    std::ofstream output(file_name_, std::ios::binary);
    ProtoCatalogue serialize_catalogue;
    const auto& all_stops = transport_catalogue_.GetStopNames();
    for (const auto& [stop_name, stop_ptr] : all_stops) {
        (*serialize_catalogue.mutable_stops())[reinterpret_cast<uint64_t>(stop_ptr)] = std::move(GetSerializeStop(stop_ptr));
    }
    for (const auto& [bus_name, bus_ptr] : transport_catalogue_.GetRouteNames()) {
        *serialize_catalogue.add_buses() = std::move(GetSerializeBus(bus_ptr));
    }
    for (const auto& [pair_from_to, distance] : transport_catalogue_.GetAllDistances()) {
        *serialize_catalogue.add_distances() = std::move(GetSerializeDistance(pair_from_to.first,
                                                                              pair_from_to.second, distance));
    }
    *serialize_catalogue.mutable_render_settings() = GetSerializeRenderSettings(map_renderer_.GetSettings());
    *serialize_catalogue.mutable_router_settings() = GetSerializeRouterSettings(router_.GetSettings());
    serialize_catalogue.SerializeToOstream(&output);
}

void Serializer::DeserializeFromFile() {
    std::ifstream input(file_name_, std::ios::binary);
    ProtoCatalogue proto_trans_catalogue;
    if (!proto_trans_catalogue.ParseFromIstream(&input)) {
        std::cerr << "Error in deserialize" << std::endl;
    } else {
        for (const auto& [stop_id, stop] : proto_trans_catalogue.stops()) {
            transport_catalogue_.AddStop(GetDeserializeStop(stop));
        }
        for (const auto& bus : proto_trans_catalogue.buses()) {
            transport_catalogue_.AddBus(GetDeserializeBus(bus, proto_trans_catalogue));
        }
        for (const auto& dist_message : proto_trans_catalogue.distances()) {
            transport_catalogue_.SetDistance(GetStopPtr(dist_message.from(), proto_trans_catalogue),
                                             GetStopPtr(dist_message.to(), proto_trans_catalogue),
                                             dist_message.distance());
        }
        map_renderer_.SetRenderSettings(GetDeserializeRenderSettings(proto_trans_catalogue.render_settings()));
        router_.SetSettingsAndBuildGraph(GetDeserializeRouterSettings(proto_trans_catalogue.router_settings()));
    }

}

proto_catalogue::Stop Serializer::GetSerializeStop(const Stop *stop_ptr) {
    proto_catalogue::Stop proto_stop;
    proto_stop.set_name(stop_ptr->name);
    proto_stop.mutable_coordinates()->set_latitude(stop_ptr->coordinates.lat);
    proto_stop.mutable_coordinates()->set_longitude(stop_ptr->coordinates.lng);
    return proto_stop;
}

Stop Serializer::GetDeserializeStop(const proto_catalogue::Stop &proto_stop) {
    Stop stop;
    stop.name = proto_stop.name();
    stop.coordinates = {proto_stop.coordinates().latitude(), proto_stop.coordinates().longitude()};
    return stop;
}

proto_catalogue::Bus Serializer::GetSerializeBus(const Bus *bus_ptr) {
    proto_catalogue::Bus proto_bus;
    proto_bus.set_name(bus_ptr->name);
    proto_bus.set_is_round_route(bus_ptr->is_round_route);
    for (const auto& stop : bus_ptr->stops) {
        proto_bus.add_stops_on_route(reinterpret_cast<uint64_t>(stop));
    }
    return proto_bus;
}

Bus Serializer::GetDeserializeBus(const proto_catalogue::Bus &proto_bus,
                                  const ProtoCatalogue& proto_trans_catalogue) {
    Bus bus;
    bus.name = proto_bus.name();
    bus.is_round_route = proto_bus.is_round_route();
    for (const auto& stop_id : proto_bus.stops_on_route()) {
        const auto& stop_name = proto_trans_catalogue.stops().at(stop_id).name();
        bus.stops.emplace_back(transport_catalogue_.FindStop(stop_name));
    }
    return bus;
}

proto_catalogue::Distances Serializer::GetSerializeDistance(const Stop *from, const Stop *to, int distance) {
    proto_catalogue::Distances proto_distance;
    proto_distance.set_from(reinterpret_cast<uint64_t>(from));
    proto_distance.set_to(reinterpret_cast<uint64_t>(to));
    proto_distance.set_distance(distance);
    return proto_distance;
}

const Stop* Serializer::GetStopPtr(uint64_t stop_id, const ProtoCatalogue &proto_trans_catalogue) {
    return transport_catalogue_.FindStop(proto_trans_catalogue.stops().at(stop_id).name());
}

proto_catalogue::Color Serializer::GetSerializeColor(const svg::Color& color) {
    proto_catalogue::Color proto_color;
    if (std::holds_alternative<std::monostate>(color)) {
        proto_color.set_no_color(true);
    } else if (std::holds_alternative<std::string>(color)) {
        proto_color.set_color_name(std::get<std::string>(color));
    } else if (std::holds_alternative<svg::Rgb>(color)) {
        svg::Rgb rgb_color = std::get<svg::Rgb>(color);
        proto_color.mutable_rgb()->set_r(rgb_color.red);
        proto_color.mutable_rgb()->set_g(rgb_color.green);
        proto_color.mutable_rgb()->set_b(rgb_color.blue);
    } else {
        svg::Rgba rgba_color = std::get<svg::Rgba>(color);
        proto_color.mutable_rgba()->set_r(rgba_color.red);
        proto_color.mutable_rgba()->set_g(rgba_color.green);
        proto_color.mutable_rgba()->set_b(rgba_color.blue);
        proto_color.mutable_rgba()->set_opacity(rgba_color.opacity);
    }
    return proto_color;
}

Serializer::ProtoMapSettings Serializer::GetSerializeRenderSettings(const MapSettings& renderer_settings) {
    proto_catalogue::MapRendererSettings proto_map_settings;
    proto_map_settings.set_width(renderer_settings.width);
    proto_map_settings.set_height(renderer_settings.height);
    proto_map_settings.set_padding(renderer_settings.padding);
    proto_map_settings.set_line_width(renderer_settings.line_width);
    proto_map_settings.set_stop_radius(renderer_settings.stop_radius);
    proto_map_settings.set_bus_label_font_size(renderer_settings.bus_label_font_size);
    proto_map_settings.mutable_bus_label_offset()->set_x(renderer_settings.bus_label_offset.x);
    proto_map_settings.mutable_bus_label_offset()->set_y(renderer_settings.bus_label_offset.y);
    proto_map_settings.set_stop_label_font_size(renderer_settings.stop_label_font_size);
    proto_map_settings.mutable_stop_label_offset()->set_x(renderer_settings.stop_label_offset.x);
    proto_map_settings.mutable_stop_label_offset()->set_y(renderer_settings.stop_label_offset.y);
    *proto_map_settings.mutable_underlayer_color() = GetSerializeColor(renderer_settings.underlayer_color);
    proto_map_settings.set_underlayer_width(renderer_settings.underlayer_width);
    for (const auto& color : renderer_settings.color_palette) {
        *proto_map_settings.add_color_palette() = GetSerializeColor(color);
    }
    return proto_map_settings;
}

svg::Color Serializer::GetColor(const proto_catalogue::Color& proto_color) {
    if (proto_color.no_color()) {
        return {};
    } else if (proto_color.has_rgb()) {
        svg::Rgb color;
        color.red = proto_color.rgb().r();
        color.green = proto_color.rgb().g();
        color.blue = proto_color.rgb().b();
        return color;
    } else if (proto_color.has_rgba()) {
        svg::Rgba color;
        color.red = proto_color.rgba().r();
        color.green = proto_color.rgba().g();
        color.blue = proto_color.rgba().b();
        color.opacity = proto_color.rgba().opacity();
        return color;
    } else {
        std::string color = proto_color.color_name();
        return color;
    }
}

Serializer::MapSettings Serializer::GetDeserializeRenderSettings(const ProtoMapSettings& proto_settings) {
    renderer::MapRendererSettings renderer_settings;
    renderer_settings.width = proto_settings.width();
    renderer_settings.height = proto_settings.height();
    renderer_settings.padding = proto_settings.padding();
    renderer_settings.line_width = proto_settings.line_width();
    renderer_settings.stop_radius = proto_settings.stop_radius();
    renderer_settings.bus_label_font_size = proto_settings.bus_label_font_size();
    renderer_settings.bus_label_offset.x = proto_settings.bus_label_offset().x();
    renderer_settings.bus_label_offset.y = proto_settings.bus_label_offset().y();
    renderer_settings.stop_label_font_size = proto_settings.stop_label_font_size();
    renderer_settings.stop_label_offset.x = proto_settings.stop_label_offset().x();
    renderer_settings.stop_label_offset.y = proto_settings.stop_label_offset().y();
    renderer_settings.underlayer_color = GetColor(proto_settings.underlayer_color());
    renderer_settings.underlayer_width = proto_settings.underlayer_width();
    for (const auto& proto_color : proto_settings.color_palette()) {
        renderer_settings.color_palette.push_back(GetColor(proto_color));
    }
    return renderer_settings;
}

Serializer::ProtoRouterSettings Serializer::GetSerializeRouterSettings(const RouterSettings& router_settings) {
    proto_catalogue::RouterSettings proto_router_settings;
    proto_router_settings.set_time(router_settings.wait_time);
    proto_router_settings.set_velocity(router_settings.velocity);
    return proto_router_settings;
}

Serializer::RouterSettings Serializer::GetDeserializeRouterSettings(const ProtoRouterSettings& proto_settings) {
    RouterSettings router_settings;
    router_settings.wait_time = proto_settings.time();
    router_settings.velocity = proto_settings.velocity();
    return router_settings;
}