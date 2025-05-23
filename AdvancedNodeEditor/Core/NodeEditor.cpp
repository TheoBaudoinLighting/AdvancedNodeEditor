#include "NodeEditor.h"

#include "imgui_internal.h"

namespace NodeEditorCore {
    std::vector<int> NodeEditor::getEvaluationOrder() const {
        return NodeEvaluator::getEvaluationOrder(const_cast<NodeEditor &>(*this));
    }

    std::vector<UUID> NodeEditor::getEvaluationOrderUUIDs() const {
        std::vector<UUID> result;
        std::vector<int> order = getEvaluationOrder();
        for (int nodeId: order) {
            result.push_back(getNodeUUID(nodeId));
        }
        return result;
    }

    void NodeEditor::setConnectionStyle(ConnectionStyleManager::ConnectionStyle style) {
        auto config = m_connectionStyleManager.getConfig();
        config.style = style;
        m_connectionStyleManager.setConfig(config);
    }

    void NodeEditor::setGridColor(const Color &color) {
        m_state.style.uiColors.grid = color;
    }

    Color NodeEditor::getGridColor() const {
        return m_state.style.uiColors.grid;
    }

    void NodeEditor::setBackgroundColor(const Color &color) {
        m_state.style.uiColors.background = color;
    }

    Color NodeEditor::getBackgroundColor() const {
        return m_state.style.uiColors.background;
    }

    void NodeEditor::setSubgraphDepthColor(int depth, const Color &color) {
        m_depthColors[depth] = color;
    }

