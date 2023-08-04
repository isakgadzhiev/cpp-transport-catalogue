#pragma once

#include <iostream>
#include <string_view>
#include <iomanip>

#include "transport_catalogue.h"

namespace input_reader {

    struct Requests {
        std::vector<std::string> buses;
        std::vector<std::string> stops;
    };

    enum class RequestType {
        BUS,
        STOP
    };

    namespace detail_input {
        std::string ReadLine(std::istream& input);
        int ReadLineWithNumber(std::istream& input);
    }

    using namespace transport_catalogue;

    std::vector<std::string> SplitIntoStops(std::string_view text, Bus& bus); //Выделяем остановки из маршрута

    std::string_view GetCommand(const std::string_view& request); //Выделяем тип запроса из строки
    RequestType GetRequestType(const std::string_view& request); //Определяем тип запроса
    std::string GetNameFromRequest(std::string_view request); //Выделяем название остановки или маршрута из запроса
    bool GetRouteType(const std::string_view& request);

    void SetDistanceBetweenStops(TransportCatalogue& catalogue, std::string_view rest_request); //Задаем дистанцию между остановками из запроса

    Stop GetStopFromRequest(std::string_view request); //Выделяем информацию об остановке из запроса
    Bus GetBusFromRequest(TransportCatalogue& catalogue, const std::string_view& request); //Выделяем информацию об автобусе из запроса

    void GetInputRequest(TransportCatalogue& catalogue, std::istream& input); //Обрабатываем запроса на добавление в базу
}