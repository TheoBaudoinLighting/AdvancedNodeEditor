#include "NodeEditor.h"
#include "Conversions.h"

namespace ANE {

NodeEditor::NodeEditor() {
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

NodeEditorCore::NodeEditorStyle NodeEditor::convertToInternalStyle(const EditorStyle& style) const {
    return NodeEditorCore::convertToInternalStyle(style);
}

EditorStyle NodeEditor::convertToAPIStyle(const NodeEditorCore::NodeEditorStyle& style) const {
    return NodeEditorCore::convertToAPIStyle(style);
}

}