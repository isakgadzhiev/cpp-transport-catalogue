#pragma once

#include "transport_catalogue.h"
#include "json.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "svg.h"

namespace json_reader {

    struct Requests {
        std::vector<json::Node> buses;
        std::vector<json::Node> stops;
    };

    void GetMapRequest(const json::Dict& request,
                request_handler::RequestHandler& request_handler); // Выделяем запрос на получение изображения

    void GetStopInfoForOutput(const json::Dict& request,
                              request_handler::RequestHandler& request_handler); // Выделяем инфо об остановке из справочника
    void GetBusInfoForOutput(const json::Dict& request,
                             request_handler::RequestHandler& request_handler); // Выделяем инфо о маршруте из справочника
    std::vector<std::string_view> GetStopsFromBusInfo(const json::Dict& bus_info); // Выделяем остановки из информации о маршруте
    Bus SetBusFromJsonRequest(transport_catalogue::TransportCatalogue& catalogue,
                              const json::Dict& bus_info); //Выделяем информацию о маршруте из запроса
    Stop SetStopFromJsonRequest(const json::Dict& stop_info); //Выделяем информацию об остановке из запроса
    void SetDistanceBetweenStops(transport_catalogue::TransportCatalogue& catalogue,
                                 const json::Dict& stop_info); //Задаем дистанцию между остановками из запроса
    svg::Color GetColorFromRequest(const json::Node& color_request); //Выделяем цвет из JSON-узла

    //Обрабатываем запросы из JSON
    void GetRenderJsonRequest(request_handler::RequestHandler& request_handler,
                              renderer::MapRenderer& map_renderer,
                              const json::Node& request_info); // Получаем параметры визуализатора
    void GetInputJsonRequest(transport_catalogue::TransportCatalogue& catalogue,
                             const json::Array& request_info); // Получаем информацию в базу
    void GetOutputJsonRequest(request_handler::RequestHandler& request_handler,
                              const json::Node& request_info,
                              [[maybe_unused]] std::ostream& output); // Получаем информацию из базы
    void GetJsonRequest(transport_catalogue::TransportCatalogue& catalogue,
                        renderer::MapRenderer& map_renderer,
                        request_handler::RequestHandler& request_handler,
                        std::istream& input, std::ostream& output); //Разделяем JSON на запросы
}