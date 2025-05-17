#include "../../NodeEditor.h"
#include "../../Core/Style/InteractionMode.h"
#include <algorithm>
#include <cfloat>

namespace NodeEditorCore {
    void NodeEditor::processInteraction() {
        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();

        // Gestion des événements de souris
        bool isMouseDoubleClicked = ImGui::IsMouseDoubleClicked(0);
        bool isMouseClicked = ImGui::IsMouseClicked(0);
        bool isMouseReleased = ImGui::IsMouseReleased(0);
        bool isMouseDragging = ImGui::IsMouseDragging(0);
        bool isMiddleMousePressed = ImGui::IsMouseDown(2);

        // Mise à jour des éléments survolés avant toute interaction
        updateHoveredElements(mousePos);

        // Gestion du double-clic sur les nœuds pour entrer/sortir des subgraphs
        if (isMouseDoubleClicked && m_state.hoveredNodeId >= 0) {
            Node* node = getNode(m_state.hoveredNodeId);
            if (node && node->isSubgraph) {
                // Double-clic sur un nœud subgraph: entrer dans le subgraph
                enterSubgraph(node->subgraphId);
                return;
            } else if (node && m_state.currentSubgraphId >= 0) {
                // Dans un subgraph, vérifier si on a cliqué sur les nœuds Input/Output
                Subgraph* subgraph = getSubgraph(m_state.currentSubgraphId);
                if (subgraph) {
                    int inputNodeId = subgraph->metadata.getAttribute<int>("inputNodeId", -1);
                    int outputNodeId = subgraph->metadata.getAttribute<int>("outputNodeId", -1);
                    if (node->id == inputNodeId || node->id == outputNodeId) {
                        // Double-clic sur le nœud d'entrée ou de sortie: sortir du subgraph
                        exitSubgraph();
                        return;
                    }
                }
            }
        }

        // Gestion du déplacement de la vue avec le bouton du milieu
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

        // Traitement des clics et sélections
        if (isMouseClicked) {
            // Priorité aux pins et aux nœuds
            if (m_state.hoveredPinId >= 0 && m_state.hoveredNodeId >= 0) {
                // Commencer une connexion à partir d'un pin
                startConnectionDrag(m_state.hoveredNodeId, m_state.hoveredPinId);
            } else if (m_state.hoveredNodeId >= 0) {
                // Sélectionner ou déplacer un nœud
                Node* node = getNode(m_state.hoveredNodeId);
                if (node) {
                    selectNode(m_state.hoveredNodeId, ImGui::GetIO().KeyCtrl);
                    startNodeDrag(m_state.hoveredNodeId, mousePos);
                }
            } else if (m_state.hoveredConnectionId >= 0) {
                // Sélectionner une connexion
                selectConnection(m_state.hoveredConnectionId, ImGui::GetIO().KeyCtrl);
            } else if (m_state.hoveredGroupId >= 0) {
                // Interagir avec un groupe
                startGroupInteraction(mousePos);
            } else {
                // Clic sur le canevas vide - sélection en boîte ou désélection
                startBoxSelect(mousePos);
                if (!ImGui::GetIO().KeyCtrl) {
                    deselectAllNodes();
                    deselectAllConnections();
                }
            }
        }

        // Mise à jour de l'interaction en cours
        if (m_state.interactionMode != InteractionMode::None) {
            if (isMouseDragging) {
                updateCurrentInteraction(mousePos);
            }

            if (isMouseReleased) {
                endCurrentInteraction();
            }
        }

        // Gestion du zoom, de la suppression, etc.
        processZoom(mousePos);
        processDeleteKeyPress();

        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            // Annuler l'interaction en cours
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

        // Convertir le delta de l'espace écran à l'espace canevas
        Vec2 scaledDelta = Vec2(mouseDelta.x / m_state.viewScale, mouseDelta.y / m_state.viewScale);

        // Mettre à jour la position de tous les nœuds sélectionnés
        for (auto& node : m_state.nodes) {
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

    void NodeEditor::startNodeDrag(int nodeId, const ImVec2& mousePos) {
        Node* node = getNode(nodeId);
        if (!node) return;

        m_state.interactionMode = InteractionMode::DragNode;
        m_state.activeNodeId = nodeId;
        m_state.activeNodeUuid = node->uuid;
        m_state.dragStart = Vec2(mousePos.x, mousePos.y);

        if (!node->selected) {
            selectNode(nodeId, ImGui::GetIO().KeyCtrl);
        }

        m_state.draggedNodePositions.clear();
        for (const auto& node : m_state.nodes) {
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
        m_state.interactionMode = InteractionMode::None;
        m_state.activeNodeId = -1;
        m_state.activeConnectionId = -1;
        m_state.activeGroupId = -1;
        m_state.connectingNodeId = -1;
        m_state.connectingPinId = -1;
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

    void NodeEditor::updateHoveredElements(const ImVec2& mousePos) {
        // Réinitialisation des éléments survolés
        m_state.hoveredNodeId = -1;
        m_state.hoveredNodeUuid = "";
        m_state.hoveredPinId = -1;
        m_state.hoveredPinUuid = "";
        m_state.hoveredConnectionId = -1;
        m_state.hoveredConnectionUuid = "";
        m_state.hoveredGroupId = -1;
        m_state.hoveredGroupUuid = "";

        ImVec2 canvasPos = ImGui::GetCursorScreenPos();

        // Recherche des connexions survolées
        for (const auto& connection : m_state.connections) {
            if (!isNodeInCurrentSubgraph(*getNode(connection.startNodeId)) ||
                !isNodeInCurrentSubgraph(*getNode(connection.endNodeId))) {
                continue;
            }

            if (isConnectionHovered(connection, canvasPos)) {
                m_state.hoveredConnectionId = connection.id;
                m_state.hoveredConnectionUuid = connection.uuid;
                return;
            }
        }

        // Recherche des nœuds et pins survolés
        for (const auto& node : m_state.nodes) {
            if (!isNodeInCurrentSubgraph(node)) continue;

            ImVec2 nodePos = canvasToScreen(node.position).toImVec2();
            ImVec2 nodeSize = Vec2(node.size.x * m_state.viewScale, node.size.y * m_state.viewScale).toImVec2();

            // Vérifier si la souris est sur un pin
            for (const auto& pin : node.inputs) {
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
                    return;
                }
            }

            for (const auto& pin : node.outputs) {
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
                    return;
                }
            }

            // Vérifier si la souris est sur le nœud
            if (isPointInRect(mousePos, nodePos, ImVec2(nodePos.x + nodeSize.x, nodePos.y + nodeSize.y))) {
                m_state.hoveredNodeId = node.id;
                m_state.hoveredNodeUuid = node.uuid;
                return;
            }
        }

        // Recherche des groupes survolés
        for (const auto& group : m_state.groups) {
            if ((m_state.currentSubgraphId == -1 && group.getSubgraphId() == -1) ||
                (m_state.currentSubgraphId >= 0 && group.getSubgraphId() == m_state.currentSubgraphId)) {

                ImVec2 groupPos = canvasToScreen(group.position).toImVec2();
                ImVec2 groupSize = Vec2(group.size.x * m_state.viewScale, group.size.y * m_state.viewScale).toImVec2();

                if (isPointInRect(mousePos, groupPos, ImVec2(groupPos.x + groupSize.x, groupPos.y + groupSize.y))) {
                    m_state.hoveredGroupId = group.id;
                    m_state.hoveredGroupUuid = group.uuid;
                    return;
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
        if (!m_state.connecting || m_state.connectingNodeId == -1 || m_state.connectingPinId == -1) return;

        ImVec2 mousePos = ImGui::GetMousePos();

        const Node *sourceNode = getNode(m_state.connectingNodeId);
        if (!sourceNode) return;

        const Pin *sourcePinInternal = sourceNode->findPin(m_state.connectingPinId);
        if (!sourcePinInternal) return;

        bool isSourceInput = sourcePinInternal->isInput;

        m_state.magnetPinNodeId = -1;
        m_state.magnetPinId = -1;
        m_state.magnetPinNodeUuid = "";
        m_state.magnetPinUuid = "";
        m_state.canConnectToMagnetPin = true;

        float closestDist = m_state.magnetThreshold * m_state.magnetThreshold;

        for (const auto &node: m_state.nodes) {
            if (node.id == m_state.connectingNodeId) continue;

            const auto &pins = isSourceInput ? node.outputs : node.inputs;

            for (const auto &pinInternal: pins) {
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

                if (dist < closestDist) {
                    Pin sourceApiPin;
                    sourceApiPin.id = sourcePinInternal->id;
                    sourceApiPin.name = sourcePinInternal->name;
                    sourceApiPin.isInput = sourcePinInternal->isInput;
                    sourceApiPin.type = static_cast<PinType>(sourcePinInternal->type);
                    sourceApiPin.shape = static_cast<PinShape>(sourcePinInternal->shape);

                    bool canConnect = canCreateConnection(isSourceInput ? apiPin : sourceApiPin,
                                                          isSourceInput ? sourceApiPin : apiPin);

                    m_state.magnetPinNodeId = node.id;
                    m_state.magnetPinId = pinInternal.id;
                    m_state.magnetPinNodeUuid = node.uuid;
                    m_state.magnetPinUuid = pinInternal.uuid;
                    m_state.canConnectToMagnetPin = canConnect;
                    closestDist = dist;
                }
            }
        }

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            if (m_state.magnetPinNodeId != -1 && m_state.canConnectToMagnetPin) {
                if (isSourceInput) {
                    createConnection(m_state.magnetPinNodeId, m_state.magnetPinId,
                                     m_state.connectingNodeId, m_state.connectingPinId);
                } else {
                    createConnection(m_state.connectingNodeId, m_state.connectingPinId,
                                     m_state.magnetPinNodeId, m_state.magnetPinId);
                }
            }

            m_state.connecting = false;
            m_state.connectingNodeId = -1;
            m_state.connectingPinId = -1;
        }
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

        float distance = std::abs(p2.y - p1.y);
        float cpOffset = std::max(50.0f, distance * 0.5f);

        ImVec2 cp1 = ImVec2(p1.x, p1.y + cpOffset);
        ImVec2 cp2 = ImVec2(p2.x, p2.y - cpOffset);

        return isPointNearCubicBezier(mousePos, p1, cp1, cp2, p2, 5.0f);
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

    void NodeEditor::processZoom(const ImVec2& mousePos) {
        float zoom = ImGui::GetIO().MouseWheel;

        // Ne rien faire si pas de zoom détecté
        if (std::abs(zoom) < 0.01f) return;

        // Calculer le point de zoom dans l'espace du canevas
        Vec2 canvasPos = screenToCanvas(Vec2(mousePos.x, mousePos.y));

        // Facteur de zoom - plus petit pour un contrôle plus fin
        float zoomFactor = 1.1f;
        float newScale = m_state.viewScale;

        if (zoom > 0) {
            // Zoom in
            newScale *= zoomFactor;
        } else {
            // Zoom out
            newScale /= zoomFactor;
        }

        // Limiter l'échelle de zoom
        newScale = std::max(0.1f, std::min(newScale, 3.0f));

        // Ajuster la position de la vue pour maintenir le point de zoom fixe
        float scaleRatio = newScale / m_state.viewScale;
        Vec2 newViewPos = Vec2(
            mousePos.x - (mousePos.x - m_state.viewPosition.x) * scaleRatio,
            mousePos.y - (mousePos.y - m_state.viewPosition.y) * scaleRatio
        );

        // Mettre à jour l'état
        m_state.viewScale = newScale;
        m_state.viewPosition = newViewPos;

        // Mettre à jour le gestionnaire de vue pour qu'il soit synchronisé
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
