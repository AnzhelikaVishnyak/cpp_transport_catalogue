#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "domain.h"
#include "svg.h"
#include "transport_router.h"

namespace handler {
    using namespace std::string_literals;
    class RequestHandler {
    public:   
         
        RequestHandler(const transport_catalogue::data_base::TransportCatalogue& db, const map_renderer::MapRenderer& renderer, const transport_router::TransportRouter& router);

        transport_catalogue::data_base::BusInfo GetBusInfo(const std::string_view& bus_name) const;
        transport_catalogue::data_base::StopInfo GetStopInfo(const std::string_view& stop_name) const;
        const svg::Document& RenderMap() const;
        std::optional<graph::Router<double>::RouteInfo> BuildRoute(const std::string& from, const std::string& to) const;
        std::pair<const graph::Edge<double>&, const transport_router::EdgeInfo&> GetFullEdgeInfo(graph::EdgeId edge_id) const;
	

    private:
        const transport_catalogue::data_base::TransportCatalogue& db_;
        const map_renderer::MapRenderer& renderer_;
        const transport_router::TransportRouter& router_;
    };
}
