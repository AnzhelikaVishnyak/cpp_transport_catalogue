#include "serialization.h"

namespace Serialization {    
    void Serialize(transport_catalogue::data_base::TransportCatalogue &db
                    , const map_renderer::RenderSettings& render_settings
                    , const transport_router::TransportRouter &router
                    , std::ostream &output)
    {
        proto_transport_db::TransportCatalogue proto_db;

        SerializeStops(db, proto_db);
        SerializeBuses(db, proto_db);
        SerializeDistances(db, proto_db);
        SerializeRenderSettings(render_settings, proto_db);
        SerializeTransportRouter(router, proto_db);

        proto_db.SerializeToOstream(&output);
    }

    std::tuple<map_renderer::RenderSettings, transport_router::TransportRouter, graph::DirectedWeightedGraph<double>> Deserialize(transport_catalogue::data_base::TransportCatalogue& db, std::istream& input)
    {        
        proto_transport_db::TransportCatalogue proto_db;
        proto_db.ParseFromIstream(&input);       
        
        DeserializeStops(db, proto_db);
        DeserializeDistances(db, proto_db);
        DeserializeBuses(db, proto_db); 

        map_renderer::RenderSettings settings = DeserializeRenderSettings(proto_db);        
        transport_router::TransportRouter router = DeserializeTransportRouter(db, proto_db.router());
        graph::DirectedWeightedGraph<double> graph = DeserializeGraph(proto_db.router().graph());

        return {std::move(settings), std::move(router), std::move(graph)};       
    }
//---------------------- Supporting Serialize Methods --------------------
    void SerializeStops(const transport_catalogue::data_base::TransportCatalogue &db, proto_transport_db::TransportCatalogue &proto_db)
    {
        const auto all_stops = db.GetStops();        
        for(const auto& stop : all_stops) {
            proto_transport_db::Stop proto_stop;
            proto_stop.set_name(stop.name);
            proto_stop.set_latitude(stop.latitude);
            proto_stop.set_longitude(stop.longitude);

            *proto_db.add_stops() = std::move(proto_stop);            
        }
    }

    void SerializeBuses(const transport_catalogue::data_base::TransportCatalogue &db, proto_transport_db::TransportCatalogue &proto_db)
    {
        const auto all_buses = db.GetBuses();
        for(const auto& bus : all_buses) {
            proto_transport_db::Bus proto_bus;
            proto_bus.set_name(bus.name);
            for(const auto& stop : bus.stops) {
                proto_bus.add_stops(stop->name);
            }
            proto_bus.set_is_circle(transport_catalogue::data_base::RouteType::CIRCLE == bus.type);
            if(bus.second_final_stop.has_value()) {
                proto_bus.set_second_final_stop(bus.second_final_stop.value()->name);
            }

            *proto_db.add_buses() = std::move(proto_bus);
        }        
    }

    void SerializeDistances(const transport_catalogue::data_base::TransportCatalogue &db, proto_transport_db::TransportCatalogue &proto_db)
    {
        const auto all_distances = db.GetAllWays();
        for(const auto& [stops, distance] : all_distances) {
            proto_transport_db::Distances proto_distance;
            proto_distance.set_from(stops.first->name);
            proto_distance.set_to(stops.second->name);
            proto_distance.set_distance(distance);

            *proto_db.add_distance() = std::move(proto_distance);
        }
    }

    void SerializeRenderSettings(const map_renderer::RenderSettings& render_settings, proto_transport_db::TransportCatalogue &proto_db)
    {
        proto_map::RenderSettings proto_render_settings;
        proto_render_settings.set_width(render_settings.width);
        proto_render_settings.set_height(render_settings.height);
        proto_render_settings.set_padding(render_settings.padding);
        proto_render_settings.set_line_width(render_settings.line_width);
        proto_render_settings.set_stop_radius(render_settings.stop_radius);
        proto_render_settings.set_bus_label_font_size(render_settings.bus_label_font_size);
        *proto_render_settings.mutable_bus_label_offset() = SerializePoint(render_settings.bus_label_offset);
        proto_render_settings.set_stop_label_font_size(render_settings.stop_label_font_size);
        *proto_render_settings.mutable_stop_label_offset() = SerializePoint(render_settings.stop_label_offset);
        *proto_render_settings.mutable_underlayer_color() = SerializeColor(render_settings.underlayer_color);
        proto_render_settings.set_underlayer_width(render_settings.underlayer_width);
        for(const auto& color : render_settings.color_palette) {
            *proto_render_settings.add_color_palette() = SerializeColor(color);
        }
        *proto_db.mutable_render_settings() = std::move(proto_render_settings);
    }

