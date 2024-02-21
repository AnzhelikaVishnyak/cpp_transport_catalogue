#include "transport_router.h"

namespace transport_router {
    TransportRouter::TransportRouter(const TransportCatalogue &tc)
		: tc_(tc) {}

    TransportRouter::TransportRouter(const transport_catalogue::data_base::TransportCatalogue& tc, const json::Dict& router_settings)
		: tc_(tc)
	{
		bus_wait_time_ = router_settings.at("bus_wait_time"s).AsInt();
		bus_velocity_ = router_settings.at("bus_velocity"s).AsDouble();
		MakeGraph();
	}

	std::optional<graph::Router<double>::RouteInfo> TransportRouter::BuildRoute(const std::string& from, const std::string& to) const
	{
		return router_.get()->BuildRoute(stopname_to_stop_id_.at(from), stopname_to_stop_id_.at(to));
	}

	void TransportRouter::MakeGraph()
	{
		const std::deque<transport_catalogue::data_base::Stop>& all_stops = tc_.GetStops();
		size_t vertex_count = all_stops.size() * 2;
		graph::DirectedWeightedGraph<double> graph(vertex_count);
		graph_ = std::move(graph);

		AddAllWaitEdges(all_stops);
		
		const std::deque<transport_catalogue::data_base::Bus>& all_buses = tc_.GetBuses();

		for (const auto& bus : all_buses) {
			switch (bus.type)
			{
			case transport_catalogue::data_base::RouteType::CIRCLE:
				AddCircleBusEdges(bus);
				break;
			case transport_catalogue::data_base::RouteType::TWO_DIRECTIONAL:
				AddLineBusEdges(bus);
				break;
			}
		}	
		router_ = std::make_unique<graph::Router<double>>(graph_);
	}

	void TransportRouter::AddAllWaitEdges(const std::deque<transport_catalogue::data_base::Stop>& all_stops)
	{
		graph::VertexId stop_id = 0;

		for (const auto& stop : all_stops) {
			stopname_to_stop_id_[stop.name] = stop_id;
			graph::EdgeId edge_id = graph_.AddEdge({ stop_id++, stop_id++, static_cast<double>(bus_wait_time_) });
			EdgeInfo edge_info;
			edge_info.type = EdgeType::WAIT;
			edge_info.stop_name = stop.name;
			edge_id_to_info_[edge_id] = std::move(edge_info);
		}
	}

	void TransportRouter::AddBusEdges(const Bus& bus, std::vector<Stop*>::const_iterator begin,  std::vector<Stop*>::const_iterator end) {
		double conversion_ratio = 1000.0 / 60.0;		// convertion from km/h to m/min
		for (auto item = begin; item != end - 1; ++item) {
			int span_count = 0;
			double distance = 0;
			auto prev = item;
			for (auto it = item + 1; it != end; ++it) {
				++span_count;
				EdgeInfo edge_info;
				edge_info.type = EdgeType::BUS;
				edge_info.bus = bus.name;
				edge_info.span_count = span_count;
				distance += tc_.FindWay({ *prev, *it });
				prev = it;
				graph::EdgeId edge_id = graph_.AddEdge({ stopname_to_stop_id_.at((*item)->name) + 1
					, stopname_to_stop_id_.at((*it)->name)
					, ((distance) / (bus_velocity_ * conversion_ratio)) });
				edge_id_to_info_[edge_id] = std::move(edge_info);
			}
		}
	}

	void TransportRouter::AddCircleBusEdges(const Bus& bus) {
		AddBusEdges(bus, bus.stops.cbegin(), bus.stops.cend());
	}

	void TransportRouter::AddLineBusEdges(const Bus& bus) {
		size_t mid_num = bus.stops.size() / 2;
		std::vector<Stop* >::const_iterator mid = bus.stops.begin() + mid_num;
		AddBusEdges(bus, bus.stops.cbegin(), mid + 1);
		AddBusEdges(bus, mid, bus.stops.cend());
	}
    
	std::pair<const graph::Edge<double>&, const transport_router::EdgeInfo&> TransportRouter::GetFullEdgeInfo(graph::EdgeId edge_id) const {
		return { graph_.GetEdge(edge_id), edge_id_to_info_.at(edge_id) };
	}
    void TransportRouter::SetGraph(graph::DirectedWeightedGraph<double> graph)
    {
		graph_ = std::move(graph);
		router_ = std::make_unique<graph::Router<double>>(graph_);
    }
    void TransportRouter::SetBusWaitTime(int bus_wait_time)
    {
		bus_wait_time_ = bus_wait_time;
    }
    void TransportRouter::SetBusVelocity(double bus_velocity)
    {
		bus_velocity_ = bus_velocity;
    }
    void TransportRouter::SetStopnameToStopIdMap(std::map<std::string, graph::VertexId> stopname_to_stop_id)
    {
		stopname_to_stop_id_ = std::move(stopname_to_stop_id);
    }
    void TransportRouter::SetEdgeIdToInfoMap(std::map<graph::EdgeId, EdgeInfo> edge_id_to_info)
    {
		edge_id_to_info_ = std::move(edge_id_to_info);
    }
    const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() const
    {
        return graph_;
    }
    const int TransportRouter::GetBusWaitTime() const
    {
        return bus_wait_time_;
    }
    const double TransportRouter::GetBusVelocity() const
    {
        return bus_velocity_;
    }
    const std::map<std::string, graph::VertexId>& TransportRouter::GetStopnameToStopIdMap() const
    {
        return stopname_to_stop_id_;
    }
    const std::map<graph::EdgeId, EdgeInfo>& TransportRouter::GetEdgeIdToInfoMap() const
    {
        return edge_id_to_info_;
    }
}
