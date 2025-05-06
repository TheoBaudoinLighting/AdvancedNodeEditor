#include "AdvancedNodeEditor.h"
#include <algorithm>
#include <cfloat>
#include <cmath>

using namespace NodeEditorCore;

namespace ANE {

EditorStyle::EditorStyle() {
    backgroundColor = Color(0.10f, 0.11f, 0.12f, 1.00f);
    gridColor = Color(0.16f, 0.17f, 0.18f, 0.50f);
    selectionColor = Color(0.00f, 0.44f, 0.80f, 0.30f);
    
    gridSpacing = 20.0f;
    nodeRounding = 5.0f;
    pinRadius = 4.0f;
    connectionThickness = 2.5f;
}

Node::Node(int id, const std::string& name, const std::string& type)
    : id(id), name(name), type(type), labelPosition(NodeLabelPosition::Right), 
      disabled(false), isTemplate(false), isCurrentFlag(false) {
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

Pin::Pin(int id, const std::string& name, bool isInput, PinType type, PinShape shape)
    : id(id), name(name), isInput(isInput), type(type), shape(shape) {
}

Group::Group(int id, const std::string& name)
    : id(id), name(name), collapsed(false), style(GroupStyle::Default) {
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

} // namespace ANE

Pin::Pin(int id, const std::string& name, bool isInput, PinType type)
    : id(id), name(name), label(name), isInput(isInput), type(type), shape(PinShape::Circle)
    , connected(false)
{
    switch (type) {
        case PinType::Flow:    color = Color(0.9f, 0.3f, 0.3f); break;
        case PinType::Bool:    color = Color(0.95f, 0.95f, 0.65f); break;
        case PinType::Int:     color = Color(0.3f, 0.8f, 0.3f); break;
        case PinType::Float:   color = Color(0.65f, 0.83f, 0.95f); break;
        case PinType::String:  color = Color(0.8f, 0.3f, 0.8f); break;
        case PinType::Vec2:    color = Color(0.8f, 0.4f, 0.2f); break;
        case PinType::Vec3:    color = Color(0.75f, 0.95f, 0.75f); break;
        case PinType::Vec4:    color = Color(0.4f, 0.4f, 0.8f); break;
        case PinType::Color:   color = Color(0.8f, 0.3f, 0.3f); break;
        default:               color = Color(0.7f, 0.7f, 0.7f); break;
    }
}

Node::Node(int id, const std::string& name, const std::string& type, const Vec2& pos)
    : id(id), name(name), type(type), position(pos), size(140.0f, 28.0f)
    , selected(false), disabled(false), groupId(-1), isTemplate(false), isCurrentFlag(false)
    , labelPosition(NodeLabelPosition::Right)
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

NodeEditorStyle::NodeEditorStyle()
    : gridSpacing(16.0f)
    , nodeRounding(4.0f)
    , pinRadius(3.5f)
    , connectionThickness(2.5f)
{
    uiColors.background = Color(0.15f, 0.15f, 0.17f, 1.00f);
    uiColors.grid = Color(0.23f, 0.23f, 0.26f, 0.314f);
    uiColors.selection = Color(0.70f, 0.80f, 1.00f, 0.392f);
    uiColors.menuBar = Color(0.20f, 0.20f, 0.23f, 1.00f);
    uiColors.tooltipBg = Color(0.27f, 0.29f, 0.32f, 0.941f);
    uiColors.tooltipText = Color(0.95f, 0.95f, 0.97f, 1.00f);

    internal::NodeColors geometryColors;
    geometryColors.baseColor = Color(0.180f, 0.220f, 0.250f, 1.0f);
    geometryColors.headerColor = Color(0.160f, 0.180f, 0.210f, 0.7f);
    geometryColors.accentColor = Color(0.650f, 0.800f, 0.850f, 1.0f);
    geometryColors.borderColor = Color(0.220f, 0.260f, 0.300f, 1.0f);
    geometryColors.selectedColor = Color(0.500f, 0.650f, 0.780f, 1.0f);
    geometryColors.hoveredColor = Color(0.450f, 0.550f, 0.700f, 1.0f);
    geometryColors.glowColor = Color(0.250f, 0.300f, 0.350f, 0.235f);
    nodeColors["Geometry"] = geometryColors;

    internal::NodeColors materialColors;
    materialColors.baseColor = Color(0.230f, 0.200f, 0.220f, 1.0f);
    materialColors.headerColor = Color(0.190f, 0.170f, 0.180f, 0.7f);
    materialColors.accentColor = Color(0.950f, 0.780f, 0.800f, 1.0f);
    materialColors.borderColor = Color(0.250f, 0.230f, 0.240f, 1.0f);
    materialColors.selectedColor = Color(0.780f, 0.650f, 0.700f, 1.0f);
    materialColors.hoveredColor = Color(0.700f, 0.580f, 0.650f, 1.0f);
    materialColors.glowColor = Color(0.300f, 0.250f, 0.280f, 0.235f);
    nodeColors["Material"] = materialColors;

    internal::NodeColors mathColors;
    mathColors.baseColor = Color(0.210f, 0.190f, 0.240f, 1.0f);
    mathColors.headerColor = Color(0.180f, 0.160f, 0.200f, 0.7f);
    mathColors.accentColor = Color(0.800f, 0.750f, 0.950f, 1.0f);
    mathColors.borderColor = Color(0.240f, 0.220f, 0.270f, 1.0f);
    mathColors.selectedColor = Color(0.680f, 0.620f, 0.780f, 1.0f);
    mathColors.hoveredColor = Color(0.600f, 0.550f, 0.700f, 1.0f);
    mathColors.glowColor = Color(0.270f, 0.250f, 0.300f, 0.235f);
    nodeColors["Math"] = mathColors;

    internal::NodeColors yellowColors;
    yellowColors.baseColor = Color(0.250f, 0.240f, 0.180f, 1.0f);
    yellowColors.headerColor = Color(0.210f, 0.200f, 0.150f, 0.7f);
    yellowColors.accentColor = Color(0.980f, 0.960f, 0.700f, 1.0f);
    yellowColors.borderColor = Color(0.280f, 0.270f, 0.200f, 1.0f);
    yellowColors.selectedColor = Color(0.850f, 0.830f, 0.580f, 1.0f);
    yellowColors.hoveredColor = Color(0.750f, 0.740f, 0.520f, 1.0f);
    yellowColors.glowColor = Color(0.300f, 0.290f, 0.210f, 0.235f);
    nodeColors["Yellow"] = yellowColors;

    internal::NodeColors defaultColors;
    defaultColors.baseColor = Color(0.200f, 0.200f, 0.200f, 1.0f);
    defaultColors.headerColor = Color(0.170f, 0.170f, 0.170f, 0.7f);
    defaultColors.accentColor = Color(0.800f, 0.800f, 0.800f, 1.0f);
    defaultColors.borderColor = Color(0.250f, 0.250f, 0.250f, 1.0f);
    defaultColors.selectedColor = Color(0.700f, 0.700f, 0.700f, 1.0f);
    defaultColors.hoveredColor = Color(0.600f, 0.600f, 0.600f, 1.0f);
    defaultColors.glowColor = Color(0.300f, 0.300f, 0.300f, 0.235f);
    nodeColors["Default"] = defaultColors;

    internal::PinColors boolColors;
    boolColors.color = Color(0.950f, 0.950f, 0.650f, 1.0f);
    boolColors.hoverColor = Color(1.000f, 1.000f, 0.750f, 1.0f);
    boolColors.connectedColor = Color(1.000f, 1.000f, 0.800f, 1.0f);
    pinColors["Bool"] = boolColors;
    pinColors["Boolean"] = boolColors;

    internal::PinColors floatColors;
    floatColors.color = Color(0.650f, 0.830f, 0.950f, 1.0f);
    floatColors.hoverColor = Color(0.750f, 0.900f, 1.000f, 1.0f);
    floatColors.connectedColor = Color(0.800f, 0.920f, 1.000f, 1.0f);
    pinColors["Float"] = floatColors;

    internal::PinColors vec3Colors;
    vec3Colors.color = Color(0.750f, 0.950f, 0.750f, 1.0f);
    vec3Colors.hoverColor = Color(0.830f, 1.000f, 0.830f, 1.0f);
    vec3Colors.connectedColor = Color(0.880f, 1.000f, 0.880f, 1.0f);
    pinColors["Vec3"] = vec3Colors;
    pinColors["Vector3"] = vec3Colors;

    internal::PinColors redColors;
    redColors.color = Color(0.980f, 0.700f, 0.700f, 1.0f);
    redColors.hoverColor = Color(1.000f, 0.800f, 0.800f, 1.0f);
    redColors.connectedColor = Color(1.000f, 0.850f, 0.850f, 1.0f);
    pinColors["Red"] = redColors;

    internal::PinColors yellowColor;
    yellowColor.color = Color(0.980f, 0.960f, 0.700f, 1.0f);
    yellowColor.hoverColor = Color(1.000f, 0.980f, 0.800f, 1.0f);
    yellowColor.connectedColor = Color(1.000f, 0.980f, 0.850f, 1.0f);
    pinColors["Yellow"] = yellowColor;

    internal::PinColors defaultPinColors;
    defaultPinColors.color = Color(0.700f, 0.700f, 0.700f, 1.0f);
    defaultPinColors.hoverColor = Color(0.820f, 0.820f, 0.820f, 1.0f);
    defaultPinColors.connectedColor = Color(0.900f, 0.900f, 0.900f, 1.0f);
    pinColors["Default"] = defaultPinColors;
    
    connectionColors.baseColor = Color(0.600f, 0.650f, 0.700f, 0.627f);
    connectionColors.selectedColor = Color(0.850f, 0.800f, 1.000f, 0.941f);
    connectionColors.hoveredColor = Color(0.750f, 0.750f, 0.880f, 0.863f);
    connectionColors.validColor = Color(0.750f, 0.950f, 0.800f, 0.902f);
    connectionColors.invalidColor = Color(0.950f, 0.750f, 0.750f, 0.784f);
}

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

void NodeEditor::render() {
    ImGui::BeginChild("Canvas", ImVec2(0, 0), false, 
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse);
    
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    drawList->AddRectFilled(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), 
                          m_state.style.uiColors.background.toImU32());
    
    ImGui::InvisibleButton("canvas", canvasSize);
    
    if (ImGui::IsItemHovered() || ImGui::IsItemActive()) {
        processInteraction();
    }
    
    drawGrid(drawList, canvasPos);
    drawGroups(drawList, canvasPos);
    drawConnections(drawList, canvasPos);
    drawNodes(drawList, canvasPos);
    
    if (m_state.boxSelecting) {
        drawBoxSelection(drawList);
        processBoxSelection(canvasPos);
    }
    
    ImGui::EndChild();
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
    Pin pin(pinId, name, isInput, type);
    
    if (shape == PinShape::Circle) {
        switch (type) {
            case PinType::Float:
                pin.shape = PinShape::Square;
                break;
            case PinType::Vec3:
                pin.shape = PinShape::Triangle;
                break;
            case PinType::Bool:
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

Pin* NodeEditor::getPin(int nodeId, int pinId) {
    Node* node = getNode(nodeId);
    if (!node) return nullptr;
    
    return node->findPin(pinId);
}

const Pin* NodeEditor::getPin(int nodeId, int pinId) const {
    const Node* node = getNode(nodeId);
    if (!node) return nullptr;
    
    return node->findPin(pinId);
}

int NodeEditor::addConnection(int startNodeId, int startPinId, int endNodeId, int endPinId) {
    if (doesConnectionExist(startNodeId, startPinId, endNodeId, endPinId)) {
        return -1;
    }
    
    const Pin* startPin = getPin(startNodeId, startPinId);
    const Pin* endPin = getPin(endNodeId, endPinId);
    
    if (!startPin || !endPin) {
        return -1;
    }
    
    if (startPin->isInput || !endPin->isInput) {
        return -1;
    }
    
    if (!canCreateConnection(*startPin, *endPin)) {
        return -1;
    }
    
    int connectionId = m_state.nextConnectionId++;
    m_state.connections.emplace_back(connectionId, startNodeId, startPinId, endNodeId, endPinId);
    
    Pin* startPinMutable = getPin(startNodeId, startPinId);
    Pin* endPinMutable = getPin(endNodeId, endPinId);
    
    if (startPinMutable) startPinMutable->connected = true;
    if (endPinMutable) endPinMutable->connected = true;
    
    if (m_state.connectionCreatedCallback) {
        m_state.connectionCreatedCallback(connectionId);
    }
    
    return connectionId;
}

void NodeEditor::removeConnection(int connectionId) {
    auto it = std::find_if(m_state.connections.begin(), m_state.connections.end(),
                          [connectionId](const Connection& conn) { return conn.id == connectionId; });
    
    if (it != m_state.connections.end()) {
        Pin* startPin = getPin(it->startNodeId, it->startPinId);
        Pin* endPin = getPin(it->endNodeId, it->endPinId);
        
        bool startPinConnected = false;
        bool endPinConnected = false;
        
        for (const auto& conn : m_state.connections) {
            if (conn.id == connectionId) continue;
            
            if (conn.startNodeId == it->startNodeId && conn.startPinId == it->startPinId) {
                startPinConnected = true;
            }
            
            if (conn.endNodeId == it->endNodeId && conn.endPinId == it->endPinId) {
                endPinConnected = true;
            }
        }
        
        if (startPin && !startPinConnected) startPin->connected = false;
        if (endPin && !endPinConnected) endPin->connected = false;
        
        if (m_state.connectionRemovedCallback) {
            m_state.connectionRemovedCallback(connectionId);
        }
        
        m_state.connections.erase(it);
    }
}

Connection* NodeEditor::getConnection(int connectionId) {
    auto it = std::find_if(m_state.connections.begin(), m_state.connections.end(),
                          [connectionId](const Connection& conn) { return conn.id == connectionId; });
    return it != m_state.connections.end() ? &(*it) : nullptr;
}

const Connection* NodeEditor::getConnection(int connectionId) const {
    auto it = std::find_if(m_state.connections.begin(), m_state.connections.end(),
                          [connectionId](const Connection& conn) { return conn.id == connectionId; });
    return it != m_state.connections.end() ? &(*it) : nullptr;
}

const std::vector<Connection>& NodeEditor::getConnections() const {
    return m_state.connections;
}

bool NodeEditor::isConnected(int nodeId, int pinId) const {
    for (const auto& conn : m_state.connections) {
        if ((conn.startNodeId == nodeId && conn.startPinId == pinId) ||
            (conn.endNodeId == nodeId && conn.endPinId == pinId)) {
            return true;
        }
    }
    return false;
}

int NodeEditor::addGroup(const std::string& name, const Vec2& pos, const Vec2& size) {
    int groupId = m_state.nextGroupId++;
    m_state.groups.emplace_back(groupId, name, pos, size);
    return groupId;
}

void NodeEditor::removeGroup(int groupId) {
    auto it = std::find_if(m_state.groups.begin(), m_state.groups.end(),
                          [groupId](const Group& group) { return group.id == groupId; });
    
    if (it != m_state.groups.end()) {
        for (int nodeId : it->nodes) {
            Node* node = getNode(nodeId);
            if (node) node->groupId = -1;
        }
        
        m_state.groups.erase(it);
    }
}

Group* NodeEditor::getGroup(int groupId) {
    auto it = std::find_if(m_state.groups.begin(), m_state.groups.end(),
                          [groupId](const Group& group) { return group.id == groupId; });
    return it != m_state.groups.end() ? &(*it) : nullptr;
}

const Group* NodeEditor::getGroup(int groupId) const {
    auto it = std::find_if(m_state.groups.begin(), m_state.groups.end(),
                          [groupId](const Group& group) { return group.id == groupId; });
    return it != m_state.groups.end() ? &(*it) : nullptr;
}

void NodeEditor::addNodeToGroup(int nodeId, int groupId) {
    Group* group = getGroup(groupId);
    Node* node = getNode(nodeId);
    
    if (!group || !node) return;
    
    if (node->groupId >= 0 && node->groupId != groupId) {
        Group* oldGroup = getGroup(node->groupId);
        if (oldGroup) {
            oldGroup->nodes.erase(nodeId);
        }
    }
    
    node->groupId = groupId;
    group->nodes.insert(nodeId);
}

void NodeEditor::removeNodeFromGroup(int nodeId, int groupId) {
    Group* group = getGroup(groupId);
    Node* node = getNode(nodeId);
    
    if (!group || !node || node->groupId != groupId) return;
    
    node->groupId = -1;
    group->nodes.erase(nodeId);
}

void NodeEditor::selectNode(int nodeId, bool append) {
    if (!append) {
        deselectAllNodes();
    }
    
    Node* node = getNode(nodeId);
    if (node) {
        node->selected = true;
    }
}

void NodeEditor::deselectNode(int nodeId) {
    Node* node = getNode(nodeId);
    if (node) {
        node->selected = false;
    }
}

void NodeEditor::selectAllNodes() {
    for (auto& node : m_state.nodes) {
        node.selected = true;
    }
}

void NodeEditor::deselectAllNodes() {
    for (auto& node : m_state.nodes) {
        node.selected = false;
    }
    
    for (auto& connection : m_state.connections) {
        connection.selected = false;
    }
    
    for (auto& group : m_state.groups) {
        group.selected = false;
    }
}

std::vector<int> NodeEditor::getSelectedNodes() const {
    std::vector<int> selectedNodes;
    for (const auto& node : m_state.nodes) {
        if (node.selected) {
            selectedNodes.push_back(node.id);
        }
    }
    return selectedNodes;
}

void NodeEditor::setViewPosition(const Vec2& position) {
    m_state.viewPosition = position;
}

Vec2 NodeEditor::getViewPosition() const {
    return m_state.viewPosition;
}

void NodeEditor::setViewScale(float scale) {
    m_state.viewScale = std::max(0.1f, std::min(scale, 5.0f));
}

float NodeEditor::getViewScale() const {
    return m_state.viewScale;
}

void NodeEditor::centerView() {
    if (m_state.nodes.empty()) {
        m_state.viewPosition = Vec2(0, 0);
        return;
    }
    
    Vec2 min(FLT_MAX, FLT_MAX);
    Vec2 max(-FLT_MAX, -FLT_MAX);
    
    for (const auto& node : m_state.nodes) {
        min.x = std::min(min.x, node.position.x);
        min.y = std::min(min.y, node.position.y);
        max.x = std::max(max.x, node.position.x + node.size.x);
        max.y = std::max(max.y, node.position.y + node.size.y);
    }
    
    Vec2 center = Vec2(
        (min.x + max.x) * 0.5f,
        (min.y + max.y) * 0.5f
    );
    
    ImVec2 windowSize = ImGui::GetWindowSize();
    m_state.viewPosition = Vec2(
        windowSize.x * 0.5f - center.x * m_state.viewScale,
        windowSize.y * 0.5f - center.y * m_state.viewScale
    );
}

void NodeEditor::centerOnNode(int nodeId) {
    const Node* node = getNode(nodeId);
    if (!node) return;
    
    Vec2 center = Vec2(
        node->position.x + node->size.x * 0.5f,
        node->position.y + node->size.y * 0.5f
    );
    
    ImVec2 windowSize = ImGui::GetWindowSize();
    m_state.viewPosition = Vec2(
        windowSize.x * 0.5f - center.x * m_state.viewScale,
        windowSize.y * 0.5f - center.y * m_state.viewScale
    );
}

void NodeEditor::setStyle(const NodeEditorStyle& style) {
    m_state.style = style;
}

const NodeEditorStyle& NodeEditor::getStyle() const {
    return m_state.style;
}

void NodeEditor::setNodeCreatedCallback(NodeCallback callback) {
    m_state.nodeCreatedCallback = callback;
}

void NodeEditor::setNodeRemovedCallback(NodeCallback callback) {
    m_state.nodeRemovedCallback = callback;
}

void NodeEditor::setConnectionCreatedCallback(ConnectionCallback callback) {
    m_state.connectionCreatedCallback = callback;
}

void NodeEditor::setConnectionRemovedCallback(ConnectionCallback callback) {
    m_state.connectionRemovedCallback = callback;
}

void NodeEditor::setCanConnectCallback(CanConnectCallback callback) {
    m_state.canConnectCallback = callback;
}

Vec2 NodeEditor::screenToCanvas(const Vec2& pos) const {
    return Vec2(
        (pos.x - m_state.viewPosition.x) / m_state.viewScale,
        (pos.y - m_state.viewPosition.y) / m_state.viewScale
    );
}

Vec2 NodeEditor::canvasToScreen(const Vec2& pos) const {
    return Vec2(
        pos.x * m_state.viewScale + m_state.viewPosition.x,
        pos.y * m_state.viewScale + m_state.viewPosition.y
    );
}

void NodeEditor::processInteraction() {
    ImVec2 mousePos = ImGui::GetMousePos();
    
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f)) {
        m_state.viewPosition.x += ImGui::GetIO().MouseDelta.x;
        m_state.viewPosition.y += ImGui::GetIO().MouseDelta.y;
    }
    
    if (ImGui::GetIO().MouseWheel != 0.0f) {
        float oldScale = m_state.viewScale;
        float newScale = m_state.viewScale * (ImGui::GetIO().MouseWheel > 0.0f ? 1.1f : 0.9f);
        
        newScale = std::max(0.2f, std::min(newScale, 3.0f));
        
        ImVec2 viewCenter = ImVec2(
            ImGui::GetWindowSize().x * 0.5f,
            ImGui::GetWindowSize().y * 0.5f
        );
        
        ImVec2 mouseRelPos = ImVec2(
            mousePos.x - ImGui::GetWindowPos().x,
            mousePos.y - ImGui::GetWindowPos().y
        );
        
        ImVec2 mouseOffset = ImVec2(
            mouseRelPos.x - viewCenter.x,
            mouseRelPos.y - viewCenter.y
        );
        
        m_state.viewPosition.x = viewCenter.x - (viewCenter.x - m_state.viewPosition.x) * (newScale / oldScale) + mouseOffset.x * (1.0f - (newScale / oldScale));
        m_state.viewPosition.y = viewCenter.y - (viewCenter.y - m_state.viewPosition.y) * (newScale / oldScale) + mouseOffset.y * (1.0f - (newScale / oldScale));
        m_state.viewScale = newScale;
    }
    
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        m_state.activeNodeId = -1;
        m_state.activeConnectionId = -1;
        m_state.activeGroupId = -1;
        
        for (auto& node : m_state.nodes) {
            ImVec2 nodePos = canvasToScreen(node.position).toImVec2();
            ImVec2 nodeSize = Vec2(node.size.x * m_state.viewScale, node.size.y * m_state.viewScale).toImVec2();
            
            if (isPointInRect(mousePos, nodePos, ImVec2(nodePos.x + nodeSize.x, nodePos.y + nodeSize.y))) {
                m_state.activeNodeId = node.id;
                
                if (!ImGui::GetIO().KeyCtrl) {
                    deselectAllNodes();
                }
                
                node.selected = true;
                m_state.dragging = true;
                m_state.dragOffset = Vec2(
                    mousePos.x - nodePos.x,
                    mousePos.y - nodePos.y
                );
                break;
            }
            
            for (const auto& pin : node.inputs) {
                if (isPinHovered(node, pin, ImGui::GetWindowPos())) {
                    m_state.hoveredNodeId = node.id;
                    m_state.hoveredPinId = pin.id;
                    
                    if (!ImGui::GetIO().KeyCtrl) {
                        deselectAllNodes();
                    }
                    
                    break;
                }
            }
            
            for (const auto& pin : node.outputs) {
                if (isPinHovered(node, pin, ImGui::GetWindowPos())) {
                    m_state.hoveredNodeId = node.id;
                    m_state.hoveredPinId = pin.id;
                    
                    m_state.connecting = true;
                    m_state.connectingNodeId = node.id;
                    m_state.connectingPinId = pin.id;
                    
                    if (!ImGui::GetIO().KeyCtrl) {
                        deselectAllNodes();
                    }
                    
                    break;
                }
            }
        }
        
        for (auto& connection : m_state.connections) {
            if (isConnectionHovered(connection, ImGui::GetWindowPos())) {
                m_state.activeConnectionId = connection.id;
                
                if (!ImGui::GetIO().KeyCtrl) {
                    deselectAllNodes();
                }
                
                connection.selected = true;
                break;
            }
        }
        
        if (m_state.activeNodeId == -1 && m_state.activeConnectionId == -1 && 
            m_state.activeGroupId == -1 && !m_state.connecting) {
            m_state.boxSelecting = true;
            m_state.boxSelectStart = Vec2::fromImVec2(mousePos);
            
            if (!ImGui::GetIO().KeyCtrl) {
                deselectAllNodes();
            }
        }
    }
    
    if (m_state.dragging && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)) {
        processNodeDragging();
    }
    
