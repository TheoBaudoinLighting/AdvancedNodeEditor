#include "../../NodeEditor.h"
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
            
            for (const auto& pinInternal : node.inputs) {
                ANE::Pin apiPin;
                apiPin.id = pinInternal.id;
                apiPin.name = pinInternal.name;
                apiPin.isInput = pinInternal.isInput;
                apiPin.type = static_cast<ANE::PinType>(pinInternal.type);
                apiPin.shape = static_cast<ANE::PinShape>(pinInternal.shape);

                if (isPinHovered(node, apiPin, ImGui::GetWindowPos())) {
                    m_state.hoveredNodeId = node.id;
                    m_state.hoveredPinId = pinInternal.id;

                    if (!ImGui::GetIO().KeyCtrl) {
                        deselectAllNodes();
                    }

                    break;
                }
            }

            for (const auto& pinInternal : node.outputs) {
                ANE::Pin apiPin;
                apiPin.id = pinInternal.id;
                apiPin.name = pinInternal.name;
                apiPin.isInput = pinInternal.isInput;
                apiPin.type = static_cast<ANE::PinType>(pinInternal.type);
                apiPin.shape = static_cast<ANE::PinShape>(pinInternal.shape);

                if (isPinHovered(node, apiPin, ImGui::GetWindowPos())) {
                    m_state.hoveredNodeId = node.id;
                    m_state.hoveredPinId = pinInternal.id;

                    m_state.connecting = true;
                    m_state.connectingNodeId = node.id;
                    m_state.connectingPinId = pinInternal.id;

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

void NodeEditor::processConnectionCreation() {
    if (!m_state.connecting || m_state.connectingNodeId == -1 || m_state.connectingPinId == -1) return;

    ImVec2 mousePos = ImGui::GetMousePos();

    for (const auto& node : m_state.nodes) {
        if (node.id == m_state.connectingNodeId) continue;

        for (const auto& pinInternal : node.inputs) {
            ANE::Pin apiPin;
            apiPin.id = pinInternal.id;
            apiPin.name = pinInternal.name;
            apiPin.isInput = pinInternal.isInput;
            apiPin.type = static_cast<ANE::PinType>(pinInternal.type);
            apiPin.shape = static_cast<ANE::PinShape>(pinInternal.shape);

            if (isPinHovered(node, apiPin, ImGui::GetWindowPos())) {
                const Node* sourceNode = getNode(m_state.connectingNodeId);
                if (!sourceNode) return;

                const Pin* sourcePinInternal = nullptr;
                for (const auto& p : sourceNode->outputs) {
                    if (p.id == m_state.connectingPinId) {
                        sourcePinInternal = &p;
                        break;
                    }
                }

                if (!sourcePinInternal) return;

                ANE::Pin sourceApiPin;
                sourceApiPin.id = sourcePinInternal->id;
                sourceApiPin.name = sourcePinInternal->name;
                sourceApiPin.isInput = sourcePinInternal->isInput;
                sourceApiPin.type = static_cast<ANE::PinType>(sourcePinInternal->type);
                sourceApiPin.shape = static_cast<ANE::PinShape>(sourcePinInternal->shape);

                if (canCreateConnection(sourceApiPin, apiPin)) {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

                    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                        createConnection(m_state.connectingNodeId, m_state.connectingPinId, node.id, pinInternal.id);
                        m_state.connecting = false;
                        m_state.connectingNodeId = -1;
                        m_state.connectingPinId = -1;
                    }

                    return;
                }
            }
        }

        for (const auto& pinInternal : node.outputs) {
            ANE::Pin apiPin;
            apiPin.id = pinInternal.id;
            apiPin.name = pinInternal.name;
            apiPin.isInput = pinInternal.isInput;
            apiPin.type = static_cast<ANE::PinType>(pinInternal.type);
            apiPin.shape = static_cast<ANE::PinShape>(pinInternal.shape);

            if (isPinHovered(node, apiPin, ImGui::GetWindowPos())) {
                const Node* targetNode = getNode(m_state.connectingNodeId);
                if (!targetNode) return;

                const Pin* targetPinInternal = nullptr;
                for (const auto& p : targetNode->inputs) {
                    if (p.id == m_state.connectingPinId) {
                        targetPinInternal = &p;
                        break;
                    }
                }

                if (!targetPinInternal) return;

                ANE::Pin targetApiPin;
                targetApiPin.id = targetPinInternal->id;
                targetApiPin.name = targetPinInternal->name;
                targetApiPin.isInput = targetPinInternal->isInput;
                targetApiPin.type = static_cast<ANE::PinType>(targetPinInternal->type);
                targetApiPin.shape = static_cast<ANE::PinShape>(targetPinInternal->shape);

                if (canCreateConnection(apiPin, targetApiPin)) {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

                    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                        createConnection(node.id, pinInternal.id, m_state.connectingNodeId, m_state.connectingPinId);
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

ImVec2 NodeEditor::getPinPos(const Node& node, const ANE::Pin& pin, const ImVec2& canvasPos) const {
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

bool NodeEditor::isPinHovered(const Node& node, const ANE::Pin& pin, const ImVec2& canvasPos) {
    ImVec2 pinPos = getPinPos(node, pin, canvasPos);
    ImVec2 mousePos = ImGui::GetMousePos();
    float pinRadius = m_state.style.pinRadius * m_state.viewScale;

    float dx = mousePos.x - pinPos.x;
    float dy = mousePos.y - pinPos.y;
    return (dx*dx + dy*dy) <= (pinRadius * pinRadius * 4.0f);
}

bool NodeEditor::isConnectionHovered(const Connection& connection, const ImVec2& canvasPos) {
    const Node* startNode = getNode(connection.startNodeId);
    const Node* endNode = getNode(connection.endNodeId);

    if (!startNode || !endNode) return false;

    const ANE::Pin* apiStartPin = getPin(connection.startNodeId, connection.startPinId);
    const ANE::Pin* apiEndPin = getPin(connection.endNodeId, connection.endPinId);

    if (!apiStartPin || !apiEndPin) return false;

    ImVec2 p1 = getPinPos(*startNode, *apiStartPin, canvasPos);
    ImVec2 p2 = getPinPos(*endNode, *apiEndPin, canvasPos);
    ImVec2 mousePos = ImGui::GetMousePos();

    float distance = std::abs(p2.y - p1.y);
    float cpOffset = std::max(50.0f, distance * 0.5f);

    ImVec2 cp1 = ImVec2(p1.x, p1.y + cpOffset);
    ImVec2 cp2 = ImVec2(p2.x, p2.y - cpOffset);

    return isPointNearCubicBezier(mousePos, p1, cp1, cp2, p2, 5.0f);
}

}