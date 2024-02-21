#include "request_handler.h"

namespace handler {
	RequestHandler::RequestHandler(const transport_catalogue::data_base::TransportCatalogue& db, const map_renderer::MapRenderer& renderer, const transport_router::TransportRouter& router)
		: db_(db), renderer_(renderer), router_(router) {}

	transport_catalogue::data_base::BusInfo RequestHandler::GetBusInfo(const std::string_view& bus_name) const
	{
		return db_.GetBusInfo(bus_name);
	}

	transport_catalogue::data_base::StopInfo RequestHandler::GetStopInfo(const std::string_view& stop_name) const
	{
		return db_.GetStopInfo(stop_name);
	}

	const svg::Document& RequestHandler::RenderMap() const
	{
		return renderer_.GetDocument();
	}

	std::optional<graph::Router<double>::RouteInfo> RequestHandler::BuildRoute(const std::string& from, const std::string& to) const
	{
		return router_.BuildRoute(from, to);	
	}
    
    std::pair<const graph::Edge<double>&, const transport_router::EdgeInfo&> RequestHandler::GetFullEdgeInfo(graph::EdgeId edge_id) const {
        return router_.GetFullEdgeInfo(edge_id);
    }
}