    if (m_state.connecting) {
        processConnectionCreation();
    }
    
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        m_state.dragging = false;
        m_state.boxSelecting = false;
        m_state.connecting = false;
    }
    
    if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
        std::vector<int> connectionsToRemove;
        for (const auto& connection : m_state.connections) {
            if (connection.selected) {
                connectionsToRemove.push_back(connection.id);
            }
        }
        
        for (int id : connectionsToRemove) {
            removeConnection(id);
        }
        
        std::vector<int> nodesToRemove;
        for (const auto& node : m_state.nodes) {
            if (node.selected) {
                nodesToRemove.push_back(node.id);
            }
        }
        
        for (int id : nodesToRemove) {
            removeNode(id);
        }
    }
}

void NodeEditor::processBoxSelection(const ImVec2& canvasPos) {
    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 boxMin = ImVec2(
        std::min(m_state.boxSelectStart.x, mousePos.x),
        std::min(m_state.boxSelectStart.y, mousePos.y)
    );
    
    ImVec2 boxMax = ImVec2(
        std::max(m_state.boxSelectStart.x, mousePos.x),
        std::max(m_state.boxSelectStart.y, mousePos.y)
    );
    
    for (auto& node : m_state.nodes) {
        ImVec2 nodePos = canvasToScreen(node.position).toImVec2();
        ImVec2 nodeSize = Vec2(node.size.x * m_state.viewScale, node.size.y * m_state.viewScale).toImVec2();
        ImVec2 nodeMin = nodePos;
        ImVec2 nodeMax = ImVec2(nodePos.x + nodeSize.x, nodePos.y + nodeSize.y);
        
        bool contained = nodeMin.x >= boxMin.x && nodeMax.x <= boxMax.x &&
                        nodeMin.y >= boxMin.y && nodeMax.y <= boxMax.y;
        
        bool intersected = !(nodeMax.x < boxMin.x || nodeMin.x > boxMax.x ||
                            nodeMax.y < boxMin.y || nodeMin.y > boxMax.y);
        
        if (contained || intersected) {
            node.selected = true;
        } else if (!ImGui::GetIO().KeyCtrl) {
            node.selected = false;
        }
    }
}

