#include "../../NodeEditor.h"
#include "../../Core/Style/InteractionMode.h"
#include <algorithm>

namespace NodeEditorCore {
    NodeEditor::State::State()
        : viewPosition(0.0f, 0.0f), viewScale(1.0f)
          , nextNodeId(1), nextPinId(1), nextConnectionId(1), nextGroupId(1)
          , hoveredNodeId(-1), hoveredNodeUuid(""), hoveredPinId(-1), hoveredPinUuid("")
          , hoveredConnectionId(-1), hoveredConnectionUuid(""), hoveredGroupId(-1), hoveredGroupUuid("")
          , activeNodeId(-1), activeNodeUuid(""), activeConnectionId(-1), activeConnectionUuid("")
          , activeGroupId(-1), activeGroupUuid("")
          , connecting(false), connectingNodeId(-1), connectingNodeUuid(""), connectingPinId(-1), connectingPinUuid("")
          , dragging(false), boxSelecting(false)
          , currentSubgraphId(-1), currentSubgraphUuid("")
          , interactionMode(InteractionMode::None)
          , contextMenuNodeId(-1), contextMenuNodeUuid(""), contextMenuConnectionId(-1), contextMenuConnectionUuid("")
          , contextMenuGroupId(-1), contextMenuGroupUuid(""), contextMenuPinId(-1), contextMenuPinUuid("")
          , dragStart(0.0f, 0.0f), groupStartSize(0.0f, 0.0f), contextMenuPos(0.0f, 0.0f) {
        nodeUuidMap.clear();
        connectionUuidMap.clear();
        groupUuidMap.clear();
    }

    NodeEditor::NodeEditor()
        : m_debugMode(false)
          , m_viewManager()
          , m_titleManager()
          , m_breadcrumbManager()
          , m_connectionStyleManager()
          , m_nodeBoundingBoxManager(std::make_shared<NodeBoundingBoxManager>())
          , m_nodeAvoidanceEnabled(false) {
        m_state = State();

        m_viewManager.setMinZoom(0.1f);
        m_viewManager.setMaxZoom(5.0f);

        GraphTitleManager::TitleConfig titleConfig;
        titleConfig.text = "Graph";
        titleConfig.position = GraphTitleManager::TitlePosition::TopCenter;
        titleConfig.style = GraphTitleManager::TitleStyle::Unreal;
        m_titleManager.setConfig(titleConfig);

        GraphTitleManager::TitleConfig breadcrumbConfig;
        breadcrumbConfig.text = "";
        breadcrumbConfig.position = GraphTitleManager::TitlePosition::TopLeft;
        breadcrumbConfig.style = GraphTitleManager::TitleStyle::Minimal;
        breadcrumbConfig.showSubgraphPath = true;
        breadcrumbConfig.fontSize = 14.0f;
        m_breadcrumbManager.setConfig(breadcrumbConfig);

        ConnectionStyleManager::ConnectionConfig connectionConfig;
        connectionConfig.style = ConnectionStyleManager::ConnectionStyle::Bezier;
        m_connectionStyleManager.setConfig(connectionConfig);

        m_depthColors[0] = Color(0.0f, 0.0f, 0.0f, 0.0f);
        m_depthColors[1] = Color(0.2f, 0.6f, 0.8f, 0.7f);
        m_depthColors[2] = Color(0.8f, 0.6f, 0.2f, 0.7f);
        m_depthColors[3] = Color(0.2f, 0.8f, 0.4f, 0.7f);
        m_depthColors[4] = Color(0.8f, 0.3f, 0.3f, 0.7f);
        m_depthColors[5] = Color(0.5f, 0.3f, 0.8f, 0.7f);

        m_connectionStyleManager.setBoundingBoxManager(m_nodeBoundingBoxManager);
    }

    NodeEditor::~NodeEditor() {
    }

    void NodeEditor::beginFrame() {
        m_state.hoveredNodeId = -1;
        m_state.hoveredPinId = -1;
        m_state.hoveredConnectionId = -1;
        m_state.hoveredGroupId = -1;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    }

