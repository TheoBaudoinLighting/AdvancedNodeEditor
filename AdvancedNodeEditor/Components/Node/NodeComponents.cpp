#include "NodeComponents.h"
#include <algorithm>

namespace NodeEditorCore {

int Subgraph::nextId = 0;

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

}

namespace ANE {

int Subgraph::nextId = 0;

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

void Node::setAsSubgraph(bool value, int id, const UUID& uuid) {
    isSubgraph = value;
    if (value && id >= 0) {
        subgraphId = id;
        if (!uuid.empty()) {
            subgraphUuid = uuid;
        }
    }
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