void NodeEditor::processNodeDragging() {
    if (m_state.activeNodeId == -1) return;
    
    ImVec2 mousePos = ImGui::GetMousePos();
    
    auto nodeIt = std::find_if(m_state.nodes.begin(), m_state.nodes.end(),
                             [id = m_state.activeNodeId](const Node& node) { return node.id == id; });
    
    if (nodeIt == m_state.nodes.end()) return;
    
    ImVec2 newScreenPos = ImVec2(
        mousePos.x - m_state.dragOffset.x,
        mousePos.y - m_state.dragOffset.y
    );
    
    Vec2 newCanvasPos = screenToCanvas(Vec2::fromImVec2(newScreenPos));
    
    nodeIt->position = newCanvasPos;
    
    if (!ImGui::GetIO().KeyCtrl) {
        Vec2 delta = newCanvasPos - nodeIt->position;
        
        for (auto& node : m_state.nodes) {
            if (node.selected && node.id != m_state.activeNodeId) {
                node.position = node.position + delta;
            }
        }
    }
}

void NodeEditor::processConnectionCreation() {
    if (!m_state.connecting || m_state.connectingNodeId == -1 || m_state.connectingPinId == -1) return;
    
    ImVec2 mousePos = ImGui::GetMousePos();
    
    for (const auto& node : m_state.nodes) {
        if (node.id == m_state.connectingNodeId) continue;
        
        for (const auto& pin : node.inputs) {
            if (isPinHovered(node, pin, ImGui::GetWindowPos())) {
                const Node* sourceNode = getNode(m_state.connectingNodeId);
                if (!sourceNode) return;
                
                const Pin* sourcePin = nullptr;
                for (const auto& p : sourceNode->outputs) {
                    if (p.id == m_state.connectingPinId) {
                        sourcePin = &p;
                        break;
                    }
                }
                
                if (sourcePin && canCreateConnection(*sourcePin, pin)) {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                    
                    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                        createConnection(m_state.connectingNodeId, m_state.connectingPinId, node.id, pin.id);
                        m_state.connecting = false;
                        m_state.connectingNodeId = -1;
                        m_state.connectingPinId = -1;
                    }
                    
                    return;
                }
            }
        }
        
        for (const auto& pin : node.outputs) {
            if (isPinHovered(node, pin, ImGui::GetWindowPos())) {
                const Node* targetNode = getNode(m_state.connectingNodeId);
                if (!targetNode) return;
                
                const Pin* targetPin = nullptr;
                for (const auto& p : targetNode->inputs) {
                    if (p.id == m_state.connectingPinId) {
                        targetPin = &p;
                        break;
                    }
                }
                
                if (targetPin && canCreateConnection(pin, *targetPin)) {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                    
                    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                        createConnection(node.id, pin.id, m_state.connectingNodeId, m_state.connectingPinId);
                        m_state.connecting = false;
                        m_state.connectingNodeId = -1;
                        m_state.connectingPinId = -1;
                    }
                    
                    return;
                }
            }
        }
    }
    
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        m_state.connecting = false;
        m_state.connectingNodeId = -1;
        m_state.connectingPinId = -1;
    }
}

