#include "svg.h"

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        for (const auto& obj : objects_) {
            obj->Render(out);
        }
        out << "</svg>"sv << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.emplace_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool is_first = true;
        for (const auto& point : points_) {
            if (is_first) {
                out << point.x << ","sv << point.y;
                is_first = false;
                continue;
            }
            out << " "sv << point.x << ","sv << point.y;
        }
        out << "\" "sv;
        RenderAttrs(context.out);
        out << "/>"sv;

    }

    // ---------- Text ------------------

    Text& Text::SetPosition(Point pos)
    {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset)
    {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size)
    {
        size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family)
    {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight)
    {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data)
    {
        data_ = data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;
        out << "<text"sv;
        RenderAttrs(context.out);
        out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << size_ << "\""sv;
        if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }

        out << ">"sv;

        for (const auto& c : data_) {
            switch (c)
            {
            case '"':
                out << "&quot;"sv;
                break;
            case '\'':
                out << "&apos;"sv;
                break;
            case '<':
                out << "&lt;"sv;
                break;
            case '>':
                out << "&gt;"sv;
                break;
            case '&':
                out << "&amp;"sv;
                break;
            default:
                out << c;
                break;
            }
        }

        out << "</text>"sv;

    }



    std::ostream& operator<<(std::ostream& out, const Color& color)
    {
        std::visit(ColorPrinter{ out }, color);
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& value)
    {
        switch (value)
        {
        case StrokeLineCap::BUTT:
            return out << "butt"sv;
        case StrokeLineCap::ROUND:
            return out << "round"sv;
        case StrokeLineCap::SQUARE:
            return out << "square";
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& value)
    {
        switch (value)
        {
        case StrokeLineJoin::ARCS:
            return out << "arcs"sv;
        case StrokeLineJoin::BEVEL:
            return out << "bevel"sv;
        case StrokeLineJoin::MITER:
            return out << "miter"sv;
        case StrokeLineJoin::MITER_CLIP:
            return out << "miter-clip";
        case StrokeLineJoin::ROUND:
            return out << "round"sv;
        }
        return out;
    }

    void ColorPrinter::operator()(std::monostate) const
    {
        out << NoneColor;
    }

    void ColorPrinter::operator()(const std::string& color) const
    {
        out << color;
    }

    void ColorPrinter::operator()(Rgb color) const
    {
        out << "rgb("sv << +color.red << ","sv << +color.green << ","sv << +color.blue << ")";
    }

    void ColorPrinter::operator()(Rgba color) const
    {
        out << "rgba("sv << +color.red << ","sv << +color.green << ","sv << +color.blue << ","sv << color.opacity << ")";
    }

}  // namespace svg