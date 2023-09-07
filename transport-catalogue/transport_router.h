#pragma once
#include <string_view>
#include <memory>
#include <iostream>

#include "transport_catalogue.h"
#include "router.h"

namespace transport_router {

    class TransportRouter {
    public:
        struct RouteSettings {
            int wait_time; // время ожидания на остановке
            double velocity; // скорость автобуса
        };

        enum class ItemType {
            WAIT,
            BUS
        };
    private:
        struct Item {
            ItemType type;
            std::string_view route_name;
            double time;
            int span_count;
        };
    public:
        struct RouteInfo {
            double time;
            std::vector<Item> items;
        };

    public:
        TransportRouter(const transport_catalogue::TransportCatalogue& catalogue);

        // Устанавливаем настройки маршрутизатора
        TransportRouter& SetRouteSettings(const RouteSettings& r_settings);

        //Строим граф и создаем маршрутизатор
        void BuildRoutes ();

        std::optional<RouteInfo> GetRouteInfo(const Stop* from, const Stop* to) const;

    private:
        // Номер вершины графа (с ожиданием)
        graph::VertexId GetStopVertexID(const Stop* from) const;

        // Номер вершины графа (без ожидания)
        graph::VertexId GetStartVertexID(const Stop* to) const;

        // Добавляем ребра маршрутов в граф
        void CreateRouteEdge(graph::VertexId from, graph::VertexId to, const std::string_view bus_name,
                             double length, int span_count);

        // Добавляем вершины графа и ребра ожидания
        void AddVertexesAndWaitEdges();

        //Добавляем ребро некольцевого маршрута
        void AddSimpleRouteEdge(const std::string_view bus_name, const Bus* bus_ptr);

        //Добавляем ребро кольцевого маршрута
        void AddRoundRouteEdge(const std::string_view bus_name, const Bus* bus_ptr);

        //Добавляем ребра маршрутов между остановками
        void AddRouteEdges();

        const transport_catalogue::TransportCatalogue& catalogue_;
        RouteSettings r_settings_; // Настройки (скорость и время ожидания) маршрута
        std::unique_ptr<graph::DirectedWeightedGraph<double>> graph_; // Граф
        std::map<const Stop*, std::pair<int, int>> vertexes_; // Вершины графа
        std::map<graph::EdgeId, Item> edges_; // Ребра графа
        std::unique_ptr<graph::Router<double>> router_; // Маршрутизатор
    };
} // namespace transport_router