void NodeEditor::drawGrid(ImDrawList* drawList, const ImVec2& canvasPos) {
    const float GRID_STEP_MAJOR = 64.0f * m_state.viewScale;
    const float GRID_STEP_MINOR = 16.0f * m_state.viewScale;
    
    ImU32 gridMinorColor = IM_COL32(
        m_state.style.uiColors.grid.r * 255 * 0.7f,
        m_state.style.uiColors.grid.g * 255 * 0.7f,
        m_state.style.uiColors.grid.b * 255 * 0.7f,
        m_state.style.uiColors.grid.a * 255 * 0.2f
    );

    ImU32 gridMajorColor = IM_COL32(
        m_state.style.uiColors.grid.r * 255,
        m_state.style.uiColors.grid.g * 255,
        m_state.style.uiColors.grid.b * 255,
        m_state.style.uiColors.grid.a * 255 * 0.4f
    );
    
    ImVec2 windowSize = ImGui::GetWindowSize();
    
    for (float x = fmodf(m_state.viewPosition.x, GRID_STEP_MINOR); x < windowSize.x; x += GRID_STEP_MINOR) {
        if (fmodf(x - fmodf(m_state.viewPosition.x, GRID_STEP_MAJOR), GRID_STEP_MAJOR) != 0.0f) {
            drawList->AddLine(
                ImVec2(canvasPos.x + x, canvasPos.y),
                ImVec2(canvasPos.x + x, canvasPos.y + windowSize.y),
                gridMinorColor, 0.5f
            );
        }
    }
    
    for (float y = fmodf(m_state.viewPosition.y, GRID_STEP_MINOR); y < windowSize.y; y += GRID_STEP_MINOR) {
        if (fmodf(y - fmodf(m_state.viewPosition.y, GRID_STEP_MAJOR), GRID_STEP_MAJOR) != 0.0f) {
            drawList->AddLine(
                ImVec2(canvasPos.x, canvasPos.y + y),
                ImVec2(canvasPos.x + windowSize.x, canvasPos.y + y),
                gridMinorColor, 0.5f
            );
        }
    }
    
    for (float x = fmodf(m_state.viewPosition.x, GRID_STEP_MAJOR); x < windowSize.x; x += GRID_STEP_MAJOR) {
        drawList->AddLine(
            ImVec2(canvasPos.x + x, canvasPos.y),
            ImVec2(canvasPos.x + x, canvasPos.y + windowSize.y),
            gridMajorColor, 1.0f
        );
    }
    
    for (float y = fmodf(m_state.viewPosition.y, GRID_STEP_MAJOR); y < windowSize.y; y += GRID_STEP_MAJOR) {
        drawList->AddLine(
            ImVec2(canvasPos.x, canvasPos.y + y),
            ImVec2(canvasPos.x + windowSize.x, canvasPos.y + y),
            gridMajorColor, 1.0f
        );
    }
    
    const float fadeWidth = 50.0f;
    ImU32 fadeColor = IM_COL32(0, 0, 0, 30);
    
    for (float i = 0; i < fadeWidth; i++) {
        float alpha = 30.0f * (1.0f - i / fadeWidth);
        ImU32 currentFadeColor = IM_COL32(0, 0, 0, static_cast<int>(alpha));
        
        drawList->AddLine(
            ImVec2(canvasPos.x + i, canvasPos.y),
            ImVec2(canvasPos.x + i, canvasPos.y + windowSize.y),
            currentFadeColor, 1.0f
        );
        
        drawList->AddLine(
            ImVec2(canvasPos.x + windowSize.x - i, canvasPos.y),
            ImVec2(canvasPos.x + windowSize.x - i, canvasPos.y + windowSize.y),
            currentFadeColor, 1.0f
        );
    }
    
    for (float i = 0; i < fadeWidth; i++) {
        float alpha = 30.0f * (1.0f - i / fadeWidth);
        ImU32 currentFadeColor = IM_COL32(0, 0, 0, static_cast<int>(alpha));
        
        drawList->AddLine(
            ImVec2(canvasPos.x, canvasPos.y + i),
            ImVec2(canvasPos.x + windowSize.x, canvasPos.y + i),
            currentFadeColor, 1.0f
        );
        
        drawList->AddLine(
            ImVec2(canvasPos.x, canvasPos.y + windowSize.y - i),
            ImVec2(canvasPos.x + windowSize.x, canvasPos.y + windowSize.y - i),
            currentFadeColor, 1.0f
        );
    }
}

void NodeEditor::drawConnections(ImDrawList* drawList, const ImVec2& canvasPos) {
    for (const auto& connection : m_state.connections) {
        const Node* startNode = getNode(connection.startNodeId);
        const Node* endNode = getNode(connection.endNodeId);
        const Pin* startPin = getPin(connection.startNodeId, connection.startPinId);
        const Pin* endPin = getPin(connection.endNodeId, connection.endPinId);
        
        if (!startNode || !endNode || !startPin || !endPin) continue;
        
        ImVec2 p1 = getPinPos(*startNode, *startPin, canvasPos);
        ImVec2 p2 = getPinPos(*endNode, *endPin, canvasPos);
        
        std::string startPinType = pinTypeToString(startPin->type);
        std::string endPinType = pinTypeToString(endPin->type);
        
        const internal::PinColors& startPinColors = m_state.style.pinColors.count(startPinType) ?
                                       m_state.style.pinColors.at(startPinType) :
                                       m_state.style.pinColors.at("Default");
                                       
        const internal::PinColors& endPinColors = m_state.style.pinColors.count(endPinType) ?
                                     m_state.style.pinColors.at(endPinType) :
                                     m_state.style.pinColors.at("Default");
        
        ImU32 startColor = IM_COL32(
            startPinColors.connectedColor.r * 255,
            startPinColors.connectedColor.g * 255,
            startPinColors.connectedColor.b * 255,
            startPinColors.connectedColor.a * 255 * 0.8f
        );
        
        ImU32 endColor = IM_COL32(
            endPinColors.connectedColor.r * 255,
            endPinColors.connectedColor.g * 255,
            endPinColors.connectedColor.b * 255,
            endPinColors.connectedColor.a * 255 * 0.8f
        );
        
        ImU32 outerColor = IM_COL32(40, 44, 52, 100);
        
        ImU32 coreColor = IM_COL32(
            (startPinColors.connectedColor.r + endPinColors.connectedColor.r) * 0.5f * 255 + 50,
            (startPinColors.connectedColor.g + endPinColors.connectedColor.g) * 0.5f * 255 + 50,
            (startPinColors.connectedColor.b + endPinColors.connectedColor.b) * 0.5f * 255 + 50,
            200
        );
        
        if (connection.selected) {
            startColor = IM_COL32(
                m_state.style.connectionColors.selectedColor.r * 255,
                m_state.style.connectionColors.selectedColor.g * 255,
                m_state.style.connectionColors.selectedColor.b * 255,
                m_state.style.connectionColors.selectedColor.a * 255
            );
            endColor = startColor;
            coreColor = IM_COL32(
                m_state.style.connectionColors.selectedColor.r * 255 + 50,
                m_state.style.connectionColors.selectedColor.g * 255 + 50,
                m_state.style.connectionColors.selectedColor.b * 255 + 50,
                m_state.style.connectionColors.selectedColor.a * 255
            );
            outerColor = IM_COL32(
                m_state.style.connectionColors.selectedColor.r * 255 * 0.7f,
                m_state.style.connectionColors.selectedColor.g * 255 * 0.7f,
                m_state.style.connectionColors.selectedColor.b * 255 * 0.7f,
                150
            );
        }
        
        float distance = std::abs(p2.y - p1.y);
        float controlPointYOffset = std::max(distance * 0.5f, 40.0f);
        
        ImVec2 cp1 = ImVec2(p1.x, p1.y + controlPointYOffset);
        ImVec2 cp2 = ImVec2(p2.x, p2.y - controlPointYOffset);
        
        const float outerThickness = 3.5f * m_state.viewScale;
        drawList->AddBezierCubic(p1, cp1, cp2, p2, outerColor, outerThickness);
        
        const float mainThickness = 2.0f * m_state.viewScale;
        const int lineSegments = 20;
        
        const float startSolidPct = 0.15f; 
        const float endSolidPct = 0.15f;  
        const float transitionPct = 0.7f; 
        
        for (int i = 0; i < lineSegments; i++) {
            float t0 = static_cast<float>(i) / lineSegments;
            float t1 = static_cast<float>(i + 1) / lineSegments;
            
            ImVec2 pos0 = ImBezierCubicCalc(p1, cp1, cp2, p2, t0);
            ImVec2 pos1 = ImBezierCubicCalc(p1, cp1, cp2, p2, t1);
            
            ImU32 segmentColor;
            if (t0 < startSolidPct) {
                segmentColor = startColor;
            } 
            else if (t0 > (1.0f - endSolidPct)) {
                segmentColor = endColor;
            }
            else {
                float transitionPos = (t0 - startSolidPct) / transitionPct;
                transitionPos = std::min(1.0f, std::max(0.0f, transitionPos));
                segmentColor = ImLerpColor(startColor, endColor, transitionPos);
            }
            
            drawList->AddLine(pos0, pos1, segmentColor, mainThickness);
        }
        
        const float coreThickness = 0.8f * m_state.viewScale;
        
        for (int i = 0; i < lineSegments; i++) {
            float t0 = static_cast<float>(i) / lineSegments;
            float t1 = static_cast<float>(i + 1) / lineSegments;
            
            ImVec2 pos0 = ImBezierCubicCalc(p1, cp1, cp2, p2, t0);
            ImVec2 pos1 = ImBezierCubicCalc(p1, cp1, cp2, p2, t1);
            
            ImVec4 brightStartVec4 = ImGui::ColorConvertU32ToFloat4(startColor);
            ImVec4 brightEndVec4 = ImGui::ColorConvertU32ToFloat4(endColor);
            
            brightStartVec4.x = std::min(brightStartVec4.x + 0.3f, 1.0f);
            brightStartVec4.y = std::min(brightStartVec4.y + 0.3f, 1.0f);
            brightStartVec4.z = std::min(brightStartVec4.z + 0.3f, 1.0f);
            brightStartVec4.w = 0.7f;
            
            brightEndVec4.x = std::min(brightEndVec4.x + 0.3f, 1.0f);
            brightEndVec4.y = std::min(brightEndVec4.y + 0.3f, 1.0f);
            brightEndVec4.z = std::min(brightEndVec4.z + 0.3f, 1.0f);
            brightEndVec4.w = 0.7f;
            
            ImU32 brightStart = ImGui::ColorConvertFloat4ToU32(brightStartVec4);
            ImU32 brightEnd = ImGui::ColorConvertFloat4ToU32(brightEndVec4);
            
            ImU32 brightSegmentColor;
            if (t0 < startSolidPct) {
                brightSegmentColor = brightStart;
            } 
            else if (t0 > (1.0f - endSolidPct)) {
                brightSegmentColor = brightEnd;
            }
            else {
                float transitionPos = (t0 - startSolidPct) / transitionPct;
                transitionPos = std::min(1.0f, std::max(0.0f, transitionPos));
                brightSegmentColor = ImLerpColor(brightStart, brightEnd, transitionPos);
            }
            
            drawList->AddLine(pos0, pos1, brightSegmentColor, coreThickness);
        }
        
        const float glowRadius = 2.5f * m_state.viewScale;
        ImU32 startGlowColor = IM_COL32(
            std::min(static_cast<int>(startPinColors.connectedColor.r * 255 + 50), 255),
            std::min(static_cast<int>(startPinColors.connectedColor.g * 255 + 50), 255),
            std::min(static_cast<int>(startPinColors.connectedColor.b * 255 + 50), 255),
            180
        );
        
        ImU32 endGlowColor = IM_COL32(
            std::min(static_cast<int>(endPinColors.connectedColor.r * 255 + 50), 255),
            std::min(static_cast<int>(endPinColors.connectedColor.g * 255 + 50), 255),
            std::min(static_cast<int>(endPinColors.connectedColor.b * 255 + 50), 255),
            180
        );
        
        drawList->AddCircleFilled(p1, glowRadius, startGlowColor);
        drawList->AddCircleFilled(p2, glowRadius, endGlowColor);
    }
    
    if (m_state.connecting && m_state.connectingNodeId != -1 && m_state.connectingPinId != -1) {
        drawDragConnection(drawList, canvasPos);
    }
}