    void NodeEditor::setupViewManager() {
        m_viewManager.setBoundingBoxProvider([this](Vec2 &outMin, Vec2 &outMax) {
            outMin = Vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
            outMax = Vec2(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

            for (const auto &node : m_state.nodes) {
                if (!isNodeInCurrentSubgraph(node)) continue;
                outMin.x = std::min(outMin.x, node.position.x);
                outMin.y = std::min(outMin.y, node.position.y);
                outMax.x = std::max(outMax.x, node.position.x + node.size.x);
                outMax.y = std::max(outMax.y, node.position.y + node.size.y);
            }
        });

        m_viewManager.setNodeBoundingBoxProvider([this](int nodeId, Vec2 &outMin, Vec2 &outMax) {
            outMin = Vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
            outMax = Vec2(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

            const Node* node = getNode(nodeId);
            if (node && isNodeInCurrentSubgraph(*node)) {
                outMin.x = node->position.x;
                outMin.y = node->position.y;
                outMax.x = node->position.x + node->size.x;
                outMax.y = node->position.y + node->size.y;
            }
        });

        m_viewManager.setSelectedNodesBoundingBoxProvider([this](Vec2 &outMin, Vec2 &outMax) {
            outMin = Vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
            outMax = Vec2(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

            bool hasSelectedNodes = false;
            for (const auto &node : m_state.nodes) {
                if (!node.selected || !isNodeInCurrentSubgraph(node)) continue;

                outMin.x = std::min(outMin.x, node.position.x);
                outMin.y = std::min(outMin.y, node.position.y);
                outMax.x = std::max(outMax.x, node.position.x + node.size.x);
                outMax.y = std::max(outMax.y, node.position.y + node.size.y);
                hasSelectedNodes = true;
            }
        });
    }

    void NodeEditor::zoomToFit(float padding) {
        Vec2 windowSize(0, 0);

        try {
            if (ImGui::GetCurrentContext() != nullptr) {
                ImGuiContext* context = ImGui::GetCurrentContext();
                if (context && context->CurrentWindow) {
                    ImVec2 size = ImGui::GetWindowSize();
                    windowSize = Vec2(size.x, size.y);
                }
            }
        } catch (...) {
            windowSize = Vec2(1280, 720);
        }

        m_viewManager.setWindowSize(windowSize);

        m_viewManager.zoomToFit(padding);

        m_state.viewPosition = m_viewManager.getViewPosition();
        m_state.viewScale = m_viewManager.getViewScale();
    }

    void NodeEditor::zoomToFitSelected(float padding) {
        ImVec2 windowSize = ImVec2(0, 0);
        if (ImGui::GetCurrentContext() != nullptr && ImGui::GetCurrentWindow() != nullptr) {
            windowSize = ImGui::GetWindowSize();
            m_viewManager.setWindowSize(Vec2(windowSize.x, windowSize.y));
        }
        m_viewManager.zoomToFitSelected(padding);
        m_state.viewPosition = m_viewManager.getViewPosition();
        m_state.viewScale = m_viewManager.getViewScale();
    }

    void NodeEditor::centerView() {
        ImVec2 windowSize = ImVec2(0, 0);
        if (ImGui::GetCurrentContext() != nullptr && ImGui::GetCurrentWindow() != nullptr) {
            windowSize = ImGui::GetWindowSize();
            m_viewManager.setWindowSize(Vec2(windowSize.x, windowSize.y));
        }
        m_viewManager.centerView();
        m_state.viewPosition = m_viewManager.getViewPosition();
        m_state.viewScale = m_viewManager.getViewScale();
    }

    void NodeEditor::smoothCenterView(float duration) {
        ImVec2 windowSize = ImVec2(0, 0);
        if (ImGui::GetCurrentContext() != nullptr && ImGui::GetCurrentWindow() != nullptr) {
            windowSize = ImGui::GetWindowSize();
            m_viewManager.setWindowSize(Vec2(windowSize.x, windowSize.y));
        }

        m_viewManager.setViewPosition(m_state.viewPosition);
        m_viewManager.setViewScale(m_state.viewScale);
        m_viewManager.centerView();

        ViewManager::ViewState currentState(m_state.viewPosition, m_state.viewScale);
        ViewManager::ViewState targetState(m_viewManager.getViewPosition(), m_viewManager.getViewScale());

        m_viewManager.setViewPosition(currentState.position);
        m_viewManager.setViewScale(currentState.scale);
        m_viewManager.startViewTransition(targetState, duration, ViewManager::ViewTransitionType::EaseInOut);
    }

    void NodeEditor::smoothCenterOnNode(int nodeId, float duration) {
        ImVec2 windowSize = ImVec2(0, 0);
        if (ImGui::GetCurrentContext() != nullptr && ImGui::GetCurrentWindow() != nullptr) {
            windowSize = ImGui::GetWindowSize();
            m_viewManager.setWindowSize(Vec2(windowSize.x, windowSize.y));
        }

        const Node* node = getNode(nodeId);
        if (!node) return;

        m_viewManager.setViewPosition(m_state.viewPosition);
        m_viewManager.setViewScale(m_state.viewScale);

        m_viewManager.centerOnNode(nodeId);

        ViewManager::ViewState currentState(m_state.viewPosition, m_state.viewScale);
        ViewManager::ViewState targetState(m_viewManager.getViewPosition(), m_viewManager.getViewScale());

        m_viewManager.setViewPosition(currentState.position);
        m_viewManager.setViewScale(currentState.scale);
        m_viewManager.startViewTransition(targetState, duration, ViewManager::ViewTransitionType::EaseInOut);
    }

    void NodeEditor::smoothCenterOnNodeByUUID(const UUID& uuid, float duration) {
        int nodeId = getNodeId(uuid);
        if (nodeId != -1) {
            smoothCenterOnNode(nodeId, duration);
        }
    }

    void NodeEditor::centerOnNode(int nodeId) {
        ImVec2 windowSize = ImVec2(0, 0);
        if (ImGui::GetCurrentContext() != nullptr && ImGui::GetCurrentWindow() != nullptr) {
            windowSize = ImGui::GetWindowSize();
            m_viewManager.setWindowSize(Vec2(windowSize.x, windowSize.y));
        }
        m_viewManager.centerOnNode(nodeId);
        m_state.viewPosition = m_viewManager.getViewPosition();
        m_state.viewScale = m_viewManager.getViewScale();
    }

    void NodeEditor::centerOnNodeByUUID(const UUID& uuid) {
        int nodeId = getNodeId(uuid);
        if (nodeId != -1) {
            centerOnNode(nodeId);
        }
    }

    void NodeEditor::setConnectionStyle(ConnectionStyle style) {
        NodeEditorCore::ConnectionStyleManager::ConnectionStyle coreStyle;
        switch (style) {
            case ConnectionStyle::Bezier:
                coreStyle = NodeEditorCore::ConnectionStyleManager::ConnectionStyle::Bezier;
                break;
            case ConnectionStyle::StraightLine:
                coreStyle = NodeEditorCore::ConnectionStyleManager::ConnectionStyle::StraightLine;
                break;
            case ConnectionStyle::AngleLine:
                coreStyle = NodeEditorCore::ConnectionStyleManager::ConnectionStyle::AngleLine;
                break;
            case ConnectionStyle::MetroLine:
                coreStyle = NodeEditorCore::ConnectionStyleManager::ConnectionStyle::MetroLine;
                break;
        }
        setConnectionStyle(coreStyle);
    }

    NodeEditor::ConnectionStyle NodeEditor::getConnectionStyle() const {
        auto coreStyle = m_connectionStyleManager.getDefaultStyle();
        switch (coreStyle) {
            case NodeEditorCore::ConnectionStyleManager::ConnectionStyle::Bezier:
                return ConnectionStyle::Bezier;
            case NodeEditorCore::ConnectionStyleManager::ConnectionStyle::StraightLine:
                return ConnectionStyle::StraightLine;
            case NodeEditorCore::ConnectionStyleManager::ConnectionStyle::AngleLine:
                return ConnectionStyle::AngleLine;
            case NodeEditorCore::ConnectionStyleManager::ConnectionStyle::MetroLine:
                return ConnectionStyle::MetroLine;
            default:
                return ConnectionStyle::Bezier;
        }
    }

    void NodeEditor::setConnectionThickness(float thickness) {
        auto config = m_connectionStyleManager.getConfig();
        config.thickness = thickness;
        m_connectionStyleManager.setConfig(config);
    }

    float NodeEditor::getConnectionThickness() const {
        return m_connectionStyleManager.getConfig().thickness;
    }

    void NodeEditor::setConnectionColor(const Color &color) {
        auto config = m_connectionStyleManager.getConfig();
        config.startColor = NodeEditorCore::Color(color.r, color.g, color.b, color.a);
        config.endColor = NodeEditorCore::Color(color.r, color.g, color.b, color.a);
        config.useGradient = false;
        m_connectionStyleManager.setConfig(config);
    }

    void NodeEditor::setConnectionGradient(const Color &startColor, const Color &endColor) {
        auto config = m_connectionStyleManager.getConfig();
        config.startColor = NodeEditorCore::Color(startColor.r, startColor.g, startColor.b, startColor.a);
        config.endColor = NodeEditorCore::Color(endColor.r, endColor.g, endColor.b, endColor.a);
        config.useGradient = true;
        m_connectionStyleManager.setConfig(config);
    }

    void NodeEditor::setConnectionSelectedColor(const Color &color) {
        auto config = m_connectionStyleManager.getConfig();
        config.selectedColor = NodeEditorCore::Color(color.r, color.g, color.b, color.a);
        m_connectionStyleManager.setConfig(config);
    }

    void NodeEditor::enableMinimap(bool enable) {
        m_minimapManager.getConfig().interactable = enable;
        m_minimapManager.setNodePositionProvider([this]() {
            std::vector<std::pair<Vec2, Vec2> > nodes;
            for (const auto &node: m_state.nodes) {
                if (isNodeInCurrentSubgraph(node)) {
                    nodes.push_back(std::make_pair(node.position, node.size));
                }
            }
            return nodes;
        });
        m_minimapManager.setViewportChangeCallback([this](const Vec2 &newViewPos) {
            m_state.viewPosition = newViewPos;
            m_viewManager.setViewPosition(newViewPos);
        });
        updateMinimapBounds();
    }

    bool NodeEditor::isMinimapEnabled() const {
        return m_minimapManager.getConfig().interactable;
    }

    void NodeEditor::setMinimapPosition(const Vec2 &position) {
        auto config = m_minimapManager.getConfig();
        config.position = position;
        m_minimapManager.setConfig(config);
    }

    void NodeEditor::setMinimapSize(const Vec2 &size) {
        auto config = m_minimapManager.getConfig();
        config.size = size;
        m_minimapManager.setConfig(config);
    }

    void NodeEditor::activateConnectionFlowTemporary(int connectionId, float duration) {
        Connection *conn = getConnection(connectionId);
        if (conn) {
            conn->isActive = true;
            m_animationManager.activateConnectionFlow(connectionId, false, duration);
        }
    }

    void NodeEditor::activateConnectionFlowInfinite(int connectionId) {
        Connection *conn = getConnection(connectionId);
        if (conn) {
            conn->isActive = true;
            m_animationManager.activateConnectionFlow(connectionId, true);
        }
    }

    void NodeEditor::deactivateConnectionFlow(int connectionId) {
        Connection *conn = getConnection(connectionId);
        if (conn) {
            conn->isActive = false;
            m_animationManager.deactivateConnectionFlow(connectionId);
        }
    }

    void NodeEditor::activateAllConnectionFlows(bool temporary, float duration) {
        for (auto &connection: m_state.connections) {
            connection.isActive = true;
            m_animationManager.activateConnectionFlow(connection.id, !temporary, duration);
        }
    }

    void NodeEditor::deactivateAllConnectionFlows() {
        for (auto &connection: m_state.connections) {
            connection.isActive = false;
            m_animationManager.deactivateConnectionFlow(connection.id);
        }
    }

    void NodeEditor::toggleConnectionFlow(int connectionId, bool active, bool temporary, float duration) {
        Connection *conn = getConnection(connectionId);
        if (!conn) return;
        conn->isActive = active;
        if (active) {
            m_animationManager.activateConnectionFlow(connectionId, !temporary, duration);
        } else {
            m_animationManager.deactivateConnectionFlow(connectionId);
        }
    }

    std::vector<UUID> NodeEditor::getAllNodeUUIDs() const {
        std::vector<UUID> uuids;
        for (const auto &node: m_state.nodes) {
            uuids.push_back(node.uuid);
        }
        return uuids;
    }

    std::vector<UUID> NodeEditor::getAllConnectionUUIDs() const {
        std::vector<UUID> uuids;
        for (const auto &connection: m_state.connections) {
            uuids.push_back(connection.uuid);
        }
        return uuids;
    }

    std::vector<UUID> NodeEditor::getAllGroupUUIDs() const {
        std::vector<UUID> uuids;
        for (const auto &group: m_state.groups) {
            uuids.push_back(group.uuid);
        }
        return uuids;
    }

    void NodeEditor::setupCommandSystem() {
        if (m_commandsInitialized) return;
        m_commandManager.getBackendRouter().setErrorHandler([this](const std::string &cmd, const std::any &data) {
            std::cerr << "[NodeEditor] Unknown backend command: " << cmd << std::endl;
            struct ErrorData {
                std::string command;
                std::string message;
            };
            dispatchToUI(NodeEditorCommands::UI::ShowError,
                         ErrorData{cmd, "Unknown backend command"});
        });
        m_commandManager.getUIRouter().setErrorHandler([this](const std::string &cmd, const std::any &data) {
            std::cerr << "[NodeEditor] Unknown UI command: " << cmd << std::endl;
        });
        setupBackendCommands();
        setupUICommands();
        m_commandsInitialized = true;
    }

    void NodeEditor::enableCommandLogging(bool enable) {
        m_commandManager.getBackendRouter().setLoggingEnabled(enable);
        m_commandManager.getUIRouter().setLoggingEnabled(enable);
    }

    void NodeEditor::setupBackendCommands() {
        bindToBackend(NodeEditorCommands::Node::Add, [this](const std::any &data) {
            try {
                struct NodeAddData {
                    std::string name;
                    std::string type;
                    Vec2 position;
                    UUID uuid;
                };
                auto nodeData = std::any_cast<NodeAddData>(data);
                int nodeId = addNode(nodeData.name, nodeData.type, nodeData.position, nodeData.uuid);
                if (nodeId >= 0) {
                    dispatchToUI(NodeEditorCommands::UI::ShowNodeCreated, getNodeUUID(nodeId));
                }
            } catch (const std::bad_any_cast &) {
                dispatchToUI(NodeEditorCommands::UI::ShowError,
                             std::string("Invalid data format for node addition"));
            }
        });

        bindToBackend(NodeEditorCommands::Node::Remove, [this](const std::any &data) {
            try {
                if (data.type() == typeid(int)) {
                    int nodeId = std::any_cast<int>(data);
                    UUID nodeUuid = getNodeUUID(nodeId);
                    removeNode(nodeId);
                    dispatchToUI(NodeEditorCommands::UI::ShowNodeRemoved, nodeUuid);
                } else if (data.type() == typeid(UUID)) {
                    UUID nodeUuid = std::any_cast<UUID>(data);
                    removeNodeByUUID(nodeUuid);
                    dispatchToUI(NodeEditorCommands::UI::ShowNodeRemoved, nodeUuid);
                }
            } catch (const std::bad_any_cast &) {
                dispatchToUI(NodeEditorCommands::UI::ShowError,
                             std::string("Invalid data format for node removal"));
            }
        });

        bindToBackend(NodeEditorCommands::Node::Select, [this](const std::any &data) {
            try {
                if (data.type() == typeid(int)) {
                    int nodeId = std::any_cast<int>(data);
                    selectNode(nodeId);
                } else if (data.type() == typeid(UUID)) {
                    UUID nodeUuid = std::any_cast<UUID>(data);
                    selectNodeByUUID(nodeUuid);
                }
            } catch (const std::bad_any_cast &) {
                dispatchToUI(NodeEditorCommands::UI::ShowError,
                             std::string("Invalid data format for node selection"));
            }
        });

        bindToBackend(NodeEditorCommands::Node::SelectAll, [this](const std::any &data) {
            selectAllNodes();
        });

        bindToBackend(NodeEditorCommands::Node::DeselectAll, [this](const std::any &data) {
            deselectAllNodes();
        });

        bindToBackend(NodeEditorCommands::Node::Duplicate, [this](const std::any &data) {
            try {
                if (data.type() == typeid(int)) {
                    int nodeId = std::any_cast<int>(data);
                    duplicateNode(nodeId);
                } else if (data.type() == typeid(UUID)) {
                    UUID nodeUuid = std::any_cast<UUID>(data);
                    int nodeId = getNodeId(nodeUuid);
                    if (nodeId != -1) {
                        duplicateNode(nodeId);
                    }
                }
            } catch (const std::bad_any_cast &) {
                dispatchToUI(NodeEditorCommands::UI::ShowError,
                             std::string("Invalid data format for node duplication"));
            }
        });

        bindToBackend(NodeEditorCommands::Node::SetExecuting, [this](const std::any &data) {
            try {
                struct NodeExecutingData {
                    int nodeId;
                    bool executing;
                };
                auto execData = std::any_cast<NodeExecutingData>(data);
                setNodeExecuting(execData.nodeId, execData.executing);
            } catch (const std::bad_any_cast &) {
                dispatchToUI(NodeEditorCommands::UI::ShowError,
                             std::string("Invalid data format for setting node execution state"));
            }
        });

        bindToBackend(NodeEditorCommands::Node::Move, [this](const std::any &data) {
            try {
                struct NodeMoveData {
                    int nodeId;
                    Vec2 position;
                };
                auto moveData = std::any_cast<NodeMoveData>(data);
                Node *node = getNode(moveData.nodeId);
                if (node) {
                    node->position = moveData.position;
                }
            } catch (const std::bad_any_cast &) {
                dispatchToUI(NodeEditorCommands::UI::ShowError,
                             std::string("Invalid data format for moving node"));
            }
        });


        bindToBackend(NodeEditorCommands::Connection::Add, [this](const std::any &data) {
            try {
                struct ConnectionAddData {
                    int startNodeId;
                    int startPinId;
                    int endNodeId;
                    int endPinId;
                    UUID uuid;
                };
                auto connData = std::any_cast<ConnectionAddData>(data);
                int connId = addConnection(connData.startNodeId, connData.startPinId,
                                           connData.endNodeId, connData.endPinId, connData.uuid);
                if (connId >= 0) {
                    dispatchToUI(NodeEditorCommands::UI::ShowConnectionCreated, getConnectionUUID(connId));
                }
            } catch (const std::bad_any_cast &) {
                dispatchToUI(NodeEditorCommands::UI::ShowError,
                             std::string("Invalid data format for connection addition"));
            }
        });

        bindToBackend(NodeEditorCommands::Connection::Remove, [this](const std::any &data) {
            try {
                if (data.type() == typeid(int)) {
                    int connId = std::any_cast<int>(data);
                    UUID connUuid = getConnectionUUID(connId);
                    removeConnection(connId);
                    dispatchToUI(NodeEditorCommands::UI::ShowConnectionRemoved, connUuid);
                } else if (data.type() == typeid(UUID)) {
                    UUID connUuid = std::any_cast<UUID>(data);
                    removeConnectionByUUID(connUuid);
                    dispatchToUI(NodeEditorCommands::UI::ShowConnectionRemoved, connUuid);
                }
            } catch (const std::bad_any_cast &) {
                dispatchToUI(NodeEditorCommands::UI::ShowError,
                             std::string("Invalid data format for connection removal"));
            }
        });

        bindToBackend(NodeEditorCommands::Connection::ActivateFlow, [this](const std::any &data) {
            try {
                struct FlowActivationData {
                    int connectionId;
                    bool temporary;
                    float duration;
                };
                auto flowData = std::any_cast<FlowActivationData>(data);
                if (flowData.temporary) {
                    activateConnectionFlowTemporary(flowData.connectionId, flowData.duration);
                } else {
                    activateConnectionFlowInfinite(flowData.connectionId);
                }
            } catch (const std::bad_any_cast &) {
                dispatchToUI(NodeEditorCommands::UI::ShowError,
                             std::string("Invalid data format for flow activation"));
            }
        });

        bindToBackend(NodeEditorCommands::Connection::DeactivateFlow, [this](const std::any &data) {
            try {
                if (data.type() == typeid(int)) {
                    int connId = std::any_cast<int>(data);
                    deactivateConnectionFlow(connId);
                } else if (data.type() == typeid(UUID)) {
                    UUID connUuid = std::any_cast<UUID>(data);
                    int connId = getConnectionId(connUuid);
                    if (connId >= 0) {
                        deactivateConnectionFlow(connId);
                    }
                }
            } catch (const std::bad_any_cast &) {
                dispatchToUI(NodeEditorCommands::UI::ShowError,
                             std::string("Invalid data format for flow deactivation"));
            }
        });

        bindToBackend(NodeEditorCommands::Connection::Select, [this](const std::any &data) {
            try {
                if (data.type() == typeid(int)) {
                    int connectionId = std::any_cast<int>(data);
                    selectConnection(connectionId);
                } else if (data.type() == typeid(UUID)) {
                    UUID connectionUuid = std::any_cast<UUID>(data);
                    selectConnectionByUUID(connectionUuid);
                }
            } catch (const std::bad_any_cast &) {
                dispatchToUI(NodeEditorCommands::UI::ShowError,
                             std::string("Invalid data format for connection selection"));
            }
        });

        bindToBackend(NodeEditorCommands::Connection::Deselect, [this](const std::any &data) {
            try {
                if (data.type() == typeid(int)) {
                    int connectionId = std::any_cast<int>(data);
                    deselectConnection(connectionId);
                } else if (data.type() == typeid(UUID)) {
                    UUID connectionUuid = std::any_cast<UUID>(data);
                    deselectConnectionByUUID(connectionUuid);
                }
            } catch (const std::bad_any_cast &) {
                dispatchToUI(NodeEditorCommands::UI::ShowError,
                             std::string("Invalid data format for connection deselection"));
            }
        });

        bindToBackend(NodeEditorCommands::Connection::ActivateAllFlows, [this](const std::any &data) {
            try {
                struct FlowActivationData {
                    bool temporary;
                    float duration;
                };
                auto flowData = std::any_cast<FlowActivationData>(data);
                activateAllConnectionFlows(flowData.temporary, flowData.duration);
            } catch (const std::bad_any_cast &) {
                dispatchToUI(NodeEditorCommands::UI::ShowError,
                             std::string("Invalid data format for activating all flows"));
            }
        });

        bindToBackend(NodeEditorCommands::Connection::DeactivateAllFlows, [this](const std::any &data) {
            deactivateAllConnectionFlows();
        });

        bindToBackend(NodeEditorCommands::Group::Add, [this](const std::any &data) {
            try {
                struct GroupAddData {
                    std::string name;
                    Vec2 position;
                    Vec2 size;
                    UUID uuid;
                };
                auto groupData = std::any_cast<GroupAddData>(data);
                addGroup(groupData.name, groupData.position, groupData.size, groupData.uuid);
            } catch (const std::bad_any_cast &) {
                dispatchToUI(NodeEditorCommands::UI::ShowError,
                             std::string("Invalid data format for group addition"));
            }
        });

        bindToBackend(NodeEditorCommands::Group::Add, [this](const std::any &data) {
            try {
                struct GroupAddData {
                    std::string name;
                    Vec2 position;
                    Vec2 size;
                    UUID uuid;
                };
                auto groupData = std::any_cast<GroupAddData>(data);
                addGroup(groupData.name, groupData.position, groupData.size, groupData.uuid);
            } catch (const std::bad_any_cast &) {
                dispatchToUI(NodeEditorCommands::UI::ShowError,
                             std::string("Invalid data format for group addition"));
            }
        });

        bindToBackend(NodeEditorCommands::Group::Remove, [this](const std::any &data) {
            try {
                if (data.type() == typeid(int)) {
                    int groupId = std::any_cast<int>(data);
                    removeGroup(groupId);
                } else if (data.type() == typeid(UUID)) {
                    UUID groupUuid = std::any_cast<UUID>(data);
                    removeGroupByUUID(groupUuid);
                }
            } catch (const std::bad_any_cast &) {
                dispatchToUI(NodeEditorCommands::UI::ShowError,
                             std::string("Invalid data format for group removal"));
            }
        });

        bindToBackend(NodeEditorCommands::Group::AddNode, [this](const std::any &data) {
            try {
                struct GroupAddNodeData {
                    int nodeId;
                    int groupId;
                };
                auto addData = std::any_cast<GroupAddNodeData>(data);
                addNodeToGroup(addData.nodeId, addData.groupId);
            } catch (const std::bad_any_cast &) {
                dispatchToUI(NodeEditorCommands::UI::ShowError,
                             std::string("Invalid data format for adding node to group"));
            }
        });

        bindToBackend(NodeEditorCommands::Group::RemoveNode, [this](const std::any &data) {
            try {
                struct GroupRemoveNodeData {
                    int nodeId;
                    int groupId;
                };
                auto removeData = std::any_cast<GroupRemoveNodeData>(data);
                removeNodeFromGroup(removeData.nodeId, removeData.groupId);
            } catch (const std::bad_any_cast &) {
                dispatchToUI(NodeEditorCommands::UI::ShowError,
                             std::string("Invalid data format for removing node from group"));
            }
        });

        bindToBackend(NodeEditorCommands::View::ZoomToFit, [this](const std::any &data) {
            try {
                float padding = 50.0f;
                if (data.has_value() && data.type() == typeid(float)) {
                    padding = std::any_cast<float>(data);
                }
                zoomToFit(padding);
            } catch (const std::bad_any_cast &) {
                zoomToFit();
            }
        });

        bindToBackend(NodeEditorCommands::View::CenterOnNode, [this](const std::any &data) {
            try {
                if (data.type() == typeid(int)) {
                    int nodeId = std::any_cast<int>(data);
                    centerOnNode(nodeId);
                } else if (data.type() == typeid(UUID)) {
                    UUID nodeUuid = std::any_cast<UUID>(data);
                    centerOnNodeByUUID(nodeUuid);
                }
            } catch (const std::bad_any_cast &) {
                dispatchToUI(NodeEditorCommands::UI::ShowError,
                             std::string("Invalid data format for centering view on node"));
            }
        });

        bindToBackend(NodeEditorCommands::View::ZoomToSelected, [this](const std::any &data) {
            try {
                float padding = 50.0f;
                if (data.has_value() && data.type() == typeid(float)) {
                    padding = std::any_cast<float>(data);
                }
                zoomToFitSelected(padding);
            } catch (const std::bad_any_cast &) {
                zoomToFitSelected();
            }
        });

        bindToBackend(NodeEditorCommands::View::CenterView, [this](const std::any &data) {
            try {
                float duration = 0.3f;
                if (data.has_value() && data.type() == typeid(float)) {
                    duration = std::any_cast<float>(data);
                }
                smoothCenterView(duration);
            } catch (const std::bad_any_cast &) {
                smoothCenterView();
            }
        });

        bindToBackend(NodeEditorCommands::View::SetPosition, [this](const std::any &data) {
            try {
                Vec2 position = std::any_cast<Vec2>(data);
                setViewPosition(position);
            } catch (const std::bad_any_cast &) {
                dispatchToUI(NodeEditorCommands::UI::ShowError,
                             std::string("Invalid data format for setting view position"));
            }
        });

        bindToBackend(NodeEditorCommands::View::SetScale, [this](const std::any &data) {
            try {
                float scale = std::any_cast<float>(data);
                setViewScale(scale);
            } catch (const std::bad_any_cast &) {
                dispatchToUI(NodeEditorCommands::UI::ShowError,
                             std::string("Invalid data format for setting view scale"));
            }
        });

        bindToBackend(NodeEditorCommands::Evaluation::Evaluate, [this](const std::any &data) {
            std::vector<int> order = getEvaluationOrder();
            std::vector<UUID> uuidOrder = getEvaluationOrderUUIDs();
            dispatchToUI(NodeEditorCommands::UI::ShowEvaluationResult, uuidOrder);
        });

        bindToBackend(NodeEditorCommands::Evaluation::GetOrder, [this](const std::any &data) {
            std::vector<int> order = getEvaluationOrder();
            std::vector<UUID> uuidOrder = getEvaluationOrderUUIDs();
            dispatchToUI(NodeEditorCommands::UI::ShowEvaluationResult, uuidOrder);
        });

        bindToBackend(NodeEditorCommands::Style::SetGridColor, [this](const std::any &data) {
            try {
                Color color = std::any_cast<Color>(data);
                setGridColor(color);
            } catch (const std::bad_any_cast &) {
                dispatchToUI(NodeEditorCommands::UI::ShowError,
                             std::string("Invalid data format for setting grid color"));
            }
        });

        bindToBackend(NodeEditorCommands::Style::SetBackgroundColor, [this](const std::any &data) {
            try {
                Color color = std::any_cast<Color>(data);
                setBackgroundColor(color);
            } catch (const std::bad_any_cast &) {
                dispatchToUI(NodeEditorCommands::UI::ShowError,
                             std::string("Invalid data format for setting background color"));
            }
        });

        bindToBackend(NodeEditorCommands::Style::SetConnectionStyle, [this](const std::any &data) {
            try {
                ConnectionStyle style = std::any_cast<ConnectionStyle>(data);
                setConnectionStyle(style);
            } catch (const std::bad_any_cast &) {
                dispatchToUI(NodeEditorCommands::UI::ShowError,
                             std::string("Invalid data format for setting connection style"));
            }
        });

        bindToBackend(NodeEditorCommands::Style::SetConnectionColor, [this](const std::any &data) {
            try {
                Color color = std::any_cast<Color>(data);
                setConnectionColor(color);
            } catch (const std::bad_any_cast &) {
                dispatchToUI(NodeEditorCommands::UI::ShowError,
                             std::string("Invalid data format for setting connection color"));
            }
        });

        bindToBackend(NodeEditorCommands::Subgraph::Create, [this](const std::any &data) {
            try {
                struct SubgraphCreateData {
                    std::string name;
                    UUID uuid;
                    bool createDefaultNodes;
                };
                auto subgraphData = std::any_cast<SubgraphCreateData>(data);
                createSubgraph(subgraphData.name, subgraphData.uuid, subgraphData.createDefaultNodes);
            } catch (const std::bad_any_cast &) {
                dispatchToUI(NodeEditorCommands::UI::ShowError,
                             std::string("Invalid data format for subgraph creation"));
            }
        });

        bindToBackend(NodeEditorCommands::Subgraph::Remove, [this](const std::any &data) {
            try {
                if (data.type() == typeid(int)) {
                    int subgraphId = std::any_cast<int>(data);
                    removeSubgraph(subgraphId);
                } else if (data.type() == typeid(UUID)) {
                    UUID subgraphUuid = std::any_cast<UUID>(data);
                    int subgraphId = getSubgraphId(subgraphUuid);
                    if (subgraphId != -1) {
                        removeSubgraph(subgraphId);
                    }
                }
            } catch (const std::bad_any_cast &) {
                dispatchToUI(NodeEditorCommands::UI::ShowError,
                             std::string("Invalid data format for subgraph removal"));
            }
        });

        bindToBackend(NodeEditorCommands::Subgraph::Enter, [this](const std::any &data) {
            try {
                if (data.type() == typeid(int)) {
                    int subgraphId = std::any_cast<int>(data);
                    enterSubgraph(subgraphId);
                } else if (data.type() == typeid(UUID)) {
                    UUID subgraphUuid = std::any_cast<UUID>(data);
                    enterSubgraphByUUID(subgraphUuid);
                }
            } catch (const std::bad_any_cast &) {
                dispatchToUI(NodeEditorCommands::UI::ShowError,
                             std::string("Invalid data format for entering subgraph"));
            }
        });

        bindToBackend(NodeEditorCommands::Subgraph::Exit, [this](const std::any &data) {
            exitSubgraph();
        });
    }

    void NodeEditor::setupUICommands() {
        bindToUI(NodeEditorCommands::UI::ShowNodeCreated, [](const std::any &data) {
            try {
                UUID nodeUuid = std::any_cast<UUID>(data);
                std::cout << "Node created: " << nodeUuid << std::endl;
            } catch (const std::bad_any_cast &) {
                std::cerr << "Invalid data format for node creation display" << std::endl;
            }
        });

        bindToUI(NodeEditorCommands::UI::ShowConnectionCreated, [](const std::any &data) {
            try {
                UUID connUuid = std::any_cast<UUID>(data);
                std::cout << "Connection created: " << connUuid << std::endl;
            } catch (const std::bad_any_cast &) {
                std::cerr << "Invalid data format for connection creation display" << std::endl;
            }
        });

        bindToUI(NodeEditorCommands::UI::ShowError, [](const std::any &data) {
            try {
                if (data.type() == typeid(std::string)) {
                    std::string message = std::any_cast<std::string>(data);
                    std::cerr << "Error: " << message << std::endl;
                } else {
                    struct ErrorData {
                        std::string command;
                        std::string message;
                    };
                    auto error = std::any_cast<ErrorData>(data);
                    std::cerr << "Error (" << error.command << "): " << error.message << std::endl;
                }
            } catch (const std::bad_any_cast &) {
                std::cerr << "Unknown error" << std::endl;
            }
        });

        bindToUI(NodeEditorCommands::UI::ShowEvaluationResult, [](const std::any &data) {
            try {
                auto uuidOrder = std::any_cast<std::vector<UUID> >(data);
                std::cout << "Evaluation order: ";
                for (const auto &uuid: uuidOrder) {
                    std::cout << uuid << " ";
                }
                std::cout << std::endl;
            } catch (const std::bad_any_cast &) {
                std::cerr << "Invalid data format for evaluation result display" << std::endl;
            }
        });

        bindToUI(NodeEditorCommands::UI::ShowNodeRemoved, [](const std::any& data) {
        try {
            UUID nodeUuid = std::any_cast<UUID>(data);
            std::cout << "Node removed: " << nodeUuid << std::endl;
        } catch(const std::bad_any_cast&) {
            std::cerr << "Invalid data format for node removal display" << std::endl;
        }
    });

        bindToUI(NodeEditorCommands::UI::ShowConnectionRemoved, [](const std::any& data) {
            try {
                UUID connUuid = std::any_cast<UUID>(data);
                std::cout << "Connection removed: " << connUuid << std::endl;
            } catch(const std::bad_any_cast&) {
                std::cerr << "Invalid data format for connection removal display" << std::endl;
            }
        });

        bindToUI(NodeEditorCommands::UI::UpdateNodeList, [](const std::any& data) {
            try {
                auto nodes = std::any_cast<std::vector<UUID>>(data);
                std::cout << "Updated node list, count: " << nodes.size() << std::endl;
            } catch(const std::bad_any_cast&) {
                std::cerr << "Invalid data format for node list update" << std::endl;
            }
        });

        bindToUI(NodeEditorCommands::UI::UpdateConnectionList, [](const std::any& data) {
            try {
                auto connections = std::any_cast<std::vector<UUID>>(data);
                std::cout << "Updated connection list, count: " << connections.size() << std::endl;
            } catch(const std::bad_any_cast&) {
                std::cerr << "Invalid data format for connection list update" << std::endl;
            }
        });
    }

    void NodeEditor::handleErrors(const std::string &command, const std::any &data) {
        struct ErrorData {
            std::string command;
            std::string message;
        };
        dispatchToUI(NodeEditorCommands::UI::ShowError,
                     ErrorData{command, "Error while executing command"});
    }

    void NodeEditor::bindToBackend(const std::string &command, std::function<void(const std::any &)> handler) {
        m_commandManager.bindToBackend(command, handler);
    }

    void NodeEditor::bindToUI(const std::string &command, std::function<void(const std::any &)> handler) {
        m_commandManager.bindToUI(command, handler);
    }

    void NodeEditor::dispatchToBackend(const std::string &command, const std::any &data) {
        m_commandManager.dispatchToBackend(command, data);
    }

    void NodeEditor::dispatchToUI(const std::string &command, const std::any &data) {
        m_commandManager.dispatchToUI(command, data);
    }
}
