#pragma once
#include "geo.h"

#include <vector>
#include <string>
#include <unordered_set>

struct Stop {
    std::string name; // Название остановки
    geo::Coordinates coordinates; // Координаты
};
struct Bus {
    std::string name; // Название маршрута
    std::vector<const Stop*> stops; // Остановки на маршруте
    bool is_round_route; // Проверка маршрута на кольцевой тип
};

struct BusInfo {
    int stops_count = 0; // Число остановок
    int unique_stops_count = 0; // Число уникальных остановок
    int route_length = 0; // Длина маршрута
    double curvature = 0.0; // Коэффициент извилистости
};

struct StopInfo {
    std::unordered_set<std::string_view> buses; // Список маршрутов, проходящих через остановку
};