void NodeEditor::drawDragConnection(ImDrawList* drawList, const ImVec2& canvasPos) {
    const Node* node = getNode(m_state.connectingNodeId);
    if (!node) return;
    
    const Pin* pin = nullptr;
    for (const auto& p : node->outputs) {
        if (p.id == m_state.connectingPinId) {
            pin = &p;
            break;
        }
    }
    
    if (!pin) {
        for (const auto& p : node->inputs) {
            if (p.id == m_state.connectingPinId) {
                pin = &p;
                break;
            }
        }
    }
    
    if (!pin) return;
    
    ImVec2 p1 = getPinPos(*node, *pin, canvasPos);
    ImVec2 p2 = ImGui::GetMousePos();
    
    std::string pinTypeName = pinTypeToString(pin->type);
    const internal::PinColors& pinColors = m_state.style.pinColors.count(pinTypeName) ?
                               m_state.style.pinColors.at(pinTypeName) :
                               m_state.style.pinColors.at("Default");
    
    ImU32 dragColor = IM_COL32(
        pinColors.color.r * 255,
        pinColors.color.g * 255,
        pinColors.color.b * 255,
        pinColors.color.a * 255 * 0.8f
    );
    
    ImU32 outerColor = IM_COL32(40, 44, 52, 100);
    float thickness = m_state.style.connectionThickness * m_state.viewScale;
    
    float distance = std::abs(p2.y - p1.y);
    float cpOffset = std::max(50.0f, distance * 0.5f);
    
    ImVec2 cp1 = ImVec2(p1.x, p1.y + cpOffset);
    ImVec2 cp2 = ImVec2(p2.x, p2.y - cpOffset);
    
    drawList->AddBezierCubic(p1, cp1, cp2, p2, outerColor, thickness + 1.5f);
    
    drawList->AddBezierCubic(p1, cp1, cp2, p2, dragColor, thickness);
    
    ImVec4 brightColor = ImGui::ColorConvertU32ToFloat4(dragColor);
    brightColor.x = std::min(brightColor.x + 0.3f, 1.0f);
    brightColor.y = std::min(brightColor.y + 0.3f, 1.0f);
    brightColor.z = std::min(brightColor.z + 0.3f, 1.0f);
    brightColor.w = 0.7f;
    
    ImU32 brightDragColor = ImGui::ColorConvertFloat4ToU32(brightColor);
    drawList->AddBezierCubic(p1, cp1, cp2, p2, brightDragColor, thickness * 0.4f);
    
    const float glowRadius = 2.5f * m_state.viewScale;
    ImU32 glowColor = IM_COL32(
        std::min(static_cast<int>(pinColors.color.r * 255 + 50), 255),
        std::min(static_cast<int>(pinColors.color.g * 255 + 50), 255),
        std::min(static_cast<int>(pinColors.color.b * 255 + 50), 255),
        180
    );
    
    drawList->AddCircleFilled(p1, glowRadius, glowColor);
}

