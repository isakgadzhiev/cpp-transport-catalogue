#pragma once

#include "transport_catalogue.h"
#include "json_builder.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_router.h"

namespace json_reader {
    struct Requests {
        std::vector<json::Node> buses;
        std::vector<json::Node> stops;
    };

    //________________________Вспомогательные функции для выдачи ответов на запросы из JSON

    // Обработка запроса на построение маршрута
    void GetRouteRequest(const json::Dict& request, request_handler::RequestHandler& request_handler,
                         json::Builder& builder);

    // Обработка запроса на получение изображения
    void GetMapRequest(const json::Dict& request, request_handler::RequestHandler& request_handler,
                       json::Builder& builder);
    // Получаем инфо об остановке из справочника
    void GetStopInfoForOutput(const json::Dict& request, request_handler::RequestHandler& request_handler,
                              json::Builder& builder);
    // Получаем инфо о маршруте из справочника
    void GetBusInfoForOutput(const json::Dict& request, request_handler::RequestHandler& request_handler,
                             json::Builder& builder);
    // Получаем все остановки из информации о маршруте
    std::vector<std::string_view> GetStopsFromBusInfo(const json::Dict& bus_info);

    //Выделяем информацию о маршруте из запроса
    Bus SetBusFromJsonRequest(transport_catalogue::TransportCatalogue& catalogue, const json::Dict& bus_info);

    //Выделяем информацию об остановке из запроса
    Stop SetStopFromJsonRequest(const json::Dict& stop_info);

    //Задаем дистанцию между остановками из запроса
    void SetDistanceBetweenStops(transport_catalogue::TransportCatalogue& catalogue, const json::Dict& stop_info);

    //Выделяем цвет из соответствующего JSON-узла
    svg::Color GetColorFromRequest(const json::Node& color_request);

    //________________________Разбиваем JSON на типовые запросы

    // Получаем параметры визуализатора (запрос render_settings)
    void GetRenderJsonRequest(request_handler::RequestHandler& request_handler, renderer::MapRenderer& map_renderer,
                              const json::Dict& request_info);

    // Добавляем информацию в базу (запрос base_requests)
    void GetInputJsonRequest(transport_catalogue::TransportCatalogue& catalogue, const json::Array& request_info);

    // Получаем информацию из базы (запрос stat_requests)
    void GetOutputJsonRequest(request_handler::RequestHandler& request_handler, const json::Array& request_info,
                              [[maybe_unused]] std::ostream& output);

    // Получаем параметры для построения маршрута (запрос routing_settings)
    void GetRouteJsonRequest(transport_router::TransportRouter& router, const json::Dict& request_info);

    //Разделяем JSON на типовые запросы
    void GetJsonRequest(transport_catalogue::TransportCatalogue& catalogue, renderer::MapRenderer& map_renderer,
                        transport_router::TransportRouter& router, std::istream& input, std::ostream& output);
}