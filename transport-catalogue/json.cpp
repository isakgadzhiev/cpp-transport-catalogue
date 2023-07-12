#include "json.h"

using namespace std;

namespace json {

//------------------ Node ------------------
    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(*this);
    }

    bool Node::IsArray() const {
        return std::holds_alternative<Array>(*this);
    }

    bool Node::IsMap() const {
        return std::holds_alternative<Dict>(*this);
    }

    bool Node::IsBool() const {
        return std::holds_alternative<bool>(*this);
    }

    bool Node::IsInt() const {
        return std::holds_alternative<int>(*this);
    }

    bool Node::IsDouble() const {
        return IsPureDouble() || IsInt();
    }

    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(*this);
    }

    bool Node::IsString() const {
        return std::holds_alternative<string>(*this);
    }

    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw std::logic_error("Value is not an array"s);
        } else {
            return std::get<Array>(*this);
        }
    }

    const Dict& Node::AsMap() const {
        if (!IsMap()) {
            throw std::logic_error("Value is not a map"s);
        } else {
            return std::get<Dict>(*this);
        }
    }

    bool Node::AsBool() const {
        if (!IsBool()) {
            throw std::logic_error("Value is not bool"s);
        } else {
            return std::get<bool>(*this);
        }
    }

    int Node::AsInt() const {
        if (!IsInt()) {
            throw std::logic_error("Value is not int"s);
        } else {
            return std::get<int>(*this);
        }
    }

    double Node::AsDouble() const {
        if (!IsDouble()) {
            throw std::logic_error("Value is not double"s);
        } else if (std::holds_alternative<double>(*this)) {
            return std::get<double>(*this);
        } else {
            return static_cast<double>(std::get<int>(*this));
        }
    }

    const std::string& Node::AsString() const {
        if (!IsString()) {
            throw std::logic_error("Value is not string"s);
        } else {
            return std::get<string>(*this);
        }
    }

    const Value& Node::GetValue() const {
        return *this;
    }

    bool operator==(const Node& lhs, const Node& rhs) {
        return lhs.GetValue() == rhs.GetValue();
    }
    bool operator!=(const Node& lhs, const Node& rhs) {
        return !(lhs == rhs);
    }

    namespace load {

        Node LoadNode(istream& input);

        Node LoadNull(istream& input) {
            std::string null_str;
            while (std::isalpha(input.peek())) {
                null_str.push_back(static_cast<char>(input.get()));
            }
            if (null_str == "null"sv) {
                return nullptr;
            } else {
                throw ParsingError("Failed to read null from stream"s);
            }
        }

        Node LoadBool(istream& input) {
            std::string bool_str;
            while (std::isalpha(input.peek())) {
                bool_str.push_back(static_cast<char>(input.get()));
            }
            if (bool_str == "true"sv) {
                return true;
            } else if (bool_str == "false"sv) {
                return false;
            } else {
                throw ParsingError("Failed to parse bool from stream"s);
            }
        }

        Node LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            } else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return std::stoi(parsed_num);
                    } catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            } catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadArray(istream& input) {
            Array result;
            char c;
            if (input >> c && c == ']') {
                return Node(std::move(result));
            } else {
                input.putback(c);
            }
            for (; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (!input) {
                throw ParsingError("Array parsing error"s);
            }
            return Node(std::move(result));
        }

        Node LoadDict(istream& input) {
            Dict result;
            for (char c; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }
                string key = LoadString(input).AsString();
                input >> c;
                result.insert({std::move(key), LoadNode(input)});
            }
            if (!input) {
                throw ParsingError("Failed to read map from stream"s);
            }
            return Node(std::move(result));
        }

        Node LoadString(std::istream& input) {
            using namespace std::literals;
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                } else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                        case 'n':
                            s.push_back('\n');
                            break;
                        case 't':
                            s.push_back('\t');
                            break;
                        case 'r':
                            s.push_back('\r');
                            break;
                        case '"':
                            s.push_back('"');
                            break;
                        case '\\':
                            s.push_back('\\');
                            break;
                        default:
                            // Встретили неизвестную escape-последовательность
                            throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                } else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                } else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return s;
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;
            if (c == '[') {
                return LoadArray(input);
            } else if (c == '{') {
                return LoadDict(input);
            } else if (c == '\"') {
                return LoadString(input);
            } else if (c == 'n') {
                input.putback(c);
                return LoadNull(input);
            } else if (c == 't' || c == 'f') {
                input.putback(c);
                return LoadBool(input);
            } else if (std::isdigit(c) || c == '-') {
                input.putback(c);
                return LoadNumber(input);
            } else {
                throw ParsingError("Incorrect JSON"s);
            }
        }
    } // namespace load

//------------------ Document ------------------
    Document::Document(Node root)
            : root_(std::move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{load::LoadNode(input)};
    }

//------------------ Print ------------------
    void PrintContext::PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }
    PrintContext PrintContext::Indented() const {
        return {out, indent_step, indent_step + indent};
    }

    void PrintValue(std::nullptr_t, const PrintContext& ctx) {
        ctx.out << "null"sv;
    }

    void PrintValue(bool value, const PrintContext& ctx) {
        if (value) {
            ctx.out << "true"sv;
        } else {
            ctx.out << "false"sv;
        }
    }

    void PrintValue(const std::string& str, const PrintContext& ctx) {
        ctx.PrintIndent();
        ctx.out << "\""sv;
        for (const char& c : str) {
            if (c == '\\') {
                ctx.out << "\\\\"sv;
            } else if (c == '\"') {
                ctx.out << "\\\""sv;
            } else if (c == '\r') {
                ctx.out << "\\r"sv;
            } else if (c == '\n') {
                ctx.out << "\\n"sv;
            } else {
                ctx.out << c;
            }
        }
        ctx.out<< "\""sv;
    }

    void PrintValue(const Array& arr, const PrintContext& ctx) {
        ctx.out << "[\n";
        for (const auto& elem : arr) {
            PrintNode(elem, ctx.Indented());
            if (&elem != &arr.back()) {
                ctx.out << ",";
            }
            ctx.out << "\n";
        }
        ctx.PrintIndent();
        ctx.out << "]";
    }

    void PrintValue(const Dict& dict, const PrintContext& ctx) {
        ctx.PrintIndent();
        if (dict.empty()) {
            ctx.out << "{}";
        } else {
            ctx.out << "{\n";
            for (const auto& [key, value] : dict) {
                PrintNode(key, ctx.Indented());
                ctx.out << ": "sv;
                PrintNode(value, ctx.Indented());
                if (&value != &dict.rbegin()->second) {
                    ctx.out << ",";
                }
                ctx.out << "\n";
            }
            ctx.PrintIndent();
            ctx.out << "}";
        }
    }

    void PrintNode(const Node& node, const PrintContext& context) {
        std::visit([&context](const auto& value){ PrintValue(value, context); },
                   node.GetValue());
    }


    void Print(const Document& doc, std::ostream& out) {
        const Node& root = doc.GetRoot();
        PrintNode(root, PrintContext{out});
    }
}  // namespace json