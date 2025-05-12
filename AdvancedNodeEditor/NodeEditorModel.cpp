#include "NodeEditorModel.h"
#include <algorithm>

namespace ANE {

NodeEditorModel::NodeEditorModel()
    : m_nextNodeId(1), m_nextPinId(1), m_nextConnectionId(1), m_nextGroupId(1), m_nextSubgraphId(1)
{
}

NodeEditorModel::~NodeEditorModel() = default;

int NodeEditorModel::addNode(const std::string& name, const std::string& type, const Vec2& position) {
    int nodeId = m_nextNodeId++;
    auto node = std::make_shared<Node>();
    
    node->id = nodeId;
    node->name = name;
    node->type = type;
    node->position = position;
    node->size = Vec2(140.0f, 28.0f);
    node->selected = false;
    node->disabled = false;
    node->groupId = -1;
    node->isTemplate = false;
    node->isCurrentFlag = false;
    node->labelPosition = NodeLabelPosition::Right;
    node->isSubgraph = false;
    node->subgraphId = -1;
    
    m_nodes.push_back(node);
    
    Event event(EventType::NodeCreated);
    event.setData("nodeId", nodeId);
    dispatchEvent(event);
    
    return nodeId;
}

void NodeEditorModel::removeNode(int nodeId) {
    auto it = std::find_if(m_nodes.begin(), m_nodes.end(), 
                          [nodeId](const std::shared_ptr<Node>& node) { return node->id == nodeId; });
    
    if (it != m_nodes.end()) {
        // Remove associated connections
        m_connections.erase(
            std::remove_if(m_connections.begin(), m_connections.end(),
                         [nodeId](const std::shared_ptr<Connection>& conn) {
                             return conn->startNodeId == nodeId || conn->endNodeId == nodeId;
                         }),
            m_connections.end());
        
        // Remove from group if it's part of one
        if ((*it)->groupId >= 0) {
            auto groupIt = std::find_if(m_groups.begin(), m_groups.end(),
                                      [groupId = (*it)->groupId](const std::shared_ptr<Group>& group) { 
                                          return group->id == groupId; 
                                      });
            if (groupIt != m_groups.end()) {
                (*groupIt)->nodes.erase(nodeId);
            }
        }
        
        Event event(EventType::NodeDeleted);
        event.setData("nodeId", nodeId);
        dispatchEvent(event);
        
        m_nodes.erase(it);
    }
}

NodeEditorModel::Node* NodeEditorModel::getNode(int nodeId) {
    auto it = std::find_if(m_nodes.begin(), m_nodes.end(), 
                          [nodeId](const std::shared_ptr<Node>& node) { return node->id == nodeId; });
    return it != m_nodes.end() ? it->get() : nullptr;
}

const NodeEditorModel::Node* NodeEditorModel::getNode(int nodeId) const {
    auto it = std::find_if(m_nodes.begin(), m_nodes.end(), 
                          [nodeId](const std::shared_ptr<Node>& node) { return node->id == nodeId; });
    return it != m_nodes.end() ? it->get() : nullptr;
}

const std::vector<std::shared_ptr<NodeEditorModel::Node>>& NodeEditorModel::getNodes() const {
    return m_nodes;
}

int NodeEditorModel::addPin(int nodeId, const std::string& name, bool isInput, PinType type, PinShape shape) {
    Node* node = getNode(nodeId);
    if (!node) return -1;
    
    int pinId = m_nextPinId++;
    Pin pin(pinId, name, isInput, type, shape);
    
    pin.id = pinId;
    pin.name = name;
    pin.isInput = isInput;
    pin.type = type;
    pin.shape = shape;
    
    if (isInput) {
        node->inputs.push_back(pin);
    } else {
        node->outputs.push_back(pin);
    }
    
    return pinId;
}

void NodeEditorModel::removePin(int nodeId, int pinId) {
    Node* node = getNode(nodeId);
    if (!node) return;
    
    // Remove connections using this pin
    m_connections.erase(
        std::remove_if(m_connections.begin(), m_connections.end(),
                     [pinId](const std::shared_ptr<Connection>& conn) {
                         return conn->startPinId == pinId || conn->endPinId == pinId;
                     }),
        m_connections.end());
    
    // Remove the pin from node inputs or outputs
    auto removeFromVec = [pinId](std::vector<Pin>& pins) {
        pins.erase(
            std::remove_if(pins.begin(), pins.end(),
                         [pinId](const Pin& pin) { return pin.id == pinId; }),
            pins.end());
    };
    
    removeFromVec(node->inputs);
    removeFromVec(node->outputs);
}

Pin* NodeEditorModel::getPin(int nodeId, int pinId) {
    Node* node = getNode(nodeId);
    if (!node) return nullptr;
    
    for (auto& pin : node->inputs) {
        if (pin.id == pinId) return &pin;
    }
    
    for (auto& pin : node->outputs) {
        if (pin.id == pinId) return &pin;
    }
    
    return nullptr;
}

const Pin* NodeEditorModel::getPin(int nodeId, int pinId) const {
    const Node* node = getNode(nodeId);
    if (!node) return nullptr;
    
    for (const auto& pin : node->inputs) {
        if (pin.id == pinId) return &pin;
    }
    
    for (const auto& pin : node->outputs) {
        if (pin.id == pinId) return &pin;
    }
    
    return nullptr;
}

int NodeEditorModel::addConnection(int startNodeId, int startPinId, int endNodeId, int endPinId) {
    // Check if the connection already exists
    auto existingConnection = std::find_if(m_connections.begin(), m_connections.end(),
                                         [=](const std::shared_ptr<Connection>& conn) {
                                             return conn->startNodeId == startNodeId &&
                                                    conn->startPinId == startPinId &&
                                                    conn->endNodeId == endNodeId &&
                                                    conn->endPinId == endPinId;
                                         });
    
    if (existingConnection != m_connections.end()) {
        return -1;
    }
    
    // Verify that pins exist and are compatible
    const Pin* startPin = getPin(startNodeId, startPinId);
    const Pin* endPin = getPin(endNodeId, endPinId);
    
    if (!startPin || !endPin) {
        return -1;
    }
    
    if (startPin->isInput || !endPin->isInput) {
        return -1;
    }
    
    int connectionId = m_nextConnectionId++;
    auto connection = std::make_shared<Connection>();
    
    connection->id = connectionId;
    connection->startNodeId = startNodeId;
    connection->startPinId = startPinId;
    connection->endNodeId = endNodeId;
    connection->endPinId = endPinId;
    connection->selected = false;
    
    m_connections.push_back(connection);
    
    Event event(EventType::ConnectionCreated);
    event.setData("connectionId", connectionId);
    dispatchEvent(event);
    
    return connectionId;
}

void NodeEditorModel::removeConnection(int connectionId) {
    auto it = std::find_if(m_connections.begin(), m_connections.end(),
                          [connectionId](const std::shared_ptr<Connection>& conn) { return conn->id == connectionId; });
    
    if (it != m_connections.end()) {
        Event event(EventType::ConnectionDeleted);
        event.setData("connectionId", connectionId);
        event.setData("startNodeId", (*it)->startNodeId);
        event.setData("startPinId", (*it)->startPinId);
        event.setData("endNodeId", (*it)->endNodeId);
        event.setData("endPinId", (*it)->endPinId);
        dispatchEvent(event);
        
        m_connections.erase(it);
    }
}

NodeEditorModel::Connection* NodeEditorModel::getConnection(int connectionId) {
    auto it = std::find_if(m_connections.begin(), m_connections.end(),
                          [connectionId](const std::shared_ptr<Connection>& conn) { return conn->id == connectionId; });
    return it != m_connections.end() ? it->get() : nullptr;
}

const NodeEditorModel::Connection* NodeEditorModel::getConnection(int connectionId) const {
    auto it = std::find_if(m_connections.begin(), m_connections.end(),
                          [connectionId](const std::shared_ptr<Connection>& conn) { return conn->id == connectionId; });
    return it != m_connections.end() ? it->get() : nullptr;
}

const std::vector<std::shared_ptr<NodeEditorModel::Connection>>& NodeEditorModel::getConnections() const {
    return m_connections;
}

bool NodeEditorModel::isConnected(int nodeId, int pinId) const {
    return std::any_of(m_connections.begin(), m_connections.end(),
                     [nodeId, pinId](const std::shared_ptr<Connection>& conn) {
                         return (conn->startNodeId == nodeId && conn->startPinId == pinId) ||
                                (conn->endNodeId == nodeId && conn->endPinId == pinId);
                     });
}

int NodeEditorModel::addGroup(const std::string& name, const Vec2& position, const Vec2& size) {
    int groupId = m_nextGroupId++;
    auto group = std::make_shared<Group>(groupId, name);
    
    group->position = position;
    group->size = size;
    
    m_groups.push_back(group);
    
    Event event(EventType::GroupCreated);
    event.setData("groupId", groupId);
    dispatchEvent(event);
    
    return groupId;
}

void NodeEditorModel::removeGroup(int groupId) {
    auto it = std::find_if(m_groups.begin(), m_groups.end(),
                          [groupId](const std::shared_ptr<Group>& group) { return group->id == groupId; });
    
    if (it != m_groups.end()) {
        // Update all nodes that were in this group
        for (int nodeId : (*it)->nodes) {
            Node* node = getNode(nodeId);
            if (node) {
                node->groupId = -1;
            }
        }
        
        Event event(EventType::GroupDeleted);
        event.setData("groupId", groupId);
        dispatchEvent(event);
        
        m_groups.erase(it);
    }
}

Group* NodeEditorModel::getGroup(int groupId) {
    auto it = std::find_if(m_groups.begin(), m_groups.end(),
                          [groupId](const std::shared_ptr<Group>& group) { return group->id == groupId; });
    return it != m_groups.end() ? it->get() : nullptr;
}

const Group* NodeEditorModel::getGroup(int groupId) const {
    auto it = std::find_if(m_groups.begin(), m_groups.end(),
                          [groupId](const std::shared_ptr<Group>& group) { return group->id == groupId; });
    return it != m_groups.end() ? it->get() : nullptr;
}

const std::vector<std::shared_ptr<Group>>& NodeEditorModel::getGroups() const {
    return m_groups;
}

void NodeEditorModel::addNodeToGroup(int nodeId, int groupId) {
    Group* group = getGroup(groupId);
    Node* node = getNode(nodeId);
    
    if (!group || !node) return;
    
    // Remove from previous group if part of one
    if (node->groupId >= 0 && node->groupId != groupId) {
        Group* oldGroup = getGroup(node->groupId);
        if (oldGroup) {
            oldGroup->nodes.erase(nodeId);
        }
    }
    
    node->groupId = groupId;
    group->nodes.insert(nodeId);
}

void NodeEditorModel::removeNodeFromGroup(int nodeId, int groupId) {
    Group* group = getGroup(groupId);
    Node* node = getNode(nodeId);
    
    if (!group || !node || node->groupId != groupId) return;
    
    node->groupId = -1;
    group->nodes.erase(nodeId);
}

int NodeEditorModel::createSubgraph(const std::string& name) {
    int subgraphId = m_nextSubgraphId++;
    auto subgraph = std::make_shared<Subgraph>(subgraphId, name);
    
    m_subgraphs[subgraphId] = subgraph;
    
    return subgraphId;
}

void NodeEditorModel::removeSubgraph(int subgraphId) {
    auto it = m_subgraphs.find(subgraphId);
    if (it != m_subgraphs.end()) {
        // Update all nodes that represent this subgraph
        for (auto& node : m_nodes) {
            if (node->isSubgraph && node->subgraphId == subgraphId) {
                node->isSubgraph = false;
                node->subgraphId = -1;
            }
        }
        
        m_subgraphs.erase(it);
    }
}

Subgraph* NodeEditorModel::getSubgraph(int subgraphId) {
    auto it = m_subgraphs.find(subgraphId);
    return it != m_subgraphs.end() ? it->second.get() : nullptr;
}

const Subgraph* NodeEditorModel::getSubgraph(int subgraphId) const {
    auto it = m_subgraphs.find(subgraphId);
    return it != m_subgraphs.end() ? it->second.get() : nullptr;
}

const std::map<int, std::shared_ptr<Subgraph>>& NodeEditorModel::getSubgraphs() const {
    return m_subgraphs;
}

NodeEditorModel::Node* NodeEditorModel::createSubgraphNode(int subgraphId, const std::string& name, const Vec2& position) {
    Subgraph* subgraph = getSubgraph(subgraphId);
    if (!subgraph) return nullptr;
    
    int nodeId = m_nextNodeId++;
    auto node = std::make_shared<Node>();
    
    node->id = nodeId;
    node->name = name;
    node->type = "Subgraph";
    node->position = position;
    node->size = Vec2(160.0f, 40.0f);
    node->selected = false;
    node->disabled = false;
    node->groupId = -1;
    node->isTemplate = false;
    node->isCurrentFlag = false;
    node->labelPosition = NodeLabelPosition::Right;
    node->isSubgraph = true;
    node->subgraphId = subgraphId;
    
    m_nodes.push_back(node);
    
    Event event(EventType::NodeCreated);
    event.setData("nodeId", nodeId);
    event.setData("isSubgraph", true);
    event.setData("subgraphId", subgraphId);
    dispatchEvent(event);
    
    return node.get();
}

void NodeEditorModel::selectNode(int nodeId, bool append) {
    if (!append) {
        deselectAllNodes();
    }
    
    Node* node = getNode(nodeId);
    if (node) {
        bool wasSelected = node->selected;
        node->selected = true;
        
        if (!wasSelected) {
            Event event(EventType::NodeSelected);
            event.setData("nodeId", nodeId);
            dispatchEvent(event);
        }
    }
}

void NodeEditorModel::deselectNode(int nodeId) {
    Node* node = getNode(nodeId);
    if (node && node->selected) {
        node->selected = false;
        
        Event event(EventType::NodeDeselected);
        event.setData("nodeId", nodeId);
        dispatchEvent(event);
    }
}

void NodeEditorModel::selectAllNodes() {
    for (auto& node : m_nodes) {
        if (!node->selected) {
            node->selected = true;
            
            Event event(EventType::NodeSelected);
            event.setData("nodeId", node->id);
            dispatchEvent(event);
        }
    }
}

void NodeEditorModel::deselectAllNodes() {
    for (auto& node : m_nodes) {
        if (node->selected) {
            node->selected = false;
            
            Event event(EventType::NodeDeselected);
            event.setData("nodeId", node->id);
            dispatchEvent(event);
        }
    }
    
    for (auto& connection : m_connections) {
        connection->selected = false;
    }
    
    for (auto& group : m_groups) {
        group->selected = false;
    }
}

std::vector<int> NodeEditorModel::getSelectedNodes() const {
    std::vector<int> selectedNodes;
    for (const auto& node : m_nodes) {
        if (node->selected) {
            selectedNodes.push_back(node->id);
        }
    }
    return selectedNodes;
}

void NodeEditorModel::setState(const std::string& key, const std::any& value) {
    m_state[key] = value;
    
    Event event(EventType::StateChanged);
    event.setData("key", key);
    dispatchEvent(event);
}

bool NodeEditorModel::hasState(const std::string& key) const {
    return m_state.find(key) != m_state.end();
}

void NodeEditorModel::addEventListener(EventType type, EventCallback callback) {
    m_eventListeners[type].push_back(callback);
}

void NodeEditorModel::removeEventListener(EventType type, EventCallback callback) {
    // Note: This is a simplified implementation that doesn't actually remove the specific callback
    // since function comparison is non-trivial. In a real implementation, you would need a way
    // to identify and remove specific callbacks.
    m_eventListeners[type].clear();
}

void NodeEditorModel::dispatchEvent(const Event& event) {
    auto it = m_eventListeners.find(event.type);
    if (it != m_eventListeners.end()) {
        for (const auto& callback : it->second) {
            callback(event);
        }
    }
    
    // Also dispatch to listeners of any event type
    auto anyIt = m_eventListeners.find(EventType::Custom);
    if (anyIt != m_eventListeners.end()) {
        for (const auto& callback : anyIt->second) {
            callback(event);
        }
    }
}

}