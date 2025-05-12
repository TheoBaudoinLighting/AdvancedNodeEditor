#include "NodeComponents.h"
#include <algorithm>

namespace NodeEditorCore {

int Subgraph::nextId = 0;

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
    : id(id), name(name) {
}

}

namespace ANE {

int Subgraph::nextId = 0;

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
    : id(id), name(name), parentSubgraphId(-1), isExpanded(true),
      viewPosition(0.0f, 0.0f), viewScale(1.0f), isTemplate(false) {
    nodeIds.clear();
    connectionIds.clear();
    groupIds.clear();
    interfaceInputs.clear();
    interfaceOutputs.clear();
    childSubgraphIds.clear();
    
    description = "";
    category = "Default";
    iconSymbol = "";
    accentColor = Color(0.4f, 0.6f, 0.8f, 1.0f);
}

void Subgraph::addNode(int nodeId) {
    if (!containsNode(nodeId)) {
        nodeIds.push_back(nodeId);
    }
}

void Subgraph::removeNode(int nodeId) {
    auto it = std::find(nodeIds.begin(), nodeIds.end(), nodeId);
    if (it != nodeIds.end()) {
        nodeIds.erase(it);
    }
}

bool Subgraph::containsNode(int nodeId) const {
    return std::find(nodeIds.begin(), nodeIds.end(), nodeId) != nodeIds.end();
}

void Subgraph::addConnection(int connectionId) {
    if (!containsConnection(connectionId)) {
        connectionIds.push_back(connectionId);
    }
}

void Subgraph::removeConnection(int connectionId) {
    auto it = std::find(connectionIds.begin(), connectionIds.end(), connectionId);
    if (it != connectionIds.end()) {
        connectionIds.erase(it);
    }
}

bool Subgraph::containsConnection(int connectionId) const {
    return std::find(connectionIds.begin(), connectionIds.end(), connectionId) != connectionIds.end();
}

void Subgraph::addGroup(int groupId) {
    if (!containsGroup(groupId)) {
        groupIds.push_back(groupId);
    }
}

void Subgraph::removeGroup(int groupId) {
    auto it = std::find(groupIds.begin(), groupIds.end(), groupId);
    if (it != groupIds.end()) {
        groupIds.erase(it);
    }
}

bool Subgraph::containsGroup(int groupId) const {
    return std::find(groupIds.begin(), groupIds.end(), groupId) != groupIds.end();
}

void Subgraph::exposeInput(int nodeId, int pinId) {
    int interfaceId = (nodeId << 16) | pinId;
    if (std::find(interfaceInputs.begin(), interfaceInputs.end(), interfaceId) == interfaceInputs.end()) {
        interfaceInputs.push_back(interfaceId);
    }
}

void Subgraph::exposeOutput(int nodeId, int pinId) {
    int interfaceId = (nodeId << 16) | pinId;
    if (std::find(interfaceOutputs.begin(), interfaceOutputs.end(), interfaceId) == interfaceOutputs.end()) {
        interfaceOutputs.push_back(interfaceId);
    }
}

void Subgraph::unexposeInput(int nodeId, int pinId) {
    int interfaceId = (nodeId << 16) | pinId;
    auto it = std::find(interfaceInputs.begin(), interfaceInputs.end(), interfaceId);
    if (it != interfaceInputs.end()) {
        interfaceInputs.erase(it);
    }
}

void Subgraph::unexposeOutput(int nodeId, int pinId) {
    int interfaceId = (nodeId << 16) | pinId;
    auto it = std::find(interfaceOutputs.begin(), interfaceOutputs.end(), interfaceId);
    if (it != interfaceOutputs.end()) {
        interfaceOutputs.erase(it);
    }
}

bool Subgraph::isInputExposed(int nodeId, int pinId) const {
    int interfaceId = (nodeId << 16) | pinId;
    return std::find(interfaceInputs.begin(), interfaceInputs.end(), interfaceId) != interfaceInputs.end();
}

bool Subgraph::isOutputExposed(int nodeId, int pinId) const {
    int interfaceId = (nodeId << 16) | pinId;
    return std::find(interfaceOutputs.begin(), interfaceOutputs.end(), interfaceId) != interfaceOutputs.end();
}

void Subgraph::addChildSubgraph(int subgraphId) {
    if (!containsSubgraph(subgraphId)) {
        childSubgraphIds.push_back(subgraphId);
    }
}

void Subgraph::removeChildSubgraph(int subgraphId) {
    auto it = std::find(childSubgraphIds.begin(), childSubgraphIds.end(), subgraphId);
    if (it != childSubgraphIds.end()) {
        childSubgraphIds.erase(it);
    }
}

bool Subgraph::containsSubgraph(int subgraphId) const {
    return std::find(childSubgraphIds.begin(), childSubgraphIds.end(), subgraphId) != childSubgraphIds.end();
}

void Subgraph::setIconSymbol(const std::string& symbol) {
    iconSymbol = symbol;
}

void Subgraph::setAccentColor(const Color& color) {
    accentColor = color;
}

void Subgraph::setIsTemplate(bool template_value) {
    isTemplate = template_value;
}

void Subgraph::setDescription(const std::string& desc) {
    description = desc;
}

void Subgraph::setCategory(const std::string& cat) {
    category = cat;
}

void Subgraph::setViewState(const Vec2& position, float scale) {
    viewPosition = position;
    viewScale = scale;
}

bool Subgraph::validate() const {
    return !name.empty() && id >= 0;
}

std::vector<std::string> Subgraph::getValidationErrors() const {
    std::vector<std::string> errors;
    
    if (name.empty()) {
        errors.push_back("Le nom du sous-graphe ne peut pas être vide");
    }
    
    if (id < 0) {
        errors.push_back("L'ID du sous-graphe doit être positif");
    }
    
    return errors;
}

}