void NodeEditor::drawNodes(ImDrawList* drawList, const ImVec2& canvasPos) {
    for (const auto& node : m_state.nodes) {
        ImVec2 nodePos = canvasToScreen(node.position).toImVec2();
        ImVec2 nodeSize = Vec2(node.size.x * m_state.viewScale, node.size.y * m_state.viewScale).toImVec2();
        
        const float cornerRadius = 4.0f * m_state.viewScale;
        const float headerHeight = 14.0f * m_state.viewScale;
        const float accentLineHeight = 1.0f * m_state.viewScale;
        
        const internal::NodeColors& nodeColors = m_state.style.nodeColors.count(node.type) ?
                                            m_state.style.nodeColors.at(node.type) :
                                            m_state.style.nodeColors.at("Default");
        
        ImU32 baseColor = IM_COL32(
            nodeColors.baseColor.r * 255,
            nodeColors.baseColor.g * 255,
            nodeColors.baseColor.b * 255,
            nodeColors.baseColor.a * 255
        );
        
        ImU32 headerColor = IM_COL32(
            nodeColors.headerColor.r * 255,
            nodeColors.headerColor.g * 255,
            nodeColors.headerColor.b * 255,
            nodeColors.headerColor.a * 255
        );
        
        ImU32 accentColor = IM_COL32(
            nodeColors.accentColor.r * 255,
            nodeColors.accentColor.g * 255,
            nodeColors.accentColor.b * 255,
            nodeColors.accentColor.a * 255
        );
        
        ImU32 borderColor = IM_COL32(
            nodeColors.borderColor.r * 255,
            nodeColors.borderColor.g * 255,
            nodeColors.borderColor.b * 255,
            nodeColors.borderColor.a * 255
        );
        
        ImU32 selectedColor = IM_COL32(
            nodeColors.selectedColor.r * 255,
            nodeColors.selectedColor.g * 255,
            nodeColors.selectedColor.b * 255,
            nodeColors.selectedColor.a * 255
        );
        
        ImU32 hoveredColor = IM_COL32(
            nodeColors.hoveredColor.r * 255,
            nodeColors.hoveredColor.g * 255,
            nodeColors.hoveredColor.b * 255,
            nodeColors.hoveredColor.a * 255
        );
        
        ImU32 glowColor = IM_COL32(
            nodeColors.glowColor.r * 255,
            nodeColors.glowColor.g * 255,
            nodeColors.glowColor.b * 255,
            nodeColors.glowColor.a * 255
        );
        
        if (node.disabled) {
            baseColor = IM_COL32(40, 40, 40, 180);
            headerColor = IM_COL32(30, 30, 35, 180);
            accentColor = IM_COL32(70, 70, 80, 150);
            borderColor = IM_COL32(60, 60, 60, 180);
        }
        
        bool isHovered = m_state.hoveredNodeId == node.id;
        if (node.selected || isHovered) {
            float glowSize = node.selected ? 8.0f : 6.0f;
            
            ImU32 enhancedGlowColor = node.selected ? 
                IM_COL32(
                    nodeColors.selectedColor.r * 255,
                    nodeColors.selectedColor.g * 255,
                    nodeColors.selectedColor.b * 255,
                    static_cast<int>(nodeColors.selectedColor.a * 100)
                ) :
                IM_COL32(
                    nodeColors.hoveredColor.r * 255,
                    nodeColors.hoveredColor.g * 255,
                    nodeColors.hoveredColor.b * 255,
                    static_cast<int>(nodeColors.hoveredColor.a * 80)
                );
            
            drawList->AddRectFilled(
                ImVec2(nodePos.x - glowSize, nodePos.y - glowSize),
                ImVec2(nodePos.x + nodeSize.x + glowSize, nodePos.y + nodeSize.y + glowSize),
                enhancedGlowColor, cornerRadius + glowSize
            );
        }
        
        {
            const float shadowOffset = 4.0f;
            const float shadowFeather = 10.0f;
            const ImU32 shadowColor = IM_COL32(0, 0, 0, 40);
            
            drawList->AddRectFilled(
                ImVec2(nodePos.x + shadowOffset, nodePos.y + shadowOffset),
                ImVec2(nodePos.x + nodeSize.x + shadowOffset, nodePos.y + nodeSize.y + shadowOffset),
                shadowColor, cornerRadius, ImDrawFlags_RoundCornersAll
            );
            
            for (float i = 1; i <= shadowFeather; i++) {
                float alpha = 40 * (1.0f - i / shadowFeather);
                ImU32 featherColor = IM_COL32(0, 0, 0, static_cast<int>(alpha));
                
                drawList->AddRect(
                    ImVec2(nodePos.x + shadowOffset - i, nodePos.y + shadowOffset - i),
                    ImVec2(nodePos.x + nodeSize.x + shadowOffset + i, nodePos.y + nodeSize.y + shadowOffset + i),
                    featherColor, cornerRadius + i, 0, 1.0f
                );
            }
        }
        
        float borderThickness = node.selected ? 2.0f : 1.0f;
        ImU32 activeBorderColor = node.selected ? selectedColor : borderColor;
        
        for (float i = 0; i < borderThickness; i += 0.5f) {
            drawList->AddRect(
                ImVec2(nodePos.x - i, nodePos.y - i),
                ImVec2(nodePos.x + nodeSize.x + i, nodePos.y + nodeSize.y + i),
                activeBorderColor, cornerRadius, 0, 1.0f
            );
        }
        
        ImVec4 baseColorVec4 = ImGui::ColorConvertU32ToFloat4(baseColor);
        
        const int gradientSteps = 10;
        float stepHeight = nodeSize.y / gradientSteps;
        
        for (int step = 0; step < gradientSteps; step++) {
            float t = static_cast<float>(step) / (gradientSteps - 1);
            
            ImU32 gradientColor = ImGui::ColorConvertFloat4ToU32(ImVec4(
                baseColorVec4.x * (1.1f - t * 0.2f),
                baseColorVec4.y * (1.1f - t * 0.2f),
                baseColorVec4.z * (1.1f - t * 0.2f),
                baseColorVec4.w
            ));
            
            float yStart = nodePos.y + step * stepHeight;
            float yEnd = nodePos.y + (step + 1) * stepHeight;
            
            ImDrawFlags cornerFlags = ImDrawFlags_RoundCornersNone;
            if (step == 0)
                cornerFlags = ImDrawFlags_RoundCornersTop;
            else if (step == gradientSteps - 1)
                cornerFlags = ImDrawFlags_RoundCornersBottom;
            
            drawList->AddRectFilled(
                ImVec2(nodePos.x, yStart),
                ImVec2(nodePos.x + nodeSize.x, yEnd),
                gradientColor, cornerRadius, cornerFlags
            );
        }
        
        drawList->AddRectFilled(
            nodePos,
            ImVec2(nodePos.x + nodeSize.x, nodePos.y + headerHeight),
            headerColor, cornerRadius, ImDrawFlags_RoundCornersTop
        );
        
        float buttonWidth = headerHeight * 0.75f;
        float buttonHeight = headerHeight;
        float separatorWidth = 1.0f * m_state.viewScale;
        float rightSideX = nodePos.x + nodeSize.x - 3 * buttonWidth - 2 * separatorWidth;
        
        ImVec2 disableBarMin = ImVec2(rightSideX, nodePos.y);
        ImVec2 disableBarMax = ImVec2(rightSideX + buttonWidth, nodePos.y + buttonHeight);
        
        ImVec2 templateBarMin = ImVec2(rightSideX + buttonWidth + separatorWidth, nodePos.y);
        ImVec2 templateBarMax = ImVec2(rightSideX + 2 * buttonWidth + separatorWidth, nodePos.y + buttonHeight);
        
        ImVec2 flagBarMin = ImVec2(rightSideX + 2 * buttonWidth + 2 * separatorWidth, nodePos.y);
        ImVec2 flagBarMax = ImVec2(rightSideX + 3 * buttonWidth + 2 * separatorWidth, nodePos.y + buttonHeight);
        
        ImVec2 mousePos = ImGui::GetIO().MousePos;
        bool hoverDisableBar = mousePos.x >= disableBarMin.x && mousePos.x <= disableBarMax.x &&
                               mousePos.y >= disableBarMin.y && mousePos.y <= disableBarMax.y;
                               
        bool hoverTemplateBar = mousePos.x >= templateBarMin.x && mousePos.x <= templateBarMax.x &&
                                mousePos.y >= templateBarMin.y && mousePos.y <= templateBarMax.y;
                               
        bool hoverFlagBar = mousePos.x >= flagBarMin.x && mousePos.x <= flagBarMax.x &&
                             mousePos.y >= flagBarMin.y && mousePos.y <= flagBarMax.y;
        
        ImU32 disableColorBase = IM_COL32(100, 90, 30, hoverDisableBar ? 200 : 140);
        ImU32 templateColorBase = IM_COL32(80, 60, 120, hoverTemplateBar ? 200 : 140);
        ImU32 flagColorBase = IM_COL32(40, 80, 130, hoverFlagBar ? 200 : 140);
        
        ImU32 disableColorActive = IM_COL32(255, 230, 150, 255);
        ImU32 templateColorActive = IM_COL32(210, 170, 255, 255);
        ImU32 flagColorActive = IM_COL32(150, 200, 255, 255);
        
        ImU32 disableColor = node.disabled ? disableColorActive : (hoverDisableBar ? disableColorBase : headerColor);
        ImU32 templateColor = node.isTemplate ? templateColorActive : (hoverTemplateBar ? templateColorBase : headerColor);
        ImU32 flagColor = node.isCurrentFlag ? flagColorActive : (hoverFlagBar ? flagColorBase : headerColor);
        
        drawList->AddRectFilled(
            ImVec2(rightSideX - separatorWidth, nodePos.y),
            ImVec2(rightSideX, nodePos.y + buttonHeight),
            accentColor
        );
        
        drawList->AddRectFilled(
            disableBarMin,
            disableBarMax,
            disableColor
        );
        
        drawList->AddRectFilled(
            ImVec2(rightSideX + buttonWidth, nodePos.y),
            ImVec2(rightSideX + buttonWidth + separatorWidth, nodePos.y + buttonHeight),
            accentColor
        );
        
        drawList->AddRectFilled(
            templateBarMin,
            templateBarMax,
            templateColor
        );
        
        drawList->AddRectFilled(
            ImVec2(rightSideX + 2 * buttonWidth + separatorWidth, nodePos.y),
            ImVec2(rightSideX + 2 * buttonWidth + 2 * separatorWidth, nodePos.y + buttonHeight),
            accentColor
        );
        
        drawList->AddRectFilled(
            flagBarMin,
            flagBarMax,
            flagColor,
            cornerRadius, ImDrawFlags_RoundCornersTopRight
        );
        
        float accentLineY = nodePos.y + nodeSize.y / 2.0f - accentLineHeight / 2.0f;
        drawList->AddRectFilled(
            ImVec2(nodePos.x, accentLineY),
            ImVec2(nodePos.x + nodeSize.x, accentLineY + accentLineHeight),
            accentColor
        );
        
        float cornerPinRadius = 3.0f * m_state.viewScale;
        float cornerPinX = nodePos.x + nodeSize.x - cornerPinRadius - 3.0f;
        float cornerPinY = nodePos.y + nodeSize.y - cornerPinRadius - 3.0f;
        
        ImVec4 accentVec4 = ImGui::ColorConvertU32ToFloat4(accentColor);
        accentVec4.x *= 0.8f;
        accentVec4.y *= 0.8f;
        accentVec4.z *= 0.8f;
        ImU32 cornerPinColor = ImGui::ColorConvertFloat4ToU32(accentVec4);
        
        drawList->AddCircleFilled(
            ImVec2(cornerPinX, cornerPinY),
            cornerPinRadius,
            cornerPinColor
        );
        
        drawList->AddCircle(
            ImVec2(cornerPinX, cornerPinY),
            cornerPinRadius,
            IM_COL32(255, 255, 255, 100),
            0,
            1.0f
        );
        
        float highlightThickness = 1.0f;
        ImU32 highlightColor = IM_COL32(255, 255, 255, 30);
        drawList->AddLine(
            ImVec2(nodePos.x + cornerRadius, nodePos.y + highlightThickness/2.0f),
            ImVec2(nodePos.x + nodeSize.x - cornerRadius, nodePos.y + highlightThickness/2.0f),
            highlightColor, 
            highlightThickness
        );
        
        if (node.labelPosition != NodeLabelPosition::None) {
            float textPosY = nodePos.y + nodeSize.y * 0.5f - ImGui::GetFontSize() * 0.5f;
            float textPosX;
            
            if (node.labelPosition == NodeLabelPosition::Right) {
                textPosX = nodePos.x + nodeSize.x + 5.0f * m_state.viewScale;
            } else { 
                float textWidth = ImGui::CalcTextSize(node.name.c_str()).x;
                textPosX = nodePos.x - textWidth - 5.0f * m_state.viewScale;
            }
            
            ImU32 textColor = IM_COL32(
                180, 180, 185, node.disabled ? 128 : 220
            );
            
            drawList->AddText(
                ImVec2(textPosX, textPosY),
                textColor,
                node.name.c_str()
            );
        }
        
        if (!node.iconSymbol.empty()) {
            float iconCenterX = nodePos.x + 8.0f * m_state.viewScale;
            float iconCenterY = nodePos.y + 8.0f * m_state.viewScale;
            
            float iconCircleRadius = 5.5f * m_state.viewScale;
            float iconInnerRadius = 5.0f * m_state.viewScale;
            float iconHighlightRadius = 3.0f * m_state.viewScale;
            
            ImVec2 iconTextSize = ImGui::CalcTextSize(node.iconSymbol.c_str());
            float textScaleFactor = m_state.viewScale;
            
            drawList->AddCircleFilled(
                ImVec2(iconCenterX + 0.8f * m_state.viewScale, iconCenterY + 0.8f * m_state.viewScale),
                iconCircleRadius,
                IM_COL32(15, 18, 25, node.disabled ? 100 : 180)
            );
            
            drawList->AddCircleFilled(
                ImVec2(iconCenterX, iconCenterY),
                iconInnerRadius,
                IM_COL32(25, 30, 40, node.disabled ? 128 : 255)
            );
            
            drawList->AddCircle(
                ImVec2(iconCenterX - 1.0f * m_state.viewScale, iconCenterY - 1.0f * m_state.viewScale),
                iconHighlightRadius,
                IM_COL32(100, 120, 140, node.disabled ? 60 : 90),
                0,
                1.0f
            );
            
            drawList->AddText(
                ImVec2(iconCenterX - iconTextSize.x * 0.5f + 0.8f * m_state.viewScale, 
                       iconCenterY - iconTextSize.y * 0.5f + 0.8f * m_state.viewScale),
                IM_COL32(0, 0, 0, node.disabled ? 80 : 160),
                node.iconSymbol.c_str()
            );
            
            drawList->AddText(
                ImVec2(iconCenterX - iconTextSize.x * 0.5f, 
                       iconCenterY - iconTextSize.y * 0.5f),
                node.disabled ? IM_COL32(100, 100, 110, 128) : accentColor,
                node.iconSymbol.c_str()
            );
        }
        
        drawNodePins(drawList, node, nodePos, nodeSize, canvasPos);
    }
}

