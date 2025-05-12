#include "NodeEditorController.h"

namespace ANE {

NodeEditorController::NodeEditorController()
    : m_model(std::make_shared<NodeEditorModel>())
    , m_currentSubgraphId(-1)
{
}

NodeEditorController::~NodeEditorController() = default;

int NodeEditorController::addNode(const std::string& name, const std::string& type, const Vec2& position) {
    return m_model->addNode(name, type, position);
}

void NodeEditorController::removeNode(int nodeId) {
    m_model->removeNode(nodeId);
}

void NodeEditorController::updateNode(int nodeId, const std::function<void(Node&)>& updateFn) {
    NodeEditorModel::Node* node = m_model->getNode(nodeId);
    if (node) {
        Node tempNode(node->id, node->name, node->type);
        tempNode.iconSymbol = node->iconSymbol;
        tempNode.labelPosition = node->labelPosition;
        tempNode.disabled = node->disabled;
        tempNode.isTemplate = node->isTemplate;
        tempNode.isCurrentFlag = node->isCurrentFlag;
        tempNode.isSubgraph = node->isSubgraph;
        tempNode.subgraphId = node->subgraphId;
        tempNode.metadata = node->metadata;
        
        updateFn(tempNode);
        
        node->name = tempNode.name;
        node->type = tempNode.type;
        node->iconSymbol = tempNode.iconSymbol;
        node->labelPosition = tempNode.labelPosition;
        node->disabled = tempNode.disabled;
        node->isTemplate = tempNode.isTemplate;
        node->isCurrentFlag = tempNode.isCurrentFlag;
        node->isSubgraph = tempNode.isSubgraph;
        node->subgraphId = tempNode.subgraphId;
        node->metadata = tempNode.metadata;
        
        Event event(EventType::StateChanged);
        event.setData("nodeId", nodeId);
        event.setData("action", "nodeUpdated");
        m_model->dispatchEvent(event);
    }
}

int NodeEditorController::addPin(int nodeId, const std::string& name, bool isInput, PinType type, PinShape shape) {
    return m_model->addPin(nodeId, name, isInput, type, shape);
}

void NodeEditorController::removePin(int nodeId, int pinId) {
    m_model->removePin(nodeId, pinId);
}

void NodeEditorController::updatePin(int nodeId, int pinId, const std::function<void(Pin&)>& updateFn) {
    Pin* pin = m_model->getPin(nodeId, pinId);
    if (pin) {
        updateFn(*pin);
        
        Event event(EventType::StateChanged);
        event.setData("nodeId", nodeId);
        event.setData("pinId", pinId);
        event.setData("action", "pinUpdated");
        m_model->dispatchEvent(event);
    }
}

int NodeEditorController::addConnection(int startNodeId, int startPinId, int endNodeId, int endPinId) {
    return m_model->addConnection(startNodeId, startPinId, endNodeId, endPinId);
}

void NodeEditorController::removeConnection(int connectionId) {
    m_model->removeConnection(connectionId);
}

void NodeEditorController::updateConnection(int connectionId, const std::function<void(NodeEditorModel::Connection&)>& updateFn) {
    NodeEditorModel::Connection* connection = m_model->getConnection(connectionId);
    if (connection) {
        updateFn(*connection);
        
        Event event(EventType::StateChanged);
        event.setData("connectionId", connectionId);
        event.setData("action", "connectionUpdated");
        m_model->dispatchEvent(event);
    }
}

int NodeEditorController::addGroup(const std::string& name, const Vec2& position, const Vec2& size) {
    return m_model->addGroup(name, position, size);
}

void NodeEditorController::removeGroup(int groupId) {
    m_model->removeGroup(groupId);
}

void NodeEditorController::updateGroup(int groupId, const std::function<void(Group&)>& updateFn) {
    Group* group = m_model->getGroup(groupId);
    if (group) {
        Group tempGroup(group->id, group->name);
        tempGroup.color = group->color;
        tempGroup.style = group->style;
        tempGroup.collapsed = group->collapsed;
        tempGroup.metadata = group->metadata;
        
        updateFn(tempGroup);
        
        group->name = tempGroup.name;
        group->color = tempGroup.color;
        group->style = tempGroup.style;
        group->collapsed = tempGroup.collapsed;
        group->metadata = tempGroup.metadata;
        
        Event event(EventType::StateChanged);
        event.setData("groupId", groupId);
        event.setData("action", "groupUpdated");
        m_model->dispatchEvent(event);
    }
}

void NodeEditorController::addNodeToGroup(int nodeId, int groupId) {
    m_model->addNodeToGroup(nodeId, groupId);
}

void NodeEditorController::removeNodeFromGroup(int nodeId, int groupId) {
    m_model->removeNodeFromGroup(nodeId, groupId);
}

int NodeEditorController::createSubgraph(const std::string& name) {
    return m_model->createSubgraph(name);
}

void NodeEditorController::removeSubgraph(int subgraphId) {
    m_model->removeSubgraph(subgraphId);
}

int NodeEditorController::addNodeToSubgraph(int nodeId, int subgraphId) {
    NodeEditorModel::Node* node = m_model->getNode(nodeId);
    if (node && m_model->getSubgraph(subgraphId)) {
        node->isSubgraph = true;
        node->subgraphId = subgraphId;
        
        Event event(EventType::StateChanged);
        event.setData("nodeId", nodeId);
        event.setData("subgraphId", subgraphId);
        event.setData("action", "nodeAddedToSubgraph");
        m_model->dispatchEvent(event);
        
        return nodeId;
    }
    return -1;
}

int NodeEditorController::addSubgraphNode(int subgraphId, const std::string& name, const Vec2& position) {
    NodeEditorModel::Node* node = m_model->createSubgraphNode(subgraphId, name, position);
    return node ? node->id : -1;
}

void NodeEditorController::enterSubgraph(int subgraphId) {
    if (m_model->getSubgraph(subgraphId)) {
        m_subgraphNavigationStack.push(m_currentSubgraphId);
        m_currentSubgraphId = subgraphId;
        
        Event event(EventType::StateChanged);
        event.setData("previousSubgraphId", m_subgraphNavigationStack.top());
        event.setData("currentSubgraphId", subgraphId);
        event.setData("action", "enteredSubgraph");
        m_model->dispatchEvent(event);
    }
}

void NodeEditorController::exitSubgraph() {
    if (!m_subgraphNavigationStack.empty()) {
        int previousSubgraphId = m_currentSubgraphId;
        m_currentSubgraphId = m_subgraphNavigationStack.top();
        m_subgraphNavigationStack.pop();
        
        Event event(EventType::StateChanged);
        event.setData("previousSubgraphId", previousSubgraphId);
        event.setData("currentSubgraphId", m_currentSubgraphId);
        event.setData("action", "exitedSubgraph");
        m_model->dispatchEvent(event);
    }
}

void NodeEditorController::selectNode(int nodeId, bool append) {
    m_model->selectNode(nodeId, append);
}

void NodeEditorController::deselectNode(int nodeId) {
    m_model->deselectNode(nodeId);
}

void NodeEditorController::selectAllNodes() {
    m_model->selectAllNodes();
}

void NodeEditorController::deselectAllNodes() {
    m_model->deselectAllNodes();
}

std::vector<int> NodeEditorController::getSelectedNodes() const {
    return m_model->getSelectedNodes();
}

void NodeEditorController::addEventListener(EventType type, EventCallback callback) {
    m_model->addEventListener(type, callback);
}

void NodeEditorController::removeEventListener(EventType type, EventCallback callback) {
    m_model->removeEventListener(type, callback);
}

void NodeEditorController::dispatchEvent(const Event& event) {
    m_model->dispatchEvent(event);
}

std::shared_ptr<NodeEditorModel> NodeEditorController::getModel() const {
    return m_model;
}

}