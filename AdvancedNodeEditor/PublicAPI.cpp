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
    NodeEditorCore::Node* node = m_editor.getNode(nodeId);
    if (!node) return nullptr;
    
    NodeEditorCore::Pin* internalPin = node->findPin(pinId);
    if (!internalPin) return nullptr;
    
    static ANE::Pin apiPin(
        internalPin->id, 
        internalPin->name, 
        internalPin->isInput, 
        NodeEditorCore::convertToANEPinType(internalPin->type),
        NodeEditorCore::convertToANEPinShape(internalPin->shape)
    );
    
    return &apiPin;
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
    m_editor.setStyle(convertToInternalStyle(style));
}

EditorStyle NodeEditor::getStyle() const {
    return convertToAPIStyle(m_editor.getStyle());
}

void NodeEditor::setCanConnectCallback(CanConnectCallback callback) {
    if (callback) {
        m_editor.setCanConnectCallback([callback](const NodeEditorCore::Pin& startPin, const NodeEditorCore::Pin& endPin) {
            ANE::Pin apiStartPin(
                startPin.id, 
                startPin.name, 
                startPin.isInput, 
                NodeEditorCore::convertToANEPinType(startPin.type),
                NodeEditorCore::convertToANEPinShape(startPin.shape)
            );
            
            ANE::Pin apiEndPin(
                endPin.id,
                endPin.name,
                endPin.isInput,
                NodeEditorCore::convertToANEPinType(endPin.type),
                NodeEditorCore::convertToANEPinShape(endPin.shape)
            );
            
            return callback(apiStartPin, apiEndPin);
        });
    }
    else {
        m_editor.setCanConnectCallback(nullptr);
    }
}

NodeEditorCore::NodeEditorStyle NodeEditor::convertToInternalStyle(const EditorStyle& style) const {
    NodeEditorCore::NodeEditorStyle internalStyle;
    
    internalStyle.uiColors.background = NodeEditorCore::convertToColor(style.backgroundColor);
    internalStyle.uiColors.grid = NodeEditorCore::convertToColor(style.gridColor);
    internalStyle.uiColors.selection = NodeEditorCore::convertToColor(style.selectionColor);
    
    for (const auto& pair : style.nodeStyles) {
        NodeEditorCore::internal::NodeColors nodeColors;
        nodeColors.baseColor = NodeEditorCore::convertToColor(pair.second.baseColor);
        nodeColors.headerColor = NodeEditorCore::convertToColor(pair.second.headerColor);
        nodeColors.accentColor = NodeEditorCore::convertToColor(pair.second.accentColor);
        nodeColors.borderColor = NodeEditorCore::convertToColor(pair.second.borderColor);
        nodeColors.selectedColor = NodeEditorCore::convertToColor(pair.second.selectedColor);
        nodeColors.hoveredColor = NodeEditorCore::convertToColor(pair.second.hoveredColor);
        nodeColors.glowColor = NodeEditorCore::convertToColor(pair.second.glowColor);
        
        internalStyle.nodeColors[pair.first] = nodeColors;
    }
    
    for (const auto& pair : style.pinStyles) {
        NodeEditorCore::internal::PinColors pinColors;
        pinColors.color = NodeEditorCore::convertToColor(pair.second.color);
        pinColors.hoverColor = NodeEditorCore::convertToColor(pair.second.hoverColor);
        pinColors.connectedColor = NodeEditorCore::convertToColor(pair.second.connectedColor);
        
        internalStyle.pinColors[pair.first] = pinColors;
    }
    
    internalStyle.connectionColors.baseColor = NodeEditorCore::convertToColor(style.connectionStyle.baseColor);
    internalStyle.connectionColors.selectedColor = NodeEditorCore::convertToColor(style.connectionStyle.selectedColor);
    internalStyle.connectionColors.hoveredColor = NodeEditorCore::convertToColor(style.connectionStyle.hoveredColor);
    internalStyle.connectionColors.validColor = NodeEditorCore::convertToColor(style.connectionStyle.validColor);
    internalStyle.connectionColors.invalidColor = NodeEditorCore::convertToColor(style.connectionStyle.invalidColor);
    
    internalStyle.gridSpacing = style.gridSpacing;
    internalStyle.nodeRounding = style.nodeRounding;
    internalStyle.pinRadius = style.pinRadius;
    internalStyle.connectionThickness = style.connectionThickness;
    
    return internalStyle;
}

EditorStyle NodeEditor::convertToAPIStyle(const NodeEditorCore::NodeEditorStyle& internalStyle) const {
    EditorStyle style;
    
    style.backgroundColor = NodeEditorCore::convertToANEColor(internalStyle.uiColors.background);
    style.gridColor = NodeEditorCore::convertToANEColor(internalStyle.uiColors.grid);
    style.selectionColor = NodeEditorCore::convertToANEColor(internalStyle.uiColors.selection);
    
    for (const auto& pair : internalStyle.nodeColors) {
        NodeStyle nodeStyle;
        nodeStyle.baseColor = NodeEditorCore::convertToANEColor(pair.second.baseColor);
        nodeStyle.headerColor = NodeEditorCore::convertToANEColor(pair.second.headerColor);
        nodeStyle.accentColor = NodeEditorCore::convertToANEColor(pair.second.accentColor);
        nodeStyle.borderColor = NodeEditorCore::convertToANEColor(pair.second.borderColor);
        nodeStyle.selectedColor = NodeEditorCore::convertToANEColor(pair.second.selectedColor);
        nodeStyle.hoveredColor = NodeEditorCore::convertToANEColor(pair.second.hoveredColor);
        nodeStyle.glowColor = NodeEditorCore::convertToANEColor(pair.second.glowColor);
        
        style.nodeStyles[pair.first] = nodeStyle;
    }
    
    for (const auto& pair : internalStyle.pinColors) {
        PinStyle pinStyle;
        pinStyle.color = NodeEditorCore::convertToANEColor(pair.second.color);
        pinStyle.hoverColor = NodeEditorCore::convertToANEColor(pair.second.hoverColor);
        pinStyle.connectedColor = NodeEditorCore::convertToANEColor(pair.second.connectedColor);
        
        style.pinStyles[pair.first] = pinStyle;
    }
    
    style.connectionStyle.baseColor = NodeEditorCore::convertToANEColor(internalStyle.connectionColors.baseColor);
    style.connectionStyle.selectedColor = NodeEditorCore::convertToANEColor(internalStyle.connectionColors.selectedColor);
    style.connectionStyle.hoveredColor = NodeEditorCore::convertToANEColor(internalStyle.connectionColors.hoveredColor);
    style.connectionStyle.validColor = NodeEditorCore::convertToANEColor(internalStyle.connectionColors.validColor);
    style.connectionStyle.invalidColor = NodeEditorCore::convertToANEColor(internalStyle.connectionColors.invalidColor);
    
    style.gridSpacing = internalStyle.gridSpacing;
    style.nodeRounding = internalStyle.nodeRounding;
    style.pinRadius = internalStyle.pinRadius;
    style.connectionThickness = internalStyle.connectionThickness;
    
    return style;
}

}