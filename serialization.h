#pragma once

#include "transport_catalogue.pb.h"
//#include "svg.pb.h"
//#include "map_renderer.pb.h"
#include "request_handler.h"

namespace Serialization {
    void Serialize(transport_catalogue::data_base::TransportCatalogue& db, const map_renderer::RenderSettings& render_settings, const transport_router::TransportRouter &router, std::ostream& output);
    std::tuple<map_renderer::RenderSettings, transport_router::TransportRouter, graph::DirectedWeightedGraph<double>> Deserialize(transport_catalogue::data_base::TransportCatalogue& db, std::istream& input);

    void SerializeStops(const transport_catalogue::data_base::TransportCatalogue& db, proto_transport_db::TransportCatalogue& proto_db);
    void SerializeBuses(const transport_catalogue::data_base::TransportCatalogue& db, proto_transport_db::TransportCatalogue& proto_db);
    void SerializeDistances(const transport_catalogue::data_base::TransportCatalogue& db, proto_transport_db::TransportCatalogue& proto_db);
    void SerializeRenderSettings(const map_renderer::RenderSettings& render_settings, proto_transport_db::TransportCatalogue& proto_db);
    proto_map::Point SerializePoint(const svg::Point& point);
    proto_map::Color SerializeColor(const svg::Color& color);
    proto_map::Rgb SerializeRgb(const svg::Rgb& rgb);
    proto_map::Rgba SerializeRgba(const svg::Rgba& rgba);  
    proto_transport_db::EdgeInfo SerializeEdgeInfo(const transport_router::EdgeInfo& edge_info);
    proto_graph::Graph SerializeGraph(const graph::DirectedWeightedGraph<double>& graph); 
    void SerializeTransportRouter(const transport_router::TransportRouter& router, proto_transport_db::TransportCatalogue &proto_db);

    void DeserializeStops(transport_catalogue::data_base::TransportCatalogue& db, const proto_transport_db::TransportCatalogue& proto_db);
    void DeserializeBuses(transport_catalogue::data_base::TransportCatalogue& db, const proto_transport_db::TransportCatalogue& proto_db);
    void DeserializeDistances(transport_catalogue::data_base::TransportCatalogue& db, const proto_transport_db::TransportCatalogue& proto_db);
    map_renderer::RenderSettings DeserializeRenderSettings(const proto_transport_db::TransportCatalogue& proto_db);
    svg::Point DeserializePoint(const proto_map::Point& proto_point);
    svg::Color DeserializeColor(const proto_map::Color& proto_color);
    transport_router::EdgeInfo DeserializeEdgeInfo(const proto_transport_db::EdgeInfo& proto_info);
    graph::Edge<double> DeserializeEdge(const proto_graph::Edge& proto_edge);
    std::vector<graph::EdgeId> DeserializeIncidenceList(const proto_graph::IncidenceList& proto_incidence_list);
    graph::DirectedWeightedGraph<double> DeserializeGraph(proto_graph::Graph proto_graph);
    std::map<std::string, graph::VertexId> DeserializeStopnameToStopIdMap(const proto_transport_db::Router &proto_router);
    std::map<graph::EdgeId, transport_router::EdgeInfo> DeserializeEdgeIdToInfoMap(const proto_transport_db::Router &proto_router);
    transport_router::TransportRouter DeserializeTransportRouter(transport_catalogue::data_base::TransportCatalogue& db, const proto_transport_db::Router &proto_router);
    
     
}