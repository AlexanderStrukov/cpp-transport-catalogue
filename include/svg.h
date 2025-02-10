#define _USE_MATH_DEFINES

#pragma once

#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <optional>

namespace svg {

    struct Rgb {
        Rgb() = default;

        Rgb(uint8_t red, uint8_t green, uint8_t blue) :
            red(red), green(green), blue(blue) {}

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba {
        Rgba() = default;

        Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity) :
            red(red), green(green), blue(blue), opacity(opacity) {}

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

    inline const Color NoneColor{"none"};

    inline void PrintColor(std::ostream& out, std::monostate);
    inline void PrintColor(std::ostream& out, std::string& color);
    inline void PrintColor(std::ostream& out, Rgb& rgb);
    inline void PrintColor(std::ostream& out, Rgba& rgba);

    std::ostream& operator<<(std::ostream& out, const Color& color);

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND
    };

    inline std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap) {
        using namespace std::literals;

        switch (line_cap)
        {
        case StrokeLineCap::BUTT:
            out << "butt"s;
            break;
        case StrokeLineCap::ROUND:
            out << "round"s;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"s;
            break;
        }

        return out;
    }

    inline std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join) {
        using namespace std::literals;

        switch (line_join)
        {
        case StrokeLineJoin::ARCS:
            out << "arcs"s;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"s;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"s;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"s;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"s;
            break;
        }

        return out;
    }

    template<typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {
            fill_color_ = color;
            return AsOwner();
        }

        Owner& SetStrokeColor(Color color) {
            stroke_color_ = color;
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width) {
            stroke_width_ = width;
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            stroke_line_cap_ = line_cap;
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            stroke_line_join_ = line_join;
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_ != std::nullopt) {
                out << "fill=\""sv << fill_color_.value() << "\" "sv;
            }
            if (stroke_color_ != std::nullopt) {
                out << "stroke=\""sv << stroke_color_.value() << "\" "sv;
            }
            if (stroke_width_ != std::nullopt) {
                out << "stroke-width=\""sv << stroke_width_.value() << "\" "sv;
            }
            if (stroke_line_cap_ != std::nullopt) {
                out << "stroke-linecap=\""sv << stroke_line_cap_.value() << "\" "sv;
            }
            if (stroke_line_join_ != std::nullopt) {
                out << "stroke-linejoin=\""sv << stroke_line_join_.value() << "\" "sv;
            }
        }

    private:
        Owner& AsOwner() { return static_cast<Owner&>(*this); }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_line_cap_;
        std::optional<StrokeLineJoin> stroke_line_join_;

    };

    struct Point {
        Point() = default;

        Point(double x, double y) :
            x(x), y(y) {}

        double x = 0;
        double y = 0;
    };

    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {}

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out), indent_step(indent_step), indent(indent) {}

        RenderContext Indented() const { return { out, indent_step, indent + indent_step }; }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        Polyline& AddPoint(Point point);

    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> points_;
    };

    class Text final : public Object, public PathProps<Text> {
    public:

        Text& SetPosition(Point pos);
        Text& SetOffset(Point offset);
        Text& SetFontSize(uint32_t size);
        Text& SetFontFamily(std::string font_family);
        Text& SetFontWeight(std::string font_weight);
        Text& SetData(std::string data);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point position_;
        Point offset_;

        std::string font_family_;
        std::string font_weight_;
        uint32_t font_size_ = 1;

        std::string data_;
    };

    class ObjectContainer {
    public:
        template <typename TypeObject>
        void Add(TypeObject object) {
            AddPtr(std::make_unique<TypeObject>(std::move(object)));
        }

        virtual void AddPtr(std::unique_ptr<Object>&&) = 0;

    protected:

        ~ObjectContainer() = default;
    };

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;

        virtual ~Drawable() = default;
    };

    class Document : public ObjectContainer {
    public:
        void AddPtr(std::unique_ptr<Object>&& obj) override {
            objects_.emplace_back(std::move(obj));
        }

        void Render(std::ostream& out) const;

    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };

}  // namespace svg