    void NodeEditor::endFrame() {
        ImGui::PopStyleVar();
    }

    int NodeEditor::addNode(const std::string &name, const std::string &type, const Vec2 &pos, const UUID &uuid) {
        int nodeId = m_state.nextNodeId++;
        Node node(uuid.empty() ? generateUUID() : uuid, nodeId, name, type, pos);

        m_state.nodes.push_back(node);
        updateNodeUuidMap();

        if (m_state.nodeCreatedCallback) {
            m_state.nodeCreatedCallback(nodeId, node.uuid);
        }

        return nodeId;
    }

    UUID NodeEditor::addNodeWithUUID(const std::string &name, const std::string &type, const Vec2 &position) {
        int nodeId = addNode(name, type, position);
        return getNodeUUID(nodeId);
    }

    void NodeEditor::removeNode(int nodeId) {
        auto it = std::find_if(m_state.nodes.begin(), m_state.nodes.end(),
                              [nodeId](const Node &node) { return node.id == nodeId; });

        if (it != m_state.nodes.end()) {
            if (it->isProtected) {
                return;
            }

            for (const auto& subgraphPair : m_subgraphs) {
                int inputNodeId = subgraphPair.second->metadata.getAttribute<int>("inputNodeId", -1);
                int outputNodeId = subgraphPair.second->metadata.getAttribute<int>("outputNodeId", -1);

                if (nodeId == inputNodeId || nodeId == outputNodeId) {
                    return;
                }
            }

            m_state.connections.erase(
                std::remove_if(m_state.connections.begin(), m_state.connections.end(),
                             [nodeId](const Connection &conn) {
                                 return conn.startNodeId == nodeId || conn.endNodeId == nodeId;
                             }),
                m_state.connections.end());

            if (it->groupId >= 0) {
                auto groupIt = std::find_if(m_state.groups.begin(), m_state.groups.end(),
                                          [groupId = it->groupId](const Group& group) {
                                              return group.id == groupId;
                                          });
                if (groupIt != m_state.groups.end()) {
                    groupIt->nodes.erase(nodeId);
                }
            }

            if (m_state.nodeRemovedCallback) {
                m_state.nodeRemovedCallback(nodeId, it->uuid);
            }

            m_state.nodes.erase(it);
            updateNodeUuidMap();
        }
    }



    const std::vector<Node> &NodeEditor::getNodes() const {
        return m_state.nodes;
    }

    const Node *NodeEditor::getNode(int nodeId) const {
        for (const auto &node: getNodes()) {
            if (node.id == nodeId) {
                return &node;
            }
        }
        return nullptr;
    }

    Node *NodeEditor::getNode(int nodeId) {
        for (auto &node: m_state.nodes) {
            if (node.id == nodeId) {
                return &node;
            }
        }
        return nullptr;
    }

    void NodeEditor::updateNodeBoundingBoxes() {
        m_nodeBoundingBoxManager->clear();

        for (const auto &node: m_state.nodes) {
            if (!isNodeInCurrentSubgraph(node)) continue;

            m_nodeBoundingBoxManager->addBoundingBox(
                node.id,
                node.position,
                Vec2(node.size.x, node.size.y)
            );
        }

        auto config = m_connectionStyleManager.getConfig();
        config.avoidNodes = m_nodeAvoidanceEnabled;
        m_connectionStyleManager.setConfig(config);
    }

    void NodeEditor::enableNodeAvoidance(bool enable) {
        m_nodeAvoidanceEnabled = enable;

        auto config = m_connectionStyleManager.getConfig();
        config.avoidNodes = enable;
        m_connectionStyleManager.setConfig(config);
    }

    bool NodeEditor::isNodeAvoidanceEnabled() const {
        return m_nodeAvoidanceEnabled;
    }

    UUID NodeEditor::getHoveredNodeUUID() const {
        return m_state.hoveredNodeUuid;
    }

