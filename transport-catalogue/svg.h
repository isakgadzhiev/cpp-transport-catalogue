#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>

using namespace std::literals;

namespace svg {
    struct Rgb {
        Rgb() = default;
        Rgb(uint8_t _red, uint8_t _green, uint8_t _blue)
                : red(_red)
                , green(_green)
                , blue(_blue) {
        }
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba : Rgb {
        Rgba() = default;
        Rgba (uint8_t _red, uint8_t _green, uint8_t _blue, double _opacity)
                : Rgb(_red, _green, _blue)
                , opacity(_opacity) {
        }
        double opacity = 1.0;
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
    inline const Color NoneColor{};

    struct ColorPrinter {
        std::ostream& out;

        void operator()(std::monostate) const;
        void operator()(const std::string& color) const;
        void operator()(const Rgb& color) const;
        void operator()(const Rgba& color) const;
    };

    std::ostream& operator<<(std::ostream& out, const Color& color);

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap);
    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join);

    struct Point {
        Point() = default;
        Point(double x, double y)
                : x(x)
                , y(y) {
        }
        double x = 0;
        double y = 0;
    };

    struct RenderContext {
        RenderContext(std::ostream& out)
                : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
                : out(out)
                , indent_step(indent_step)
                , indent(indent) {
        }

        RenderContext Indented() const {
            return {out, indent_step, indent + indent_step};
        }

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
        virtual void Render(const RenderContext& context) const;
        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    template <typename Owner>
    class PathProps {
    public:
        PathProps() = default;
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeWidth(double width) {
            stroke_width_ = width;
            return AsOwner();
        }
        Owner& SetStrokeLineCap(const StrokeLineCap& line_cap) {
            stroke_line_cap_ = line_cap;
            return AsOwner();
        }
        Owner& SetStrokeLineJoin(const StrokeLineJoin& line_join) {
            stroke_line_join_ = line_join;
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) {
                out << R"(fill=")" << *fill_color_ << R"(")";
            }
            if (stroke_color_) {
                out << R"( stroke=")" << *stroke_color_ << R"(")";
            }
            if (stroke_width_) {
                out << R"( stroke-width=")" << *stroke_width_ << R"(")";
            }
            if (stroke_line_cap_) {
                out << R"( stroke-linecap=")" << *stroke_line_cap_ << R"(")";
            }
            if (stroke_line_join_) {
                out << R"( stroke-linejoin=")" << *stroke_line_join_ << R"(")";
            }
        }

    private:
        Owner& AsOwner() {
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineJoin> stroke_line_join_;
        std::optional<StrokeLineCap> stroke_line_cap_;
    };

    class ObjectContainer {
    public:
        template <typename T>
        void Add(T obj) {
            AddPtr(std::make_unique<T>(std::move(obj)));
        }

        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

    protected:
        ~ObjectContainer() = default;
    };

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& object_container) const = 0;
        virtual ~Drawable() = default;
    };

    class Document : public ObjectContainer {
    public:
        void AddPtr(std::unique_ptr<Object>&& obj);
        void Render(std::ostream& out) const;
    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };

    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle() = default;
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_ = {0.0, 0.0};
        double radius_ = 1.0;
    };

    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        Polyline() = default;
        Polyline& AddPoint(Point point);

    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> points_ = {};
    };

    class Text final : public Object, public PathProps<Text> {
    public:
        Text() = default;
        Text& SetPosition(Point pos);
        Text& SetOffset(Point offset);
        Text& SetFontSize(uint32_t size);
        Text& SetFontFamily(std::string font_family);
        Text& SetFontWeight(std::string font_weight);
        Text& SetData(std::string data);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point position_ = {0.0, 0.0};
        Point offset_ = {0.0, 0.0};
        uint32_t font_size_ = 1;
        std::optional<std::string> font_weight_;
        std::optional<std::string> font_family_;
        std::string data_ = ""s;
    };
}  // namespace svg