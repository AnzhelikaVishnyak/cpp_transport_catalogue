#include "json_reader.h"
#include "json_builder.h"

namespace json {
	void JSONReader::ProcessStops(const json::Array& base_requests, TransportCatalogue& transport_catalogue) {
		std::vector<Way> stop_to_stop_distance;
		stop_to_stop_distance.reserve(base_requests.size() * 2);

		for (const auto& request : base_requests) {
			const auto dict = request.AsDict();
			if (dict.at("type"s) == "Stop"s) {
				std::string name_from = dict.at("name"s).AsString();
				transport_catalogue.AddStop(name_from, dict.at("latitude"s).AsDouble(), dict.at("longitude"s).AsDouble());
				for (const auto& item : dict.at("road_distances"s).AsDict()) {
					Way way;
					way.from_stop = name_from;
					way.to_stop = item.first;
					way.distance_ = item.second.AsInt();
					stop_to_stop_distance.emplace_back(way);
				}
			}
		}

		for (const auto& way : stop_to_stop_distance) {
			transport_catalogue.AddWay(way.from_stop, way.to_stop, way.distance_);
		}
	}

	void JSONReader::ProcessBuses(const json::Array& base_requests, TransportCatalogue& transport_catalogue) {
		for (const auto& request : base_requests) {
			const auto dict = request.AsDict();
			if (dict.at("type"s) == "Bus"s) {
				std::vector<std::string> stops;
				stops.reserve(dict.at("stops"s).AsArray().size());
				bool is_roundtrip = dict.at("is_roundtrip"s).AsBool();
				std::string second_final_stop = "";
				for (const auto& stop : dict.at("stops"s).AsArray()) {
					stops.emplace_back(stop.AsString());
				}
				if (!is_roundtrip) {
					std::vector<std::string> reversed_stops = stops;
					second_final_stop = stops.back();
					std::reverse(reversed_stops.begin(), reversed_stops.end());
					stops.reserve(stops.size() * 2);
					bool is_first = true;
					for (auto& stop : reversed_stops) {
						if (is_first) {
							is_first = false;
							continue;
						}
						stops.emplace_back(std::move(stop));
					}
				}
				transport_catalogue.AddBus(dict.at("name"s).AsString(), std::move(stops), is_roundtrip, std::move(second_final_stop));
			}
		}
	}

	Input JSONReader::LoadInputMakeBase(std::istream& input, TransportCatalogue& transport_catalogue) {
		json::Document doc = json::Load(input);
		json::Array base_requests = doc.GetRoot().AsDict().at("base_requests"s).AsArray();
		json::Dict render_settings = doc.GetRoot().AsDict().at("render_settings"s).AsDict();
		json::Dict routing_settings = doc.GetRoot().AsDict().at("routing_settings"s).AsDict();
		json::Dict serialization_settings = doc.GetRoot().AsDict().at("serialization_settings"s).AsDict();

		ProcessStops(base_requests, transport_catalogue);
		ProcessBuses(base_requests, transport_catalogue);

		Input result;
		result.base_requests = std::move(base_requests);
		result.render_settings = std::move(render_settings);
		result.routing_settings = std::move(routing_settings);
		result.serialization_settings = std::move(serialization_settings);
		return result;
	}

    Input JSONReader::LoadInputProessRequests(std::istream &input, TransportCatalogue &transport_catalogue)
    {
        json::Document doc = json::Load(input);
		json::Array stat_requests = doc.GetRoot().AsDict().at("stat_requests"s).AsArray();
		json::Dict serialization_settings = doc.GetRoot().AsDict().at("serialization_settings"s).AsDict();

		Input result;
		result.stat_requests = std::move(stat_requests);
		result.serialization_settings = std::move(serialization_settings);
		return result;
    }

    BusInfo JSONReader::ProcessBusInfo(const std::string_view& requests_bus_info, const handler::RequestHandler& request_handler)
	{
		if (requests_bus_info.empty()) {
			return BusInfo();
		}
		BusInfo bus_info = request_handler.GetBusInfo(requests_bus_info);
		return bus_info;
	}

	StopInfo JSONReader::ProcessStopInfo(const std::string_view& requests_stop_info, const handler::RequestHandler& request_handler)
	{
		if (requests_stop_info.empty()) {
			return StopInfo();
		}
		StopInfo stop_info = request_handler.GetStopInfo(requests_stop_info);
		return stop_info;
	}

