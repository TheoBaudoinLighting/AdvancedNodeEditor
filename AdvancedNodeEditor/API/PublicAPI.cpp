#include "../NodeEditor.h"
#include "../Core/Conversions/Conversions.h"

namespace ANE {
    NodeEditor::NodeEditor() : m_currentSubgraphId(-1) {
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

    void NodeEditor::setDebugMode(bool enable) {
        m_editor.setDebugMode(enable);
    }

    bool NodeEditor::isDebugMode() const {
        return m_editor.isDebugMode();
    }

    int NodeEditor::addNode(const std::string &name, const std::string &type, const Vec2 &position, const UUID& uuid) {
        int nodeId = m_editor.addNode(name, type, NodeEditorCore::convertToVec2(position), uuid);

        std::unordered_map<std::string, std::any> data;
        data["nodeId"] = nodeId;
        data["nodeUuid"] = m_editor.getNodeUUID(nodeId);
        data["name"] = name;
        data["type"] = type;
        data["position"] = position;
        dispatchAction("nodeAdded", data);

        return nodeId;
    }

    UUID NodeEditor::addNodeWithUUID(const std::string &name, const std::string &type, const Vec2 &position) {
        UUID uuid = generateUUID();
        int nodeId = m_editor.addNode(name, type, NodeEditorCore::convertToVec2(position), uuid);

        std::unordered_map<std::string, std::any> data;
        data["nodeId"] = nodeId;
        data["nodeUuid"] = uuid;
        data["name"] = name;
        data["type"] = type;
        data["position"] = position;
        dispatchAction("nodeAdded", data);

        return uuid;
    }

    Node *NodeEditor::getNode(int nodeId) {
        NodeEditorCore::Node *internalNode = m_editor.getNode(nodeId);
        if (!internalNode) return nullptr;

        static ANE::Node apiNode(internalNode->id, internalNode->name, internalNode->type);
        apiNode.uuid = internalNode->uuid;
        apiNode.iconSymbol = internalNode->iconSymbol;
        apiNode.labelPosition = NodeEditorCore::convertToANENodeLabelPosition(internalNode->labelPosition);
        apiNode.disabled = internalNode->disabled;
        apiNode.isTemplate = internalNode->isTemplate;
        apiNode.isCurrentFlag = internalNode->isCurrentFlag;
        apiNode.isSubgraph = internalNode->isSubgraph;
        apiNode.subgraphId = internalNode->subgraphId;
        apiNode.subgraphUuid = internalNode->subgraphId >= 0 ? getSubgraphUUID(internalNode->subgraphId) : "";

        return &apiNode;
    }

    Node *NodeEditor::getNodeByUUID(const UUID& uuid) {
        NodeEditorCore::Node *internalNode = m_editor.getNodeByUUID(uuid);
        if (!internalNode) return nullptr;

        static ANE::Node apiNode(internalNode->id, internalNode->name, internalNode->type);
        apiNode.uuid = internalNode->uuid;
        apiNode.iconSymbol = internalNode->iconSymbol;
        apiNode.labelPosition = NodeEditorCore::convertToANENodeLabelPosition(internalNode->labelPosition);
        apiNode.disabled = internalNode->disabled;
        apiNode.isTemplate = internalNode->isTemplate;
        apiNode.isCurrentFlag = internalNode->isCurrentFlag;
        apiNode.isSubgraph = internalNode->isSubgraph;
        apiNode.subgraphId = internalNode->subgraphId;
        apiNode.subgraphUuid = internalNode->subgraphId >= 0 ? getSubgraphUUID(internalNode->subgraphId) : "";

        return &apiNode;
    }

    void NodeEditor::removeNode(int nodeId) {
        Node *node = getNode(nodeId);
        if (node) {
            std::unordered_map<std::string, std::any> data;
            data["nodeId"] = nodeId;
            data["nodeUuid"] = node->uuid;
            data["name"] = node->name;
            data["type"] = node->type;

            m_editor.removeNode(nodeId);

            dispatchAction("nodeRemoved", data);
        } else {
            m_editor.removeNode(nodeId);
        }
    }

    void NodeEditor::removeNodeByUUID(const UUID& uuid) {
        Node *node = getNodeByUUID(uuid);
        if (node) {
            std::unordered_map<std::string, std::any> data;
            data["nodeId"] = node->id;
            data["nodeUuid"] = uuid;
            data["name"] = node->name;
            data["type"] = node->type;

            m_editor.removeNodeByUUID(uuid);

            dispatchAction("nodeRemoved", data);
        } else {
            m_editor.removeNodeByUUID(uuid);
        }
    }

    UUID NodeEditor::getNodeUUID(int nodeId) const {
        return m_editor.getNodeUUID(nodeId);
    }

    int NodeEditor::getNodeId(const UUID& uuid) const {
        return m_editor.getNodeId(uuid);
    }

    int NodeEditor::addPin(int nodeId, const std::string &name, bool isInput, PinType type, PinShape shape, const UUID& uuid) {
        return m_editor.addPin(
            nodeId,
            name,
            isInput,
            NodeEditorCore::convertToPinType(type),
            NodeEditorCore::convertToPinShape(shape),
            uuid
        );
    }

    UUID NodeEditor::addPinWithUUID(int nodeId, const std::string &name, bool isInput, PinType type, PinShape shape) {
        UUID uuid = generateUUID();
        int pinId = addPin(nodeId, name, isInput, type, shape, uuid);
        return pinId >= 0 ? uuid : "";
    }

    UUID NodeEditor::addPinWithUUIDByNodeUUID(const UUID& nodeUuid, const std::string &name, bool isInput,
                                          PinType type, PinShape shape) {
        int nodeId = m_editor.getNodeId(nodeUuid);
        if (nodeId == -1) return "";

        return addPinWithUUID(nodeId, name, isInput, type, shape);
    }

    Pin *NodeEditor::getPin(int nodeId, int pinId) {
        return m_editor.getPin(nodeId, pinId);
    }

    Pin *NodeEditor::getPinByUUID(const UUID& nodeUuid, const UUID& pinUuid) {
        return m_editor.getPinByUUID(nodeUuid, pinUuid);
    }

    int NodeEditor::addConnection(int startNodeId, int startPinId, int endNodeId, int endPinId, const UUID& uuid) {
        int connectionId = m_editor.addConnection(startNodeId, startPinId, endNodeId, endPinId, uuid);

        if (connectionId >= 0) {
            std::unordered_map<std::string, std::any> data;
            data["connectionId"] = connectionId;
            data["connectionUuid"] = m_editor.getConnectionUUID(connectionId);
            data["startNodeId"] = startNodeId;
            data["startNodeUuid"] = m_editor.getNodeUUID(startNodeId);
            data["startPinId"] = startPinId;
            data["startPinUuid"] = m_editor.getPinUUID(startNodeId, startPinId);
            data["endNodeId"] = endNodeId;
            data["endNodeUuid"] = m_editor.getNodeUUID(endNodeId);
            data["endPinId"] = endPinId;
            data["endPinUuid"] = m_editor.getPinUUID(endNodeId, endPinId);
            dispatchAction("connectionAdded", data);
        }

        return connectionId;
    }

    UUID NodeEditor::addConnectionWithUUID(int startNodeId, int startPinId, int endNodeId, int endPinId) {
        UUID uuid = generateUUID();
        int connectionId = addConnection(startNodeId, startPinId, endNodeId, endPinId, uuid);
        return connectionId >= 0 ? uuid : "";
    }

    UUID NodeEditor::addConnectionWithUUIDByUUID(const UUID& startNodeUuid, const UUID& startPinUuid,
                                             const UUID& endNodeUuid, const UUID& endPinUuid) {
        UUID uuid = generateUUID();
        int connectionId = m_editor.addConnectionByUUID(startNodeUuid, startPinUuid, endNodeUuid, endPinUuid, uuid);
        return connectionId >= 0 ? uuid : "";
    }

    void NodeEditor::removeConnection(int connectionId) {
        NodeEditorCore::Connection *connection = m_editor.getConnection(connectionId);
        if (connection) {
            std::unordered_map<std::string, std::any> data;
            data["connectionId"] = connectionId;
            data["connectionUuid"] = connection->uuid;
            data["startNodeId"] = connection->startNodeId;
            data["startNodeUuid"] = connection->startNodeUuid;
            data["startPinId"] = connection->startPinId;
            data["startPinUuid"] = connection->startPinUuid;
            data["endNodeId"] = connection->endNodeId;
            data["endNodeUuid"] = connection->endNodeUuid;
            data["endPinId"] = connection->endPinId;
            data["endPinUuid"] = connection->endPinUuid;

            m_editor.removeConnection(connectionId);

            dispatchAction("connectionRemoved", data);
        } else {
            m_editor.removeConnection(connectionId);
        }
    }

    void NodeEditor::removeConnectionByUUID(const UUID& uuid) {
        NodeEditorCore::Connection *connection = m_editor.getConnectionByUUID(uuid);
        if (connection) {
            std::unordered_map<std::string, std::any> data;
            data["connectionId"] = connection->id;
            data["connectionUuid"] = uuid;
            data["startNodeId"] = connection->startNodeId;
            data["startNodeUuid"] = connection->startNodeUuid;
            data["startPinId"] = connection->startPinId;
            data["startPinUuid"] = connection->startPinUuid;
            data["endNodeId"] = connection->endNodeId;
            data["endNodeUuid"] = connection->endNodeUuid;
            data["endPinId"] = connection->endPinId;
            data["endPinUuid"] = connection->endPinUuid;

            m_editor.removeConnectionByUUID(uuid);

            dispatchAction("connectionRemoved", data);
        } else {
            m_editor.removeConnectionByUUID(uuid);
        }
    }

    int NodeEditor::addGroup(const std::string &name, const Vec2 &position, const Vec2 &size, const UUID& uuid) {
        return m_editor.addGroup(
            name,
            NodeEditorCore::convertToVec2(position),
            NodeEditorCore::convertToVec2(size),
            uuid
        );
    }

    UUID NodeEditor::addGroupWithUUID(const std::string &name, const Vec2 &position, const Vec2 &size) {
        UUID uuid = generateUUID();
        int groupId = addGroup(name, position, size, uuid);
        return groupId >= 0 ? uuid : "";
    }

    Group *NodeEditor::getGroup(int groupId) {
        NodeEditorCore::Group *internalGroup = m_editor.getGroup(groupId);
        if (!internalGroup) return nullptr;

        static ANE::Group apiGroup(internalGroup->id, internalGroup->name);
        apiGroup.uuid = internalGroup->uuid;
        apiGroup.color = NodeEditorCore::convertToANEColor(internalGroup->color);
        apiGroup.style = NodeEditorCore::convertToANEGroupStyle(internalGroup->style);
        apiGroup.collapsed = internalGroup->collapsed;

        return &apiGroup;
    }

    Group *NodeEditor::getGroupByUUID(const UUID& uuid) {
        NodeEditorCore::Group *internalGroup = m_editor.getGroupByUUID(uuid);
        if (!internalGroup) return nullptr;

        static ANE::Group apiGroup(internalGroup->id, internalGroup->name);
        apiGroup.uuid = internalGroup->uuid;
        apiGroup.color = NodeEditorCore::convertToANEColor(internalGroup->color);
        apiGroup.style = NodeEditorCore::convertToANEGroupStyle(internalGroup->style);
        apiGroup.collapsed = internalGroup->collapsed;

        return &apiGroup;
    }

    void NodeEditor::removeGroup(int groupId) {
        m_editor.removeGroup(groupId);
    }

    void NodeEditor::removeGroupByUUID(const UUID& uuid) {
        m_editor.removeGroupByUUID(uuid);
    }

    void NodeEditor::addNodeToGroup(int nodeId, int groupId) {
        m_editor.addNodeToGroup(nodeId, groupId);
    }

    void NodeEditor::addNodeToGroupByUUID(const UUID& nodeUuid, const UUID& groupUuid) {
        m_editor.addNodeToGroupByUUID(nodeUuid, groupUuid);
    }

    void NodeEditor::selectNode(int nodeId, bool append) {
        m_editor.selectNode(nodeId, append);
    }

    void NodeEditor::selectNodeByUUID(const UUID& uuid, bool append) {
        m_editor.selectNodeByUUID(uuid, append);
    }

    std::vector<int> NodeEditor::getSelectedNodes() const {
        return m_editor.getSelectedNodes();
    }

    std::vector<UUID> NodeEditor::getSelectedNodeUUIDs() const {
        return m_editor.getSelectedNodeUUIDs();
    }

    void NodeEditor::centerView() {
        m_editor.centerView();
    }

    void NodeEditor::centerOnNode(int nodeId) {
        m_editor.centerOnNode(nodeId);
    }

    void NodeEditor::setViewScale(float scale) {
        m_editor.setViewScale(scale);
    }

    float NodeEditor::getViewScale() const {
        return m_editor.getViewScale();
    }

    void NodeEditor::setViewPosition(const Vec2& position) {
        m_editor.setViewPosition(NodeEditorCore::convertToVec2(position));
    }

    Vec2 NodeEditor::getViewPosition() const {
        return NodeEditorCore::convertToANEVec2(m_editor.getViewPosition());
    }

    void NodeEditor::setStyle(const EditorStyle &style) {
        m_editor.setStyle(NodeEditorCore::convertToInternalStyle(style));
    }

    EditorStyle NodeEditor::getStyle() const {
        return NodeEditorCore::convertToAPIStyle(m_editor.getStyle());
    }

    void NodeEditor::setCanConnectCallback(CanConnectCallback callback) {
        if (callback) {
            m_editor.setCanConnectCallback([callback](const ANE::Pin &startPin, const ANE::Pin &endPin) {
                return callback(startPin, endPin);
            });
        } else {
            m_editor.setCanConnectCallback(nullptr);
        }
    }

    void NodeEditor::setNodeEvaluateCallback(NodeEvaluateCallback callback) {
        m_nodeEvaluateCallback = callback;
    }

    void NodeEditor::setNodeEvaluateCallbackUUID(NodeEvaluateCallbackUUID callback) {
        m_nodeEvaluateCallbackUUID = callback;
    }

    void NodeEditor::evaluateNode(int nodeId, const NodeEvaluationContext &context) {
        Node *node = getNode(nodeId);
        if (node && m_nodeEvaluateCallback) {
            m_nodeEvaluateCallback(*node, context.getValues());
        }

        if (node && m_nodeEvaluateCallbackUUID) {
            m_nodeEvaluateCallbackUUID(node->uuid, *node, context.getValues());
        }
    }

    void NodeEditor::evaluateNodeByUUID(const UUID& uuid, const NodeEvaluationContext &context) {
        Node *node = getNodeByUUID(uuid);
        if (node && m_nodeEvaluateCallbackUUID) {
            m_nodeEvaluateCallbackUUID(uuid, *node, context.getValues());
        } else if (node && m_nodeEvaluateCallback) {
            m_nodeEvaluateCallback(*node, context.getValues());
        }
    }

    void NodeEditor::setActionCallback(ActionCallback callback) {
        m_actionCallback = callback;
    }

    void NodeEditor::dispatchAction(const std::string &action, const std::unordered_map<std::string, std::any> &data) {
        if (m_actionCallback) {
            m_actionCallback(action, data);
        }
    }

    void NodeEditor::setNodeOverlayCallback(NodeOverlayCallback callback) {
        m_nodeOverlayCallback = callback;
    }

    void NodeEditor::setNodeOverlayCallbackUUID(NodeOverlayCallbackUUID callback) {
        m_nodeOverlayCallbackUUID = callback;
    }

    void NodeEditor::registerNodeType(const std::string &type, const std::string &category, const std::string &description,
                                    std::function<Node*(const Vec2 &)> builder) {
        NodeTypeInfo info;
        info.name = type;
        info.category = category;
        info.description = description;
        info.builder = builder;
        m_registeredNodeTypes[type] = info;
    }

    std::vector<NodeTypeInfo> NodeEditor::getRegisteredNodeTypes() const {
        std::vector<NodeTypeInfo> types;
        for (const auto &pair: m_registeredNodeTypes) {
            types.push_back(pair.second);
        }
        return types;
    }

    Node *NodeEditor::createNodeOfType(const std::string &type, const Vec2 &position) {
        auto it = m_registeredNodeTypes.find(type);
        if (it != m_registeredNodeTypes.end()) {
            return it->second.builder(position);
        }
        return nullptr;
    }

    Node *NodeEditor::createNodeOfTypeWithUUID(const std::string &type, const Vec2 &position, UUID& outUuid) {
        auto it = m_registeredNodeTypes.find(type);
        if (it != m_registeredNodeTypes.end()) {
            Node* node = it->second.builder(position);
            if (node) {
                outUuid = node->uuid;
            }
            return node;
        }
        outUuid = "";
        return nullptr;
    }

    // Implémentation des méthodes de sous-graphes
    int NodeEditor::createSubgraph(const std::string &name, const UUID& uuid) {
        int subgraphId = Subgraph::nextId++;
        UUID subgraphUuid = uuid.empty() ? generateUUID() : uuid;

        auto subgraph = std::make_shared<Subgraph>(uuid.empty() ? subgraphId : (subgraphUuid, subgraphId), name);
        subgraph->uuid = subgraphUuid;

        m_subgraphs[subgraphId] = subgraph;
        m_subgraphsByUuid[subgraphUuid] = subgraph;

        return subgraphId;
    }

    UUID NodeEditor::createSubgraphWithUUID(const std::string &name) {
        UUID uuid = generateUUID();
        createSubgraph(name, uuid);
        return uuid;
    }

    Subgraph *NodeEditor::getSubgraph(int subgraphId) {
        auto it = m_subgraphs.find(subgraphId);
        if (it != m_subgraphs.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    Subgraph *NodeEditor::getSubgraphByUUID(const UUID& uuid) {
        auto it = m_subgraphsByUuid.find(uuid);
        if (it != m_subgraphsByUuid.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    void NodeEditor::removeSubgraph(int subgraphId) {
        auto it = m_subgraphs.find(subgraphId);
        if (it == m_subgraphs.end()) {
            return;
        }

        UUID subgraphUuid = it->second->uuid;

        if (m_currentSubgraphId == subgraphId) {
            exitSubgraph();
        }

        bool inStack = false;
        std::stack<int> tempStack;
        while (!m_subgraphStack.empty()) {
            int id = m_subgraphStack.top();
            m_subgraphStack.pop();

            if (id == subgraphId) {
                inStack = true;
            } else {
                tempStack.push(id);
            }
        }

        while (!tempStack.empty()) {
            m_subgraphStack.push(tempStack.top());
            tempStack.pop();
        }

        if (inStack) {
            std::stack<UUID> tempUuidStack;
            while (!m_subgraphUuidStack.empty()) {
                UUID uuid = m_subgraphUuidStack.top();
                m_subgraphUuidStack.pop();

                if (uuid != subgraphUuid) {
                    tempUuidStack.push(uuid);
                }
            }

            while (!tempUuidStack.empty()) {
                m_subgraphUuidStack.push(tempUuidStack.top());
                tempUuidStack.pop();
            }
        }

        m_subgraphsByUuid.erase(subgraphUuid);
        m_subgraphs.erase(it);
    }

    void NodeEditor::removeSubgraphByUUID(const UUID& uuid) {
        auto it = m_subgraphsByUuid.find(uuid);
        if (it != m_subgraphsByUuid.end()) {
            removeSubgraph(it->second->id);
        }
    }

    UUID NodeEditor::getSubgraphUUID(int subgraphId) const {
        auto it = m_subgraphs.find(subgraphId);
        return it != m_subgraphs.end() ? it->second->uuid : "";
    }

    int NodeEditor::getSubgraphId(const UUID& uuid) const {
        auto it = m_subgraphsByUuid.find(uuid);
        return it != m_subgraphsByUuid.end() ? it->second->id : -1;
    }

    Node *NodeEditor::createSubgraphNode(int subgraphId, const std::string &name, const Vec2 &position, const UUID& uuid) {
        if (m_subgraphs.find(subgraphId) == m_subgraphs.end()) {
            return nullptr;
        }

        int nodeId = addNode(name.empty() ? m_subgraphs[subgraphId]->name : name, "Subgraph", position, uuid);
        Node *node = getNode(nodeId);
        if (node) {
            node->setAsSubgraph(true, subgraphId, m_subgraphs[subgraphId]->uuid);

            if (!m_subgraphs[subgraphId]->iconSymbol.empty()) {
                node->setIconSymbol(m_subgraphs[subgraphId]->iconSymbol);
            } else {
                node->setIconSymbol("S");
            }
        }
        return node;
    }

    Node *NodeEditor::createSubgraphNodeByUUID(const UUID& subgraphUuid, const std::string &name, const Vec2 &position, const UUID& uuid) {
        auto it = m_subgraphsByUuid.find(subgraphUuid);
        if (it == m_subgraphsByUuid.end()) {
            return nullptr;
        }

        return createSubgraphNode(it->second->id, name, position, uuid);
    }

    bool NodeEditor::enterSubgraph(int subgraphId) {
        if (m_subgraphs.find(subgraphId) == m_subgraphs.end()) {
            return false;
        }

        saveSubgraphViewState(m_currentSubgraphId);

        std::unordered_map<std::string, std::any> data;
        data["previousSubgraphId"] = m_currentSubgraphId;
        data["previousSubgraphUuid"] = m_currentSubgraphUuid;
        data["subgraphId"] = subgraphId;
        data["subgraphUuid"] = m_subgraphs[subgraphId]->uuid;

        if (m_currentSubgraphId >= 0) {
            m_subgraphStack.push(m_currentSubgraphId);
            m_subgraphUuidStack.push(m_currentSubgraphUuid);
        }
        m_currentSubgraphId = subgraphId;
        m_currentSubgraphUuid = m_subgraphs[subgraphId]->uuid;
        m_editor.setCurrentSubgraphId(subgraphId);

        restoreSubgraphViewState(subgraphId);

        dispatchAction("enterSubgraph", data);

        return true;
    }

    bool NodeEditor::enterSubgraphByUUID(const UUID& uuid) {
        auto it = m_subgraphsByUuid.find(uuid);
        if (it == m_subgraphsByUuid.end()) {
            return false;
        }

        return enterSubgraph(it->second->id);
    }


    bool NodeEditor::exitSubgraph() {
        if (m_currentSubgraphId < 0) {
            return false;
        }

        saveSubgraphViewState(m_currentSubgraphId);

        std::unordered_map<std::string, std::any> data;
        data["previousSubgraphId"] = m_currentSubgraphId;
        data["previousSubgraphUuid"] = m_currentSubgraphUuid;

        int parentSubgraphId = -1;
        UUID parentSubgraphUuid = "";

        if (!m_subgraphStack.empty()) {
            parentSubgraphId = m_subgraphStack.top();
            m_subgraphStack.pop();

            if (!m_subgraphUuidStack.empty()) {
                parentSubgraphUuid = m_subgraphUuidStack.top();
                m_subgraphUuidStack.pop();
            }
        }

        data["subgraphId"] = parentSubgraphId;
        data["subgraphUuid"] = parentSubgraphUuid;

        m_currentSubgraphId = parentSubgraphId;
        m_currentSubgraphUuid = parentSubgraphUuid;
        m_editor.setCurrentSubgraphId(parentSubgraphId);

        restoreSubgraphViewState(m_currentSubgraphId);

        dispatchAction("exitSubgraph", data);

        return true;
    }

    int NodeEditor::getCurrentSubgraphId() const {
        return m_currentSubgraphId;
    }

    UUID NodeEditor::getCurrentSubgraphUUID() const {
        return m_currentSubgraphUuid;
    }

    std::vector<int> NodeEditor::getSubgraphStack() const {
        std::vector<int> stack;
        std::stack<int> tempStack = m_subgraphStack;

        while (!tempStack.empty()) {
            stack.push_back(tempStack.top());
            tempStack.pop();
        }

        std::reverse(stack.begin(), stack.end());

        return stack;
    }

    std::vector<UUID> NodeEditor::getSubgraphStackUUIDs() const {
        std::vector<UUID> stack;
        std::stack<UUID> tempStack = m_subgraphUuidStack;

        while (!tempStack.empty()) {
            stack.push_back(tempStack.top());
            tempStack.pop();
        }

        std::reverse(stack.begin(), stack.end());

        return stack;
    }

    void NodeEditor::exposeNodeInput(int nodeId, int pinId) {
        if (m_currentSubgraphId < 0) {
            return;
        }

        auto it = m_subgraphs.find(m_currentSubgraphId);
        if (it != m_subgraphs.end()) {
            it->second->exposeInput(nodeId, pinId);
        }
    }

    void NodeEditor::exposeNodeInputByUUID(const UUID& nodeUuid, const UUID& pinUuid) {
        if (m_currentSubgraphId < 0) {
            return;
        }

        int nodeId = m_editor.getNodeId(nodeUuid);
        if (nodeId == -1) return;

        NodeEditorCore::Node* node = m_editor.getNode(nodeId);
        if (!node) return;

        int pinId = -1;
        for (const auto& pin : node->inputs) {
            if (pin.uuid == pinUuid) {
                pinId = pin.id;
                break;
            }
        }

        if (pinId == -1) return;

        exposeNodeInput(nodeId, pinId);
    }

    void NodeEditor::exposeNodeOutput(int nodeId, int pinId) {
        if (m_currentSubgraphId < 0) {
            return;
        }

        auto it = m_subgraphs.find(m_currentSubgraphId);
        if (it != m_subgraphs.end()) {
            it->second->exposeOutput(nodeId, pinId);
        }
    }

    void NodeEditor::exposeNodeOutputByUUID(const UUID& nodeUuid, const UUID& pinUuid) {
        if (m_currentSubgraphId < 0) {
            return;
        }

        int nodeId = m_editor.getNodeId(nodeUuid);
        if (nodeId == -1) return;

        NodeEditorCore::Node* node = m_editor.getNode(nodeId);
        if (!node) return;

        int pinId = -1;
        for (const auto& pin : node->outputs) {
            if (pin.uuid == pinUuid) {
                pinId = pin.id;
                break;
            }
        }

        if (pinId == -1) return;

        exposeNodeOutput(nodeId, pinId);
    }

    void NodeEditor::unexposeNodeInput(int nodeId, int pinId) {
        if (m_currentSubgraphId < 0) {
            return;
        }

        auto it = m_subgraphs.find(m_currentSubgraphId);
        if (it != m_subgraphs.end()) {
            it->second->unexposeInput(nodeId, pinId);
        }
    }

    void NodeEditor::unexposeNodeInputByUUID(const UUID& nodeUuid, const UUID& pinUuid) {
        if (m_currentSubgraphId < 0) {
            return;
        }

        int nodeId = m_editor.getNodeId(nodeUuid);
        if (nodeId == -1) return;

        NodeEditorCore::Node* node = m_editor.getNode(nodeId);
        if (!node) return;

        int pinId = -1;
        for (const auto& pin : node->inputs) {
            if (pin.uuid == pinUuid) {
                pinId = pin.id;
                break;
            }
        }

        if (pinId == -1) return;

        unexposeNodeInput(nodeId, pinId);
    }

    void NodeEditor::unexposeNodeOutput(int nodeId, int pinId) {
        if (m_currentSubgraphId < 0) {
            return;
        }

        auto it = m_subgraphs.find(m_currentSubgraphId);
        if (it != m_subgraphs.end()) {
            it->second->unexposeOutput(nodeId, pinId);
        }
    }

    void NodeEditor::unexposeNodeOutputByUUID(const UUID& nodeUuid, const UUID& pinUuid) {
        if (m_currentSubgraphId < 0) {
            return;
        }

        int nodeId = m_editor.getNodeId(nodeUuid);
        if (nodeId == -1) return;

        NodeEditorCore::Node* node = m_editor.getNode(nodeId);
        if (!node) return;

        int pinId = -1;
        for (const auto& pin : node->outputs) {
            if (pin.uuid == pinUuid) {
                pinId = pin.id;
                break;
            }
        }

        if (pinId == -1) return;

        unexposeNodeOutput(nodeId, pinId);
    }

    void NodeEditor::saveSubgraphViewState(int subgraphId) {
        if (subgraphId < 0) {
            return;
        }

        auto it = m_subgraphs.find(subgraphId);
        if (it == m_subgraphs.end()) {
            return;
        }

        float viewScale = getViewScale();
        Vec2 viewPosition = getViewPosition();

        it->second->setViewState(viewPosition, viewScale);
    }

    void NodeEditor::saveSubgraphViewStateByUUID(const UUID& uuid) {
        auto it = m_subgraphsByUuid.find(uuid);
        if (it == m_subgraphsByUuid.end()) {
            return;
        }

        saveSubgraphViewState(it->second->id);
    }

    void NodeEditor::restoreSubgraphViewState(int subgraphId) {
        if (subgraphId < 0) {
            setViewScale(1.0f);
            setViewPosition(Vec2(0, 0));
            return;
        }

        auto it = m_subgraphs.find(subgraphId);
        if (it == m_subgraphs.end()) {
            return;
        }

        setViewScale(it->second->viewScale);
        setViewPosition(it->second->viewPosition);
    }

    void NodeEditor::restoreSubgraphViewStateByUUID(const UUID& uuid) {
        auto it = m_subgraphsByUuid.find(uuid);
        if (it == m_subgraphsByUuid.end()) {
            return;
        }

        restoreSubgraphViewState(it->second->id);
    }

    NodeEditorCore::NodeEditorStyle NodeEditor::convertToInternalStyle(const EditorStyle &style) const {
        return NodeEditorCore::convertToInternalStyle(style);
    }

    EditorStyle NodeEditor::convertToAPIStyle(const NodeEditorCore::NodeEditorStyle &style) const {
        return NodeEditorCore::convertToAPIStyle(style);
    }

    void NodeEditor::addNodeToSubgraph(int nodeId, int subgraphId) {
        auto it = m_subgraphs.find(subgraphId);
        if (it == m_subgraphs.end()) {
            return;
        }

        Node *node = getNode(nodeId);
        if (!node) {
            return;
        }

        it->second->addNode(nodeId, node->uuid);
        NodeEditorCore::Node *internalNode = m_editor.getNode(nodeId);
        if (internalNode) {
            internalNode->setSubgraphId(subgraphId);
        }
    }

    void NodeEditor::addNodeToSubgraphByUUID(const UUID& nodeUuid, const UUID& subgraphUuid) {
        auto it = m_subgraphsByUuid.find(subgraphUuid);
        if (it == m_subgraphsByUuid.end()) {
            return;
        }

        Node *node = getNodeByUUID(nodeUuid);
        if (!node) {
            return;
        }

        it->second->addNode(node->id, nodeUuid);
        NodeEditorCore::Node *internalNode = m_editor.getNodeByUUID(nodeUuid);
        if (internalNode) {
            internalNode->setSubgraphId(it->second->id);
        }
    }

    void NodeEditor::removeNodeFromSubgraph(int nodeId, int subgraphId) {
        auto it = m_subgraphs.find(subgraphId);
        if (it == m_subgraphs.end()) {
            return;
        }

        Node *node = getNode(nodeId);
        if (!node) {
            return;
        }

        it->second->removeNode(nodeId);
        NodeEditorCore::Node *internalNode = m_editor.getNode(nodeId);
        if (internalNode) {
            internalNode->setSubgraphId(-1);
        }

        std::vector<int> connectionsToRemove;
        for (int connectionId: getConnectionsInSubgraph(subgraphId)) {
            ConnectionInfo info = getConnectionInfo(connectionId);
            if (info.startNodeId == nodeId || info.endNodeId == nodeId) {
                connectionsToRemove.push_back(connectionId);
            }
        }

        for (int connectionId: connectionsToRemove) {
            it->second->removeConnection(connectionId);
            NodeEditorCore::Connection *connection = m_editor.getConnection(connectionId);
            if (connection) {
                connection->setSubgraphId(-1);
            }
        }
    }

    void NodeEditor::removeNodeFromSubgraphByUUID(const UUID& nodeUuid, const UUID& subgraphUuid) {
        auto it = m_subgraphsByUuid.find(subgraphUuid);
        if (it == m_subgraphsByUuid.end()) {
            return;
        }

        Node *node = getNodeByUUID(nodeUuid);
        if (!node) {
            return;
        }

        it->second->removeNodeByUUID(nodeUuid);
        NodeEditorCore::Node *internalNode = m_editor.getNodeByUUID(nodeUuid);
        if (internalNode) {
            internalNode->setSubgraphId(-1);
        }

        std::vector<UUID> connectionsToRemove;
        for (const UUID& connectionUuid: getConnectionsInSubgraphByUUID(subgraphUuid)) {
            ConnectionInfo info = getConnectionInfoByUUID(connectionUuid);
            if (info.startNodeUuid == nodeUuid || info.endNodeUuid == nodeUuid) {
                connectionsToRemove.push_back(connectionUuid);
            }
        }

        for (const UUID& connectionUuid: connectionsToRemove) {
            it->second->removeConnectionByUUID(connectionUuid);
            NodeEditorCore::Connection *connection = m_editor.getConnectionByUUID(connectionUuid);
            if (connection) {
                connection->setSubgraphId(-1);
            }
        }
    }

    void NodeEditor::addConnectionToSubgraph(int connectionId, int subgraphId) {
        auto it = m_subgraphs.find(subgraphId);
        if (it == m_subgraphs.end()) {
            return;
        }

        NodeEditorCore::Connection *connection = m_editor.getConnection(connectionId);
        if (!connection) {
            return;
        }

        it->second->addConnection(connectionId, connection->uuid);
        connection->setSubgraphId(subgraphId);
    }

    void NodeEditor::addConnectionToSubgraphByUUID(const UUID& connectionUuid, const UUID& subgraphUuid) {
        auto it = m_subgraphsByUuid.find(subgraphUuid);
        if (it == m_subgraphsByUuid.end()) {
            return;
        }

        NodeEditorCore::Connection *connection = m_editor.getConnectionByUUID(connectionUuid);
        if (!connection) {
            return;
        }

        it->second->addConnection(connection->id, connectionUuid);
        connection->setSubgraphId(it->second->id);
    }

    void NodeEditor::removeConnectionFromSubgraph(int connectionId, int subgraphId) {
        auto it = m_subgraphs.find(subgraphId);
        if (it == m_subgraphs.end()) {
            return;
        }

        NodeEditorCore::Connection *connection = m_editor.getConnection(connectionId);
        if (!connection) {
            return;
        }

        it->second->removeConnection(connectionId);
        connection->setSubgraphId(-1);
    }

    void NodeEditor::removeConnectionFromSubgraphByUUID(const UUID& connectionUuid, const UUID& subgraphUuid) {
        auto it = m_subgraphsByUuid.find(subgraphUuid);
        if (it == m_subgraphsByUuid.end()) {
            return;
        }

        NodeEditorCore::Connection *connection = m_editor.getConnectionByUUID(connectionUuid);
        if (!connection) {
            return;
        }

        it->second->removeConnectionByUUID(connectionUuid);
        connection->setSubgraphId(-1);
    }

    std::vector<int> NodeEditor::getNodesInSubgraph(int subgraphId) const {
        auto it = m_subgraphs.find(subgraphId);
        if (it == m_subgraphs.end()) {
            return {};
        }

        return it->second->nodeIds;
    }

    std::vector<UUID> NodeEditor::getNodesInSubgraphByUUID(const UUID& subgraphUuid) const {
        auto it = m_subgraphsByUuid.find(subgraphUuid);
        if (it == m_subgraphsByUuid.end()) {
            return {};
        }

        return it->second->nodeUuids;
    }

    std::vector<int> NodeEditor::getConnectionsInSubgraph(int subgraphId) const {
        auto it = m_subgraphs.find(subgraphId);
        if (it == m_subgraphs.end()) {
            return {};
        }

        return it->second->connectionIds;
    }

    std::vector<UUID> NodeEditor::getConnectionsInSubgraphByUUID(const UUID& subgraphUuid) const {
        auto it = m_subgraphsByUuid.find(subgraphUuid);
        if (it == m_subgraphsByUuid.end()) {
            return {};
        }

        return it->second->connectionUuids;
    }

    NodeEditor::ConnectionInfo NodeEditor::getConnectionInfo(int connectionId) const {
        ConnectionInfo info;
        info.id = connectionId;
        info.startNodeId = -1;
        info.endNodeId = -1;

        const NodeEditorCore::Connection *connection = m_editor.getConnection(connectionId);
        if (connection) {
            info.id = connectionId;
            info.uuid = connection->uuid;
            info.startNodeId = connection->startNodeId;
            info.startNodeUuid = connection->startNodeUuid;
            info.endNodeId = connection->endNodeId;
            info.endNodeUuid = connection->endNodeUuid;
        }

        return info;
    }

    NodeEditor::ConnectionInfo NodeEditor::getConnectionInfoByUUID(const UUID& uuid) const {
        ConnectionInfo info;
        info.uuid = uuid;
        info.startNodeId = -1;
        info.endNodeId = -1;

        const NodeEditorCore::Connection *connection = m_editor.getConnectionByUUID(uuid);
        if (connection) {
            info.id = connection->id;
            info.uuid = uuid;
            info.startNodeId = connection->startNodeId;
            info.startNodeUuid = connection->startNodeUuid;
            info.endNodeId = connection->endNodeId;
            info.endNodeUuid = connection->endNodeUuid;
        }

        return info;
    }

    void NodeEditor::updateSubgraphUuidMap() {
        m_subgraphsByUuid.clear();
        for (const auto& pair : m_subgraphs) {
            m_subgraphsByUuid[pair.second->uuid] = pair.second;
        }
    }

    std::vector<UUID> NodeEditor::getAllNodeUUIDs() const {
        std::vector<UUID> result;
        for (const auto& node : m_editor.getNodes()) {
            result.push_back(node.uuid);
        }
        return result;
    }

    std::vector<UUID> NodeEditor::getAllConnectionUUIDs() const {
        std::vector<UUID> result;
        for (const auto& connection : m_editor.getConnections()) {
            result.push_back(connection.uuid);
        }
        return result;
    }

    std::vector<UUID> NodeEditor::getAllGroupUUIDs() const {
        std::vector<UUID> result;
        for (int i = 0; i < 10000; i++) {
            const NodeEditorCore::Group* group = m_editor.getGroup(i);
            if (group) {
                result.push_back(group->uuid);
            }
        }
        return result;
    }

    std::vector<UUID> NodeEditor::getAllSubgraphUUIDs() const {
        std::vector<UUID> result;
        result.reserve(m_subgraphsByUuid.size());

        for (const auto& pair : m_subgraphsByUuid) {
            result.push_back(pair.first);
        }

        return result;
    }

    // Méthodes d'évaluation
    std::vector<int> NodeEditor::getEvaluationOrder() {
        return m_editor.getEvaluationOrder();
    }

    std::vector<UUID> NodeEditor::getEvaluationOrderUUIDs() {
        return m_editor.getEvaluationOrderUUIDs();
    }

    std::vector<NodeEvaluator::ConnectionInfo> NodeEditor::getInputConnections(int nodeId) {
        return m_editor.getInputConnections(nodeId);
    }

    std::vector<NodeEvaluator::ConnectionInfo> NodeEditor::getInputConnectionsByUUID(const UUID& nodeUuid) {
        return m_editor.getInputConnectionsByUUID(nodeUuid);
    }

    std::vector<NodeEvaluator::ConnectionInfo> NodeEditor::getOutputConnections(int nodeId) {
        return m_editor.getOutputConnections(nodeId);
    }

    std::vector<NodeEvaluator::ConnectionInfo> NodeEditor::getOutputConnectionsByUUID(const UUID& nodeUuid) {
        return m_editor.getOutputConnectionsByUUID(nodeUuid);
    }
}