void NodeEditor::drawNodePins(ImDrawList* drawList, const Node& node, const ImVec2& nodePos, const ImVec2& nodeSize, const ImVec2& canvasPos) {
    float pinRadius = m_state.style.pinRadius * m_state.viewScale;
    
    for (size_t i = 0; i < node.inputs.size(); ++i) {
        const auto& pin = node.inputs[i];
        ImVec2 pinPos = getPinPos(node, pin, canvasPos);
        
        std::string pinTypeName = pinTypeToString(pin.type);
        const internal::PinColors& pinColors = m_state.style.pinColors.count(pinTypeName) ?
                                   m_state.style.pinColors.at(pinTypeName) :
                                   m_state.style.pinColors.at("Default");
        
        ImU32 pinColor = IM_COL32(
            pinColors.color.r * 255,
            pinColors.color.g * 255,
            pinColors.color.b * 255,
            pinColors.color.a * 255
        );
        
        ImU32 pinOutlineColor = IM_COL32(80, 80, 90, 180);
        float pinOutlineThickness = 1.0f;
        
        ImVec2 mousePos = ImGui::GetMousePos();
        float dx = mousePos.x - pinPos.x;
        float dy = mousePos.y - pinPos.y;
        bool pinHovered = (dx*dx + dy*dy) <= (pinRadius * pinRadius * 2.0f);
        
        if (pin.connected) {
            pinColor = IM_COL32(
                pinColors.connectedColor.r * 255,
                pinColors.connectedColor.g * 255,
                pinColors.connectedColor.b * 255,
                pinColors.connectedColor.a * 255
            );
            pinOutlineColor = IM_COL32(255, 255, 255, 100);
            pinOutlineThickness = 1.5f;
        }
        else if (pinHovered) {
            pinColor = IM_COL32(
                pinColors.hoverColor.r * 255,
                pinColors.hoverColor.g * 255,
                pinColors.hoverColor.b * 255,
                pinColors.hoverColor.a * 255
            );
        }
        
        if (node.disabled) {
            ImVec4 colorVec = ImGui::ColorConvertU32ToFloat4(pinColor);
            colorVec.w *= 0.5f;
            pinColor = ImGui::ColorConvertFloat4ToU32(colorVec);
            
            ImVec4 outlineVec = ImGui::ColorConvertU32ToFloat4(pinOutlineColor);
            outlineVec.w *= 0.5f;
            pinOutlineColor = ImGui::ColorConvertFloat4ToU32(outlineVec);
        }
        
        drawPinShape(drawList, pinPos, pinRadius, pin.shape, pinColor, pinOutlineColor, pinOutlineThickness, pinHovered);
    }
    
    for (size_t i = 0; i < node.outputs.size(); ++i) {
        const auto& pin = node.outputs[i];
        ImVec2 pinPos = getPinPos(node, pin, canvasPos);
        
        std::string pinTypeName = pinTypeToString(pin.type);
        const internal::PinColors& pinColors = m_state.style.pinColors.count(pinTypeName) ?
                                   m_state.style.pinColors.at(pinTypeName) :
                                   m_state.style.pinColors.at("Default");
        
        ImU32 pinColor = IM_COL32(
            pinColors.color.r * 255,
            pinColors.color.g * 255,
            pinColors.color.b * 255,
            pinColors.color.a * 255
        );
        
        ImU32 pinOutlineColor = IM_COL32(80, 80, 90, 180);
        float pinOutlineThickness = 1.0f;
        
        ImVec2 mousePos = ImGui::GetMousePos();
        float dx = mousePos.x - pinPos.x;
        float dy = mousePos.y - pinPos.y;
        bool pinHovered = (dx*dx + dy*dy) <= (pinRadius * pinRadius * 2.0f);
        
        if (pin.connected) {
            pinColor = IM_COL32(
                pinColors.connectedColor.r * 255,
                pinColors.connectedColor.g * 255,
                pinColors.connectedColor.b * 255,
                pinColors.connectedColor.a * 255
            );
            pinOutlineColor = IM_COL32(255, 255, 255, 100);
            pinOutlineThickness = 1.5f;
        }
        else if (pinHovered) {
            pinColor = IM_COL32(
                pinColors.hoverColor.r * 255,
                pinColors.hoverColor.g * 255,
                pinColors.hoverColor.b * 255,
                pinColors.hoverColor.a * 255
            );
        }
        
        if (node.disabled) {
            ImVec4 colorVec = ImGui::ColorConvertU32ToFloat4(pinColor);
            colorVec.w *= 0.5f;
            pinColor = ImGui::ColorConvertFloat4ToU32(colorVec);
            
            ImVec4 outlineVec = ImGui::ColorConvertU32ToFloat4(pinOutlineColor);
            outlineVec.w *= 0.5f;
            pinOutlineColor = ImGui::ColorConvertFloat4ToU32(outlineVec);
        }
        
        drawPinShape(drawList, pinPos, pinRadius, pin.shape, pinColor, pinOutlineColor, pinOutlineThickness, pinHovered);
    }
}

void NodeEditor::drawPinShape(ImDrawList* drawList, const ImVec2& center, float radius, PinShape shape, ImU32 fillColor, ImU32 borderColor, float borderThickness, bool isHovered) {
    if (isHovered) {
        ImU32 hoverEffectColor = IM_COL32(255, 255, 255, 100);
        
        float hoverRadius = radius * 1.6f;
        
        switch (shape) {
            case PinShape::Square:
                drawList->AddRectFilled(
                    ImVec2(center.x - hoverRadius, center.y - hoverRadius),
                    ImVec2(center.x + hoverRadius, center.y + hoverRadius),
                    hoverEffectColor
                );
                break;
                
            case PinShape::Triangle:
                {
                    ImVec2 p1(center.x, center.y - hoverRadius);
                    ImVec2 p2(center.x - hoverRadius, center.y + hoverRadius);
                    ImVec2 p3(center.x + hoverRadius, center.y + hoverRadius);
                    
                    drawList->AddTriangleFilled(p1, p2, p3, hoverEffectColor);
                }
                break;
                
            case PinShape::Diamond:
                {
                    ImVec2 p1(center.x, center.y - hoverRadius);
                    ImVec2 p2(center.x - hoverRadius, center.y);
                    ImVec2 p3(center.x, center.y + hoverRadius);
                    ImVec2 p4(center.x + hoverRadius, center.y);
                    
                    drawList->AddQuadFilled(p1, p2, p3, p4, hoverEffectColor);
                }
                break;
                
            default: 
                drawList->AddCircleFilled(center, hoverRadius, hoverEffectColor);
                break;
        }
    }
    
    switch (shape) {
        case PinShape::Square:
            drawList->AddRectFilled(
                ImVec2(center.x - radius, center.y - radius),
                ImVec2(center.x + radius, center.y + radius),
                fillColor
            );
            drawList->AddRect(
                ImVec2(center.x - radius, center.y - radius),
                ImVec2(center.x + radius, center.y + radius),
                borderColor, 0, 0, borderThickness
            );
            break;
            
        case PinShape::Triangle:
            {
                ImVec2 p1(center.x, center.y - radius);
                ImVec2 p2(center.x - radius, center.y + radius);
                ImVec2 p3(center.x + radius, center.y + radius);
                
                drawList->AddTriangleFilled(p1, p2, p3, fillColor);
                drawList->AddTriangle(p1, p2, p3, borderColor, borderThickness);
            }
            break;
            
        case PinShape::Diamond:
            {
                ImVec2 p1(center.x, center.y - radius);
                ImVec2 p2(center.x - radius, center.y);
                ImVec2 p3(center.x, center.y + radius);
                ImVec2 p4(center.x + radius, center.y);
                
                drawList->AddQuadFilled(p1, p2, p3, p4, fillColor);
                drawList->AddQuad(p1, p2, p3, p4, borderColor, borderThickness);
            }
            break;
            
        default:
            drawList->AddCircleFilled(center, radius, fillColor);
            drawList->AddCircle(center, radius, borderColor, 0, borderThickness);
            break;
    }
}

void NodeEditor::drawGroups(ImDrawList* drawList, const ImVec2& canvasPos) {
    for (const auto& group : m_state.groups) {
        ImVec2 groupPos = canvasToScreen(group.position).toImVec2();
        ImVec2 groupSize = Vec2(group.size.x * m_state.viewScale, group.size.y * m_state.viewScale).toImVec2();
        
        float headerHeight = 30.0f * m_state.viewScale;
        
        ImU32 fillColor = IM_COL32(group.color.r * 255, group.color.g * 255, group.color.b * 255, group.color.a * 255);
        ImU32 borderColor = group.selected ? 
                         IM_COL32(255, 255, 255, 100) : 
                         IM_COL32(group.color.r * 255 * 0.8f, group.color.g * 255 * 0.8f, group.color.b * 255 * 0.8f, 100);
        
        ImU32 headerColor = IM_COL32(
            group.color.r * 255 * 1.2f,
            group.color.g * 255 * 1.2f,
            group.color.b * 255 * 1.2f,
            group.color.a * 255
        );
        
        drawList->AddRectFilled(
            groupPos,
            ImVec2(groupPos.x + groupSize.x, groupPos.y + groupSize.y),
            fillColor,
            4.0f
        );
        
        drawList->AddRectFilled(
            groupPos,
            ImVec2(groupPos.x + groupSize.x, groupPos.y + headerHeight),
            headerColor,
            4.0f,
            ImDrawFlags_RoundCornersTop
        );
        
        drawList->AddRect(
            groupPos,
            ImVec2(groupPos.x + groupSize.x, groupPos.y + groupSize.y),
            borderColor,
            4.0f,
            0,
            1.5f
        );
        
        ImVec2 textSize = ImGui::CalcTextSize(group.name.c_str());
        float textPosX = groupPos.x + 10.0f;
        float textPosY = groupPos.y + (headerHeight - textSize.y) * 0.5f;
        
        drawList->AddText(
            ImVec2(textPosX, textPosY),
            IM_COL32(255, 255, 255, 220),
            group.name.c_str()
        );
    }
}