	void JSONReader::PrintBusInfo(int id, const BusInfo& bus_info, std::ostream& out)
	{
		json::Builder answer;
		answer.StartDict().Key("request_id"s).Value(id);

		if (bus_info.status == transport_catalogue::data_base::ReserchStatus::NOT_FOUND) {
			answer.Key("error_message"s).Value("not found"s);
		}
		else {
			answer.Key("route_length"s).Value(bus_info.way)
				.Key("stop_count"s).Value(bus_info.stops_count)
				.Key("unique_stop_count"s).Value(bus_info.unique_stops_count)
				.Key("curvature"s).Value(bus_info.way / bus_info.route_length);
		}
		answer.EndDict();
		json::Print(json::Document{ answer.Build() }, out);
	}

	void JSONReader::PrintStopInfo(int id, const StopInfo& stop_info, std::ostream& out)
	{
		json::Builder answer;
		answer.StartDict().Key("request_id"s).Value(id);

		if (stop_info.status == transport_catalogue::data_base::ReserchStatus::NOT_FOUND) {
			answer.Key("error_message"s).Value("not found");
		}
		else {
			answer.Key("buses"s).StartArray();
			for (const auto& bus : stop_info.buses) {
				answer.Value(static_cast<std::string>(bus));
			}
			answer.EndArray();
		}
		answer.EndDict();
		json::Print(json::Document{ answer.Build() }, out);
	}

	void JSONReader::PrintMapInfo(int id, const handler::RequestHandler& request_handler, std::ostream& out)
	{
		std::ostringstream oss;
		request_handler.RenderMap().Render(oss);

		json::Builder answer;
		answer.StartDict()
			.Key("request_id"s).Value(id)
			.Key("map"s).Value(oss.str())
			.EndDict();
		json::Print(json::Document{ answer.Build() }, out);
	}

	void JSONReader::PrintRouteInfo(const json::Dict& request, const handler::RequestHandler& request_handler, std::ostream& out)
	{
		json::Builder answer;
		answer.StartDict()
			.Key("request_id"s).Value(request.at("id"s));

		const auto route = request_handler.BuildRoute(request.at("from"s).AsString(), request.at("to"s).AsString());

		if (!route.has_value()) {
			answer.Key("error_message"s).Value("not found"s);
		}
		else {

			answer.Key("items"s).StartArray();

			for (const auto& edge_id : route.value().edges) {
				answer.StartDict();
				const auto [edge, edge_info] = request_handler.GetFullEdgeInfo(edge_id);
				switch (edge_info.type)
				{
				case transport_router::EdgeType::BUS:
					answer.Key("type"s).Value("Bus"s)
						.Key("bus"s).Value(std::string(edge_info.bus))
						.Key("span_count"s).Value(edge_info.span_count)						
						.Key("time").Value(edge.weight);
					break;
				case transport_router::EdgeType::WAIT:
					answer.Key("type"s).Value("Wait"s)
						.Key("stop_name"s).Value(std::string(edge_info.stop_name))
						.Key("time"s).Value(edge.weight);
					break;
				}
				answer.EndDict();
			}

			answer.EndArray();

			answer.Key("total_time").Value(route.value().weight);

		}			
			answer.EndDict();
		json::Print(json::Document{ answer.Build() }, out);
	}

	void JSONReader::ProcessStatRequests(const json::Array& stat_requests, const handler::RequestHandler& request_handler, std::ostream& out)
	{
		bool is_first = true;
		out << '[';
		for (const auto& request : stat_requests) {
			const auto dict = request.AsDict();
			if (dict.at("type"s) == "Bus"s) {
				if (is_first) {
					PrintBusInfo(dict.at("id"s).AsInt(), ProcessBusInfo(dict.at("name"s).AsString(), request_handler), out);
					is_first = false;
					continue;
				}
				out << ',';
				PrintBusInfo(dict.at("id"s).AsInt(), ProcessBusInfo(dict.at("name"s).AsString(), request_handler), out);
			}
			else if (dict.at("type"s) == "Stop"s) {
				if (is_first) {
					PrintStopInfo(dict.at("id"s).AsInt(), ProcessStopInfo(dict.at("name"s).AsString(), request_handler), out);
					is_first = false;
					continue;
				}
				out << ',';
				PrintStopInfo(dict.at("id"s).AsInt(), ProcessStopInfo(dict.at("name"s).AsString(), request_handler), out);
			}
			else if (dict.at("type"s) == "Map"s) {
				if (is_first) {
					PrintMapInfo(dict.at("id"s).AsInt(), request_handler, out);
					is_first = false;
					continue;
				}
				out << ',';
				PrintMapInfo(dict.at("id"s).AsInt(), request_handler, out);
			}
			else if (dict.at("type"s) == "Route"s) {
				if (is_first) {
					PrintRouteInfo(dict, request_handler, out);
					is_first = false;
					continue;
				}
				out << ',';
				PrintRouteInfo(dict, request_handler, out);
			}
		}
		out << ']';
	}
}