    UUID NodeEditor::getHoveredPinUUID() const {
        return m_state.hoveredPinUuid;
    }

    UUID NodeEditor::getHoveredConnectionUUID() const {
        return m_state.hoveredConnectionUuid;
    }

    UUID NodeEditor::getHoveredGroupUUID() const {
        return m_state.hoveredGroupUuid;
    }

    UUID NodeEditor::getCurrentSubgraphUUID() const {
        return m_state.currentSubgraphUuid;
    }

    void NodeEditor::setCurrentSubgraphByUUID(const UUID &uuid) {
        m_state.currentSubgraphUuid = uuid;
    }

    bool NodeEditor::isNodeInSubgraphByUUID(const Node &node, const UUID &subgraphUuid) const {
        return node.metadata.getAttribute<UUID>("subgraphUuid", "") == subgraphUuid;
    }

    bool NodeEditor::isSubgraphContainerByUUID(const UUID &uuid) const {
        int nodeId = getNodeId(uuid);
        if (nodeId == -1) return false;
        return isSubgraphContainer(nodeId);
    }

    UUID NodeEditor::getSubgraphUUIDFromNode(const UUID &nodeUuid) const {
        int nodeId = getNodeId(nodeUuid);
        if (nodeId == -1) return "";

        const Node *node = getNode(nodeId);
        if (!node || !node->isSubgraph) return "";

        return node->metadata.getAttribute<UUID>("subgraphUuid", "");
    }

    UUID NodeEditor::getNodeSubgraphUUID(const UUID &nodeUuid) const {
        int nodeId = getNodeId(nodeUuid);
        if (nodeId == -1) return "";

        const Node *node = getNode(nodeId);
        if (!node) return "";

        return node->metadata.getAttribute<UUID>("subgraphUuid", "");
    }

    int NodeEditor::addPin(int nodeId, const std::string& name, bool isInput, PinType type, PinShape shape, const UUID& uuid) {
        Node* node = getNode(nodeId);
        if (!node) {
            return -1;
        }

        int pinId = m_state.nextPinId++;
        Pin pin(uuid, pinId, name, isInput, type, shape);

        if (isInput) {
            node->inputs.push_back(pin);
        } else {
            node->outputs.push_back(pin);
        }

        return pinId;
    }

    void NodeEditor::removePin(int nodeId, int pinId) {
        Node *node = getNode(nodeId);
        if (!node) return;

        auto removeFromVec = [pinId](std::vector<Pin> &pins) {
            pins.erase(
                std::remove_if(pins.begin(), pins.end(),
                               [pinId](const Pin &pin) { return pin.id == pinId; }),
                pins.end());
        };

        removeFromVec(node->inputs);
        removeFromVec(node->outputs);
    }

    const Pin* NodeEditor::getPin(int nodeId, int pinId) const {
        const Node* node = getNode(nodeId);
        if (!node) return nullptr;

        for (const auto& pin : node->inputs) {
            if (pin.id == pinId) {
                static thread_local Pin result;
                result.id = pin.id;
                result.uuid = pin.uuid;
                result.name = pin.name;
                result.isInput = pin.isInput;
                result.type = static_cast<PinType>(pin.type);
                result.shape = static_cast<PinShape>(pin.shape);
                result.connected = pin.connected;
                return &result;
            }
        }

        for (const auto& pin : node->outputs) {
            if (pin.id == pinId) {
                static thread_local Pin result;
                result.id = pin.id;
                result.uuid = pin.uuid;
                result.name = pin.name;
                result.isInput = pin.isInput;
                result.type = static_cast<PinType>(pin.type);
                result.shape = static_cast<PinShape>(pin.shape);
                result.connected = pin.connected;
                return &result;
            }
        }

        return nullptr;
    }



