#include <fstream>
#include <iostream>
#include <string_view>

#include "map_renderer.h"
#include "transport_catalogue.h"
#include "json_reader.h"
#include "request_handler.h"
#include "serialization.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {

        transport_catalogue::data_base::TransportCatalogue transport_catalogue;
        json::JSONReader json_reader;
        json::Input input = json_reader.LoadInputMakeBase(std::cin, transport_catalogue);
        map_renderer::RenderSettings render_settings(input.render_settings);
        transport_router::TransportRouter router(transport_catalogue, input.routing_settings);

        std::ofstream file_out(input.serialization_settings.at("file"s).AsString(), std::ios::binary);
        if(file_out.is_open()) {
            Serialization::Serialize(transport_catalogue, render_settings, router,  file_out);
        }

    } else if (mode == "process_requests"sv) {
        transport_catalogue::data_base::TransportCatalogue transport_catalogue;
        json::JSONReader json_reader;
        json::Input input = json_reader.LoadInputProessRequests(std::cin, transport_catalogue);      
           
        std::ifstream in_file(input.serialization_settings.at("file"s).AsString(), std::ios::binary);
        if(in_file) {
            auto [render_settings, router, graph] = Serialization::Deserialize(transport_catalogue, in_file);

            map_renderer::MapRenderer renderer(transport_catalogue, render_settings);
            router.SetGraph(graph);
            handler::RequestHandler request_handler(transport_catalogue, renderer, router);

            json_reader.ProcessStatRequests(input.stat_requests, request_handler, std::cout);
        }

    } else {
        PrintUsage();
        return 1;
    }
}