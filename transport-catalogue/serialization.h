#pragma once
#include <fstream>

#include "transport_catalogue.pb.h"
#include "map_renderer.pb.h"
#include "svg.pb.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_router.h"
#include "transport_router.pb.h"

namespace serialize {

class Serializer {
public:
    Serializer(transport_catalogue::TransportCatalogue& transport_catalogue,
               renderer::MapRenderer& map_renderer, transport_router::TransportRouter& router)
        : transport_catalogue_(transport_catalogue)
        , map_renderer_(map_renderer)
        , router_(router) {
    }

    void SetSetting(const std::string& file_name);

    void SerializeToFile();
    void DeserializeFromFile();

private:
    std::string file_name_;
    transport_catalogue::TransportCatalogue& transport_catalogue_;
    renderer::MapRenderer& map_renderer_;
    transport_router::TransportRouter& router_;

    // Сериализация/десериализация запросов информации по остановкам и маршрутам
    using ProtoCatalogue = proto_catalogue::TransportCatalogue;

    proto_catalogue::Stop GetSerializeStop(const Stop* stop_ptr);
    Stop GetDeserializeStop(const proto_catalogue::Stop& proto_stop);
    proto_catalogue::Bus GetSerializeBus(const Bus* bus_ptr);
    Bus GetDeserializeBus(const proto_catalogue::Bus& proto_bus, const ProtoCatalogue& proto_trans_catalogue);
    proto_catalogue::Distances GetSerializeDistance(const Stop* from, const Stop* to, int distance);
    const Stop* GetStopPtr(uint64_t stop_id, const ProtoCatalogue& proto_trans_catalogue);

    // Сериализация/десериализация настроек построения карты маршрутов
    using MapSettings = renderer::MapRendererSettings;
    using ProtoMapSettings =  proto_catalogue::MapRendererSettings;

    proto_catalogue::Color GetSerializeColor(const svg::Color& color);
    ProtoMapSettings GetSerializeRenderSettings (const MapSettings& renderer_settings);
    svg::Color GetColor(const proto_catalogue::Color& proto_color);
    MapSettings GetDeserializeRenderSettings(const ProtoMapSettings& proto_settings);

    // Сериализация/десериализация настроек маршрутизатора
    using RouterSettings = transport_router::TransportRouter::RouteSettings;
    using ProtoRouterSettings = proto_catalogue::RouterSettings;

    ProtoRouterSettings GetSerializeRouterSettings(const RouterSettings& router_settings);
    RouterSettings GetDeserializeRouterSettings(const ProtoRouterSettings& proto_settings);
};
} //namespace serialize