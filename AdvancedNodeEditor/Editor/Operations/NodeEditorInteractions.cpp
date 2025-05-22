#include "../../Core/NodeEditor.h"
#include "../../Core/Style/InteractionMode.h"
#include <algorithm>
#include <cfloat>

namespace NodeEditorCore {
    void NodeEditor::processInteraction() {
        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();

        ImDrawList *drawList = ImGui::GetWindowDrawList();

        bool isMouseDoubleClicked = ImGui::IsMouseDoubleClicked(0);
        bool isMouseClicked = ImGui::IsMouseClicked(0);
        bool isMouseReleased = ImGui::IsMouseReleased(0);
        bool isMouseDragging = ImGui::IsMouseDragging(0);
        bool isMiddleMousePressed = ImGui::IsMouseDown(2);

        updateHoveredElements(mousePos);
        updateRerouteHover(mousePos, canvasPos);

        // DEBUG TEMPORAIRE - Toujours affiché
        char debugText[256];
        sprintf(debugText, "Double-click: %s, HoveredConn: %d",
                isMouseDoubleClicked ? "YES" : "NO", m_state.hoveredConnectionId);
        drawList->AddText(ImVec2(10, 400), IM_COL32(255, 255, 0, 255), debugText);

        if (isMouseDoubleClicked) {
            printf("DOUBLE-CLICK DETECTED! HoveredConnection: %d\n", m_state.hoveredConnectionId);

            if (m_state.hoveredConnectionId >= 0) {
                printf("Processing connection double-click...\n");

                const Connection *connection = getConnection(m_state.hoveredConnectionId);
                if (!connection) {
                    printf("ERROR: Connection %d not found!\n", m_state.hoveredConnectionId);
                    drawList->AddText(ImVec2(10, 420), IM_COL32(255, 0, 0, 255), "ERROR: Connection not found!");
                    return;
                }

                printf("Connection found, calculating distance...\n");
                int insertIndex = 0;
                float distance = getDistanceToConnection(*connection, mousePos, canvasPos, insertIndex);

                printf("Distance: %.1f, Threshold: %.1f, InsertIndex: %d\n",
                       distance, m_rerouteStyle.dragThreshold, insertIndex);

                char distanceText[256];
                sprintf(distanceText, "Distance: %.1f, Threshold: %.1f",
                        distance, m_rerouteStyle.dragThreshold);
                drawList->AddText(ImVec2(10, 420), IM_COL32(255, 255, 255, 255), distanceText);

                if (distance < m_rerouteStyle.dragThreshold) {
                    Vec2 reroutePos = screenToCanvas(Vec2(mousePos.x, mousePos.y));
                    printf("Creating reroute at (%.1f, %.1f) with index %d\n", reroutePos.x, reroutePos.y, insertIndex);

                    int rerouteId = addReroute(m_state.hoveredConnectionId, reroutePos, insertIndex);

                    printf("SUCCESS: Created reroute %d\n", rerouteId);
                    char successText[256];
                    sprintf(successText, "SUCCESS: Created reroute %d", rerouteId);
                    drawList->AddText(ImVec2(10, 440), IM_COL32(0, 255, 0, 255), successText);
                } else {
                    printf("Distance too large - no reroute created\n");
                    drawList->AddText(ImVec2(10, 440), IM_COL32(255, 0, 0, 255), "Distance too large");
                }
                return;
            } else {
                printf("No connection hovered\n");
                drawList->AddText(ImVec2(10, 420), IM_COL32(255, 0, 0, 255), "No connection hovered");
            }
        }

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
            RerouteHitZone hitZone;
            int hoveredRerouteId = findRerouteAtPosition(mousePos, canvasPos, hitZone);

            if (hoveredRerouteId != -1) {
                bool altPressed = ImGui::GetIO().KeyAlt;
                bool ctrlPressed = ImGui::GetIO().KeyCtrl;

                if (altPressed) {
                    removeReroute(hoveredRerouteId);
                } else {
                    selectReroute(hoveredRerouteId, ctrlPressed);

                    if (hitZone == RerouteHitZone::Inner) {
                        m_state.interactionMode = InteractionMode::DragReroute;
                        m_activeRerouteId = hoveredRerouteId;
                        m_state.dragStart = Vec2(mousePos.x, mousePos.y);
                    } else if (hitZone == RerouteHitZone::Outer) {
                        startRerouteConnection(hoveredRerouteId, mousePos);
                    }
                }
            } else if (m_state.hoveredPinId >= 0 && m_state.hoveredNodeId >= 0) {
                startConnectionDrag(m_state.hoveredNodeId, m_state.hoveredPinId);
            } else if (m_state.hoveredNodeId >= 0) {
                Node *node = getNode(m_state.hoveredNodeId);
                if (node) {
                    bool isAlreadySelected = node->selected;
                    bool ctrlPressed = ImGui::GetIO().KeyCtrl;

                    if (!ctrlPressed) {
                        deselectAllConnections();
                        deselectAllReroutes();
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
                        deselectAllReroutes();
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
                    deselectAllReroutes();
                }
            }
        }

        if (m_state.interactionMode != InteractionMode::None) {
            if (isMouseDragging) {
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
                                } else {
                                    connectionId = addConnection(m_state.connectingNodeId, m_state.connectingPinId,
                                                                 m_state.magnetPinNodeId, m_state.magnetPinId);
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

        if (ImGui::IsKeyPressed(ImGuiKey_R) && !m_state.connections.empty()) {
            Vec2 testPos = screenToCanvas(Vec2(400, 300));
            int testReroute = addReroute(m_state.connections[0].id, testPos, 0);
            printf("Test reroute created: %d\n", testReroute);
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

            case InteractionMode::DragReroute:
                processRerouteDrag(mousePos);
                break;

            default:
                break;
        }
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

        m_activeRerouteId = -1;
        m_connectingFromReroute = false;
        m_connectingRerouteId = -1;

        ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
    }

    void NodeEditor::processDeleteKeyPress() {
        std::vector<int> reroutesToRemove;
        for (const auto &reroute: m_reroutes) {
            if (reroute.selected) {
                reroutesToRemove.push_back(reroute.id);
            }
        }

        for (int id: reroutesToRemove) {
            removeReroute(id);
        }

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

        ImVec2 mousePos = ImGui::GetMousePos();
        float threshold = std::max(8.0f, 12.0f * m_state.viewScale);

        std::vector<Reroute> reroutes = getReroutesForConnection(connection.id);

        if (reroutes.empty()) {
            Pin startPin;
            startPin.id = apiStartPin->id;
            startPin.isInput = apiStartPin->isInput;
            startPin.type = apiStartPin->type;

            Pin endPin;
            endPin.id = apiEndPin->id;
            endPin.isInput = apiEndPin->isInput;
            endPin.type = apiEndPin->type;

            ImVec2 p1 = getPinPos(*startNode, startPin, canvasPos);
            ImVec2 p2 = getPinPos(*endNode, endPin, canvasPos);

            ConnectionStyleManager::ConnectionStyle style = m_connectionStyleManager.getDefaultStyle();
            float tension = m_connectionStyleManager.getConfig().curveTension;

            switch (style) {
                case ConnectionStyleManager::ConnectionStyle::Bezier: {
                    auto [cp1, cp2] = calculateBezierControlPoints(p1, p2, startPin.isInput, endPin.isInput, tension);
                    return getDistanceToBezierCubic(mousePos, p1, cp1, cp2, p2) <= threshold;
                }
                case ConnectionStyleManager::ConnectionStyle::AngleLine: {
                    ImVec2 middle = ImVec2(p2.x, p1.y);
                    float dist1 = getDistanceToLineSegment(mousePos, p1, middle);
                    float dist2 = getDistanceToLineSegment(mousePos, middle, p2);
                    return std::min(dist1, dist2) <= threshold;
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

                    float dist1 = getDistanceToLineSegment(mousePos, p1, middle1);
                    float dist2 = getDistanceToLineSegment(mousePos, middle1, middle2);
                    float dist3 = getDistanceToLineSegment(mousePos, middle2, p2);
                    return std::min({dist1, dist2, dist3}) <= threshold;
                }
                default:
                    return getDistanceToLineSegment(mousePos, p1, p2) <= threshold;
            }
        } else {
            std::vector<ImVec2> pathPoints = getConnectionPathWithReroutesForDetection(connection, canvasPos);
            
            for (size_t i = 0; i < pathPoints.size() - 1; i++) {
                ImVec2 segmentStart = pathPoints[i];
                ImVec2 segmentEnd = pathPoints[i + 1];

                bool segmentStartInput, segmentEndInput;

                if (i == 0) {
                    segmentStartInput = apiStartPin->isInput;
                } else {
                    segmentStartInput = false;
                }

                if (i == pathPoints.size() - 2) {
                    segmentEndInput = apiEndPin->isInput;
                } else {
                    segmentEndInput = true;
                }

                ConnectionStyleManager::ConnectionStyle style = m_connectionStyleManager.getDefaultStyle();
                float tension = m_connectionStyleManager.getConfig().curveTension;

                float segmentDistance = FLT_MAX;

                switch (style) {
                    case ConnectionStyleManager::ConnectionStyle::Bezier: {
                        auto [cp1, cp2] = calculateBezierControlPoints(segmentStart, segmentEnd, segmentStartInput, segmentEndInput, tension);
                        segmentDistance = getDistanceToBezierCubic(mousePos, segmentStart, cp1, cp2, segmentEnd);
                        break;
                    }
                    case ConnectionStyleManager::ConnectionStyle::AngleLine: {
                        ImVec2 middle = ImVec2(segmentEnd.x, segmentStart.y);
                        float dist1 = getDistanceToLineSegment(mousePos, segmentStart, middle);
                        float dist2 = getDistanceToLineSegment(mousePos, middle, segmentEnd);
                        segmentDistance = std::min(dist1, dist2);
                        break;
                    }
                    case ConnectionStyleManager::ConnectionStyle::MetroLine: {
                        float dx = segmentEnd.x - segmentStart.x;
                        float dy = segmentEnd.y - segmentStart.y;

                        ImVec2 middle1, middle2;
                        if (std::abs(dx) > std::abs(dy)) {
                            middle1 = ImVec2(segmentStart.x + dx * 0.5f, segmentStart.y);
                            middle2 = ImVec2(segmentStart.x + dx * 0.5f, segmentEnd.y);
                        } else {
                            middle1 = ImVec2(segmentStart.x, segmentStart.y + dy * 0.5f);
                            middle2 = ImVec2(segmentEnd.x, segmentStart.y + dy * 0.5f);
                        }

                        float dist1 = getDistanceToLineSegment(mousePos, segmentStart, middle1);
                        float dist2 = getDistanceToLineSegment(mousePos, middle1, middle2);
                        float dist3 = getDistanceToLineSegment(mousePos, middle2, segmentEnd);
                        segmentDistance = std::min({dist1, dist2, dist3});
                        break;
                    }
                    default:
                        segmentDistance = getDistanceToLineSegment(mousePos, segmentStart, segmentEnd);
                        break;
                }

                if (segmentDistance <= threshold) {
                    return true;
                }
            }
            return false;
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

            ImVec2 mousePos = ImGui::GetMousePos();

            float threshold = std::max(8.0f, 12.0f * m_state.viewScale);
            
            std::vector<ImVec2> pathPoints = getConnectionPathWithReroutesForDetection(connection, canvasPos);

            if (pathPoints.size() < 2) continue;

            ConnectionStyleManager::ConnectionStyle style = m_connectionStyleManager.getDefaultStyle();
            const float tension = m_connectionStyleManager.getConfig().curveTension;

            for (size_t i = 0; i < pathPoints.size() - 1; i++) {
                ImVec2 segmentStart = pathPoints[i];
                ImVec2 segmentEnd = pathPoints[i + 1];

                ImU32 segmentColor = (i % 2 == 0) ? IM_COL32(0, 255, 255, 255) : IM_COL32(255, 255, 0, 255);

                bool segmentStartInput, segmentEndInput;

                if (i == 0) {
                    segmentStartInput = startPin->isInput;
                } else {
                    segmentStartInput = false;
                }

                if (i == pathPoints.size() - 2) {
                    segmentEndInput = endPin->isInput;
                } else {
                    segmentEndInput = true;
                }

                switch (style) {
                    case ConnectionStyleManager::ConnectionStyle::Bezier: {
                        auto [cp1, cp2] = calculateBezierControlPoints(segmentStart, segmentEnd, segmentStartInput, segmentEndInput, tension);

                        drawList->AddCircle(cp1, 4.0f, IM_COL32(255, 255, 0, 255));
                        drawList->AddCircle(cp2, 4.0f, IM_COL32(255, 255, 0, 255));

                        const int steps = 20;
                        ImVec2 prev = segmentStart;
                        for (int j = 1; j <= steps; ++j) {
                            float t = j / (float) steps;
                            float u = 1.0f - t;
                            float w1 = u * u * u;
                            float w2 = 3 * u * u * t;
                            float w3 = 3 * u * t * t;
                            float w4 = t * t * t;

                            ImVec2 current(
                                w1 * segmentStart.x + w2 * cp1.x + w3 * cp2.x + w4 * segmentEnd.x,
                                w1 * segmentStart.y + w2 * cp1.y + w3 * cp2.y + w4 * segmentEnd.y
                            );

                            drawList->AddLine(prev, current, segmentColor, 2.0f);
                            drawList->AddCircle(current, threshold, IM_COL32(255, 0, 255, 50));
                            prev = current;
                        }
                        break;
                    }

                    case ConnectionStyleManager::ConnectionStyle::StraightLine: {
                        drawList->AddLine(segmentStart, segmentEnd, segmentColor, 3.0f);
                        
                        float dx = segmentEnd.x - segmentStart.x;
                        float dy = segmentEnd.y - segmentStart.y;
                        float length2 = dx * dx + dy * dy;

                        if (length2 > 0.0001f) {
                            float t = ((mousePos.x - segmentStart.x) * dx + (mousePos.y - segmentStart.y) * dy) / length2;
                            t = std::max(0.0f, std::min(1.0f, t));

                            ImVec2 closest = ImVec2(segmentStart.x + t * dx, segmentStart.y + t * dy);
                            drawList->AddCircle(closest, threshold, IM_COL32(255, 0, 255, 50));
                        }
                        break;
                    }

                    case ConnectionStyleManager::ConnectionStyle::AngleLine: {
                        ImVec2 middle = ImVec2(segmentEnd.x, segmentStart.y);
                        drawList->AddLine(segmentStart, middle, segmentColor, 3.0f);
                        drawList->AddLine(middle, segmentEnd, segmentColor, 3.0f);
                        drawList->AddCircle(middle, 4.0f, IM_COL32(255, 0, 255, 150));
                        drawList->AddCircle(middle, threshold, IM_COL32(255, 0, 255, 50));
                        break;
                    }

                    case ConnectionStyleManager::ConnectionStyle::MetroLine: {
                        float dx = segmentEnd.x - segmentStart.x;
                        float dy = segmentEnd.y - segmentStart.y;

                        ImVec2 middle1, middle2;
                        if (std::abs(dx) > std::abs(dy)) {
                            middle1 = ImVec2(segmentStart.x + dx * 0.5f, segmentStart.y);
                            middle2 = ImVec2(segmentStart.x + dx * 0.5f, segmentEnd.y);
                        } else {
                            middle1 = ImVec2(segmentStart.x, segmentStart.y + dy * 0.5f);
                            middle2 = ImVec2(segmentEnd.x, segmentStart.y + dy * 0.5f);
                        }

                        drawList->AddLine(segmentStart, middle1, segmentColor, 3.0f);
                        drawList->AddLine(middle1, middle2, segmentColor, 3.0f);
                        drawList->AddLine(middle2, segmentEnd, segmentColor, 3.0f);
                        drawList->AddCircle(middle1, 4.0f, IM_COL32(255, 0, 255, 150));
                        drawList->AddCircle(middle2, 4.0f, IM_COL32(255, 0, 255, 150));
                        drawList->AddCircle(middle1, threshold, IM_COL32(255, 0, 255, 50));
                        drawList->AddCircle(middle2, threshold, IM_COL32(255, 0, 255, 50));
                        break;
                    }

                    default:
                        drawList->AddLine(segmentStart, segmentEnd, segmentColor, 3.0f);
                        break;
                }

                char segmentText[64];
                sprintf(segmentText, "S%zu", i);
                ImVec2 segmentMid = ImVec2((segmentStart.x + segmentEnd.x) * 0.5f,
                                           (segmentStart.y + segmentEnd.y) * 0.5f);
                drawList->AddText(segmentMid, IM_COL32(255, 255, 255, 255), segmentText);
            }

            for (size_t i = 0; i < pathPoints.size(); i++) {
                ImU32 pointColor = (i == 0 || i == pathPoints.size() - 1)
                                    ? IM_COL32(255, 255, 0, 255)
                                    : IM_COL32(0, 255, 0, 255);
                drawList->AddCircle(pathPoints[i], 6.0f, pointColor);
            }

            int insertIndex = 0;
            float minDist = getDistanceToConnection(connection, mousePos, canvasPos, insertIndex);

            char debugText[256];
            sprintf(debugText, "Conn%d: %.1fpx (%.1f) %s reroutes:%zu",
                    connection.id, minDist, threshold,
                    minDist <= threshold ? "HIT" : "MISS",
                    pathPoints.size() - 2);
            ImVec2 textPos = ImVec2((pathPoints.front().x + pathPoints.back().x) * 0.5f, 
                                   (pathPoints.front().y + pathPoints.back().y) * 0.5f - 20);
            drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), debugText);
        }

        for (const auto &reroute: m_reroutes) {
            const Connection *connection = getConnection(reroute.connectionId);
            if (!connection) continue;

            const Node *startNode = getNode(connection->startNodeId);
            const Node *endNode = getNode(connection->endNodeId);
            if (!startNode || !endNode) continue;
            if (!isNodeInCurrentSubgraph(*startNode) || !isNodeInCurrentSubgraph(*endNode)) continue;

            Vec2 rerouteScreenPos = canvasToScreen(reroute.position);
            ImVec2 center = rerouteScreenPos.toImVec2();

            float outerRadius = m_rerouteStyle.outerRadius * m_state.viewScale;
            float innerRadius = m_rerouteStyle.innerRadius * m_state.viewScale;

            drawList->AddCircle(center, outerRadius, IM_COL32(255, 255, 0, 150), 0, 2.0f);
            drawList->AddCircle(center, innerRadius, IM_COL32(255, 100, 0, 150), 0, 2.0f);

            ImVec2 mousePos = ImGui::GetMousePos();
            float dx = mousePos.x - center.x;
            float dy = mousePos.y - center.y;
            float distance = sqrt(dx * dx + dy * dy);

            RerouteHitZone hitZone = getRerouteHitZone(reroute, mousePos, canvasPos);
            ImU32 textColor = IM_COL32(255, 255, 255, 255);
            if (hitZone == RerouteHitZone::Inner) textColor = IM_COL32(255, 100, 0, 255);
            else if (hitZone == RerouteHitZone::Outer) textColor = IM_COL32(255, 255, 0, 255);

            char rerouteText[128];
            sprintf(rerouteText, "R%d[%d] d:%.1f %s", reroute.id, reroute.index, distance,
                    hitZone == RerouteHitZone::Inner ? "INNER" : hitZone == RerouteHitZone::Outer ? "OUTER" : "NONE");
            drawList->AddText(ImVec2(center.x + 15, center.y - 10), textColor, rerouteText);
        }

        ImVec2 textPos = ImVec2(canvasPos.x + 10, canvasPos.y + 10);
        std::string modeText = "Mode: " + getInteractionModeName();
        drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), modeText.c_str());

        textPos.y += 20;
        char buffer[128];
        sprintf(buffer, "Hovered: Node %d, Pin %d, Conn %d, Group %d, Reroute %d",
                m_state.hoveredNodeId, m_state.hoveredPinId, m_state.hoveredConnectionId, m_state.hoveredGroupId,
                m_hoveredRerouteId);
        drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), buffer);

        textPos.y += 20;
        sprintf(buffer, "Total reroutes: %zu, Connections: %zu", m_reroutes.size(), m_state.connections.size());
        drawList->AddText(textPos, IM_COL32(200, 200, 200, 255), buffer);
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
