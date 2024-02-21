#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <set>
#include <optional>

namespace transport_catalogue {
	namespace data_base {
		enum class ReserchStatus
		{
			FOUND,
			NOT_FOUND,
		};

		enum class RouteType
		{
			CIRCLE,
			TWO_DIRECTIONAL
		};

		struct Stop
		{
			std::string name;
			double latitude;
			double longitude;
		};

		struct Bus {
			std::string name;
			std::vector<Stop*> stops;
			RouteType type = RouteType::CIRCLE;
			std::optional<Stop*> second_final_stop{};
		};

		struct BusInfo {
			ReserchStatus status = ReserchStatus::NOT_FOUND;
			std::string_view bus_name{};
			int stops_count{};
			int unique_stops_count{};
			double route_length{};
			int way{};

			BusInfo() = default;
		};

		struct StopInfo
		{
			ReserchStatus status = ReserchStatus::NOT_FOUND;
			std::string_view stop_name{};
			std::set<std::string_view> buses{};

			StopInfo() = default;
		};
	}
}