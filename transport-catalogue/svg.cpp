#include "svg.h"

namespace svg {

    using namespace std::literals;

    void ColorPrinter::operator()(std::monostate) const {
        out << "none"s;
    }

    void ColorPrinter::operator()(const std::string& color) const {
        out << color;
    }

    void ColorPrinter::operator()(const Rgb& color) const {
        out << "rgb("sv << std::to_string(color.red) << ","sv
            << std::to_string(color.green) << ","sv
            << std::to_string(color.blue) << ")"sv;
    }

    void ColorPrinter::operator()(const Rgba& color) const {
        out << "rgba("sv << std::to_string(color.red) << ","sv
            << std::to_string(color.green) << ","sv
            << std::to_string(color.blue) << ","sv
            << color.opacity << ")"sv;
    }

    std::ostream& operator<<(std::ostream& out, const Color& color) {
        std::visit(ColorPrinter{out}, color);
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap) {
        if (line_cap == StrokeLineCap::BUTT) {
            out << "butt";
        } else if (line_cap == StrokeLineCap::ROUND) {
            out << "round";
        } else if (line_cap == StrokeLineCap::SQUARE) {
            out << "square";
        } else {
            throw std::out_of_range("Invalid StrokeLineCap");
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join) {
        if (line_join == StrokeLineJoin::ARCS) {
            out << "arcs";
        } else if (line_join == StrokeLineJoin::BEVEL) {
            out << "bevel";
        } else if (line_join == StrokeLineJoin::MITER) {
            out << "miter";
        } else if (line_join == StrokeLineJoin::MITER_CLIP) {
            out << "miter-clip";
        } else if (line_join == StrokeLineJoin::ROUND) {
            out << "round";
        } else {
            throw std::out_of_range("Invalid StrokeLineJoin");
        }
        return out;
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();
        RenderObject(context);
        context.out << std::endl;
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
        out << R"(<circle cx=")" << center_.x << R"(" cy=")" << center_.y << R"(" )";
        out << R"(r=")" << radius_ << R"(" )";
        RenderAttrs(out);
        out << "/>"sv;
    }

// ---------- Polyline ------------------
    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << R"(<polyline points=")";
        bool f_time = true;
        for (const auto& point : points_) {
            if (!f_time) {
                out << " "sv;
            }
            out << point.x << ","sv << point.y;
            f_time = false;
        }
        out << "\" "sv;
        RenderAttrs(out);
        out << "/>"sv;
    }
// ---------- Text ------------------
    Text& Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
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
        out << "<text ";
        RenderAttrs(out);
        out << R"( x=")" << position_.x << R"(" y=")" << position_.y << R"(" )";
        out << R"(dx=")" << offset_.x << R"(" dy=")" << offset_.y << R"(" )";
        out << R"(font-size=")" << font_size_;
        if (font_family_) {
            out  << R"(" font-family=")" << *font_family_;
        }
        if (font_weight_) {
            out << R"(" font-weight=")" << *font_weight_;
        }
        out << "\">"sv;
        for (char c : data_) {
            if (c == '\"') {
                out << "&quot;"sv;
            } else if (c == '\'') {
                out << "&apos;"sv;
            } else if (c == '<') {
                out << "&lt;"sv;
            } else if (c == '>') {
                out << "&gt;"sv;
            } else if (c == '&') {
                out << "&amp;"sv;
            } else {
                out << c;
            }
        }
        out << "</text>"sv;
    }

// ---------- Document ------------------
    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"s << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"s << std::endl;
        RenderContext context(out, 2);
        for (const auto& object : objects_) {
            object->Render(context.Indented());
        }
        out << "</svg>";
    }
}  // namespace svg