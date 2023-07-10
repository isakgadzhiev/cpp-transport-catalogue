#pragma once

#include "domain.h"
#include "transport_catalogue.h"
#include "json.h"
#include "svg.h"
#include "map_renderer.h"

namespace request_handler {
    class RequestHandler {
    public:
        RequestHandler(const transport_catalogue::TransportCatalogue& db, const renderer::MapRenderer& renderer)
            : db_(db)
            , renderer_(renderer) {
        }

        std::optional<BusInfo> GetBusStat(const std::string_view& bus_name) const; // Возвращает информацию о маршруте (запрос Bus)
        std::optional<StopInfo> GetBusesByStop(const std::string_view& stop_name) const; // Возвращает маршруты, проходящие через остановку

        void AddOutputRequest(const json::Node& output_request); // Добавляет запрос на вывод информации
        void PrintOutputRequests(std::ostream& output); // Вывод информации в поток

        svg::Document RenderMap() const;

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        std::vector<json::Node> output_request_handler_;
        const transport_catalogue::TransportCatalogue& db_;
        const renderer::MapRenderer& renderer_;
    };
}