#include "transport_catalogue.h"

namespace transport_catalogue {
	namespace data_base {
		void TransportCatalogue::AddStop(const std::string& name, double latitude, double longitude)
		{
			Stop stop = { name, latitude, longitude };
			stops_.emplace_back(std::move(stop));
			stopname_to_stop_[stops_.back().name] = &stops_.back();
			stop_to_buses_[FindStop(stops_.back().name)];
		}

		Stop* TransportCatalogue::FindStop(std::string_view name) const
		{
			if (!stopname_to_stop_.count(name)) {
				return nullptr;
			}

			return stopname_to_stop_.at(name);
		}

		void TransportCatalogue::AddWay(std::string_view from_stop, std::string_view to_stop, int distance)
		{
			stop_to_stop_distance_[{FindStop(from_stop), FindStop(to_stop)}] = distance;
		}

		int TransportCatalogue::FindWay(std::pair<Stop*, Stop*> from_stop_to_stop) const
		{
			if (stop_to_stop_distance_.count(from_stop_to_stop)) {
				return stop_to_stop_distance_.at(from_stop_to_stop);
			}
			std::pair<Stop*, Stop*> reversed_pair;
			reversed_pair = std::make_pair(from_stop_to_stop.second, from_stop_to_stop.first);
			if (stop_to_stop_distance_.count(reversed_pair)) {
				return stop_to_stop_distance_.at(reversed_pair);
			}
			return 0;
		}

        const std::unordered_map<std::pair<Stop *, Stop *>, int, TransportCatalogue::PairPtrHasher<Stop, Stop>> &TransportCatalogue::GetAllWays() const
        {
            return stop_to_stop_distance_;
        }

        void TransportCatalogue::AddBus(const std::string& bus, const std::vector<std::string>& stops, bool is_roundtrip, const std::string second_final_stop)
		{
			std::vector<Stop*> stops_ptr;
			stops_ptr.reserve(stops.size());
			for (const auto& stop : stops) {
				stops_ptr.push_back(FindStop(stop));
			}
			RouteType type;
			is_roundtrip ? type = RouteType::CIRCLE : type = RouteType::TWO_DIRECTIONAL;
			Bus new_bus = { bus, std::move(stops_ptr), type };
			if (!second_final_stop.empty()) {
				new_bus.second_final_stop.emplace(FindStop(second_final_stop));
			}
			buses_.emplace_back(std::move(new_bus));
			busname_to_bus_[buses_.back().name] = &buses_.back();
			const auto& current_bus = FindBus(buses_.back().name);
			for (const auto& stop_ptr : current_bus->stops) {
				stop_to_buses_[stop_ptr].insert(current_bus->name);
			}
		}

		Bus* TransportCatalogue::FindBus(std::string_view name) const
		{
			if (!busname_to_bus_.count(name)) {
				return nullptr;
			}

			return busname_to_bus_.at(name);
		}

		BusInfo TransportCatalogue::GetBusInfo(std::string_view name) const
		{
			const auto bus = FindBus(name);
			BusInfo bus_info;
			if (bus == nullptr) {
				bus_info.status = ReserchStatus::NOT_FOUND;
				bus_info.bus_name = name;
				return bus_info;
			}

			bus_info.status = ReserchStatus::FOUND;
			bus_info.bus_name = bus->name;
			bus_info.stops_count = bus->stops.size();

			std::unordered_set<std::string_view, StringViewHasher> unique_stops;

			geo::Coordinates from;
			geo::Coordinates to;
			bus_info.route_length = 0.;
			bool is_first = true;
			std::pair<Stop*, Stop*> from_stop_to_stop;
			for (const auto& stop : bus->stops) {
				if (is_first) {
					unique_stops.insert(stop->name);
					from.lat = stop->latitude;
					from.lng = stop->longitude;
					from_stop_to_stop.first = FindStop(stop->name);
					is_first = false;
					continue;
				}
				unique_stops.insert(stop->name);
				to.lat = stop->latitude;
				to.lng = stop->longitude;

				bus_info.route_length += ComputeDistance(from, to);
				from = to;

				from_stop_to_stop.second = FindStop(stop->name);
				bus_info.way += FindWay(from_stop_to_stop);
				from_stop_to_stop.first = from_stop_to_stop.second;
			}

			bus_info.unique_stops_count = unique_stops.size();

			return bus_info;
		}

		StopInfo TransportCatalogue::GetStopInfo(std::string_view name) const
		{
			StopInfo stop_info;
			if (FindStop(name) == nullptr) {
				stop_info.status = ReserchStatus::NOT_FOUND;
				stop_info.stop_name = name;
				return stop_info;
			}

			const auto& stop = FindStop(name);
			stop_info.status = ReserchStatus::FOUND;
			stop_info.stop_name = stop->name;
			stop_info.buses = stop_to_buses_.at(stop);
			return stop_info;
		}

		const std::deque<Bus>& TransportCatalogue::GetBuses() const
		{
			return buses_;
		}	
        
        const std::deque<Stop>& TransportCatalogue::GetStops() const
		{
			return stops_;
		}	

		size_t TransportCatalogue::StringViewHasher::operator()(const std::string_view& str) const
		{
			return std::hash<std::string_view>{}(str);
		}
	}
}