void NodeEditor::drawBoxSelection(ImDrawList* drawList) {
    ImVec2 mousePos = ImGui::GetMousePos();
    
    ImVec2 boxMin = ImVec2(
        std::min(m_state.boxSelectStart.x, mousePos.x),
        std::min(m_state.boxSelectStart.y, mousePos.y)
    );
    
    ImVec2 boxMax = ImVec2(
        std::max(m_state.boxSelectStart.x, mousePos.x),
        std::max(m_state.boxSelectStart.y, mousePos.y)
    );
    
    drawList->AddRectFilled(
        boxMin, boxMax,
        m_state.style.uiColors.selection.toImU32()
    );
    
    drawList->AddRect(
        boxMin, boxMax,
        IM_COL32(m_state.style.uiColors.selection.r * 255 * 1.5f,
                m_state.style.uiColors.selection.g * 255 * 1.5f,
                m_state.style.uiColors.selection.b * 255 * 1.5f,
                200),
        0.0f, 0, 1.0f
    );
}

ImVec2 NodeEditor::getPinPos(const Node& node, const Pin& pin, const ImVec2& canvasPos) const {
    ImVec2 nodePos = canvasToScreen(node.position).toImVec2();
    ImVec2 nodeSize = Vec2(node.size.x * m_state.viewScale, node.size.y * m_state.viewScale).toImVec2();
    
    float pinSpacing = 25.0f * m_state.viewScale;
    float leftMargin = 20.0f * m_state.viewScale;
    
    if (pin.isInput) {
        int pinIndex = -1;
        
        for (size_t i = 0; i < node.inputs.size(); ++i) {
            if (node.inputs[i].id == pin.id) {
                pinIndex = static_cast<int>(i);
                break;
            }
        }
        
        if (pinIndex < 0) return ImVec2(0, 0);
        
        float pinX = nodePos.x + leftMargin + pinIndex * pinSpacing;
        float pinY = nodePos.y;
        
        return ImVec2(pinX, pinY);
    } else {
        int pinIndex = -1;
        
        for (size_t i = 0; i < node.outputs.size(); ++i) {
            if (node.outputs[i].id == pin.id) {
                pinIndex = static_cast<int>(i);
                break;
            }
        }
        
        if (pinIndex < 0) return ImVec2(0, 0);
        
        float pinX = nodePos.x + leftMargin + pinIndex * pinSpacing;
        float pinY = nodePos.y + nodeSize.y;
        
        return ImVec2(pinX, pinY);
    }
}

bool NodeEditor::isPinHovered(const Node& node, const Pin& pin, const ImVec2& canvasPos) {
    ImVec2 pinPos = getPinPos(node, pin, canvasPos);
    ImVec2 mousePos = ImGui::GetMousePos();
    float pinRadius = m_state.style.pinRadius * m_state.viewScale;
    
    float dx = mousePos.x - pinPos.x;
    float dy = mousePos.y - pinPos.y;
    return (dx * dx + dy * dy) <= (pinRadius * pinRadius * 4.0f);
}

bool NodeEditor::isConnectionHovered(const Connection& connection, const ImVec2& canvasPos) {
    const Node* startNode = getNode(connection.startNodeId);
    const Node* endNode = getNode(connection.endNodeId);
    const Pin* startPin = getPin(connection.startNodeId, connection.startPinId);
    const Pin* endPin = getPin(connection.endNodeId, connection.endPinId);
    
    if (!startNode || !endNode || !startPin || !endPin) return false;
    
    ImVec2 p1 = getPinPos(*startNode, *startPin, canvasPos);
    ImVec2 p2 = getPinPos(*endNode, *endPin, canvasPos);
    ImVec2 mousePos = ImGui::GetMousePos();
    
    float distance = std::abs(p2.y - p1.y);
    float cpOffset = std::max(50.0f, distance * 0.5f);
    
    ImVec2 cp1 = ImVec2(p1.x, p1.y + cpOffset);
    ImVec2 cp2 = ImVec2(p2.x, p2.y - cpOffset);
    
    return isPointNearCubicBezier(mousePos, p1, cp1, cp2, p2, 5.0f);
}

bool NodeEditor::doesConnectionExist(int startNodeId, int startPinId, int endNodeId, int endPinId) const {
    return std::any_of(m_state.connections.begin(), m_state.connections.end(),
                     [=](const Connection& conn) {
                         return conn.startNodeId == startNodeId &&
                                conn.startPinId == startPinId &&
                                conn.endNodeId == endNodeId &&
                                conn.endPinId == endPinId;
                     });
}

bool NodeEditor::canCreateConnection(const Pin& startPin, const Pin& endPin) const {
    if (startPin.isInput == endPin.isInput) {
        return false;
    }
    
    if (m_state.canConnectCallback) {
        const Pin& outputPin = startPin.isInput ? endPin : startPin;
        const Pin& inputPin = startPin.isInput ? startPin : endPin;
        return m_state.canConnectCallback(outputPin, inputPin);
    }
    
    return startPin.type == endPin.type || 
           startPin.type == PinType::Flow || 
           endPin.type == PinType::Flow;
}

void NodeEditor::createConnection(int startNodeId, int startPinId, int endNodeId, int endPinId) {
    const Pin* startPin = getPin(startNodeId, startPinId);
    const Pin* endPin = getPin(endNodeId, endPinId);
    
    if (!startPin || !endPin) return;
    
    if (startPin->isInput) {
        std::swap(startNodeId, endNodeId);
        std::swap(startPinId, endPinId);
    }
    
    addConnection(startNodeId, startPinId, endNodeId, endPinId);
}

bool NodeEditor::isPointInRect(const ImVec2& point, const ImVec2& rectMin, const ImVec2& rectMax) const {
    return point.x >= rectMin.x && point.x <= rectMax.x && 
           point.y >= rectMin.y && point.y <= rectMax.y;
}

bool NodeEditor::isPointNearLine(const ImVec2& point, const ImVec2& lineStart, const ImVec2& lineEnd, float threshold) const {
    float length2 = (lineEnd.x - lineStart.x) * (lineEnd.x - lineStart.x) +
                  (lineEnd.y - lineStart.y) * (lineEnd.y - lineStart.y);
    
    if (length2 <= 0.0001f) {
        float dx = point.x - lineStart.x;
        float dy = point.y - lineStart.y;
        return (dx * dx + dy * dy) <= (threshold * threshold);
    }
    
    float t = ((point.x - lineStart.x) * (lineEnd.x - lineStart.x) +
              (point.y - lineStart.y) * (lineEnd.y - lineStart.y)) / length2;
    
    t = std::max(0.0f, std::min(1.0f, t));
    
    float projX = lineStart.x + t * (lineEnd.x - lineStart.x);
    float projY = lineStart.y + t * (lineEnd.y - lineStart.y);
    
    float dx = point.x - projX;
    float dy = point.y - projY;
    
    return (dx * dx + dy * dy) <= (threshold * threshold);
}

bool NodeEditor::isPointNearCubicBezier(const ImVec2& point, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, float threshold) const {
    const int steps = 10;
    ImVec2 prev = p1;
    
    for (int i = 1; i <= steps; ++i) {
        float t = i / (float)steps;
        float u = 1.0f - t;
        
        float w1 = u * u * u;
        float w2 = 3 * u * u * t;
        float w3 = 3 * u * t * t;
        float w4 = t * t * t;
        
        ImVec2 current(
            w1 * p1.x + w2 * p2.x + w3 * p3.x + w4 * p4.x,
            w1 * p1.y + w2 * p2.y + w3 * p3.y + w4 * p4.y
        );
        
        if (isPointNearLine(point, prev, current, threshold)) {
            return true;
        }
        
        prev = current;
    }
    
    return false;
}

std::string NodeEditor::pinTypeToString(PinType type) const {
    switch (type) {
        case PinType::Flow:    return "Flow";
        case PinType::Bool:    return "Bool";
        case PinType::Int:     return "Int";
        case PinType::Float:   return "Float";
        case PinType::String:  return "String";
        case PinType::Vec2:    return "Vec2";
        case PinType::Vec3:    return "Vec3";
        case PinType::Vec4:    return "Vec4";
        case PinType::Color:   return "Color";
        case PinType::Object:  return "Object";
        case PinType::Custom:  return "Custom";
        default:               return "Default";
    }
}

ImU32 NodeEditor::ImLerpColor(ImU32 col_a, ImU32 col_b, float t) {
    ImVec4 a = ImGui::ColorConvertU32ToFloat4(col_a);
    ImVec4 b = ImGui::ColorConvertU32ToFloat4(col_b);
    
    return ImGui::ColorConvertFloat4ToU32(ImVec4(
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t,
        a.w + (b.w - a.w) * t
    ));
}

ImVec2 NodeEditor::ImBezierCubicCalc(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, float t) {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    ImVec2 result = ImVec2(
        uuu * p1.x + 3 * uu * t * p2.x + 3 * u * tt * p3.x + ttt * p4.x,
        uuu * p1.y + 3 * uu * t * p2.y + 3 * u * tt * p3.y + ttt * p4.y
    );
    return result;
}