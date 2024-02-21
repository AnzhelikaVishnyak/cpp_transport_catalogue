#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <set>

#include "domain.h"
#include "geo.h"

namespace transport_catalogue {
	namespace data_base {
		class TransportCatalogue {
		private:

			struct StringViewHasher
			{
				size_t operator()(const std::string_view& str) const;
			};

			template<typename T, typename U>
			struct PairPtrHasher
			{
				size_t operator()(const std::pair<T*, U*>& p) const {
					std::hash<T*> hasher_t;
					std::hash<U*> hasher_u;
					return hasher_t(p.first) * 51 + hasher_u(p.second) * (51 * 51);
				}
			};

			std::deque<Stop> stops_;
			std::unordered_map<std::string_view, Stop*, StringViewHasher> stopname_to_stop_;
			std::deque<Bus> buses_;
			std::unordered_map<std::string_view, Bus*, StringViewHasher> busname_to_bus_;
			std::unordered_map<Stop*, std::set<std::string_view>> stop_to_buses_;
			std::unordered_map<std::pair<Stop*, Stop*>, int, PairPtrHasher<Stop, Stop>> stop_to_stop_distance_;

		public:

			void AddStop(const std::string& name, double latitude, double longitude);
			Stop* FindStop(std::string_view name) const;
			void AddWay(std::string_view from_stop, std::string_view to_stop, int distance);
			int FindWay(std::pair<Stop*, Stop*> from_stop_to_stop) const;
			const std::unordered_map<std::pair<Stop*, Stop*>, int, PairPtrHasher<Stop, Stop>>& GetAllWays() const;
			void AddBus(const std::string& bus, const std::vector<std::string>& stops, bool is_roundtrip, const std::string second_final_stop);
			Bus* FindBus(std::string_view name) const;
			BusInfo GetBusInfo(std::string_view name) const;
			StopInfo GetStopInfo(std::string_view name) const;
			const std::deque<Bus>& GetBuses() const;
            const std::deque<Stop>& GetStops() const;			
		};
	}
}