    proto_map::Point SerializePoint(const svg::Point &point)
    {
        proto_map::Point proto_point;
        proto_point.set_x(point.x);
        proto_point.set_y(point.y);
        return std::move(proto_point);
    }

    proto_map::Color SerializeColor(const svg::Color &color)
    {
        proto_map::Color proto_color;
        if(std::holds_alternative<std::string>(color)) {
            proto_color.set_color(std::get<std::string>(color));
        } else if (std::holds_alternative<svg::Rgb>(color)) {
            *proto_color.mutable_rgb() = SerializeRgb(std::get<svg::Rgb>(color));
        } else if (std::holds_alternative<svg::Rgba>(color)) {
            *proto_color.mutable_rgba() = SerializeRgba(std::get<svg::Rgba>(color));
        }

        return std::move(proto_color);
    }

    proto_map::Rgb SerializeRgb(const svg::Rgb &rgb)
    {
        proto_map::Rgb proto_rgb;
        proto_rgb.set_red(rgb.red);
        proto_rgb.set_green(rgb.green);
        proto_rgb.set_blue(rgb.blue);
        return std::move(proto_rgb);
    }

    proto_map::Rgba SerializeRgba(const svg::Rgba &rgba)
    {
        proto_map::Rgba proto_rgba;
        proto_rgba.set_red(rgba.red);
        proto_rgba.set_green(rgba.green);
        proto_rgba.set_blue(rgba.blue);
        proto_rgba.set_opacity(rgba.opacity);
        return std::move(proto_rgba);
    }

    proto_transport_db::EdgeInfo SerializeEdgeInfo(const transport_router::EdgeInfo &edge_info)
    {
        using namespace std::string_literals;
        proto_transport_db::EdgeInfo proto_edge_info;
        const auto edge_type = edge_info.type;        

        switch (edge_type)
        {
        case transport_router::EdgeType::WAIT:
            proto_edge_info.set_name(edge_info.stop_name);
            proto_edge_info.set_type("Wait"s);
            break;  
        case transport_router::EdgeType::BUS:
            proto_edge_info.set_name(edge_info.bus);
            proto_edge_info.set_type("Bus"s);
            break;      
        default:
            break;
        }

        proto_edge_info.set_span_count(edge_info.span_count);
        return std::move(proto_edge_info);
    }

    proto_graph::Graph SerializeGraph(const graph::DirectedWeightedGraph<double> &graph)
    {
        proto_graph::Graph proto_graph;

        for(size_t i = 0; i < graph.GetEdgeCount(); ++i) {
            const auto edge = graph.GetEdge(i);
            proto_graph::Edge proto_edge;
            proto_edge.set_from(edge.from);
            proto_edge.set_to(edge.to);
            proto_edge.set_weight(edge.weight);
            *proto_graph.add_edge() = proto_edge;
        }

        for(size_t i = 0; i < graph.GetVertexCount(); ++i) {
            proto_graph::IncidenceList proto_list;
            for(const auto& edge_id : graph.GetIncidentEdges(i)) {
                proto_list.add_edge_id(edge_id);
            }
            *proto_graph.add_incidence_list() = proto_list;
        }

        return std::move(proto_graph);
    }