    Pin* NodeEditor::getPin(int nodeId, int pinId) {
        Node* node = getNode(nodeId);
        if (!node) return nullptr;

        for (auto& pin : node->inputs) {
            if (pin.id == pinId) {
                static thread_local Pin result;
                result.id = pin.id;
                result.uuid = pin.uuid;
                result.name = pin.name;
                result.isInput = pin.isInput;
                result.type = static_cast<PinType>(pin.type);
                result.shape = static_cast<PinShape>(pin.shape);
                result.connected = pin.connected;
                return &result;
            }
        }

        for (auto& pin : node->outputs) {
            if (pin.id == pinId) {
                static thread_local Pin result;
                result.id = pin.id;
                result.uuid = pin.uuid;
                result.name = pin.name;
                result.isInput = pin.isInput;
                result.type = static_cast<PinType>(pin.type);
                result.shape = static_cast<PinShape>(pin.shape);
                result.connected = pin.connected;
                return &result;
            }
        }

        return nullptr;
    }

    int NodeEditor::addPinByNodeUUID(const UUID &nodeUuid, const std::string &name, bool isInput,
                                     PinType type, PinShape shape, const UUID &uuid) {
        int nodeId = getNodeId(nodeUuid);
        if (nodeId == -1) return -1;

        return addPin(nodeId, name, isInput, type, shape, uuid);
    }

    void NodeEditor::registerNodeType(const std::string &type, const std::string &category,
                                      const std::string &description, std::function<Node*(const Vec2 &)> builder) {
        NodeTypeInfo info;
        info.name = type;
        info.category = category;
        info.description = description;
        info.builder = builder;

        m_registeredNodeTypes[type] = info;
    }

    UUID NodeEditor::addPinWithUUID(int nodeId, const std::string &name, bool isInput, PinType type, PinShape shape) {
        int pinId = addPin(nodeId, name, isInput, type, shape);
        return getPinUUID(nodeId, pinId);
    }

    UUID NodeEditor::addPinWithUUIDByNodeUUID(const UUID &nodeUuid, const std::string &name, bool isInput, PinType type,
                                              PinShape shape) {
        int nodeId = getNodeId(nodeUuid);
        if (nodeId == -1) return "";

        return addPinWithUUID(nodeId, name, isInput, type, shape);
    }

    UUID NodeEditor::addGroupWithUUID(const std::string &name, const Vec2 &position, const Vec2 &size) {
        int groupId = addGroup(name, position, size);
        return getGroupUUID(groupId);
    }



    Node *NodeEditor::createNodeOfType(const std::string &type, const Vec2 &position) {
        auto it = m_registeredNodeTypes.find(type);
        if (it == m_registeredNodeTypes.end()) {
            return nullptr;
        }

        const NodeTypeInfo &info = it->second;
        Node *node = info.builder(position);

        if (node) {
            int nodeId = addNode(node->name, type, position);
            Node *createdNode = getNode(nodeId);

            if (createdNode) {
                for (const auto &pin: node->inputs) {
                    addPin(nodeId, pin.name, true, pin.type, pin.shape);
                }

                for (const auto &pin: node->outputs) {
                    addPin(nodeId, pin.name, false, pin.type, pin.shape);
                }

                createdNode->iconSymbol = node->iconSymbol;
                createdNode->labelPosition = node->labelPosition;

                delete node;

                return createdNode;
            }

            delete node;
        }

        return nullptr;
    }



    void NodeEditor::removePinByUUID(const UUID &nodeUuid, const UUID &pinUuid) {
        int nodeId = getNodeId(nodeUuid);
        if (nodeId == -1) return;

        Node *node = getNode(nodeId);
        if (!node) return;

        for (const auto &pin: node->inputs) {
            if (pin.uuid == pinUuid) {
                removePin(nodeId, pin.id);
                return;
            }
        }

        for (const auto &pin: node->outputs) {
            if (pin.uuid == pinUuid) {
                removePin(nodeId, pin.id);
                return;
            }
        }
    }

    void NodeEditor::centerOnNodeByUUID(const UUID &uuid) {
        int nodeId = getNodeId(uuid);
        if (nodeId != -1) {
            centerOnNode(nodeId);
        }
    }
}
