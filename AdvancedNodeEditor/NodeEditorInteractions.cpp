#include "NodeEditor.h"
#include <algorithm>
#include <cfloat>

namespace NodeEditorCore {

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
    
    Vec2 delta = newCanvasPos - nodeIt->position;
    nodeIt->position = newCanvasPos;
    
    if (!ImGui::GetIO().KeyCtrl) {
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

}