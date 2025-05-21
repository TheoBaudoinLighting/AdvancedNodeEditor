#include "../../Core/NodeEditor.h"
#include "../../Core/Style/InteractionMode.h"
#include <algorithm>
#include <cfloat>

namespace NodeEditorCore {
    void NodeEditor::processInteraction() {
        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();

        ImDrawList *drawList = ImGui::GetWindowDrawList();
        drawList->AddText(ImVec2(10, 10), IM_COL32(255, 0, 0, 255),
                          ("Interaction Mode: " + std::to_string(static_cast<int>(m_state.interactionMode))).c_str());
        drawList->AddText(ImVec2(10, 30), IM_COL32(255, 0, 0, 255),
                          ("Mouse Position: " + std::to_string(static_cast<int>(mousePos.x)) + ", " +
                           std::to_string(static_cast<int>(mousePos.y))).c_str());
        drawList->AddText(ImVec2(10, 50), IM_COL32(255, 0, 0, 255),
                          ("Hovered Pin: " + std::to_string(m_state.hoveredPinId)).c_str());
        drawList->AddText(ImVec2(10, 70), IM_COL32(255, 0, 0, 255),
                          ("Connecting: " + std::string(m_state.connecting ? "true" : "false")).c_str());
        drawList->AddText(ImVec2(10, 90), IM_COL32(255, 0, 0, 255),
                          ("Magnet Pin: " + std::to_string(m_state.magnetPinId)).c_str());

        bool isMouseDoubleClicked = ImGui::IsMouseDoubleClicked(0);
        bool isMouseClicked = ImGui::IsMouseClicked(0);
        bool isMouseReleased = ImGui::IsMouseReleased(0);
        bool isMouseDragging = ImGui::IsMouseDragging(0);
        bool isMiddleMousePressed = ImGui::IsMouseDown(2);

        drawList->AddText(ImVec2(10, 110), IM_COL32(255, 0, 0, 255),
                          ("Mouse Clicked: " + std::string(isMouseClicked ? "true" : "false")).c_str());
        drawList->AddText(ImVec2(10, 130), IM_COL32(255, 0, 0, 255),
                          ("Mouse Released: " + std::string(isMouseReleased ? "true" : "false")).c_str());
        drawList->AddText(ImVec2(10, 150), IM_COL32(255, 0, 0, 255),
                          ("Mouse Dragging: " + std::string(isMouseDragging ? "true" : "false")).c_str());

        updateHoveredElements(mousePos);

        drawList->AddText(ImVec2(10, 170), IM_COL32(255, 0, 0, 255),
                          ("Hovered Node: " + std::to_string(m_state.hoveredNodeId)).c_str());
        drawList->AddText(ImVec2(10, 190), IM_COL32(255, 0, 0, 255),
                          ("Hovered Pin: " + std::to_string(m_state.hoveredPinId)).c_str());

        if (isMouseDoubleClicked && m_state.hoveredNodeId >= 0) {
            Node *node = getNode(m_state.hoveredNodeId);
            if (node && node->isSubgraph) {
                enterSubgraph(node->subgraphId);
                return;
            } else if (node && m_state.currentSubgraphId >= 0) {
                Subgraph *subgraph = getSubgraph(m_state.currentSubgraphId);
                if (subgraph) {
                    int inputNodeId = subgraph->metadata.getAttribute<int>("inputNodeId", -1);
                    int outputNodeId = subgraph->metadata.getAttribute<int>("outputNodeId", -1);
                    if (node->id == inputNodeId || node->id == outputNodeId) {
                        exitSubgraph();
                        return;
                    }
                }
            }
        }

        if (isMiddleMousePressed) {
            if (!m_state.dragging) {
                m_state.dragging = true;
                m_state.dragOffset = Vec2(mousePos.x, mousePos.y);
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
            } else {
                float dx = mousePos.x - m_state.dragOffset.x;
                float dy = mousePos.y - m_state.dragOffset.y;
                m_state.viewPosition.x += dx;
                m_state.viewPosition.y += dy;
                m_state.dragOffset = Vec2(mousePos.x, mousePos.y);
            }
            return;
        } else if (m_state.dragging && !isMiddleMousePressed) {
            m_state.dragging = false;
            ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
        }

        if (isMouseClicked) {
            drawList->AddText(ImVec2(10, 210), IM_COL32(255, 0, 0, 255), "MOUSE CLICKED!");

            if (m_state.hoveredPinId >= 0 && m_state.hoveredNodeId >= 0) {
                drawList->AddText(ImVec2(10, 230), IM_COL32(255, 0, 0, 255),
                                  ("Starting Connection on Pin " + std::to_string(m_state.hoveredPinId)).c_str());

                startConnectionDrag(m_state.hoveredNodeId, m_state.hoveredPinId);
            } else if (m_state.hoveredNodeId >= 0) {
                Node *node = getNode(m_state.hoveredNodeId);
                if (node) {
                    bool isAlreadySelected = node->selected;
                    bool ctrlPressed = ImGui::GetIO().KeyCtrl;

                    if (!ctrlPressed) {
                        deselectAllConnections();
                    }

                    if (ctrlPressed) {
                        if (isAlreadySelected) {
                            deselectNode(m_state.hoveredNodeId);
                        } else {
                            selectNode(m_state.hoveredNodeId, true);
                        }
                    } else {
                        if (!isAlreadySelected) {
                            selectNode(m_state.hoveredNodeId, false);
                        }
                    }

                    if (node->selected) {
                        startNodeDrag(m_state.hoveredNodeId, mousePos);
                    }
                }
            } else if (m_state.hoveredConnectionId >= 0) {
                bool ctrlPressed = ImGui::GetIO().KeyCtrl;
                bool altPressed = ImGui::GetIO().KeyAlt;

                if (altPressed) {
                    removeConnection(m_state.hoveredConnectionId);
                } else {
                    if (!ctrlPressed) {
                        deselectAllNodes();
                    }

                    Connection *connection = getConnection(m_state.hoveredConnectionId);
                    if (connection) {
                        if (ctrlPressed) {
                            if (connection->selected) {
                                deselectConnection(m_state.hoveredConnectionId);
                            } else {
                                selectConnection(m_state.hoveredConnectionId, true);
                            }
                        } else {
                            deselectAllConnections();
                            selectConnection(m_state.hoveredConnectionId, false);
                        }
                    }
                }
            } else if (m_state.hoveredGroupId >= 0) {
                startGroupInteraction(mousePos);
            } else {
                startBoxSelect(mousePos);
                if (!ImGui::GetIO().KeyCtrl) {
                    deselectAllNodes();
                    deselectAllConnections();
                }
            }
        }

        if (m_state.interactionMode != InteractionMode::None) {
            if (isMouseDragging) {
                drawList->AddText(ImVec2(10, 250), IM_COL32(255, 0, 0, 255),
                                  ("Dragging with Mode: " + std::to_string(static_cast<int>(m_state.interactionMode))).
                                  c_str());
                updateCurrentInteraction(mousePos);
            }

            if (m_state.interactionMode == InteractionMode::DragConnection) {
                processConnectionCreation();
            }

            if (isMouseReleased) {
                if (m_state.interactionMode == InteractionMode::DragConnection) {
                    if (m_state.magnetPinNodeId != -1 && m_state.magnetPinId != -1) {
                        const Node *sourceNode = getNode(m_state.connectingNodeId);
                        const Node *targetNode = getNode(m_state.magnetPinNodeId);

                        if (sourceNode && targetNode) {
                            const Pin *sourcePin = sourceNode->findPin(m_state.connectingPinId);
                            const Pin *targetPin = targetNode->findPin(m_state.magnetPinId);

                            if (sourcePin && targetPin) {
                                int connectionId = -1;

                                if (sourcePin->isInput) {
                                    connectionId = addConnection(m_state.magnetPinNodeId, m_state.magnetPinId,
                                                                 m_state.connectingNodeId, m_state.connectingPinId);
                                    drawList->AddText(ImVec2(10, 330), IM_COL32(255, 0, 0, 255),
                                                      ("Inverted connection: output to input"));
                                } else {
                                    connectionId = addConnection(m_state.connectingNodeId, m_state.connectingPinId,
                                                                 m_state.magnetPinNodeId, m_state.magnetPinId);
                                    drawList->AddText(ImVec2(10, 330), IM_COL32(255, 0, 0, 255),
                                                      ("Normal connection: output to input"));
                                }
                            }
                        }
                    }
                }

                endCurrentInteraction();
            }
        }

        processZoom(mousePos);

        if (ImGui::IsKeyPressed(ImGuiKey_Delete) || ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
            processDeleteKeyPress();
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            endCurrentInteraction();
        }
    }

