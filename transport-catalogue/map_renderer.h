#pragma once

#include "geo.h"
#include "svg.h"
#include "domain.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <map>

namespace renderer {
    namespace detail {
        inline const double EPSILON = 1e-6;
        bool IsZero(double value);
    }

    class SphereProjector {
    public:
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                        double max_width, double max_height, double padding)
                : padding_(padding) //
        {
            // Если точки поверхности сферы не заданы, вычислять нечего
            if (points_begin == points_end) {
                return;
            }

            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = std::minmax_element(
                    points_begin, points_end,
                    [](auto lhs, auto rhs) { return lhs->coordinates.lng < rhs->coordinates.lng; });
            min_lon_ = (*left_it)->coordinates.lng;
            const double max_lon = (*right_it)->coordinates.lng;

            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = std::minmax_element(
                    points_begin, points_end,
                    [](auto lhs, auto rhs) { return lhs->coordinates.lat < rhs->coordinates.lat; });
            const double min_lat = (*bottom_it)->coordinates.lat;
            max_lat_ = (*top_it)->coordinates.lat;

            // Вычисляем коэффициент масштабирования вдоль координаты x
            std::optional<double> width_zoom;
            if (!detail::IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Вычисляем коэффициент масштабирования вдоль координаты y
            std::optional<double> height_zoom;
            if (!detail::IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // Коэффициенты масштабирования по ширине и высоте ненулевые, берём минимальный из них
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            } else if (width_zoom) {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            } else if (height_zoom) {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(geo::Coordinates coords) const {
            return {
                    (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                    (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    struct MapRendererSettings {
        double width = 0.0;
        double height = 0.0;
        double padding = 0.0;
        double line_width = 0.0;
        double stop_radius = 0.0;
        int bus_label_font_size = 0;
        svg::Point bus_label_offset;
        int stop_label_font_size = 0;
        svg::Point stop_label_offset;
        svg::Color underlayer_color;
        double underlayer_width = 0.0;
        std::vector<svg::Color> color_palette;
    };

    class MapRenderer {
    public:
        MapRenderer() = default;

        void SetRenderSettings(const MapRendererSettings& renderer_settings);
        svg::Document AddRoutesOnMap(const std::map<std::string_view, const Bus*>& all_routes) const;
        MapRendererSettings GetSettings() const;

    private:
        MapRendererSettings map_renderer_;

        svg::Polyline DrawRoutePolyline(const Bus* bus_info, SphereProjector& sphere_projector, int route_index) const;
        svg::Text DrawRouteName(std::string bus_name, const Stop* stop_info, SphereProjector& sphere_projector, int route_index) const;
        svg::Text DrawRouteNameBackground(std::string bus_name, const Stop* stop_info, SphereProjector& sphere_projector) const;
        svg::Circle DrawStopSign(const Stop* stop_info, SphereProjector& sphere_projector) const;
        svg::Text DrawStopName(const Stop* stop_info, SphereProjector& sphere_projector) const;
        svg::Text DrawStopNameBackground(const Stop* stop_info, SphereProjector& sphere_projector) const;
    };
} // namespace renderer