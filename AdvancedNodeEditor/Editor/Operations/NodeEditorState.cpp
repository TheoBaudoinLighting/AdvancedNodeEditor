#include "../../NodeEditor.h"
#include "../../Core/Style/InteractionMode.h"
#include <algorithm>

namespace NodeEditorCore {

NodeEditor::State::State()
    : viewPosition(0.0f, 0.0f), viewScale(1.0f)
    , nextNodeId(1), nextPinId(1), nextConnectionId(1), nextGroupId(1)
    , hoveredNodeId(-1), hoveredNodeUuid(""), hoveredPinId(-1), hoveredPinUuid("")
    , hoveredConnectionId(-1), hoveredConnectionUuid(""), hoveredGroupId(-1), hoveredGroupUuid("")
    , activeNodeId(-1), activeNodeUuid(""), activeConnectionId(-1), activeConnectionUuid("")
    , activeGroupId(-1), activeGroupUuid("")
    , connecting(false), connectingNodeId(-1), connectingNodeUuid(""), connectingPinId(-1), connectingPinUuid("")
    , dragging(false), boxSelecting(false)
    , currentSubgraphId(-1), currentSubgraphUuid("")
    , interactionMode(InteractionMode::None)
    , contextMenuNodeId(-1), contextMenuNodeUuid(""), contextMenuConnectionId(-1), contextMenuConnectionUuid("")
    , contextMenuGroupId(-1), contextMenuGroupUuid(""), contextMenuPinId(-1), contextMenuPinUuid("")
    , dragStart(0.0f, 0.0f), groupStartSize(0.0f, 0.0f), contextMenuPos(0.0f, 0.0f)
{
    nodeUuidMap.clear();
    connectionUuidMap.clear();
    groupUuidMap.clear();
}

NodeEditor::NodeEditor() 
    : m_debugMode(false) {
    m_state = State();
}

NodeEditor::~NodeEditor() {
}

void NodeEditor::beginFrame() {
    m_state.hoveredNodeId = -1;
    m_state.hoveredPinId = -1;
    m_state.hoveredConnectionId = -1;
    m_state.hoveredGroupId = -1;
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
}

void NodeEditor::endFrame() {
    ImGui::PopStyleVar();
}

int NodeEditor::addNode(const std::string& name, const std::string& type, const Vec2& pos, const ANE::UUID& uuid) {
    int nodeId = m_state.nextNodeId++;
    Node node(uuid.empty() ? ANE::generateUUID() : uuid, nodeId, name, type, pos);
    
    m_state.nodes.push_back(node);
    updateNodeUuidMap();
    
    if (m_state.nodeCreatedCallback) {
        m_state.nodeCreatedCallback(nodeId, node.uuid);
    }
    
    return nodeId;
}

void NodeEditor::removeNode(int nodeId) {
    auto it = std::find_if(m_state.nodes.begin(), m_state.nodes.end(), 
                          [nodeId](const Node& node) { return node.id == nodeId; });
    
    if (it != m_state.nodes.end()) {
        m_state.connections.erase(
            std::remove_if(m_state.connections.begin(), m_state.connections.end(),
                         [nodeId](const Connection& conn) {
                             return conn.startNodeId == nodeId || conn.endNodeId == nodeId;
                         }),
            m_state.connections.end());
        
        if (m_state.nodeRemovedCallback) {
            m_state.nodeRemovedCallback(nodeId, it->uuid);
        }
        
        m_state.nodes.erase(it);
        updateNodeUuidMap();
    }
}

int NodeEditor::addConnection(int startNodeId, int startPinId, int endNodeId, int endPinId, const ANE::UUID &uuid) {
    if (doesConnectionExist(startNodeId, startPinId, endNodeId, endPinId)) {
        return -1;
    }

    const ANE::Pin* startPin = getPin(startNodeId, startPinId);
    const ANE::Pin* endPin = getPin(endNodeId, endPinId);

    if (!startPin || !endPin) {
        return -1;
    }

    if (startPin->isInput || !endPin->isInput) {
        return -1;
    }

    if (m_state.canConnectCallback && !m_state.canConnectCallback(*startPin, *endPin)) {
        return -1;
    }

    int connectionId = m_state.nextConnectionId++;
    m_state.connections.emplace_back(connectionId, startNodeId, startPinId, endNodeId, endPinId);
    
    Connection* connection = getConnection(connectionId);
    if (connection) {
        connection->uuid = uuid.empty() ? ANE::generateUUID() : uuid;
        connection->startNodeUuid = getNodeUUID(startNodeId);
        connection->startPinUuid = getPinUUID(startNodeId, startPinId);
        connection->endNodeUuid = getNodeUUID(endNodeId);
        connection->endPinUuid = getPinUUID(endNodeId, endPinId);
        updateConnectionUuidMap();
    }

    Node* startNode = getNode(startNodeId);
    Node* endNode = getNode(endNodeId);
    Pin* startPinInternal = startNode ? startNode->findPin(startPinId) : nullptr;
    Pin* endPinInternal = endNode ? endNode->findPin(endPinId) : nullptr;

    if (startPinInternal) startPinInternal->connected = true;
    if (endPinInternal) endPinInternal->connected = true;

    if (m_state.connectionCreatedCallback) {
        m_state.connectionCreatedCallback(connectionId, connection ? connection->uuid : "");
    }

    return connectionId;
}

const std::vector<Node>& NodeEditor::getNodes() const {
    return m_state.nodes;
}

const Node* NodeEditor::getNode(int nodeId) const {
    for (const auto& node : getNodes()) {
        if (node.id == nodeId) {
            return &node;
        }
    }
    return nullptr;
}

Node* NodeEditor::getNode(int nodeId) {
    for (auto& node : m_state.nodes) {
        if (node.id == nodeId) {
            return &node;
        }
    }
    return nullptr;
}

ANE::UUID NodeEditor::getHoveredNodeUUID() const {
    return m_state.hoveredNodeUuid;
}

ANE::UUID NodeEditor::getHoveredPinUUID() const {
    return m_state.hoveredPinUuid;
}

ANE::UUID NodeEditor::getHoveredConnectionUUID() const {
    return m_state.hoveredConnectionUuid;
}

ANE::UUID NodeEditor::getHoveredGroupUUID() const {
    return m_state.hoveredGroupUuid;
}

ANE::UUID NodeEditor::getCurrentSubgraphUUID() const {
    return m_state.currentSubgraphUuid;
}

void NodeEditor::setCurrentSubgraphByUUID(const ANE::UUID& uuid) {
    m_state.currentSubgraphUuid = uuid;
}

bool NodeEditor::isNodeInSubgraphByUUID(const Node& node, const ANE::UUID& subgraphUuid) const {
    return node.metadata.getAttribute<ANE::UUID>("subgraphUuid", "") == subgraphUuid;
}

bool NodeEditor::isSubgraphContainerByUUID(const ANE::UUID& uuid) const {
    int nodeId = getNodeId(uuid);
    if (nodeId == -1) return false;
    return isSubgraphContainer(nodeId);
}

ANE::UUID NodeEditor::getSubgraphUUIDFromNode(const ANE::UUID& nodeUuid) const {
    int nodeId = getNodeId(nodeUuid);
    if (nodeId == -1) return "";

    const Node* node = getNode(nodeId);
    if (!node || !node->isSubgraph) return "";

    return node->metadata.getAttribute<ANE::UUID>("subgraphUuid", "");
}

ANE::UUID NodeEditor::getNodeSubgraphUUID(const ANE::UUID& nodeUuid) const {
    int nodeId = getNodeId(nodeUuid);
    if (nodeId == -1) return "";

    const Node* node = getNode(nodeId);
    if (!node) return "";

    return node->metadata.getAttribute<ANE::UUID>("subgraphUuid", "");
}

int NodeEditor::addPin(int nodeId, const std::string& name, bool isInput, 
                     PinType type, PinShape shape, const ANE::UUID& uuid) {
    Node* node = getNode(nodeId);
    if (!node) return -1;
    
    int pinId = m_state.nextPinId++;
    Pin pin(uuid, pinId, name, isInput, type, shape);
    
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
    
    auto removeFromVec = [pinId](std::vector<Pin>& pins) {
        pins.erase(
            std::remove_if(pins.begin(), pins.end(),
                         [pinId](const Pin& pin) { return pin.id == pinId; }),
            pins.end());
    };
    
    removeFromVec(node->inputs);
    removeFromVec(node->outputs);
}

const ANE::Pin* NodeEditor::getPin(int nodeId, int pinId) const {
    static ANE::Pin convertedPin;
    
    const Node* node = getNode(nodeId);
    if (!node) return nullptr;
    
    for (const auto& pin : node->inputs) {
        if (pin.id == pinId) {
            convertedPin.id = pin.id;
            convertedPin.uuid = pin.uuid;
            convertedPin.name = pin.name;
            convertedPin.isInput = pin.isInput;
            convertedPin.type = static_cast<ANE::PinType>(pin.type);
            convertedPin.shape = static_cast<ANE::PinShape>(pin.shape);
            return &convertedPin;
        }
    }
    
    for (const auto& pin : node->outputs) {
        if (pin.id == pinId) {
            convertedPin.id = pin.id;
            convertedPin.uuid = pin.uuid;
            convertedPin.name = pin.name;
            convertedPin.isInput = pin.isInput;
            convertedPin.type = static_cast<ANE::PinType>(pin.type);
            convertedPin.shape = static_cast<ANE::PinShape>(pin.shape);
            return &convertedPin;
        }
    }
    
    return nullptr;
}

ANE::Pin* NodeEditor::getPin(int nodeId, int pinId) {
    static ANE::Pin convertedPin;
    
    Node* node = getNode(nodeId);
    if (!node) return nullptr;
    
    for (auto& pin : node->inputs) {
        if (pin.id == pinId) {
            convertedPin.id = pin.id;
            convertedPin.uuid = pin.uuid;
            convertedPin.name = pin.name;
            convertedPin.isInput = pin.isInput;
            convertedPin.type = static_cast<ANE::PinType>(pin.type);
            convertedPin.shape = static_cast<ANE::PinShape>(pin.shape);
            return &convertedPin;
        }
    }
    
    for (auto& pin : node->outputs) {
        if (pin.id == pinId) {
            convertedPin.id = pin.id;
            convertedPin.uuid = pin.uuid;
            convertedPin.name = pin.name;
            convertedPin.isInput = pin.isInput;
            convertedPin.type = static_cast<ANE::PinType>(pin.type);
            convertedPin.shape = static_cast<ANE::PinShape>(pin.shape);
            return &convertedPin;
        }
    }
    
    return nullptr;
}

int NodeEditor::addPinByNodeUUID(const ANE::UUID& nodeUuid, const std::string& name, bool isInput,
                               PinType type, PinShape shape, const ANE::UUID& uuid) {
    int nodeId = getNodeId(nodeUuid);
    if (nodeId == -1) return -1;

    return addPin(nodeId, name, isInput, type, shape, uuid);
}

void NodeEditor::removePinByUUID(const ANE::UUID& nodeUuid, const ANE::UUID& pinUuid) {
    int nodeId = getNodeId(nodeUuid);
    if (nodeId == -1) return;

    Node* node = getNode(nodeId);
    if (!node) return;

    for (const auto& pin : node->inputs) {
        if (pin.uuid == pinUuid) {
            removePin(nodeId, pin.id);
            return;
        }
    }

    for (const auto& pin : node->outputs) {
        if (pin.uuid == pinUuid) {
            removePin(nodeId, pin.id);
            return;
        }
    }
}

void NodeEditor::centerOnNodeByUUID(const ANE::UUID& uuid) {
    int nodeId = getNodeId(uuid);
    if (nodeId != -1) {
        centerOnNode(nodeId);
    }
}

}