    void NodeEditor::updateCurrentInteraction(const ImVec2 &mousePos) {
        switch (m_state.interactionMode) {
            case InteractionMode::DragNode:
                processNodeDragging();
                break;

            case InteractionMode::DragConnection:
                processConnectionCreation();
                break;

            case InteractionMode::BoxSelect:
                processBoxSelection(ImGui::GetWindowPos());
                break;

            case InteractionMode::DragGroup:
                processGroupDragging();
                break;

            case InteractionMode::ResizeGroup:
                processGroupResize();
                break;

            default:
                break;
        }
    }

    void NodeEditor::processNodeDragging() {
        if (m_state.interactionMode != InteractionMode::DragNode) return;

        ImVec2 mousePos = ImGui::GetMousePos();
        Vec2 mouseDelta = Vec2(mousePos.x - m_state.dragStart.x, mousePos.y - m_state.dragStart.y);
        Vec2 scaledDelta = Vec2(mouseDelta.x / m_state.viewScale, mouseDelta.y / m_state.viewScale);

        bool needsRefresh = false;
        for (auto &node: m_state.nodes) {
            if (node.selected && m_state.draggedNodePositions.find(node.id) == m_state.draggedNodePositions.end()) {
                needsRefresh = true;
                break;
            }
        }

        if (needsRefresh) {
            for (auto &node: m_state.nodes) {
                if (node.selected) {
                    m_state.draggedNodePositions[node.id] = node.position;
                }
            }
        }

        for (auto &node: m_state.nodes) {
            if (node.selected) {
                auto it = m_state.draggedNodePositions.find(node.id);
                if (it != m_state.draggedNodePositions.end()) {
                    node.position = it->second + scaledDelta;
                }
            }
        }
    }

    void NodeEditor::startConnectionDrag(int nodeId, int pinId) {
        m_state.interactionMode = InteractionMode::DragConnection;
        m_state.connectingNodeId = nodeId;
        m_state.connectingPinId = pinId;
        m_state.connecting = true;

        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }

    void NodeEditor::startNodeDrag(int nodeId, const ImVec2 &mousePos) {
        Node *node = getNode(nodeId);
        if (!node) return;

        m_state.interactionMode = InteractionMode::DragNode;
        m_state.activeNodeId = nodeId;
        m_state.activeNodeUuid = node->uuid;
        m_state.dragStart = Vec2(mousePos.x, mousePos.y);

        if (!node->selected) {
            selectNode(nodeId, ImGui::GetIO().KeyCtrl);
        }

        m_state.draggedNodePositions.clear();
        for (const auto &node: m_state.nodes) {
            if (node.selected) {
                m_state.draggedNodePositions[node.id] = node.position;
            }
        }
    }

