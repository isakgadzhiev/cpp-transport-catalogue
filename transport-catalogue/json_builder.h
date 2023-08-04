#pragma once
#include "json.h"

namespace json {
    class KeyItemContext;
    class ArrayItemContext;
    class DictItemContext;

    class Builder {
    public:
        Builder() = default;

        KeyItemContext Key(const std::string& key);
        Builder& Value (Node::Value value);
        ArrayItemContext StartArray();
        Builder& EndArray();
        DictItemContext StartDict();
        Builder& EndDict();
        Node Build();

        ~Builder() = default;

    private:
        Node root_;
        std::vector<Node*> nodes_stack_;

        Node* GetCurrentNodePtr();
        bool IsEmpty();
        Node* AddNode(const Node& node);
    };

    class BaseContext {
    public:
        BaseContext(Builder& builder)
            : builder_(builder) {
        }
        KeyItemContext Key(const std::string& key);
        ArrayItemContext StartArray();
        Builder& EndArray();
        DictItemContext StartDict();
        Builder& EndDict();

    protected:
        Builder& builder_;
    };

    class KeyItemContext : public BaseContext {
    public:
        DictItemContext Value(Node::Value value);
    private:
        KeyItemContext Key(const std::string& key) = delete;
        Builder& EndArray() = delete;
        Builder& EndDict() = delete;
    };

    class ArrayItemContext : public BaseContext {
    public:
        ArrayItemContext Value(Node::Value value);
    private:
        KeyItemContext Key(const std::string& key) = delete;
        Builder& EndDict() = delete;
    };

    class DictItemContext : public BaseContext {
    private:
        ArrayItemContext StartArray() = delete;
        Builder& EndArray() = delete;
        DictItemContext StartDict() = delete;
    };
}