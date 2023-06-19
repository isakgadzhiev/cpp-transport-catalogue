#pragma once

#include <string>
#include <deque>
#include <unordered_map>
#include <vector>
#include <optional>
#include <set>

#include "geo.h"

namespace transport_catalogue {
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
        int stops_count; // Число остановок
        int unique_stops_count; // Число уникальных остановок
        int route_length; // Длина маршрута
        double curvature; // Извилистость
    };

    struct StopInfo {
        std::set<std::string_view> buses; // Список маршрутов, проходящих через остановку
    };

    namespace detail {
        template <typename T>
        class Hasher{
        public:
            size_t operator() (const std::pair<T, T> pair_for_hash) const {
                return hasher_(pair_for_hash.first) ^ hasher_(pair_for_hash.second);
            }
        private:
            std::hash<T> hasher_;
        };
    }

    using DirectDistanceTable =  std::unordered_map<std::pair<const Stop*, const Stop*>, double, detail::Hasher<const Stop*>>;
    using RealDistanceTable =  std::unordered_map<std::pair<const Stop*, const Stop*>, int, detail::Hasher<const Stop*>>;

    class TransportCatalogue {
    public:
        // Добавление данных об остановках (маршрутах) в каталог
        void AddStop(const Stop& stop);
        void AddBus(const Bus& bus);

        // Поиск данных об остановке (маршруте) по названию
        const Stop* FindStop(const std::string_view& stop_name) const;
        const Bus* FindBus(std::string_view bus_name) const;

        void SetDistance(const Stop* from, const Stop* to, int distance); // Запись дистанции между остановками в каталог
        void SetDirectDistance(const Stop* from, const Stop* to, double distance); // Запись прямых расстояний между остановками
        void CountDirectDistances(const Bus* bus); // Расчет прямой длины маршрута
        int GetRealDistance(const Stop* from, const Stop* to) const; // Получение фактического расстояния между остановками
        double GetDirectDistance(const Stop* from, const Stop* to) const; // Получение прямого расстояния между остановками

        std::optional<BusInfo> GetBusInfo(const Bus* bus); // Получение данных о маршруте
        std::optional<StopInfo> GetStopInfo(const std::string_view& stop_name); // Получение данных об остановке
        const RealDistanceTable& GetAllDistances() const; // Для реализации тестов
        std::unordered_map<std::string_view, const Stop*> GetStopNames() const; // Для реализации тестов
        std::unordered_map<std::string_view, const Bus*> GetRouteNames() const; // Для реализации тестов

    private:
        std::deque<Stop> stops_;  //Хранилище остановок
        std::deque<Bus> buses_;  // Хранилище маршрутов
        std::unordered_map<std::string_view, const Stop*> stop_names_; // Таблица названий остановок и указателей на данные о них
        std::unordered_map<std::string_view, const Bus*> route_names_; // Таблица названий маршрутов и указателей на данные о них
        DirectDistanceTable direct_distances_; // Хэш-таблица прямых расстояний между остановками
        RealDistanceTable real_distances_; // Хэш-таблица фактических расстояний между остановками
    };

    namespace tests {
        void TestGetStopNames(const TransportCatalogue& catalogue);
        void TestGetBusNames(const TransportCatalogue& catalogue);
        void TestGetDistancesBetweenStops(const TransportCatalogue& catalogue);
    }
}