#include "map_renderer.h"

namespace map_renderer {
    using transport_catalogue::data_base::TransportCatalogue;
    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }


    svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

    RenderSettings::RenderSettings(const json::Dict& render_settings) {
        width = render_settings.at("width").AsDouble();
        height = render_settings.at("height").AsDouble();
        padding = render_settings.at("padding").AsDouble();
        line_width = render_settings.at("line_width").AsDouble();
        stop_radius = render_settings.at("stop_radius").AsDouble();
        bus_label_font_size = render_settings.at("bus_label_font_size").AsInt();
        bus_label_offset = { render_settings.at("bus_label_offset").AsArray()[0].AsDouble(), render_settings.at("bus_label_offset").AsArray()[1].AsDouble() };
        stop_label_font_size = render_settings.at("stop_label_font_size").AsInt();
        stop_label_offset = { render_settings.at("stop_label_offset").AsArray()[0].AsDouble(), render_settings.at("stop_label_offset").AsArray()[1].AsDouble() };

        {
            if (render_settings.at("underlayer_color").IsString()) {
                underlayer_color = render_settings.at("underlayer_color").AsString();
            }
            else {
                const auto arr_underlayer_color = render_settings.at("underlayer_color").AsArray();
                if (arr_underlayer_color.size() == 3) {
                    svg::Rgb rgb(arr_underlayer_color[0].AsInt(), arr_underlayer_color[1].AsInt(), arr_underlayer_color[2].AsInt());
                    underlayer_color = svg::Color(rgb);
                }
                else if (arr_underlayer_color.size() == 4) {
                    svg::Rgba rgba(arr_underlayer_color[0].AsInt(), arr_underlayer_color[1].AsInt(), arr_underlayer_color[2].AsInt(), arr_underlayer_color[3].AsDouble());
                    underlayer_color = svg::Color(rgba);
                }
            }

        }

        underlayer_width = render_settings.at("underlayer_width").AsDouble();

        {
            const auto color_palette_json = render_settings.at("color_palette").AsArray();
            std::vector<svg::Color> result_color_palette;
            result_color_palette.reserve(color_palette_json.size());
            for (size_t i = 0; i < color_palette_json.size(); ++i) {
                svg::Color color;
                if (color_palette_json[i].IsString()) {
                    color = color_palette_json[i].AsString();
                }
                else {
                    const auto color_arr = color_palette_json[i].AsArray();
                    if (color_arr.size() == 3) {
                        svg::Rgb rgb(color_arr[0].AsInt(), color_arr[1].AsInt(), color_arr[2].AsInt());
                        color = svg::Color(rgb);
                    }
                    else if (color_arr.size() == 4) {
                        svg::Rgba rgba(color_arr[0].AsInt(), color_arr[1].AsInt(), color_arr[2].AsInt(), color_arr[3].AsDouble());
                        color = svg::Color(rgba);
                    }
                }
                result_color_palette.emplace_back(color);
            }
            color_palette = result_color_palette;
        }
    }

    //----------------------Map Renderer ------------------

    MapRenderer::MapRenderer(const TransportCatalogue &transport_catalogue, RenderSettings render_settings)
        :db_(transport_catalogue)
        , render_settings_(std::move(render_settings)) 
        {
            this->RenderMap();
        }    

    double MapRenderer::GetWidth() const
    {
        return render_settings_.width;
    }
    double MapRenderer::GetHeight() const
    {
        return render_settings_.height;
    }
    double MapRenderer::GetPadding() const
    {
        return render_settings_.padding;
    }
    double MapRenderer::GetLineWidth() const
    {
        return render_settings_.line_width;
    }
    double MapRenderer::GetStopRadius() const
    {
        return render_settings_.stop_radius;
    }
    int MapRenderer::GetBusLabelFontSize() const
    {
        return render_settings_.bus_label_font_size;
    }
    svg::Point MapRenderer::GetBusLabelOffset() const
    {
        return render_settings_.bus_label_offset;
    }
    int MapRenderer::GetStopLabelFontSize() const
    {
        return render_settings_.stop_label_font_size;
    }
    svg::Point MapRenderer::GetStopLabelOffset() const
    {
        return render_settings_.stop_label_offset;
    }
    svg::Color MapRenderer::GetUnderlayerColor() const
    {
        return render_settings_.underlayer_color;
    }
    double MapRenderer::GetUnderlayerWidth() const
    {
        return render_settings_.underlayer_width;
    }
    std::vector<svg::Color> MapRenderer::GetColorPalette() const
    {
        return render_settings_.color_palette;
    }

    const RenderSettings &MapRenderer::GetRenderSettings() const
    {
        return render_settings_;
    }

    void MapRenderer::RenderRoutes(const std::vector<std::vector<geo::Coordinates>>& routes_coordinates, const SphereProjector& proj)
    {
        const auto color_palette = this->GetColorPalette();
        size_t n = color_palette.size();

        for (size_t i = 0; i < routes_coordinates.size(); ++i) {
            svg::Color color;
            svg::Polyline route;
            color = color_palette[i % n];

            for (const auto& rout_coordinates : routes_coordinates[i]) {
                route.AddPoint(proj(rout_coordinates));
            }

            this->doc_.Add(route.SetStrokeColor(color).
                SetFillColor("none").
                SetStrokeWidth(this->GetLineWidth()).
                SetStrokeLineCap(svg::StrokeLineCap::ROUND).
                SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
        }
    }

    void MapRenderer::RenderRoutNames(const std::vector<std::string_view>& buses_names, const SphereProjector& proj)
    {
        const auto color_palette = this->GetColorPalette();
        size_t n = color_palette.size();
        size_t count_not_empty_routes = 0;

        for (const auto bus_name : buses_names) {
            const auto bus = db_.FindBus(bus_name);
            if (bus->stops.empty()) {
                continue;
            }
            svg::Color color = color_palette[count_not_empty_routes % n];
            svg::Text underlauer_text;
            svg::Text route_name_text;
            
            const auto first_stop_point = proj({ bus->stops[0]->latitude, bus->stops[0]->longitude });
            doc_.Add(underlauer_text.SetFillColor(this->GetUnderlayerColor()).
                SetStrokeColor(this->GetUnderlayerColor()).
                SetStrokeWidth(this->GetUnderlayerWidth()).
                SetStrokeLineCap(svg::StrokeLineCap::ROUND).
                SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).
                SetPosition(first_stop_point).
                SetOffset(this->GetBusLabelOffset()).
                SetFontSize(this->GetBusLabelFontSize()).
                SetFontFamily("Verdana"s).
                SetFontWeight("bold"s).
                SetData(static_cast<std::string>(bus_name)));

            doc_.Add(route_name_text.SetFillColor(color).
                SetPosition(first_stop_point).
                SetOffset(this->GetBusLabelOffset()).
                SetFontSize(this->GetBusLabelFontSize()).
                SetFontFamily("Verdana"s).
                SetFontWeight("bold"s).
                SetData(static_cast<std::string>(bus_name)));

            if (bus->type == transport_catalogue::data_base::RouteType::TWO_DIRECTIONAL && bus->stops[0] != bus->second_final_stop) {
                const auto second_stop_point = proj({ bus->second_final_stop.value()->latitude, bus->second_final_stop.value()->longitude});
                doc_.Add(underlauer_text.SetFillColor(this->GetUnderlayerColor()).
                    SetStrokeColor(this->GetUnderlayerColor()).
                    SetStrokeWidth(this->GetUnderlayerWidth()).
                    SetStrokeLineCap(svg::StrokeLineCap::ROUND).
                    SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).
                    SetPosition(second_stop_point).
                    SetOffset(this->GetBusLabelOffset()).
                    SetFontSize(this->GetBusLabelFontSize()).
                    SetFontFamily("Verdana"s).
                    SetFontWeight("bold"s).
                    SetData(static_cast<std::string>(bus_name)));

                doc_.Add(route_name_text.SetFillColor(color).
                    SetPosition(second_stop_point).
                    SetOffset(this->GetBusLabelOffset()).
                    SetFontSize(this->GetBusLabelFontSize()).
                    SetFontFamily("Verdana"s).
                    SetFontWeight("bold"s).
                    SetData(static_cast<std::string>(bus_name)));
            }
            ++count_not_empty_routes;
        }
    }

    void MapRenderer::RenderStopCircles(const std::set<transport_catalogue::data_base::Stop*, CmpStop>& all_stops, const SphereProjector& proj)
    {
        for (const auto stop : all_stops) {
            svg::Circle stop_circle;
            svg::Point center = proj({ stop->latitude, stop->longitude });
            doc_.Add(stop_circle.
                SetCenter(center).
                SetRadius(this->GetStopRadius()).
                SetFillColor("white"s));
        }
    }

    void MapRenderer::RenderStopNames(const std::set<transport_catalogue::data_base::Stop*, CmpStop>& all_stops, const SphereProjector& proj)
    {
        for (const auto stop : all_stops) {
            svg::Text underlauer_text;
            svg::Text stop_name_text;
            svg::Point position = proj({ stop->latitude, stop->longitude });
            doc_.Add(underlauer_text.
                SetFillColor(this->GetUnderlayerColor()).
                SetStrokeColor(this->GetUnderlayerColor()).
                SetStrokeWidth(this->GetUnderlayerWidth()).
                SetStrokeLineCap(svg::StrokeLineCap::ROUND).
                SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).
                SetPosition(position).
                SetOffset(this->GetStopLabelOffset()).
                SetFontSize(this->GetStopLabelFontSize()).
                SetFontFamily("Verdana"s).
                SetData(stop->name));

            doc_.Add(stop_name_text.
                SetFillColor("black"s).                
                SetPosition(position).
                SetOffset(this->GetStopLabelOffset()).
                SetFontSize(this->GetStopLabelFontSize()).
                SetFontFamily("Verdana"s).
                SetData(stop->name));
        }
    }

    void MapRenderer::RenderMap()
    {
        auto buses = db_.GetBuses();
        std::vector<std::string_view> buses_names;
        for (const auto& bus : buses) {
            buses_names.emplace_back(bus.name);
        }
        std::sort(buses_names.begin(), buses_names.end());

        std::vector<std::vector<geo::Coordinates>> routes_coordinates;
        routes_coordinates.reserve(buses_names.size());

        std::for_each(buses_names.begin(), buses_names.end(),
            [&routes_coordinates, this]
            (std::string_view name)
            {routes_coordinates.emplace_back(GetRouteCoordinates(name)); });

        std::vector<geo::Coordinates> all_coordinates;
        std::for_each(routes_coordinates.begin(), routes_coordinates.end(),
            [&all_coordinates](const auto& route) {for (const auto& coordinate : route) {
            all_coordinates.emplace_back(coordinate); };
            });

        const SphereProjector proj(all_coordinates.begin(), all_coordinates.end(),
            this->GetWidth(),
            this->GetHeight(),
            this->GetPadding());

        std::set<transport_catalogue::data_base::Stop*, CmpStop> all_stops;
        for (const auto bus_name : buses_names) {
            const auto bus = db_.FindBus(bus_name);
            for (const auto stop : bus->stops) {
                all_stops.insert(stop);
            }
        }

        RenderRoutes(routes_coordinates, proj);
        RenderRoutNames(buses_names, proj);
        RenderStopCircles(all_stops, proj);
        RenderStopNames(all_stops, proj);
    }

    const svg::Document& MapRenderer::GetDocument() const
    {        
        return doc_;
    }

    std::vector<geo::Coordinates> MapRenderer::GetRouteCoordinates(std::string_view name_bus)
    {
        std::vector<geo::Coordinates> result;
        auto bus = db_.FindBus(name_bus);
        for (const auto& stop : bus->stops) {
            geo::Coordinates coordinates;
            coordinates.lat = stop->latitude;
            coordinates.lng = stop->longitude;
            result.push_back(coordinates);
        }
        return result;
    }
}