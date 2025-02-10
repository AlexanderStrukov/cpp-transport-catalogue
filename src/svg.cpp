#include "svg.h"

#include <string_view>

using namespace std::literals;

namespace svg {

    using namespace std::literals;

    inline void PrintColor(std::ostream& out, std::monostate) {
        out << NoneColor;
    }

    inline void PrintColor(std::ostream& out, std::string& color) {
        out << color;
    }

    inline void PrintColor(std::ostream& out, Rgb& rgb) {
        out << "rgb("sv << static_cast<short>(rgb.red)
            << ","sv << static_cast<short>(rgb.green) << ","sv
            << static_cast<short>(rgb.blue) << ")"sv;
    }

    inline void PrintColor(std::ostream& out, Rgba& rgba) {
        out << "rgba("sv << static_cast<short>(rgba.red)
            << ","sv << static_cast<short>(rgba.green) << ","sv
            << static_cast<short>(rgba.blue) << ","sv << rgba.opacity << ")"sv;
    }

    std::ostream& operator<<(std::ostream& out, const Color& color) {
        std::visit([&out](auto value) { PrintColor(out, value); }, color);

        return out;
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();
        RenderObject(context);
        context.out << std::endl;
    }

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

        RenderAttrs(out);
        out << "/>"sv;
    }

    Polyline& Polyline::AddPoint(Point point) {
        points_.emplace_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;

        out << "<polyline points=\""sv;
        for (size_t i = 0; i < points_.size(); ++i) {
            out << points_[i].x << ","sv << points_[i].y;

            if (i + 1 < points_.size())
                out << " "sv;
        }

        out << "\" ";
        RenderAttrs(out);
        out << "/>"sv;
    }

    Text& Text::SetPosition(Point position) {
        position_ = position;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t font_size) {
        font_size_ = font_size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = std::move(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;

        out << "<text "sv;
        RenderAttrs(out);

        out << "x=\""sv << position_.x << "\" y=\""sv << position_.y
            << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" font-size=\""sv << font_size_ << "\""sv;

        if (!font_family_.empty())
            out << " font-family=\""sv << font_family_ << "\""sv;

        if (!font_weight_.empty())
            out << " font-weight=\""sv << font_weight_ << "\""sv;

        out << ">"sv << data_ << "</text>"sv;
    }

    void Document::Render(std::ostream& out) const {
        int indent = 2;
        int indent_step = 2;

        RenderContext context(out, indent_step, indent);

        const std::string_view xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv;
        const std::string_view svg = "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv;

        out << xml << "\n"sv << svg << "\n"sv;

        for (const auto& object : objects_) {
            object->Render(context);
        }

        out << "</svg>"sv;
    }

}  // namespace svg