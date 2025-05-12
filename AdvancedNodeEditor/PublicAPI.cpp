#include "NodeEditor.h"
#include "Conversions.h"
#include "NodeEditorModel.h"

namespace ANE {

NodeEditor::NodeEditor() : m_currentSubgraphId(-1) {
}

NodeEditor::~NodeEditor() {
}

void NodeEditor::beginFrame() {
    m_editor.beginFrame();
}

void NodeEditor::render() {
    m_editor.render();
}

void NodeEditor::endFrame() {
    m_editor.endFrame();
}

int NodeEditor::addNode(const std::string& name, const std::string& type, const Vec2& position) {
    return m_editor.addNode(name, type, NodeEditorCore::convertToVec2(position));
}

Node* NodeEditor::getNode(int nodeId) {
    NodeEditorCore::Node* internalNode = m_editor.getNode(nodeId);
    if (!internalNode) return nullptr;
    
    static ANE::Node apiNode(internalNode->id, internalNode->name, internalNode->type);
    apiNode.iconSymbol = internalNode->iconSymbol;
    apiNode.labelPosition = NodeEditorCore::convertToANENodeLabelPosition(internalNode->labelPosition);
    apiNode.disabled = internalNode->disabled;
    apiNode.isTemplate = internalNode->isTemplate;
    apiNode.isCurrentFlag = internalNode->isCurrentFlag;
    
    return &apiNode;
}

void NodeEditor::removeNode(int nodeId) {
    m_editor.removeNode(nodeId);
}

int NodeEditor::addPin(int nodeId, const std::string& name, bool isInput, PinType type, PinShape shape) {
    return m_editor.addPin(
        nodeId, 
        name, 
        isInput, 
        NodeEditorCore::convertToPinType(type), 
        NodeEditorCore::convertToPinShape(shape)
    );
}

Pin* NodeEditor::getPin(int nodeId, int pinId) {
    return m_editor.getPin(nodeId, pinId);
}

int NodeEditor::addConnection(int startNodeId, int startPinId, int endNodeId, int endPinId) {
    return m_editor.addConnection(startNodeId, startPinId, endNodeId, endPinId);
}

void NodeEditor::removeConnection(int connectionId) {
    m_editor.removeConnection(connectionId);
}

int NodeEditor::addGroup(const std::string& name, const Vec2& position, const Vec2& size) {
    return m_editor.addGroup(
        name,
        NodeEditorCore::convertToVec2(position),
        NodeEditorCore::convertToVec2(size)
    );
}

Group* NodeEditor::getGroup(int groupId) {
    NodeEditorCore::Group* internalGroup = m_editor.getGroup(groupId);
    if (!internalGroup) return nullptr;

    static ANE::Group apiGroup(internalGroup->id, internalGroup->name);
    apiGroup.color = NodeEditorCore::convertToANEColor(internalGroup->color);
    apiGroup.style = NodeEditorCore::convertToANEGroupStyle(internalGroup->style);
    apiGroup.collapsed = internalGroup->collapsed;

    return &apiGroup;
}

void NodeEditor::addNodeToGroup(int nodeId, int groupId) {
    m_editor.addNodeToGroup(nodeId, groupId);
}

void NodeEditor::selectNode(int nodeId, bool append) {
    m_editor.selectNode(nodeId, append);
}

std::vector<int> NodeEditor::getSelectedNodes() const {
    return m_editor.getSelectedNodes();
}

void NodeEditor::centerView() {
    m_editor.centerView();
}

void NodeEditor::setViewScale(float scale) {
    m_editor.setViewScale(scale);
}

float NodeEditor::getViewScale() const {
    return m_editor.getViewScale();
}

void NodeEditor::setStyle(const EditorStyle& style) {
    m_editor.setStyle(NodeEditorCore::convertToInternalStyle(style));
}

EditorStyle NodeEditor::getStyle() const {
    return NodeEditorCore::convertToAPIStyle(m_editor.getStyle());
}

void NodeEditor::setCanConnectCallback(CanConnectCallback callback) {
    if (callback) {
        m_editor.setCanConnectCallback([callback](const ANE::Pin& startPin, const ANE::Pin& endPin) {
            return callback(startPin, endPin);
        });
    }
    else {
        m_editor.setCanConnectCallback(nullptr);
    }
}

int NodeEditor::createSubgraph(const std::string& name) {
    int subgraphId = Subgraph::nextId++;
    auto subgraph = std::make_shared<Subgraph>(subgraphId, name);
    m_subgraphs[subgraphId] = subgraph;
    return subgraphId;
}

Subgraph* NodeEditor::getSubgraph(int subgraphId) {
    auto it = m_subgraphs.find(subgraphId);
    if (it != m_subgraphs.end()) {
        return it->second.get();
    }
    return nullptr;
}

void NodeEditor::removeSubgraph(int subgraphId) {
    auto it = m_subgraphs.find(subgraphId);
    if (it == m_subgraphs.end()) {
        return;
    }
    
    if (m_currentSubgraphId == subgraphId) {
        exitSubgraph();
    }
    
    bool inStack = false;
    std::stack<int> tempStack;
    while (!m_subgraphStack.empty()) {
        int id = m_subgraphStack.top();
        m_subgraphStack.pop();
        
        if (id == subgraphId) {
            inStack = true;
        } else {
            tempStack.push(id);
        }
    }
    
    while (!tempStack.empty()) {
        m_subgraphStack.push(tempStack.top());
        tempStack.pop();
    }
    
    m_subgraphs.erase(it);
}

Node* NodeEditor::createSubgraphNode(int subgraphId, const std::string& name, const Vec2& position) {
    if (m_subgraphs.find(subgraphId) == m_subgraphs.end()) {
        return nullptr;
    }
    
    int nodeId = addNode(name.empty() ? m_subgraphs[subgraphId]->name : name, "Subgraph", position);
    Node* node = getNode(nodeId);
    if (node) {
        node->setAsSubgraph(true, subgraphId);
        
        if (!m_subgraphs[subgraphId]->iconSymbol.empty()) {
            node->setIconSymbol(m_subgraphs[subgraphId]->iconSymbol);
        } else {
            node->setIconSymbol("S");
        }
    }
    return node;
}

bool NodeEditor::enterSubgraph(int subgraphId) {
    if (m_subgraphs.find(subgraphId) == m_subgraphs.end()) {
        return false;
    }
    
    saveSubgraphViewState(m_currentSubgraphId);
    
    if (m_currentSubgraphId >= 0) {
        m_subgraphStack.push(m_currentSubgraphId);
    }
    m_currentSubgraphId = subgraphId;
    
    restoreSubgraphViewState(subgraphId);
    
    return true;
}

bool NodeEditor::exitSubgraph() {
    if (m_currentSubgraphId < 0 || m_subgraphStack.empty()) {
        return false;
    }
    
    saveSubgraphViewState(m_currentSubgraphId);
    
    int parentSubgraphId = m_subgraphStack.top();
    m_subgraphStack.pop();
    m_currentSubgraphId = parentSubgraphId;
    
    restoreSubgraphViewState(m_currentSubgraphId);
    
    return true;
}

int NodeEditor::getCurrentSubgraphId() const {
    return m_currentSubgraphId;
}

std::vector<int> NodeEditor::getSubgraphStack() const {
    std::vector<int> stack;
    std::stack<int> tempStack = m_subgraphStack;
    
    while (!tempStack.empty()) {
        stack.push_back(tempStack.top());
        tempStack.pop();
    }
    
    std::reverse(stack.begin(), stack.end());
    
    return stack;
}

void NodeEditor::exposeNodeInput(int nodeId, int pinId) {
    if (m_currentSubgraphId < 0) {
        return;
    }
    
    auto it = m_subgraphs.find(m_currentSubgraphId);
    if (it != m_subgraphs.end()) {
        it->second->exposeInput(nodeId, pinId);
    }
}

void NodeEditor::exposeNodeOutput(int nodeId, int pinId) {
    if (m_currentSubgraphId < 0) {
        return;
    }
    
    auto it = m_subgraphs.find(m_currentSubgraphId);
    if (it != m_subgraphs.end()) {
        it->second->exposeOutput(nodeId, pinId);
    }
}

void NodeEditor::unexposeNodeInput(int nodeId, int pinId) {
    if (m_currentSubgraphId < 0) {
        return;
    }
    
    auto it = m_subgraphs.find(m_currentSubgraphId);
    if (it != m_subgraphs.end()) {
        it->second->unexposeInput(nodeId, pinId);
    }
}

void NodeEditor::unexposeNodeOutput(int nodeId, int pinId) {
    if (m_currentSubgraphId < 0) {
        return;
    }
    
    auto it = m_subgraphs.find(m_currentSubgraphId);
    if (it != m_subgraphs.end()) {
        it->second->unexposeOutput(nodeId, pinId);
    }
}

void NodeEditor::saveSubgraphViewState(int subgraphId) {
    if (subgraphId < 0) {
        return;
    }
    
    auto it = m_subgraphs.find(subgraphId);
    if (it == m_subgraphs.end()) {
        return;
    }
    
    float viewScale = getViewScale();
    Vec2 viewPosition = it->second->viewPosition;
    
    it->second->setViewState(viewPosition, viewScale);
}

void NodeEditor::restoreSubgraphViewState(int subgraphId) {
    if (subgraphId < 0) {
        return;
    }
    
    auto it = m_subgraphs.find(subgraphId);
    if (it == m_subgraphs.end()) {
        return;
    }
    
    setViewScale(it->second->viewScale);
    centerView();
}

NodeEditorCore::NodeEditorStyle NodeEditor::convertToInternalStyle(const EditorStyle& style) const {
    return NodeEditorCore::convertToInternalStyle(style);
}

EditorStyle NodeEditor::convertToAPIStyle(const NodeEditorCore::NodeEditorStyle& style) const {
    return NodeEditorCore::convertToAPIStyle(style);
}

void NodeEditor::addNodeToSubgraph(int nodeId, int subgraphId) {
    auto it = m_subgraphs.find(subgraphId);
    if (it == m_subgraphs.end()) {
        return;
    }
    
    Node* node = getNode(nodeId);
    if (!node) {
        return;
    }
    
    it->second->addNode(nodeId);
}

void NodeEditor::removeNodeFromSubgraph(int nodeId, int subgraphId) {
    auto it = m_subgraphs.find(subgraphId);
    if (it == m_subgraphs.end()) {
        return;
    }
    
    Node* node = getNode(nodeId);
    if (!node) {
        return;
    }
    
    it->second->removeNode(nodeId);
    
    std::vector<int> connectionsToRemove;
    for (int connectionId : getConnectionsInSubgraph(subgraphId)) {
        ConnectionInfo info = getConnectionInfo(connectionId);
        if (info.startNodeId == nodeId || info.endNodeId == nodeId) {
            connectionsToRemove.push_back(connectionId);
        }
    }
    
    for (int connectionId : connectionsToRemove) {
        it->second->removeConnection(connectionId);
    }
}

void NodeEditor::addConnectionToSubgraph(int connectionId, int subgraphId) {
    auto it = m_subgraphs.find(subgraphId);
    if (it == m_subgraphs.end()) {
        return;
    }
    
    it->second->addConnection(connectionId);
}

void NodeEditor::removeConnectionFromSubgraph(int connectionId, int subgraphId) {
    auto it = m_subgraphs.find(subgraphId);
    if (it == m_subgraphs.end()) {
        return;
    }
    
    it->second->removeConnection(connectionId);
}

std::vector<int> NodeEditor::getNodesInSubgraph(int subgraphId) const {
    auto it = m_subgraphs.find(subgraphId);
    if (it == m_subgraphs.end()) {
        return {};
    }
    
    return it->second->nodeIds;
}

std::vector<int> NodeEditor::getConnectionsInSubgraph(int subgraphId) const {
    auto it = m_subgraphs.find(subgraphId);
    if (it == m_subgraphs.end()) {
        return {};
    }
    
    return it->second->connectionIds;
}

NodeEditor::ConnectionInfo NodeEditor::getConnectionInfo(int connectionId) const {
    ConnectionInfo info;
    info.id = connectionId;
    info.startNodeId = -1;
    info.endNodeId = -1;
    
    const NodeEditorCore::Connection* connection = m_editor.getConnection(connectionId);
    if (connection) {
        info.startNodeId = connection->startNodeId;
        info.endNodeId = connection->endNodeId;
    }
    
    return info;
}

}