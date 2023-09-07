#pragma once

#include "domain.h"
#include "transport_catalogue.h"
#include "json.h"
#include "svg.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace request_handler {
    class RequestHandler {
    public:
        RequestHandler(const transport_catalogue::TransportCatalogue& catalogue,
                       const renderer::MapRenderer& renderer,
                       const transport_router::TransportRouter& router)
            : catalogue_(catalogue),
            renderer_(renderer),
            router_(router) {}

        // Возвращает информацию о маршруте (запрос Bus)
        std::optional<BusInfo> GetBusStat(const std::string_view& bus_name) const;
        // Возвращает маршруты, проходящие через остановку
        std::optional<StopInfo> GetBusesByStop(const std::string_view& stop_name) const;

        // Добавляет запрос на вывод информации
        void AddOutputRequest(const json::Node& output_request);
        // Вывод информации в поток
        void PrintOutputRequests(std::ostream& output);

        svg::Document RenderMap() const;

        using RouteInfo = std::optional<transport_router::TransportRouter::RouteInfo>;
        RouteInfo GetRouteInfo(const std::string_view start, const std::string_view stop) const;

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник", "Визуализатор Карты"
        // и маршрутизатор
        std::vector<json::Node> output_request_handler_;
        const transport_catalogue::TransportCatalogue& catalogue_;
        const renderer::MapRenderer& renderer_;
        const transport_router::TransportRouter& router_;
    };
}