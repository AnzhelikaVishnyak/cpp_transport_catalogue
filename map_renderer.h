#pragma once
#include "geo.h"
#include "svg.h"
#include "json.h"
#include "transport_catalogue.h"
#include "domain.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>

namespace map_renderer {
    using transport_catalogue::data_base::TransportCatalogue;
    using namespace::std::literals;

    inline const double EPSILON = 1e-6;
    bool IsZero(double value);

    class SphereProjector {
    public:
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding)
            : padding_(padding) //
        {
            // Если точки поверхности сферы не заданы, вычислять нечего
            if (points_begin == points_end) {
                return;
            }

            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // Вычисляем коэффициент масштабирования вдоль координаты x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Вычисляем коэффициент масштабирования вдоль координаты y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // Коэффициенты масштабирования по ширине и высоте ненулевые,
                // берём минимальный из них
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(geo::Coordinates coords) const;

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    struct RenderSettings
    {
        RenderSettings() = default;
        RenderSettings(const json::Dict& render_settings);
        double width = 0;
        double height = 0;
        double padding = 0;
        double line_width = 0;
        double stop_radius = 0;
        int bus_label_font_size = 0;
        svg::Point bus_label_offset{};
        int stop_label_font_size = 0;
        svg::Point stop_label_offset{};
        svg::Color underlayer_color{};
        double underlayer_width = 0;
        std::vector<svg::Color> color_palette{};
    };
    
    struct CmpStop {
        bool operator()(const transport_catalogue::data_base::Stop* lhs, const transport_catalogue::data_base::Stop* rhs) const {
            return lhs->name < rhs->name;
        }
    };

    class MapRenderer
    {
    public:
        MapRenderer(const TransportCatalogue& transport_catalogue, RenderSettings render_settings);
        
        double GetWidth() const;
        double GetHeight() const;
        double GetPadding() const;
        double GetLineWidth() const;
        double GetStopRadius() const;
        int GetBusLabelFontSize() const;
        svg::Point GetBusLabelOffset() const;
        int GetStopLabelFontSize() const;
        svg::Point GetStopLabelOffset() const;
        svg::Color GetUnderlayerColor() const;
        double GetUnderlayerWidth() const;
        std::vector<svg::Color> GetColorPalette() const;
        const RenderSettings& GetRenderSettings() const;

        std::vector<geo::Coordinates> GetRouteCoordinates(std::string_view name_bus);

        void RenderRoutes(const std::vector<std::vector<geo::Coordinates>>& routes_coordinates, const SphereProjector& proj);
        void RenderRoutNames(const std::vector<std::string_view>& buses_names, const SphereProjector& proj);
        void RenderStopCircles(const std::set<transport_catalogue::data_base::Stop*, CmpStop>& all_stops, const SphereProjector& proj);
        void RenderStopNames(const std::set<transport_catalogue::data_base::Stop*, CmpStop>& all_stops, const SphereProjector& proj);
        void RenderMap();

        const svg::Document& GetDocument() const;

    private:
        const TransportCatalogue& db_{};
        RenderSettings render_settings_{};
        svg::Document doc_;
    };
}