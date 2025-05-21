#include "../../Core/NodeEditor.h"
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
          , m_connectionStyleManager()
          , m_nodeBoundingBoxManager(std::make_shared<NodeBoundingBoxManager>())
          , m_nodeAvoidanceEnabled(false)
          , m_isSynchronizing(false)
          , m_commandsInitialized(false) {
        m_state = State();

        m_viewManager.setMinZoom(0.1f);
        m_viewManager.setMaxZoom(5.0f);

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

        setupCommandSystem();

        //m_debugMode = true;
    }

    NodeEditor::~NodeEditor() {
    }

    void NodeEditor::beginFrame() {
        static bool firstFrame = true;

        if (firstFrame) {
            try {
                setupSubgraphCallbacks();

                refreshPinConnectionStates();
                updateAllSubgraphs();
            } catch (...) {
            }

            firstFrame = false;
        }

        if (!m_viewManager.isViewTransitioning()) {
            m_viewManager.setViewPosition(m_state.viewPosition);
            m_viewManager.setViewScale(m_state.viewScale);
        }

        m_state.hoveredNodeId = -1;
        m_state.hoveredPinId = -1;
        m_state.hoveredConnectionId = -1;
        m_state.hoveredGroupId = -1;

        m_state.magnetPinNodeId = -1;
        m_state.magnetPinId = -1;
        m_state.magnetPinNodeUuid = "";
        m_state.magnetPinUuid = "";
        m_state.canConnectToMagnetPin = true;

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

            for (const auto &subgraphPair: m_subgraphs) {
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
                                            [groupId = it->groupId](const Group &group) {
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

    int NodeEditor::getHoveredNodeId() const {
        return m_state.hoveredNodeId;
    }

    UUID NodeEditor::getHoveredNodeUUID() const {
        return m_state.hoveredNodeUuid;
    }

    int NodeEditor::getHoveredPinId() const {
        return m_state.hoveredPinId;
    }

    UUID NodeEditor::getHoveredPinUUID() const {
        return m_state.hoveredPinUuid;
    }

    int NodeEditor::getHoveredConnectionId() const {
        return m_state.hoveredConnectionId;
    }

    UUID NodeEditor::getHoveredConnectionUUID() const {
        return m_state.hoveredConnectionUuid;
    }

    int NodeEditor::getHoveredGroupId() const {
        return m_state.hoveredGroupId;
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

    int NodeEditor::addPin(int nodeId, const std::string &name, bool isInput, PinType type, PinShape shape,
                           const UUID &uuid) {
        Node *node = getNode(nodeId);
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

    const Pin *NodeEditor::getPin(int nodeId, int pinId) const {
        const Node *node = getNode(nodeId);
        if (!node) return nullptr;

        for (const auto &pin: node->inputs) {
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

        for (const auto &pin: node->outputs) {
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


    Pin *NodeEditor::getPin(int nodeId, int pinId) {
        Node *node = getNode(nodeId);
        if (!node) return nullptr;

        for (auto &pin: node->inputs) {
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

        for (auto &pin: node->outputs) {
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

    void NodeEditor::loadGraphState(const SerializedState &state) {
        m_state.nodes.clear();
        m_state.connections.clear();
        m_state.groups.clear();
        m_subgraphs.clear();

        for (const auto &serializedNode: state.nodes) {
            Node node;
            node.id = serializedNode.id;
            node.uuid = serializedNode.uuid;
            node.name = serializedNode.name;
            node.type = serializedNode.type;
            node.position = serializedNode.position;
            node.size = serializedNode.size;
            node.isSubgraph = serializedNode.isSubgraph;
            node.subgraphId = serializedNode.subgraphId;
            node.subgraphUuid = serializedNode.subgraphUuid;
            node.metadata = serializedNode.metadata;

            for (const auto &serializedPin: serializedNode.inputs) {
                Pin pin;
                pin.id = serializedPin.id;
                pin.uuid = serializedPin.uuid;
                pin.name = serializedPin.name;
                pin.isInput = serializedPin.isInput;
                pin.type = serializedPin.type;
                pin.shape = serializedPin.shape;
                pin.metadata = serializedPin.metadata;
                pin.connected = false;

                node.inputs.push_back(pin);
            }

            for (const auto &serializedPin: serializedNode.outputs) {
                Pin pin;
                pin.id = serializedPin.id;
                pin.uuid = serializedPin.uuid;
                pin.name = serializedPin.name;
                pin.isInput = serializedPin.isInput;
                pin.type = serializedPin.type;
                pin.shape = serializedPin.shape;
                pin.metadata = serializedPin.metadata;
                pin.connected = false;

                node.outputs.push_back(pin);
            }

            m_state.nodes.push_back(node);
        }

        for (const auto &serializedConnection: state.connections) {
            Connection connection;
            connection.id = serializedConnection.id;
            connection.uuid = serializedConnection.uuid;
            connection.startNodeId = serializedConnection.startNodeId;
            connection.startNodeUuid = serializedConnection.startNodeUuid;
            connection.startPinId = serializedConnection.startPinId;
            connection.startPinUuid = serializedConnection.startPinUuid;
            connection.endNodeId = serializedConnection.endNodeId;
            connection.endNodeUuid = serializedConnection.endNodeUuid;
            connection.endPinId = serializedConnection.endPinId;
            connection.endPinUuid = serializedConnection.endPinUuid;
            connection.metadata = serializedConnection.metadata;

            m_state.connections.push_back(connection);
        }

        for (const auto &serializedGroup: state.groups) {
            Group group(serializedGroup.uuid, serializedGroup.id, serializedGroup.name,
                        serializedGroup.position, serializedGroup.size);
            group.color = serializedGroup.color;
            group.style = serializedGroup.style;
            group.collapsed = serializedGroup.collapsed;

            for (const int nodeId: serializedGroup.nodeIds) {
                group.nodes.insert(nodeId);
            }

            for (const auto &nodeUuid: serializedGroup.nodeUuids) {
                group.nodeUuids.insert(nodeUuid);
            }

            group.metadata = serializedGroup.metadata;

            m_state.groups.push_back(group);
        }

        for (const auto &serializedSubgraph: state.subgraphs) {
            auto subgraph = std::make_shared<Subgraph>();
            subgraph->id = serializedSubgraph.id;
            subgraph->uuid = serializedSubgraph.uuid;
            subgraph->name = serializedSubgraph.name;
            subgraph->nodeIds = serializedSubgraph.nodeIds;
            subgraph->nodeUuids = serializedSubgraph.nodeUuids;
            subgraph->connectionIds = serializedSubgraph.connectionIds;
            subgraph->connectionUuids = serializedSubgraph.connectionUuids;
            subgraph->groupIds = serializedSubgraph.groupIds;
            subgraph->groupUuids = serializedSubgraph.groupUuids;
            subgraph->interfaceInputs = serializedSubgraph.interfaceInputs;
            subgraph->interfaceOutputs = serializedSubgraph.interfaceOutputs;
            subgraph->parentSubgraphId = serializedSubgraph.parentSubgraphId;
            subgraph->parentSubgraphUuid = serializedSubgraph.parentSubgraphUuid;
            subgraph->childSubgraphIds = serializedSubgraph.childSubgraphIds;
            subgraph->childSubgraphUuids = serializedSubgraph.childSubgraphUuids;
            subgraph->viewPosition = serializedSubgraph.viewPosition;
            subgraph->viewScale = serializedSubgraph.viewScale;
            subgraph->description = serializedSubgraph.description;
            subgraph->category = serializedSubgraph.category;
            subgraph->isTemplate = serializedSubgraph.isTemplate;
            subgraph->iconSymbol = serializedSubgraph.iconSymbol;
            subgraph->accentColor = serializedSubgraph.accentColor;
            subgraph->metadata = serializedSubgraph.metadata;

            m_subgraphs[subgraph->id] = subgraph;
        }

        m_state.viewPosition = state.viewPosition;
        m_state.viewScale = state.viewScale;

        updateNodeUuidMap();
        updateConnectionUuidMap();
        updateGroupUuidMap();

        refreshPinConnectionStates();

        updateAllSubgraphs();

        updateNextIds();
    }

    void NodeEditor::updateNextIds() {
        int maxNodeId = 0;
        for (const auto &node: m_state.nodes) {
            maxNodeId = std::max(maxNodeId, node.id);
        }
        m_state.nextNodeId = maxNodeId + 1;

        int maxPinId = 0;
        for (const auto &node: m_state.nodes) {
            for (const auto &pin: node.inputs) {
                maxPinId = std::max(maxPinId, pin.id);
            }
            for (const auto &pin: node.outputs) {
                maxPinId = std::max(maxPinId, pin.id);
            }
        }
        m_state.nextPinId = maxPinId + 1;

        int maxConnectionId = 0;
        for (const auto &connection: m_state.connections) {
            maxConnectionId = std::max(maxConnectionId, connection.id);
        }
        m_state.nextConnectionId = maxConnectionId + 1;

        int maxGroupId = 0;
        for (const auto &group: m_state.groups) {
            maxGroupId = std::max(maxGroupId, group.id);
        }
        m_state.nextGroupId = maxGroupId + 1;
    }

    void NodeEditor::refreshPinConnectionStates() {
        for (auto &node: m_state.nodes) {
            for (auto &pin: node.inputs) {
                pin.connected = false;
            }
            for (auto &pin: node.outputs) {
                pin.connected = false;
            }
        }

        for (const auto &connection: m_state.connections) {
            Node *startNode = getNode(connection.startNodeId);
            Node *endNode = getNode(connection.endNodeId);

            if (startNode && endNode) {
                Pin *startPin = startNode->findPin(connection.startPinId);
                Pin *endPin = endNode->findPin(connection.endPinId);

                if (startPin) startPin->connected = true;
                if (endPin) endPin->connected = true;
            }
        }
    }
}
