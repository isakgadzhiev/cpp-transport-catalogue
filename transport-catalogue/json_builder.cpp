#include "json_builder.h"

#include <utility>

using namespace json;

KeyItemContext Builder::Key(const std::string& key) {
    if (!GetCurrentNodePtr()->IsDict()) {
        throw std::logic_error("Attempting to set a key to a value outside the map");
    } else {
        auto& current_node = const_cast<Dict&>(GetCurrentNodePtr()->AsDict());
        current_node.emplace(key, key);
        nodes_stack_.push_back(&current_node[key]);
        return {*this};
    }
}

Builder& Builder::Value(Node::Value value) {
    AddNode(value);
    return *this;
}

ArrayItemContext Builder::StartArray() {
    nodes_stack_.push_back(AddNode(std::move(Array())));
    return {*this};
}

Builder& Builder::EndArray(){
    if (!GetCurrentNodePtr()->IsArray()) {
        throw std::logic_error("Attempting to end an array outside the context");
    }
    nodes_stack_.pop_back();
    return *this;
}

DictItemContext Builder::StartDict(){
    nodes_stack_.push_back(AddNode(std::move(Dict())));
    return {*this};
}

Builder& Builder::EndDict(){
    if (!GetCurrentNodePtr()->IsDict()) {
        throw std::logic_error("Attempting to end a map outside the context");
    }
    nodes_stack_.pop_back();
    return *this;
}

Node Builder::Build(){
    if (!IsEmpty() || root_.IsNull()) {
        throw std::logic_error("Attempting to build unfinished JSON");
    }
    return std::move(root_);
}

bool Builder::IsEmpty() {
    return nodes_stack_.empty();
}

Node* Builder::GetCurrentNodePtr() {
    if (IsEmpty()) {
        throw std::logic_error("Stack is empty");
    } else {
        return nodes_stack_.back();
    }
}

Node* Builder::AddNode(const Node& node) {
    if (IsEmpty() && root_.IsNull()) {
        root_ = Node(node);
        return &root_;
    } else if (GetCurrentNodePtr()->IsArray()) {
        auto& current_array = const_cast<Array&>(nodes_stack_.back()->AsArray());
        current_array.push_back(node);
        return &current_array.back();
    } else if (GetCurrentNodePtr()->IsString()) {
        const auto key = nodes_stack_.back()->AsString();
        nodes_stack_.pop_back();
        if (!nodes_stack_.back()->IsDict()) {
            throw std::logic_error("Incorrect attempt to add value for key");
        } else {
            auto& current_node = const_cast<Dict&>(nodes_stack_.back()->AsDict());
            current_node[key] = node;
            return &current_node[key];
        }
    } else {
        throw std::logic_error("Attempting to add value outside the context");
    }
}

// Helper classes methods

KeyItemContext BaseContext::Key(const std::string& key) {
    return builder_.Key(key);
}

ArrayItemContext BaseContext::StartArray() {
    return builder_.StartArray();
}

Builder& BaseContext::EndArray() {
    return builder_.EndArray();
}

DictItemContext BaseContext::StartDict() {
    return builder_.StartDict();
}

Builder& BaseContext::EndDict() {
    return builder_.EndDict();
}

DictItemContext KeyItemContext::Value(Node::Value value) {
    return {builder_.Value(std::move(value))};
}

ArrayItemContext ArrayItemContext::Value(Node::Value value) {
    return {builder_.Value(std::move(value))};
}