    void SerializeTransportRouter(const transport_router::TransportRouter &router, proto_transport_db::TransportCatalogue &proto_db)
    {
        proto_transport_db::Router proto_router;
        proto_router.set_bus_wait_time(router.GetBusWaitTime());
        proto_router.set_bus_velocity(router.GetBusVelocity());
        *proto_router.mutable_graph() = SerializeGraph(router.GetGraph());

        for(const auto& [stop_name, stop_id] : router.GetStopnameToStopIdMap()) {
            proto_transport_db::StopNameToId proto_stop;
            proto_stop.set_stop_name(stop_name);
            proto_stop.set_stop_id(stop_id);
            *proto_router.add_stopname_to_id() = proto_stop;
        }

        for(const auto& [edge_id, edge_info] : router.GetEdgeIdToInfoMap()) {
            proto_transport_db::EdgeIdToInfo proto_edge_id_to_info;
            proto_edge_id_to_info.set_edge_id(edge_id);
            *proto_edge_id_to_info.mutable_edge_info() = SerializeEdgeInfo(edge_info);
            *proto_router.add_edge_id_to_info() = std::move(proto_edge_id_to_info);
        }

        *proto_db.mutable_router() = std::move(proto_router);
    }
//------------------ Supporting Deserialize Methods ------------------------------
    void DeserializeStops(transport_catalogue::data_base::TransportCatalogue &db, const proto_transport_db::TransportCatalogue &proto_db)
    {
        for(size_t i = 0; i < proto_db.stops_size(); ++i) {
            const auto proto_stop = proto_db.stops(i);
            db.AddStop(proto_stop.name(), proto_stop.latitude(), proto_stop.longitude());            
        }
    }

    void DeserializeBuses(transport_catalogue::data_base::TransportCatalogue &db, const proto_transport_db::TransportCatalogue &proto_db)
    {
        for(size_t i = 0; i < proto_db.buses_size(); ++i) {
            const auto proto_bus = proto_db.buses(i);
            std::vector<std::string> stops;
            stops.reserve(proto_bus.stops_size());
            for(size_t j = 0; j < proto_bus.stops_size(); ++j) {
                stops.push_back(proto_bus.stops(j));
            }
            db.AddBus(proto_bus.name(), stops, proto_bus.is_circle(), proto_bus.second_final_stop());
        }
    }

