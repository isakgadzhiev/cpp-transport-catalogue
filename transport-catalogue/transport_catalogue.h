#pragma once

#include "geo.h"
#include "domain.h"

#include <deque>
#include <map>
#include <unordered_map>
#include <optional>

namespace transport_catalogue {
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

    using RealDistanceTable = std::unordered_map<std::pair<const Stop*, const Stop*>, int, detail::Hasher<const Stop*>>;

    class TransportCatalogue {
    public:
        // Добавление данных об остановках / маршрутах в каталог
        void AddStop(const Stop& stop);
        void AddBus(const Bus& bus);

        // Поиск данных об остановке / маршруте по названию
        const Stop* FindStop(const std::string_view& stop_name) const;
        const Bus* FindBus(std::string_view bus_name) const;

        void SetDistance(const Stop* from, const Stop* to, int distance); // Запись дистанции между остановками в каталог
        int GetRealDistance(const Stop* from, const Stop* to) const; // Получение фактического расстояния между остановками

        std::optional<BusInfo> GetBusInfo(const Bus* bus) const; // Получение данных о маршруте
        std::optional<StopInfo> GetStopInfo(const std::string_view& stop_name) const; // Получение данных об остановке
        std::map<std::string_view, const Bus*> GetRouteNames() const; // Получение всех маршрутов из каталога
        std::unordered_map<std::string_view, const Stop*> GetStopNames() const; // Получение всех остановок из каталога

        size_t GetStopsCount() const;
        size_t GetBusesCount() const;

        const RealDistanceTable& GetAllDistances() const;

        // Тесты
        void TestGetStopNames();
        void TestGetBusNames();
        void TestGetDistancesBetweenStops();

    private:
        std::deque<Stop> stops_;  //Хранилище остановок
        std::deque<Bus> buses_;  // Хранилище маршрутов
        std::unordered_map<std::string_view, const Stop*> stop_names_; // Таблица названий остановок и указателей на данные о них
        std::map<std::string_view, const Bus*> route_names_; // Таблица названий маршрутов и указателей на данные о них
        RealDistanceTable real_distances_; // Хэш-таблица фактических расстояний между остановками
    };
}