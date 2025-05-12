#include "NodeComponents.h"

namespace NodeEditorCore {

// L'implémentation de Pin est maintenant dans le header

Node::Node(int id, const std::string& name, const std::string& type, const Vec2& pos)
    : id(id), name(name), type(type), position(pos), size(140.0f, 28.0f)
    , selected(false), disabled(false), groupId(-1), isTemplate(false), isCurrentFlag(false)
    , labelPosition(NodeLabelPosition::Right), isSubgraph(false), subgraphId(-1)
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

Subgraph::Subgraph(int id, const std::string& name)
    : id(id), name(name)
{
}

}

namespace ANE {

Node::Node(int id, const std::string& name, const std::string& type)
    : id(id), name(name), type(type), labelPosition(NodeLabelPosition::Right),
      disabled(false), isTemplate(false), isCurrentFlag(false), isSubgraph(false),
      subgraphId(-1), position(0.0f, 0.0f), size(140.0f, 28.0f), selected(false) {
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

void Node::setAsSubgraph(bool value, int id) {
    isSubgraph = value;
    if (value && id >= 0) {
        subgraphId = id;
    }
}

Group::Group(int id, const std::string& name)
    : id(id), name(name), collapsed(false), style(GroupStyle::Default),
      selected(false), position(0.0f, 0.0f), size(200.0f, 150.0f) {
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

Subgraph::Subgraph(int id, const std::string& name)
    : id(id), name(name) {
}

}