    void DeserializeDistances(transport_catalogue::data_base::TransportCatalogue &db, const proto_transport_db::TransportCatalogue &proto_db)
    {
        for(size_t i = 0; i < proto_db.distance_size(); ++i) {
            const auto proto_distance = proto_db.distance(i);
            db.AddWay(proto_distance.from(), proto_distance.to(), proto_distance.distance());
        }
    }
    map_renderer::RenderSettings DeserializeRenderSettings(const proto_transport_db::TransportCatalogue &proto_db)
    {
        proto_map::RenderSettings proto_settings = proto_db.render_settings();
        map_renderer::RenderSettings render_settings;
        render_settings.width = proto_settings.width();
        render_settings.height = proto_settings.height();
        render_settings.padding = proto_settings.padding();
        render_settings.line_width = proto_settings.line_width();
        render_settings.stop_radius = proto_settings.stop_radius();
        render_settings.bus_label_font_size = proto_settings.bus_label_font_size();
        render_settings.bus_label_offset = DeserializePoint(proto_settings.bus_label_offset());
        render_settings.stop_label_font_size = proto_settings.stop_label_font_size();
        render_settings.stop_label_offset = DeserializePoint(proto_settings.stop_label_offset());
        render_settings.underlayer_color = DeserializeColor(proto_settings.underlayer_color());
        render_settings.underlayer_width = proto_settings.underlayer_width();
        
        render_settings.color_palette.reserve(proto_settings.color_palette_size());
        for(size_t i = 0; i < proto_settings.color_palette_size(); ++i) {
            render_settings.color_palette.emplace_back(DeserializeColor(proto_settings.color_palette(i)));
        }

        return std::move(render_settings);
    }
    svg::Point DeserializePoint(const proto_map::Point &proto_point)
    {
        svg::Point point;
        point.x = proto_point.x();
        point.y = proto_point.y();
        return std::move(point);
    }
    svg::Color DeserializeColor(const proto_map::Color &proto_color)
    {
        svg::Color color;
        if(proto_color.has_rgb()) {
            svg::Rgb rgb(proto_color.rgb().red(), proto_color.rgb().green(), proto_color.rgb().blue());
            color = svg::Color(rgb);
        } else if(proto_color.has_rgba()) {
            svg::Rgba rgba(proto_color.rgba().red(), proto_color.rgba().green(), proto_color.rgba().blue(), proto_color.rgba().opacity());
            color = svg::Color(rgba);
        } else {
            color = proto_color.color();            
        }

        return std::move(color);
    }
    transport_router::EdgeInfo DeserializeEdgeInfo(const proto_transport_db::EdgeInfo &proto_info)
    {
        using namespace std::string_literals;
        transport_router::EdgeInfo edge_info;
        std::string type = proto_info.type();
        if ("Wait"s == type) {
            edge_info.type = transport_router::EdgeType::WAIT;
            edge_info.stop_name = proto_info.name();
        } else if ("Bus"s == type) {
            edge_info.type = transport_router::EdgeType::BUS;
            edge_info.bus = proto_info.name();
        }

        edge_info.span_count = proto_info.span_count();
        return std::move(edge_info);
    }
    graph::Edge<double> DeserializeEdge(const proto_graph::Edge &proto_edge)
    {
        graph::Edge<double> edge;
        edge.from = proto_edge.from();
        edge.to = proto_edge.to();
        edge.weight = proto_edge.weight();

        return std::move(edge);
    }
    std::vector<graph::EdgeId> DeserializeIncidenceList(const proto_graph::IncidenceList &proto_incidence_list)
    {
        std::vector<graph::EdgeId> list;
        list.reserve(proto_incidence_list.edge_id_size());
        for(size_t i = 0; i < proto_incidence_list.edge_id_size(); ++i) {
            list.push_back(proto_incidence_list.edge_id(i));
        }
        return std::move(list);
    }
    graph::DirectedWeightedGraph<double> DeserializeGraph(proto_graph::Graph proto_graph)
    {
        std::vector<graph::Edge<double>> edges;
        edges.reserve(proto_graph.edge_size());
        for(size_t i = 0; i < proto_graph.edge_size(); ++i) {           
            edges.emplace_back(DeserializeEdge(proto_graph.edge(i)));
        }  

        std::vector<std::vector<graph::EdgeId>> incidence_lists;
        incidence_lists.reserve(proto_graph.incidence_list_size());
        for(size_t i = 0; i < proto_graph.incidence_list_size(); ++i) {
            incidence_lists.emplace_back(DeserializeIncidenceList(proto_graph.incidence_list(i)));
        }

        return graph::DirectedWeightedGraph<double>(edges, incidence_lists);
    }
    std::map<std::string, graph::VertexId> DeserializeStopnameToStopIdMap(const proto_transport_db::Router &proto_router)
    {
        std::map<std::string, graph::VertexId> stopname_to_stop_id;
        for(const auto& proto_stopname_id : proto_router.stopname_to_id()) {
            stopname_to_stop_id[proto_stopname_id.stop_name()] = proto_stopname_id.stop_id();
        }
        return std::move(stopname_to_stop_id);
    }
    std::map<graph::EdgeId, transport_router::EdgeInfo> DeserializeEdgeIdToInfoMap(const proto_transport_db::Router &proto_router)
    {
        std::map<graph::EdgeId, transport_router::EdgeInfo> edge_id_to_info;
        for(const auto& proto_edge_id_info : proto_router.edge_id_to_info()) {
            edge_id_to_info[proto_edge_id_info.edge_id()] = DeserializeEdgeInfo(proto_edge_id_info.edge_info());
        }
        return std::move(edge_id_to_info);
    }
    transport_router::TransportRouter DeserializeTransportRouter(transport_catalogue::data_base::TransportCatalogue &db, const proto_transport_db::Router &proto_router)
    {
        transport_router::TransportRouter router(db);
        
        router.SetBusWaitTime(proto_router.bus_wait_time());
        router.SetBusVelocity(proto_router.bus_velocity());
        router.SetStopnameToStopIdMap(DeserializeStopnameToStopIdMap(proto_router));
        router.SetEdgeIdToInfoMap(DeserializeEdgeIdToInfoMap(proto_router));
        
        return std::move(router);
    }
}