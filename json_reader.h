#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <string_view>
#include <algorithm>
#include <sstream>

#include "request_handler.h"
#include "transport_catalogue.h"
#include "json.h"

namespace json {
	using namespace std::string_literals;
	using transport_catalogue::data_base::TransportCatalogue;
	using transport_catalogue::data_base::BusInfo;
	using transport_catalogue::data_base::StopInfo;

	struct Way
	{
		std::string from_stop{};
		std::string to_stop{};
		int distance_{};
	};

	struct Input {
		json::Array base_requests;
		json::Array stat_requests;
		json::Dict render_settings;
		json::Dict routing_settings;
		json::Dict serialization_settings;
	};

	class JSONReader
	{
	public:
		void ProcessStops(const json::Array& base_requests, TransportCatalogue& transport_catalogue);
		void ProcessBuses(const json::Array& base_requests, TransportCatalogue& transport_catalogue);
		Input LoadInputMakeBase(std::istream& input, TransportCatalogue& transport_catalogue);
		Input LoadInputProessRequests(std::istream& input, TransportCatalogue& transport_catalogue);

		void ProcessStatRequests(const json::Array& stat_requests, const handler::RequestHandler& request_handler, std::ostream& out);
		BusInfo ProcessBusInfo(const std::string_view& requests_bus_info, const handler::RequestHandler& request_handler);
		StopInfo ProcessStopInfo(const std::string_view& requests_stop_info, const handler::RequestHandler& request_handler);
		void PrintBusInfo(int id, const BusInfo& bus_info, std::ostream& out);
		void PrintStopInfo(int id, const StopInfo& stop_info, std::ostream& out);
		void PrintMapInfo(int id, const handler::RequestHandler& request_handler, std::ostream& out);
		void PrintRouteInfo(const json::Dict& request, const handler::RequestHandler& request_handler, std::ostream& out);
	};	
}