#include "NodeEditor.h"
#include <algorithm>
#include <cfloat>
#include <cmath>

namespace NodeEditorCore {

NodeEditor::State::State()
    : viewPosition(0.0f, 0.0f), viewScale(1.0f)
    , nextNodeId(1), nextPinId(1), nextConnectionId(1), nextGroupId(1)
    , hoveredNodeId(-1), hoveredPinId(-1), hoveredConnectionId(-1), hoveredGroupId(-1)
    , activeNodeId(-1), activeConnectionId(-1), activeGroupId(-1)
    , connecting(false), connectingNodeId(-1), connectingPinId(-1)
    , dragging(false), boxSelecting(false)
{
}

NodeEditor::NodeEditor() {
}

NodeEditor::~NodeEditor() {
}

void NodeEditor::beginFrame() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
}

void NodeEditor::endFrame() {
    ImGui::PopStyleVar();
}

int NodeEditor::addNode(const std::string& name, const std::string& type, const Vec2& pos) {
    int nodeId = m_state.nextNodeId++;
    m_state.nodes.emplace_back(nodeId, name, type, pos);
    
    if (m_state.nodeCreatedCallback) {
        m_state.nodeCreatedCallback(nodeId);
    }
    
    return nodeId;
}

void NodeEditor::removeNode(int nodeId) {
    auto nodeIt = std::find_if(m_state.nodes.begin(), m_state.nodes.end(), 
                             [nodeId](const Node& node) { return node.id == nodeId; });
    
    if (nodeIt != m_state.nodes.end()) {
        m_state.connections.erase(
            std::remove_if(m_state.connections.begin(), m_state.connections.end(),
                         [nodeId](const Connection& conn) {
                             return conn.startNodeId == nodeId || conn.endNodeId == nodeId;
                         }),
            m_state.connections.end());
        
        if (nodeIt->groupId >= 0) {
            auto groupIt = std::find_if(m_state.groups.begin(), m_state.groups.end(),
                                      [id = nodeIt->groupId](const Group& group) { return group.id == id; });
            if (groupIt != m_state.groups.end()) {
                groupIt->nodes.erase(nodeId);
            }
        }
        
        if (m_state.nodeRemovedCallback) {
            m_state.nodeRemovedCallback(nodeId);
        }
        
        m_state.nodes.erase(nodeIt);
    }
}

Node* NodeEditor::getNode(int nodeId) {
    auto it = std::find_if(m_state.nodes.begin(), m_state.nodes.end(), 
                          [nodeId](const Node& node) { return node.id == nodeId; });
    return it != m_state.nodes.end() ? &(*it) : nullptr;
}

const Node* NodeEditor::getNode(int nodeId) const {
    auto it = std::find_if(m_state.nodes.begin(), m_state.nodes.end(), 
                          [nodeId](const Node& node) { return node.id == nodeId; });
    return it != m_state.nodes.end() ? &(*it) : nullptr;
}

const std::vector<Node>& NodeEditor::getNodes() const {
    return m_state.nodes;
}

int NodeEditor::addPin(int nodeId, const std::string& name, bool isInput, PinType type, PinShape shape) {
    Node* node = getNode(nodeId);
    if (!node) return -1;
    
    int pinId = m_state.nextPinId++;
    Pin pin(pinId, name, isInput, type, shape);

    pin.id = pinId;
    pin.name = name;
    pin.isInput = isInput;
    pin.type = type;
    pin.shape = shape;
    
    if (shape == PinShape::Circle) {
        switch (type) {
            case PinType::Yellow:
                pin.shape = PinShape::Square;
                break;
            case PinType::Orange:
                pin.shape = PinShape::Triangle;
                break;
            case PinType::Red:
                pin.shape = PinShape::Diamond;
                break;
            default:
                pin.shape = PinShape::Circle;
                break;
        }
    } else {
        pin.shape = shape;
    }
    
    if (isInput) {
        node->inputs.push_back(pin);
    } else {
        node->outputs.push_back(pin);
    }
    
    return pinId;
}

void NodeEditor::removePin(int nodeId, int pinId) {
    Node* node = getNode(nodeId);
    if (!node) return;
    
    m_state.connections.erase(
        std::remove_if(m_state.connections.begin(), m_state.connections.end(),
                     [pinId](const Connection& conn) {
                         return conn.startPinId == pinId || conn.endPinId == pinId;
                     }),
        m_state.connections.end());
    
    node->inputs.erase(
        std::remove_if(node->inputs.begin(), node->inputs.end(),
                     [pinId](const Pin& pin) { return pin.id == pinId; }),
        node->inputs.end());
    
    node->outputs.erase(
        std::remove_if(node->outputs.begin(), node->outputs.end(),
                     [pinId](const Pin& pin) { return pin.id == pinId; }),
        node->outputs.end());
}

ANE::Pin* NodeEditor::getPin(int nodeId, int pinId) {
    Node* node = getNode(nodeId);
    if (!node) return nullptr;

    Pin* internalPin = node->findPin(pinId);
    if (!internalPin) return nullptr;

    static ANE::Pin apiPin;
    apiPin.id = internalPin->id;
    apiPin.name = internalPin->name;
    apiPin.isInput = internalPin->isInput;
    apiPin.type = static_cast<ANE::PinType>(internalPin->type);
    apiPin.shape = static_cast<ANE::PinShape>(internalPin->shape);

    return &apiPin;
}

const ANE::Pin* NodeEditor::getPin(int nodeId, int pinId) const {
    const Node* node = getNode(nodeId);
    if (!node) return nullptr;

    const Pin* internalPin = node->findPin(pinId);
    if (!internalPin) return nullptr;

    static ANE::Pin apiPin;
    apiPin.id = internalPin->id;
    apiPin.name = internalPin->name;
    apiPin.isInput = internalPin->isInput;
    apiPin.type = static_cast<ANE::PinType>(internalPin->type);
    apiPin.shape = static_cast<ANE::PinShape>(internalPin->shape);

    return &apiPin;
}

int NodeEditor::getHoveredNodeId() const {
    return m_state.hoveredNodeId;
}

int NodeEditor::getHoveredPinId() const {
    return m_state.hoveredPinId;
}

int NodeEditor::getHoveredConnectionId() const {
    return m_state.hoveredConnectionId;
}

int NodeEditor::getHoveredGroupId() const {
    return m_state.hoveredGroupId;
}

}