    void NodeEditor::startGroupInteraction(const ImVec2 &mousePos) {
        Group *group = getGroup(m_state.hoveredGroupId);
        if (!group) return;

        ImVec2 groupPos = canvasToScreen(group->position).toImVec2();
        ImVec2 groupSize = Vec2(group->size.x * m_state.viewScale, group->size.y * m_state.viewScale).toImVec2();

        float titleHeight = 20.0f * m_state.viewScale;
        bool onTitle = mousePos.y <= groupPos.y + titleHeight;

        bool onResizeHandle = mousePos.x >= groupPos.x + groupSize.x - 10.0f &&
                              mousePos.y >= groupPos.y + groupSize.y - 10.0f;

        if (onResizeHandle) {
            m_state.interactionMode = InteractionMode::ResizeGroup;
            m_state.activeGroupId = group->id;
            m_state.dragStart = Vec2::fromImVec2(mousePos);
            m_state.groupStartSize = group->size;

            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNESW);
        } else if (onTitle) {
            m_state.interactionMode = InteractionMode::DragGroup;
            m_state.activeGroupId = group->id;
            m_state.dragOffset = Vec2(
                mousePos.x - groupPos.x,
                mousePos.y - groupPos.y
            );

            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
        }
    }

    void NodeEditor::startBoxSelect(const ImVec2 &mousePos) {
        m_state.interactionMode = InteractionMode::BoxSelect;
        m_state.boxSelectStart = Vec2::fromImVec2(mousePos);
        m_state.boxSelecting = true;

        if (!ImGui::GetIO().KeyCtrl) {
            deselectAllNodes();
        }
    }

    void NodeEditor::startPanCanvas() {
        m_state.interactionMode = InteractionMode::PanCanvas;

        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
    }

    void NodeEditor::endCurrentInteraction() {
        if (m_state.interactionMode == InteractionMode::DragConnection &&
            m_state.magnetPinNodeId != -1 && m_state.magnetPinId != -1) {
        }

        m_state.interactionMode = InteractionMode::None;
        m_state.activeNodeId = -1;
        m_state.activeNodeUuid = "";
        m_state.activeConnectionId = -1;
        m_state.activeConnectionUuid = "";
        m_state.activeGroupId = -1;
        m_state.activeGroupUuid = "";
        m_state.connectingNodeId = -1;
        m_state.connectingNodeUuid = "";
        m_state.connectingPinId = -1;
        m_state.connectingPinUuid = "";
        m_state.dragging = false;
        m_state.connecting = false;
        m_state.boxSelecting = false;

        ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
    }

    void NodeEditor::processDeleteKeyPress() {
        std::vector<int> connectionsToRemove;
        for (const auto &connection: m_state.connections) {
            if (connection.selected) {
                connectionsToRemove.push_back(connection.id);
            }
        }

        for (int id: connectionsToRemove) {
            removeConnection(id);
        }

        std::vector<int> nodesToRemove;
        for (const auto &node: m_state.nodes) {
            if (node.selected) {
                nodesToRemove.push_back(node.id);
            }
        }

        for (int id: nodesToRemove) {
            removeNode(id);
        }
    }

    void NodeEditor::updateHoveredElements(const ImVec2 &mousePos) {
        m_state.hoveredNodeId = -1;
        m_state.hoveredNodeUuid = "";
        m_state.hoveredPinId = -1;
        m_state.hoveredPinUuid = "";
        m_state.hoveredConnectionId = -1;
        m_state.hoveredConnectionUuid = "";
        m_state.hoveredGroupId = -1;
        m_state.hoveredGroupUuid = "";

        ImVec2 canvasPos = ImGui::GetCursorScreenPos();

        for (const auto &connection: m_state.connections) {
            if (!isNodeInCurrentSubgraph(*getNode(connection.startNodeId)) ||
                !isNodeInCurrentSubgraph(*getNode(connection.endNodeId))) {
                continue;
            }

            if (isConnectionHovered(connection, canvasPos)) {
                m_state.hoveredConnectionId = connection.id;
                m_state.hoveredConnectionUuid = connection.uuid;
                break;
            }
        }

        for (const auto &node: m_state.nodes) {
            if (!isNodeInCurrentSubgraph(node)) continue;

            ImVec2 nodePos = canvasToScreen(node.position).toImVec2();
            ImVec2 nodeSize = Vec2(node.size.x * m_state.viewScale, node.size.y * m_state.viewScale).toImVec2();

            bool pinHovered = false;

            for (const auto &pin: node.inputs) {
                Pin apiPin;
                apiPin.id = pin.id;
                apiPin.name = pin.name;
                apiPin.isInput = pin.isInput;
                apiPin.type = static_cast<PinType>(pin.type);
                apiPin.shape = static_cast<PinShape>(pin.shape);

                if (isPinHovered(node, apiPin, canvasPos)) {
                    m_state.hoveredNodeId = node.id;
                    m_state.hoveredNodeUuid = node.uuid;
                    m_state.hoveredPinId = pin.id;
                    m_state.hoveredPinUuid = pin.uuid;
                    pinHovered = true;
                    break;
                }
            }

            if (pinHovered) break;

            for (const auto &pin: node.outputs) {
                Pin apiPin;
                apiPin.id = pin.id;
                apiPin.name = pin.name;
                apiPin.isInput = pin.isInput;
                apiPin.type = static_cast<PinType>(pin.type);
                apiPin.shape = static_cast<PinShape>(pin.shape);

                if (isPinHovered(node, apiPin, canvasPos)) {
                    m_state.hoveredNodeId = node.id;
                    m_state.hoveredNodeUuid = node.uuid;
                    m_state.hoveredPinId = pin.id;
                    m_state.hoveredPinUuid = pin.uuid;
                    pinHovered = true;
                    break;
                }
            }

            if (pinHovered) break;

            if (isPointInRect(mousePos, nodePos, ImVec2(nodePos.x + nodeSize.x, nodePos.y + nodeSize.y))) {
                m_state.hoveredNodeId = node.id;
                m_state.hoveredNodeUuid = node.uuid;
                break;
            }
        }

        if (m_state.hoveredNodeId == -1 && m_state.hoveredPinId == -1) {
            for (const auto &group: m_state.groups) {
                if ((m_state.currentSubgraphId == -1 && group.getSubgraphId() == -1) ||
                    (m_state.currentSubgraphId >= 0 && group.getSubgraphId() == m_state.currentSubgraphId)) {
                    ImVec2 groupPos = canvasToScreen(group.position).toImVec2();
                    ImVec2 groupSize = Vec2(group.size.x * m_state.viewScale, group.size.y * m_state.viewScale).
                            toImVec2();

                    if (isPointInRect(mousePos, groupPos, ImVec2(groupPos.x + groupSize.x, groupPos.y + groupSize.y))) {
                        m_state.hoveredGroupId = group.id;
                        m_state.hoveredGroupUuid = group.uuid;
                        break;
                    }
                }
            }
        }
    }

    void NodeEditor::startConnectionDragByUUID(const UUID &nodeUuid, const UUID &pinUuid) {
        int nodeId = getNodeId(nodeUuid);
        if (nodeId == -1) return;

        Node *node = getNode(nodeId);
        if (!node) return;

        int pinId = -1;
        for (const auto &pin: node->outputs) {
            if (pin.uuid == pinUuid) {
                pinId = pin.id;
                break;
            }
        }

        if (pinId != -1) {
            startConnectionDrag(nodeId, pinId);
            m_state.connectingNodeUuid = nodeUuid;
            m_state.connectingPinUuid = pinUuid;
        }
    }

    void NodeEditor::startNodeDragByUUID(const UUID &nodeUuid, const ImVec2 &mousePos) {
        int nodeId = getNodeId(nodeUuid);
        if (nodeId != -1) {
            startNodeDrag(nodeId, mousePos);
            m_state.activeNodeUuid = nodeUuid;
        }
    }

    void NodeEditor::processGroupDragging() {
        if (m_state.activeGroupId == -1) return;

        ImVec2 mousePos = ImGui::GetMousePos();
        Group *group = getGroup(m_state.activeGroupId);
        if (!group) return;

        ImVec2 newScreenPos = ImVec2(
            mousePos.x - m_state.dragOffset.x,
            mousePos.y - m_state.dragOffset.y
        );

        Vec2 newCanvasPos = screenToCanvas(Vec2::fromImVec2(newScreenPos));
        Vec2 delta = newCanvasPos - group->position;
        group->position = newCanvasPos;

        for (int nodeId: group->nodes) {
            Node *node = getNode(nodeId);
            if (node) {
                node->position = node->position + delta;
            }
        }
    }

    void NodeEditor::processGroupResize() {
        if (m_state.activeGroupId == -1) return;

        ImVec2 mousePos = ImGui::GetMousePos();
        Group *group = getGroup(m_state.activeGroupId);
        if (!group) return;

        Vec2 dragDelta = Vec2::fromImVec2(mousePos) - m_state.dragStart;
        Vec2 newSize = m_state.groupStartSize + Vec2(dragDelta.x / m_state.viewScale, dragDelta.y / m_state.viewScale);

        newSize.x = std::max(100.0f, newSize.x);
        newSize.y = std::max(50.0f, newSize.y);

        group->size = newSize;
    }

    void NodeEditor::processContextMenu() {
        ImVec2 mousePos = ImGui::GetMousePos();

        m_state.interactionMode = InteractionMode::ContextMenu;
        m_state.contextMenuPos = Vec2::fromImVec2(mousePos);

        m_state.contextMenuNodeId = -1;
        m_state.contextMenuPinId = -1;
        m_state.contextMenuConnectionId = -1;
        m_state.contextMenuGroupId = -1;

        if (m_state.hoveredPinId != -1 && m_state.hoveredNodeId != -1) {
            m_state.contextMenuNodeId = m_state.hoveredNodeId;
            m_state.contextMenuPinId = m_state.hoveredPinId;
        } else if (m_state.hoveredNodeId != -1) {
            m_state.contextMenuNodeId = m_state.hoveredNodeId;
        } else if (m_state.hoveredConnectionId != -1) {
            m_state.contextMenuConnectionId = m_state.hoveredConnectionId;
        } else if (m_state.hoveredGroupId != -1) {
            m_state.contextMenuGroupId = m_state.hoveredGroupId;
        }
    }

    void NodeEditor::drawContextMenu(ImDrawList *drawList) {
        if (ImGui::BeginPopup("NodeEditorContextMenu")) {
            if (m_state.contextMenuNodeId != -1) {
                Node *node = getNode(m_state.contextMenuNodeId);
                if (node) {
                    ImGui::Text("Node: %s (%d)", node->name.c_str(), node->id);
                    ImGui::Separator();

                    if (ImGui::MenuItem("Delete Node")) {
                        removeNode(m_state.contextMenuNodeId);
                        ImGui::CloseCurrentPopup();
                    }

                    if (ImGui::MenuItem("Duplicate Node")) {
                        duplicateNode(m_state.contextMenuNodeId);
                        ImGui::CloseCurrentPopup();
                    }

                    if (ImGui::MenuItem("Select Node")) {
                        selectNode(m_state.contextMenuNodeId, ImGui::GetIO().KeyCtrl);
                        ImGui::CloseCurrentPopup();
                    }
                }
            } else if (m_state.contextMenuConnectionId != -1) {
                Connection *connection = getConnection(m_state.contextMenuConnectionId);
                if (connection) {
                    ImGui::Text("Connection: %d", m_state.contextMenuConnectionId);
                    ImGui::Separator();

                    if (ImGui::MenuItem("Delete Connection")) {
                        removeConnection(m_state.contextMenuConnectionId);
                        ImGui::CloseCurrentPopup();
                    }
                }
            } else if (m_state.contextMenuGroupId != -1) {
                Group *group = getGroup(m_state.contextMenuGroupId);
                if (group) {
                    ImGui::Text("Group: %s (%d)", group->name.c_str(), group->id);
                    ImGui::Separator();

                    if (ImGui::MenuItem("Delete Group")) {
                        removeGroup(m_state.contextMenuGroupId);
                        ImGui::CloseCurrentPopup();
                    }
                }
            } else {
                ImGui::Text("Canvas");
                ImGui::Separator();

                if (ImGui::MenuItem("Add Group")) {
                    Vec2 canvasPos = screenToCanvas(m_state.contextMenuPos);
                    addGroup("New Group", canvasPos, Vec2(200.0f, 150.0f));
                    ImGui::CloseCurrentPopup();
                }

                if (ImGui::MenuItem("Center View")) {
                    centerView();
                    ImGui::CloseCurrentPopup();
                }

                if (ImGui::MenuItem("Toggle Debug Mode")) {
                    m_debugMode = !m_debugMode;
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::EndPopup();
        } else {
            if (m_state.interactionMode == InteractionMode::ContextMenu) {
                endCurrentInteraction();
            }
        }

        ImVec2 popupPos = ImVec2(m_state.contextMenuPos.x, m_state.contextMenuPos.y);
        ImGui::SetNextWindowPos(popupPos);
        ImGui::OpenPopup("NodeEditorContextMenu");
    }

    bool NodeEditor::isPinHovered(const Node &node, const Pin &pin, const ImVec2 &canvasPos) {
        ImVec2 pinPos = getPinPos(node, pin, canvasPos);

        ImVec2 mousePos = ImGui::GetMousePos();

        float pinRadius = m_state.style.pinRadius * m_state.viewScale;

        float clickableRadius = pinRadius * 3.0f;

        float dx = mousePos.x - pinPos.x;
        float dy = mousePos.y - pinPos.y;

        return (dx * dx + dy * dy) <= (clickableRadius * clickableRadius);
    }

    void NodeEditor::processConnectionCreation() {
        if (!m_state.connecting || m_state.connectingNodeId == -1 || m_state.connectingPinId == -1)
            return;

        ImVec2 mousePos = ImGui::GetMousePos();
        ImDrawList *drawList = ImGui::GetWindowDrawList();

        const Node *sourceNode = getNode(m_state.connectingNodeId);
        if (!sourceNode) return;

        const Pin *sourcePinInternal = sourceNode->findPin(m_state.connectingPinId);
        if (!sourcePinInternal) return;

        bool isSourceInput = sourcePinInternal->isInput;

        m_state.magnetPinNodeId = -1;
        m_state.magnetPinId = -1;
        m_state.magnetPinNodeUuid = "";
        m_state.magnetPinUuid = "";
        m_state.canConnectToMagnetPin = false;

        float closestDist = m_state.magnetThreshold * m_state.magnetThreshold;

        drawList->AddText(ImVec2(10, 370), IM_COL32(255, 0, 0, 255),
                          ("Searching: " + std::string(isSourceInput ? "Output pins" : "Input pins") +
                           ", Nodes: " + std::to_string(m_state.nodes.size())).c_str());

        int eligiblePins = 0;

        for (const auto &node: m_state.nodes) {
            if (node.id == m_state.connectingNodeId || !isNodeInCurrentSubgraph(node))
                continue;

            const auto &pins = isSourceInput ? node.outputs : node.inputs;
            drawList->AddText(ImVec2(10, 390), IM_COL32(255, 0, 0, 255),
                              ("Node " + std::to_string(node.id) + " has " +
                               std::to_string(pins.size()) + " eligible pins").c_str());

            for (const auto &pinInternal: pins) {
                eligiblePins++;

                Pin apiPin;
                apiPin.id = pinInternal.id;
                apiPin.name = pinInternal.name;
                apiPin.isInput = pinInternal.isInput;
                apiPin.type = static_cast<PinType>(pinInternal.type);
                apiPin.shape = static_cast<PinShape>(pinInternal.shape);

                ImVec2 pinPos = getPinPos(node, apiPin, ImGui::GetWindowPos());

                float dx = mousePos.x - pinPos.x;
                float dy = mousePos.y - pinPos.y;
                float dist = dx * dx + dy * dy;

                drawList->AddCircle(pinPos, std::sqrt(closestDist), IM_COL32(0, 255, 0, 100));

                if (dist < closestDist) {
                    Pin sourceApiPin;
                    sourceApiPin.id = sourcePinInternal->id;
                    sourceApiPin.name = sourcePinInternal->name;
                    sourceApiPin.isInput = sourcePinInternal->isInput;
                    sourceApiPin.type = static_cast<PinType>(sourcePinInternal->type);
                    sourceApiPin.shape = static_cast<PinShape>(sourcePinInternal->shape);

                    bool canConnect = false;

                    if (isSourceInput && !pinInternal.isInput) {
                        canConnect = canCreateConnection(apiPin, sourceApiPin);
                    } else if (!isSourceInput && pinInternal.isInput) {
                        canConnect = canCreateConnection(sourceApiPin, apiPin);
                    }

                    drawList->AddText(pinPos, canConnect ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 0, 0, 255),
                                      canConnect ? "OK" : "X");

                    if (canConnect) {
                        m_state.magnetPinNodeId = node.id;
                        m_state.magnetPinId = pinInternal.id;
                        m_state.magnetPinNodeUuid = node.uuid;
                        m_state.magnetPinUuid = pinInternal.uuid;
                        m_state.canConnectToMagnetPin = true;
                        closestDist = dist;

                        drawList->AddCircleFilled(pinPos, 8.0f, IM_COL32(0, 255, 0, 200));
                    }
                }
            }
        }

        drawList->AddText(ImVec2(10, 410), IM_COL32(255, 0, 0, 255),
                          ("Eligible pins found: " + std::to_string(eligiblePins)).c_str());

        drawList->AddText(ImVec2(10, 430), IM_COL32(255, 0, 0, 255),
                          ("Magnet found: " + std::string(m_state.magnetPinNodeId != -1 ? "YES" : "NO") +
                           ", Node: " + std::to_string(m_state.magnetPinNodeId) +
                           ", Pin: " + std::to_string(m_state.magnetPinId)).c_str());
    }

    ImVec2 NodeEditor::getPinPos(const Node &node, const Pin &pin, const ImVec2 &canvasPos) const {
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

            return ImVec2(pinX, nodePos.y);
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

            return ImVec2(pinX, nodePos.y + nodeSize.y);
        }
    }

    bool NodeEditor::isConnectionHovered(const Connection &connection, const ImVec2 &canvasPos) {
        const Node *startNode = getNode(connection.startNodeId);
        const Node *endNode = getNode(connection.endNodeId);

        if (!startNode || !endNode) return false;

        const Pin *apiStartPin = getPin(connection.startNodeId, connection.startPinId);
        const Pin *apiEndPin = getPin(connection.endNodeId, connection.endPinId);

        if (!apiStartPin || !apiEndPin) return false;

        ImVec2 p1 = getPinPos(*startNode, *apiStartPin, canvasPos);
        ImVec2 p2 = getPinPos(*endNode, *apiEndPin, canvasPos);
        ImVec2 mousePos = ImGui::GetMousePos();

        float threshold = std::max(8.0f, 12.0f * m_state.viewScale);

        ConnectionStyleManager::ConnectionStyle style = m_connectionStyleManager.getDefaultStyle();

        switch (style) {
            case ConnectionStyleManager::ConnectionStyle::Bezier: {
                const float distance = std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
                const float tension = m_connectionStyleManager.getConfig().curveTension;
                const float cpDistance = distance * tension;

                ImVec2 cp1, cp2;
                if (apiStartPin->isInput) {
                    cp1 = ImVec2(p1.x, p1.y - cpDistance);
                } else {
                    cp1 = ImVec2(p1.x, p1.y + cpDistance);
                }

                if (apiEndPin->isInput) {
                    cp2 = ImVec2(p2.x, p2.y - cpDistance);
                } else {
                    cp2 = ImVec2(p2.x, p2.y + cpDistance);
                }

                return isPointNearCubicBezier(mousePos, p1, cp1, cp2, p2, threshold);
            }

            case ConnectionStyleManager::ConnectionStyle::StraightLine: {
                return isPointNearLine(mousePos, p1, p2, threshold);
            }

            case ConnectionStyleManager::ConnectionStyle::AngleLine: {
                ImVec2 middle = ImVec2(p2.x, p1.y);
                return isPointNearLine(mousePos, p1, middle, threshold) ||
                       isPointNearLine(mousePos, middle, p2, threshold);
            }

            case ConnectionStyleManager::ConnectionStyle::MetroLine: {
                float dx = p2.x - p1.x;
                float dy = p2.y - p1.y;

                ImVec2 middle1, middle2;
                if (std::abs(dx) > std::abs(dy)) {
                    middle1 = ImVec2(p1.x + dx * 0.5f, p1.y);
                    middle2 = ImVec2(p1.x + dx * 0.5f, p2.y);
                } else {
                    middle1 = ImVec2(p1.x, p1.y + dy * 0.5f);
                    middle2 = ImVec2(p2.x, p1.y + dy * 0.5f);
                }

                return isPointNearLine(mousePos, p1, middle1, threshold) ||
                       isPointNearLine(mousePos, middle1, middle2, threshold) ||
                       isPointNearLine(mousePos, middle2, p2, threshold);
            }

            default:
                return isPointNearLine(mousePos, p1, p2, threshold);
        }
    }

    void NodeEditor::drawDebugHitboxes(ImDrawList *drawList, const ImVec2 &canvasPos) {
        for (const auto &node: m_state.nodes) {
            if (!isNodeInCurrentSubgraph(node)) continue;

            ImVec2 nodePos = canvasToScreen(node.position).toImVec2();
            ImVec2 nodeSize = Vec2(node.size.x * m_state.viewScale, node.size.y * m_state.viewScale).toImVec2();

            drawList->AddRect(
                nodePos,
                ImVec2(nodePos.x + nodeSize.x, nodePos.y + nodeSize.y),
                IM_COL32(0, 255, 0, 128), 0, 0, 1.0f
            );

            for (const auto &pin: node.inputs) {
                Pin apiPin;
                apiPin.id = pin.id;
                apiPin.name = pin.name;
                apiPin.isInput = pin.isInput;
                apiPin.type = static_cast<PinType>(pin.type);
                apiPin.shape = static_cast<PinShape>(pin.shape);

                ImVec2 pinPos = getPinPos(node, apiPin, canvasPos);
                float radius = m_state.style.pinRadius * m_state.viewScale * 2.0f;

                drawList->AddCircle(
                    pinPos,
                    radius,
                    IM_COL32(255, 0, 0, 128),
                    0, 1.0f
                );
            }

            for (const auto &pin: node.outputs) {
                Pin apiPin;
                apiPin.id = pin.id;
                apiPin.name = pin.name;
                apiPin.isInput = pin.isInput;
                apiPin.type = static_cast<PinType>(pin.type);
                apiPin.shape = static_cast<PinShape>(pin.shape);

                ImVec2 pinPos = getPinPos(node, apiPin, canvasPos);
                float radius = m_state.style.pinRadius * m_state.viewScale * 2.0f;

                drawList->AddCircle(
                    pinPos,
                    radius,
                    IM_COL32(255, 0, 0, 128),
                    0, 1.0f
                );
            }
        }

        for (const auto &connection: m_state.connections) {
            const Node *startNode = getNode(connection.startNodeId);
            const Node *endNode = getNode(connection.endNodeId);

            if (!startNode || !endNode) continue;
            if (!isNodeInCurrentSubgraph(*startNode) || !isNodeInCurrentSubgraph(*endNode)) continue;

            const Pin *startPin = getPin(connection.startNodeId, connection.startPinId);
            const Pin *endPin = getPin(connection.endNodeId, connection.endPinId);

            if (!startPin || !endPin) continue;

            ImVec2 p1 = getPinPos(*startNode, *startPin, canvasPos);
            ImVec2 p2 = getPinPos(*endNode, *endPin, canvasPos);
            ImVec2 mousePos = ImGui::GetMousePos();

            float threshold = std::max(8.0f, 12.0f * m_state.viewScale);
            ConnectionStyleManager::ConnectionStyle style = m_connectionStyleManager.getDefaultStyle();

            float minDist = FLT_MAX;

            switch (style) {
                case ConnectionStyleManager::ConnectionStyle::Bezier: {
                    const float distance = std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
                    const float tension = m_connectionStyleManager.getConfig().curveTension;
                    const float cpDistance = distance * tension;

                    ImVec2 cp1, cp2;
                    if (startPin->isInput) {
                        cp1 = ImVec2(p1.x, p1.y - cpDistance);
                    } else {
                        cp1 = ImVec2(p1.x, p1.y + cpDistance);
                    }

                    if (endPin->isInput) {
                        cp2 = ImVec2(p2.x, p2.y - cpDistance);
                    } else {
                        cp2 = ImVec2(p2.x, p2.y + cpDistance);
                    }

                    drawList->AddCircle(cp1, 4.0f, IM_COL32(255, 255, 0, 255));
                    drawList->AddCircle(cp2, 4.0f, IM_COL32(255, 255, 0, 255));

                    const int steps = 20;
                    ImVec2 prev = p1;
                    for (int i = 1; i <= steps; ++i) {
                        float t = i / (float) steps;
                        ImVec2 current = ImBezierCubicCalc(p1, cp1, cp2, p2, t);

                        drawList->AddLine(prev, current, IM_COL32(0, 255, 255, 255), 2.0f);
                        drawList->AddCircle(current, threshold, IM_COL32(255, 0, 255, 100));

                        float dx = mousePos.x - current.x;
                        float dy = mousePos.y - current.y;
                        float dist = sqrt(dx * dx + dy * dy);
                        minDist = std::min(minDist, dist);
                        prev = current;
                    }
                    break;
                }

                case ConnectionStyleManager::ConnectionStyle::StraightLine: {
                    drawList->AddLine(p1, p2, IM_COL32(0, 255, 255, 255), 3.0f);

                    float dx = p2.x - p1.x;
                    float dy = p2.y - p1.y;
                    float length2 = dx * dx + dy * dy;

                    if (length2 > 0.0001f) {
                        float t = ((mousePos.x - p1.x) * dx + (mousePos.y - p1.y) * dy) / length2;
                        t = std::max(0.0f, std::min(1.0f, t));

                        ImVec2 closest = ImVec2(p1.x + t * dx, p1.y + t * dy);
                        drawList->AddCircle(closest, threshold, IM_COL32(255, 0, 255, 100));

                        dx = mousePos.x - closest.x;
                        dy = mousePos.y - closest.y;
                        minDist = sqrt(dx * dx + dy * dy);
                    }
                    break;
                }

                case ConnectionStyleManager::ConnectionStyle::AngleLine: {
                    ImVec2 middle = ImVec2(p2.x, p1.y);
                    drawList->AddLine(p1, middle, IM_COL32(0, 255, 255, 255), 3.0f);
                    drawList->AddLine(middle, p2, IM_COL32(0, 255, 255, 255), 3.0f);
                    drawList->AddCircle(middle, 4.0f, IM_COL32(255, 255, 0, 255));

                    float dist1 = FLT_MAX, dist2 = FLT_MAX;

                    float dx = middle.x - p1.x;
                    float dy = middle.y - p1.y;
                    float length2 = dx * dx + dy * dy;
                    if (length2 > 0.0001f) {
                        float t = ((mousePos.x - p1.x) * dx + (mousePos.y - p1.y) * dy) / length2;
                        t = std::max(0.0f, std::min(1.0f, t));
                        ImVec2 closest = ImVec2(p1.x + t * dx, p1.y + t * dy);
                        drawList->AddCircle(closest, threshold, IM_COL32(255, 0, 255, 100));
                        dx = mousePos.x - closest.x;
                        dy = mousePos.y - closest.y;
                        dist1 = sqrt(dx * dx + dy * dy);
                    }

                    dx = p2.x - middle.x;
                    dy = p2.y - middle.y;
                    length2 = dx * dx + dy * dy;
                    if (length2 > 0.0001f) {
                        float t = ((mousePos.x - middle.x) * dx + (mousePos.y - middle.y) * dy) / length2;
                        t = std::max(0.0f, std::min(1.0f, t));
                        ImVec2 closest = ImVec2(middle.x + t * dx, middle.y + t * dy);
                        drawList->AddCircle(closest, threshold, IM_COL32(255, 0, 255, 100));
                        dx = mousePos.x - closest.x;
                        dy = mousePos.y - closest.y;
                        dist2 = sqrt(dx * dx + dy * dy);
                    }

                    minDist = std::min(dist1, dist2);
                    break;
                }

                case ConnectionStyleManager::ConnectionStyle::MetroLine: {
                    float dx = p2.x - p1.x;
                    float dy = p2.y - p1.y;

                    ImVec2 middle1, middle2;
                    if (std::abs(dx) > std::abs(dy)) {
                        middle1 = ImVec2(p1.x + dx * 0.5f, p1.y);
                        middle2 = ImVec2(p1.x + dx * 0.5f, p2.y);
                    } else {
                        middle1 = ImVec2(p1.x, p1.y + dy * 0.5f);
                        middle2 = ImVec2(p2.x, p1.y + dy * 0.5f);
                    }

                    drawList->AddLine(p1, middle1, IM_COL32(0, 255, 255, 255), 3.0f);
                    drawList->AddLine(middle1, middle2, IM_COL32(0, 255, 255, 255), 3.0f);
                    drawList->AddLine(middle2, p2, IM_COL32(0, 255, 255, 255), 3.0f);
                    drawList->AddCircle(middle1, 4.0f, IM_COL32(255, 255, 0, 255));
                    drawList->AddCircle(middle2, 4.0f, IM_COL32(255, 255, 0, 255));

                    std::vector<ImVec2> points = {p1, middle1, middle2, p2};
                    minDist = FLT_MAX;

                    for (size_t i = 0; i < points.size() - 1; ++i) {
                        ImVec2 segStart = points[i];
                        ImVec2 segEnd = points[i + 1];

                        float dx = segEnd.x - segStart.x;
                        float dy = segEnd.y - segStart.y;
                        float length2 = dx * dx + dy * dy;

                        if (length2 > 0.0001f) {
                            float t = ((mousePos.x - segStart.x) * dx + (mousePos.y - segStart.y) * dy) / length2;
                            t = std::max(0.0f, std::min(1.0f, t));
                            ImVec2 closest = ImVec2(segStart.x + t * dx, segStart.y + t * dy);
                            drawList->AddCircle(closest, threshold, IM_COL32(255, 0, 255, 100));

                            dx = mousePos.x - closest.x;
                            dy = mousePos.y - closest.y;
                            float dist = sqrt(dx * dx + dy * dy);
                            minDist = std::min(minDist, dist);
                        }
                    }
                    break;
                }

                default: {
                    drawList->AddLine(p1, p2, IM_COL32(0, 255, 255, 255), 3.0f);

                    float dx = p2.x - p1.x;
                    float dy = p2.y - p1.y;
                    float length2 = dx * dx + dy * dy;

                    if (length2 > 0.0001f) {
                        float t = ((mousePos.x - p1.x) * dx + (mousePos.y - p1.y) * dy) / length2;
                        t = std::max(0.0f, std::min(1.0f, t));

                        ImVec2 closest = ImVec2(p1.x + t * dx, p1.y + t * dy);
                        drawList->AddCircle(closest, threshold, IM_COL32(255, 0, 255, 100));

                        dx = mousePos.x - closest.x;
                        dy = mousePos.y - closest.y;
                        minDist = sqrt(dx * dx + dy * dy);
                    }
                    break;
                }
            }

            drawList->AddCircle(p1, 6.0f, IM_COL32(255, 255, 0, 255));
            drawList->AddCircle(p2, 6.0f, IM_COL32(255, 255, 0, 255));

            const char *styleNames[] = {"Bezier", "Straight", "Angle", "Metro", "Unknown"};
            int styleIndex = std::min(static_cast<int>(style), 4);

            char debugText[256];
            sprintf(debugText, "Conn%d (%s): %.1fpx (%.1f)",
                    connection.id, styleNames[styleIndex], minDist, threshold);
            ImVec2 midPoint = ImVec2((p1.x + p2.x) * 0.5f, (p1.y + p2.y) * 0.5f);
            drawList->AddText(midPoint, IM_COL32(255, 255, 255, 255), debugText);
        }

        ImVec2 textPos = ImVec2(canvasPos.x + 10, canvasPos.y + 10);
        std::string modeText = "Mode: " + getInteractionModeName();
        drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), modeText.c_str());

        textPos.y += 20;
        char buffer[128];
        sprintf(buffer, "Hovered: Node %d, Pin %d, Conn %d, Group %d",
                m_state.hoveredNodeId, m_state.hoveredPinId, m_state.hoveredConnectionId, m_state.hoveredGroupId);
        drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), buffer);
    }

    std::string NodeEditor::getInteractionModeName() const {
        switch (m_state.interactionMode) {
            case InteractionMode::None: return "None";
            case InteractionMode::PanCanvas: return "PanCanvas";
            case InteractionMode::BoxSelect: return "BoxSelect";
            case InteractionMode::DragNode: return "DragNode";
            case InteractionMode::ResizeNode: return "ResizeNode";
            case InteractionMode::DragConnection: return "DragConnection";
            case InteractionMode::DragGroup: return "DragGroup";
            case InteractionMode::ResizeGroup: return "ResizeGroup";
            case InteractionMode::ContextMenu: return "ContextMenu";
            default: return "Unknown";
        }
    }

    void NodeEditor::processZoom(const ImVec2 &mousePos) {
        float zoom = ImGui::GetIO().MouseWheel;

        if (std::abs(zoom) < 0.01f) return;

        Vec2 canvasPos = screenToCanvas(Vec2(mousePos.x, mousePos.y));

        float zoomFactor = 1.1f;
        float newScale = m_state.viewScale;

        if (zoom > 0) {
            newScale *= zoomFactor;
        } else {
            newScale /= zoomFactor;
        }

        newScale = std::max(0.1f, std::min(newScale, 3.0f));

        float scaleRatio = newScale / m_state.viewScale;
        Vec2 newViewPos = Vec2(
            mousePos.x - (mousePos.x - m_state.viewPosition.x) * scaleRatio,
            mousePos.y - (mousePos.y - m_state.viewPosition.y) * scaleRatio
        );

        m_state.viewScale = newScale;
        m_state.viewPosition = newViewPos;

        m_viewManager.setViewScale(newScale);
        m_viewManager.setViewPosition(newViewPos);
    }

    void NodeEditor::duplicateNode(int nodeId) {
        const Node *srcNode = getNode(nodeId);
        if (!srcNode) return;

        Vec2 offset(20.0f, 20.0f);
        Vec2 newPos = srcNode->position + offset;

        int newNodeId = addNode(srcNode->name + " (copy)", srcNode->type, newPos);
        Node *newNode = getNode(newNodeId);
        if (!newNode) return;

        newNode->iconSymbol = srcNode->iconSymbol;
        newNode->labelPosition = srcNode->labelPosition;

        for (const auto &pin: srcNode->inputs) {
            addPin(newNodeId, pin.name, true, pin.type, pin.shape);
        }

        for (const auto &pin: srcNode->outputs) {
            addPin(newNodeId, pin.name, false, pin.type, pin.shape);
        }
    }
}
