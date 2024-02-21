#pragma once

#include <map>
#include <memory>

#include "router.h"
#include "transport_catalogue.h"
#include "json.h"

namespace transport_router {
	using namespace std::string_literals;

	enum EdgeType
	{
		WAIT,
		BUS,
	};


	struct EdgeInfo
	{
		EdgeType type{};
		std::string stop_name{};
		std::string bus{};
        int span_count = 0;
	};
    
	class TransportRouter {
	public:

		using TransportCatalogue = transport_catalogue::data_base::TransportCatalogue;
		using Stop = transport_catalogue::data_base::Stop;
		using Bus = transport_catalogue::data_base::Bus;

		TransportRouter(const TransportCatalogue& tc);

		TransportRouter(const TransportCatalogue& tc, const json::Dict& router_settings);

		std::optional<graph::Router<double>::RouteInfo> BuildRoute(const std::string& from, const std::string& to) const;
        
        std::pair<const graph::Edge<double>&, const transport_router::EdgeInfo&> GetFullEdgeInfo(graph::EdgeId edge_id) const;
		
		void SetGraph(graph::DirectedWeightedGraph<double> graph);
		void SetBusWaitTime(int bus_wait_time);
		void SetBusVelocity(double bus_velocity);
		void SetStopnameToStopIdMap(std::map<std::string, graph::VertexId> stopname_to_stop_id);
		void SetEdgeIdToInfoMap(std::map<graph::EdgeId, EdgeInfo> edge_id_to_info);

		const graph::DirectedWeightedGraph<double>& GetGraph() const;
		const int GetBusWaitTime() const;
		const double GetBusVelocity() const;
		const std::map<std::string, graph::VertexId>& GetStopnameToStopIdMap() const;
		const std::map<graph::EdgeId, EdgeInfo>& GetEdgeIdToInfoMap() const;


	private:

		void MakeGraph();
		void AddAllWaitEdges(const std::deque<Stop>& all_stops);
		void AddBusEdges(const Bus& bus, std::vector<Stop*>::const_iterator begin, std::vector<Stop*>::const_iterator end);
		void AddCircleBusEdges(const Bus& bus);
		void AddLineBusEdges(const Bus& bus);

		const TransportCatalogue& tc_;

		int bus_wait_time_;
		double bus_velocity_;

		graph::DirectedWeightedGraph<double> graph_;
		std::unique_ptr<graph::Router<double>> router_;
		std::map<std::string, graph::VertexId> stopname_to_stop_id_;
        std::map<graph::EdgeId, EdgeInfo> edge_id_to_info_;
	};
}