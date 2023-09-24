#include "map_renderer.h"
#include "transport_catalogue.h"

#include <set>
#include <utility>

using namespace renderer;

bool detail::IsZero(double value) {
    return std::abs(value) < EPSILON;
}

void MapRenderer::SetRenderSettings(const MapRendererSettings& renderer_settings) {
    map_renderer_ = renderer_settings;
}

svg::Document MapRenderer::AddRoutesOnMap(const std::map<std::string_view, const Bus*>& all_routes) const {
    svg::Document result;
    auto comparator = [] (const Stop* lhs, const Stop* rhs) {
        return lhs->name < rhs->name;
    };
    std::set<const Stop*, decltype(comparator)> all_stops(comparator);
    for (const auto& [bus_name, bus_info] : all_routes) {
        all_stops.insert(bus_info->stops.begin(), bus_info->stops.end());
    }
    SphereProjector sphere_projector(all_stops.begin(), all_stops.end(),
                                     map_renderer_.width, map_renderer_.height,
                                     map_renderer_.padding);
    int route_index = 0;
    for (const auto& [bus_name, bus_info] : all_routes) { //Рисуем линии маршрутов
        if (bus_info->stops.empty()) {
            continue;
        } else {
            result.Add(DrawRoutePolyline(bus_info, sphere_projector, route_index));
            ++route_index;
        }
    }
    route_index = 0;
    for (const auto& [bus_name, bus_info] : all_routes) { // Добавляем названия маршрутов
        if (bus_info->stops.empty()) {
            continue;
        } else  {
            result.Add(DrawRouteNameBackground(bus_info->name, bus_info->stops[0], sphere_projector));
            result.Add(DrawRouteName(bus_info->name, bus_info->stops[0], sphere_projector, route_index));
            if (!(bus_info->is_round_route)) {
                size_t last_stop = (bus_info->stops.size()) / 2;
                if (bus_info->stops[last_stop] != bus_info->stops[0]) {
                    result.Add(DrawRouteNameBackground(bus_info->name, bus_info->stops[last_stop], sphere_projector));
                    result.Add(DrawRouteName(bus_info->name, bus_info->stops[last_stop], sphere_projector, route_index));
                }
            }
            ++route_index;
        }
    }
    for (const auto& stop : all_stops) {
        result.Add(DrawStopSign(stop, sphere_projector));
    }
    for (const auto& stop : all_stops) {
        result.Add(DrawStopNameBackground(stop, sphere_projector));
        result.Add(DrawStopName(stop, sphere_projector));
    }
    //result.Render(std::cout);
    return result;
}

MapRendererSettings MapRenderer::GetSettings() const {
    return map_renderer_;
}

svg::Polyline MapRenderer::DrawRoutePolyline(const Bus* bus_info, SphereProjector& sphere_projector, int route_index) const {
    svg::Polyline route_polyline;
    route_polyline.SetFillColor("none"s);
    route_polyline.SetStrokeColor(map_renderer_.color_palette[route_index % map_renderer_.color_palette.size()]);
    route_polyline.SetStrokeWidth(map_renderer_.line_width);
    route_polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    route_polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    for (const auto& stop : bus_info->stops) {
        route_polyline.AddPoint(sphere_projector(stop->coordinates));
    }
    return route_polyline;
}

svg::Text MapRenderer::DrawRouteNameBackground(std::string bus_name, const Stop* stop_info, SphereProjector& sphere_projector) const {
    svg::Text route_name_background;
    route_name_background.SetPosition(sphere_projector(stop_info->coordinates));
    route_name_background.SetOffset(map_renderer_.bus_label_offset);
    route_name_background.SetFontSize(map_renderer_.bus_label_font_size);
    route_name_background.SetFontFamily("Verdana");
    route_name_background.SetFontWeight("bold");
    route_name_background.SetData(std::move(bus_name));
    route_name_background.SetFillColor(map_renderer_.underlayer_color);
    route_name_background.SetStrokeColor(map_renderer_.underlayer_color);
    route_name_background.SetStrokeWidth(map_renderer_.underlayer_width);
    route_name_background.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    route_name_background.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    return route_name_background;
}

svg::Text MapRenderer::DrawRouteName(std::string bus_name, const Stop* stop_info, SphereProjector& sphere_projector, int route_index) const {
    svg::Text route_name;
    route_name.SetPosition(sphere_projector(stop_info->coordinates));
    route_name.SetOffset(map_renderer_.bus_label_offset);
    route_name.SetFontSize(map_renderer_.bus_label_font_size);
    route_name.SetFontFamily("Verdana");
    route_name.SetFontWeight("bold");
    route_name.SetData(std::move(bus_name));
    route_name.SetFillColor(map_renderer_.color_palette[route_index % map_renderer_.color_palette.size()]);
    return route_name;
}

svg::Circle MapRenderer::DrawStopSign(const Stop* stop_info, SphereProjector& sphere_projector) const {
    svg::Circle stop_sign;
    stop_sign.SetCenter(sphere_projector(stop_info->coordinates));
    stop_sign.SetRadius(map_renderer_.stop_radius);
    stop_sign.SetFillColor("white");
    return stop_sign;
}

svg::Text MapRenderer::DrawStopNameBackground(const Stop* stop_info, SphereProjector& sphere_projector) const {
    svg::Text stop_name_background;
    stop_name_background.SetPosition(sphere_projector(stop_info->coordinates));
    stop_name_background.SetOffset(map_renderer_.stop_label_offset);
    stop_name_background.SetFontSize(map_renderer_.stop_label_font_size);
    stop_name_background.SetFontFamily("Verdana");
    stop_name_background.SetData(stop_info->name);
    stop_name_background.SetFillColor(map_renderer_.underlayer_color);
    stop_name_background.SetStrokeColor(map_renderer_.underlayer_color);
    stop_name_background.SetStrokeWidth(map_renderer_.underlayer_width);
    stop_name_background.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    stop_name_background.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    return stop_name_background;
}

svg::Text MapRenderer::DrawStopName(const Stop* stop_info, SphereProjector& sphere_projector) const {
    svg::Text stop_name;
    stop_name.SetPosition(sphere_projector(stop_info->coordinates));
    stop_name.SetOffset(map_renderer_.stop_label_offset);
    stop_name.SetFontSize(map_renderer_.stop_label_font_size);
    stop_name.SetFontFamily("Verdana");
    stop_name.SetData(stop_info->name);
    stop_name.SetFillColor("black");
    return stop_name;
}