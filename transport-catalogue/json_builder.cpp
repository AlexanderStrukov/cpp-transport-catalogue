#include "json_builder.h"
#include <exception>
#include <variant>
#include <utility>

using namespace std::literals;

namespace json {

    Builder::Builder() : root_(), nodes_stack_{ &root_ } {}

    Node Builder::Build() {
        if (!nodes_stack_.empty()) {
            throw std::logic_error("Attempt to build JSON which isn't finalized"s);
        }

        if (root_.IsNull()) {
            throw std::logic_error("JSON is not defined"s);
        }

        return std::move(root_);
    }

    Builder::DictValueContext Builder::Key(std::string key) {
        if (nodes_stack_.empty()) {
            throw std::logic_error("JSON object is already finalized"s);
        }

        if (!nodes_stack_.back()->IsDict()) {
            throw std::logic_error("Key can only be added to a dictionary"s);
        }

        Node::Value& value = nodes_stack_.back()->GetValue();
        nodes_stack_.push_back(&std::get<Dict>(value)[std::move(key)]);

        return BaseContext{ *this };
    }

    Builder::BaseContext Builder::Value(Node::Value value) {
        AddObject(std::move(value), false);
        return *this;
    }

    Builder::DictItemContext Builder::StartDict() {
        AddObject(Dict{}, true);
        return BaseContext{ *this };
    }

    Builder::ArrayItemContext Builder::StartArray() {
        AddObject(Array{}, true);
        return BaseContext{ *this };
    }

    Builder::BaseContext Builder::EndDict() {
        if (nodes_stack_.empty()) {
            throw std::logic_error("JSON object is already finalized"s);
        }

        if (!nodes_stack_.back()->IsDict()) {
            throw std::logic_error("JSON object is not a dictionary"s);
        }

        nodes_stack_.pop_back();
        return *this;
    }

    Builder::BaseContext Builder::EndArray() {
        if (nodes_stack_.empty()) {
            throw std::logic_error("JSON object is already finalized"s);
        }

        if (!nodes_stack_.back()->IsArray()) {
            throw std::logic_error("JSON object is not an array"s);
        }

        nodes_stack_.pop_back();
        return *this;
    }

    void Builder::AddObject(Node::Value value, bool isStart) {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Attempt to change finalized JSON"s);
        }

        Node::Value& last_value = nodes_stack_.back()->GetValue();
        if (nodes_stack_.back()->IsArray()) {
            Node& node
                = std::get<Array>(last_value).emplace_back(std::move(value));
            if (isStart) {
                nodes_stack_.push_back(&node);
            }
        }
        else {
            if (!std::holds_alternative<std::nullptr_t>(last_value)) {
                throw std::logic_error("New object in wrong context"s);
            }

            last_value = std::move(value);
            if (!isStart) {
                nodes_stack_.pop_back();
            }
        }
    }

}  // namespace json