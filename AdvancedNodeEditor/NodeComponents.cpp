#include "NodeComponents.h"

namespace NodeEditorCore {

Pin::Pin(int id, const std::string& name, bool isInput, PinType type)
    : id(id), name(name), label(name), isInput(isInput), type(type), shape(PinShape::Circle)
    , connected(false)
{
    switch (type) {
        case PinType::Flow:    color = Color(0.9f, 0.3f, 0.3f); break;
        case PinType::Bool:    color = Color(0.95f, 0.95f, 0.65f); break;
        case PinType::Int:     color = Color(0.3f, 0.8f, 0.3f); break;
        case PinType::Float:   color = Color(0.65f, 0.83f, 0.95f); break;
        case PinType::String:  color = Color(0.8f, 0.3f, 0.8f); break;
        case PinType::Vec2:    color = Color(0.8f, 0.4f, 0.2f); break;
        case PinType::Vec3:    color = Color(0.75f, 0.95f, 0.75f); break;
        case PinType::Vec4:    color = Color(0.4f, 0.4f, 0.8f); break;
        case PinType::Color:   color = Color(0.8f, 0.3f, 0.3f); break;
        default:               color = Color(0.7f, 0.7f, 0.7f); break;
    }
}

Node::Node(int id, const std::string& name, const std::string& type, const Vec2& pos)
    : id(id), name(name), type(type), position(pos), size(140.0f, 28.0f)
    , selected(false), disabled(false), groupId(-1), isTemplate(false), isCurrentFlag(false)
    , labelPosition(NodeLabelPosition::Right)
{
}

Pin* Node::findPin(int pinId) {
    for (auto& pin : inputs) {
        if (pin.id == pinId) return &pin;
    }
    for (auto& pin : outputs) {
        if (pin.id == pinId) return &pin;
    }
    return nullptr;
}

const Pin* Node::findPin(int pinId) const {
    for (const auto& pin : inputs) {
        if (pin.id == pinId) return &pin;
    }
    for (const auto& pin : outputs) {
        if (pin.id == pinId) return &pin;
    }
    return nullptr;
}

Connection::Connection(int id, int startNodeId, int startPinId, int endNodeId, int endPinId)
    : id(id), startNodeId(startNodeId), startPinId(startPinId)
    , endNodeId(endNodeId), endPinId(endPinId), selected(false)
{
}

Group::Group(int id, const std::string& name, const Vec2& pos, const Vec2& size)
    : id(id), name(name), position(pos), size(size)
    , collapsed(false), selected(false), style(GroupStyle::Default)
{
    color = Color(0.3f, 0.3f, 0.4f, 0.4f);
}

}

namespace ANE {

Node::Node(int id, const std::string& name, const std::string& type)
    : id(id), name(name), type(type), labelPosition(NodeLabelPosition::Right), 
      disabled(false), isTemplate(false), isCurrentFlag(false) {
}

void Node::setIconSymbol(const std::string& symbol) {
    iconSymbol = symbol;
}

void Node::setLabelPosition(NodeLabelPosition position) {
    labelPosition = position;
}

void Node::setDisabled(bool value) {
    disabled = value;
}

void Node::setAsTemplate(bool value) {
    isTemplate = value;
}

void Node::setCurrentFlag(bool value) {
    isCurrentFlag = value;
}

Pin::Pin(int id, const std::string& name, bool isInput, PinType type, PinShape shape)
    : id(id), name(name), isInput(isInput), type(type), shape(shape) {
}

Group::Group(int id, const std::string& name)
    : id(id), name(name), collapsed(false), style(GroupStyle::Default) {
    color = Color(0.2f, 0.2f, 0.25f, 0.25f);
}

void Group::setColor(const Color& newColor) {
    color = newColor;
}

void Group::setStyle(GroupStyle newStyle) {
    style = newStyle;
}

void Group::setCollapsed(bool value) {
    collapsed = value;
}

}