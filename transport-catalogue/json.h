#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <array>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

//------------------ Node ------------------
    class Node : private Value {
    public:
        using variant::variant;

        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;
        bool IsBool() const;
        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsString() const;

        const Array& AsArray() const;
        const Dict& AsMap() const;
        bool AsBool() const;
        int AsInt() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const Value& GetValue() const;

    };

    bool operator==(const Node& lhs, const Node& rhs);
    bool operator!=(const Node& lhs, const Node& rhs);

//------------------ Parsing / Load ------------------
// Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };
    namespace load {

        Node LoadNode(std::istream& input);
        Node LoadNull(std::istream& input);
        Node LoadBool(std::istream& input);
        Node LoadArray(std::istream& input);
        Node LoadNumber(std::istream& input);
        Node LoadString(std::istream& input);
        Node LoadDict(std::istream& input);
    } // namespace load

//------------------ Document ------------------
    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    inline bool operator==(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    inline bool operator!=(const Document& lhs, const Document& rhs) {
        return !(lhs == rhs);
    }

//------------------ Print ------------------
    // Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const;
        // Возвращает новый контекст вывода с увеличенным смещением
        PrintContext Indented() const;
    };

    // Шаблон для вывода простых значений (int, double, bool)
    template <typename Value>
    void PrintValue(const Value& value, const PrintContext& ctx) {
        ctx.out << value;
    }

    // Перегрузка функции PrintValue для вывода значений null
    void PrintValue(std::nullptr_t, const PrintContext& ctx);
    // Перегрузка функции PrintValue для вывода строки в кавычках
    void PrintValue(const std::string& str, const PrintContext& ctx);
    // Перегрузка функции PrintValue для вывода массива Array
    void PrintValue(const Array& arr, const PrintContext& ctx);
    // Перегрузка функции PrintValue для вывода словаря Dict
    void PrintValue(const Dict& dict, const PrintContext& ctx);
    void PrintNode(const Node& node, const PrintContext& context);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json