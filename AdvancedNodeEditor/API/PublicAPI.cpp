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

    int NodeEditor::addNode(const std::string &name, const std::string &type, const Vec2 &position) {
        int nodeId = m_editor.addNode(name, type, NodeEditorCore::convertToVec2(position));

        std::unordered_map<std::string, std::any> data;
        data["nodeId"] = nodeId;
        data["name"] = name;
        data["type"] = type;
        data["position"] = position;
        dispatchAction("nodeAdded", data);

        return nodeId;
    }

    Node *NodeEditor::getNode(int nodeId) {
        NodeEditorCore::Node *internalNode = m_editor.getNode(nodeId);
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
        Node *node = getNode(nodeId);
        if (node) {
            std::unordered_map<std::string, std::any> data;
            data["nodeId"] = nodeId;
            data["name"] = node->name;
            data["type"] = node->type;

            m_editor.removeNode(nodeId);

            dispatchAction("nodeRemoved", data);
        } else {
            m_editor.removeNode(nodeId);
        }
    }

    int NodeEditor::addPin(int nodeId, const std::string &name, bool isInput, PinType type, PinShape shape) {
        return m_editor.addPin(
            nodeId,
            name,
            isInput,
            NodeEditorCore::convertToPinType(type),
            NodeEditorCore::convertToPinShape(shape)
        );
    }

    Pin *NodeEditor::getPin(int nodeId, int pinId) {
        return m_editor.getPin(nodeId, pinId);
    }

    int NodeEditor::addConnection(int startNodeId, int startPinId, int endNodeId, int endPinId) {
        int connectionId = m_editor.addConnection(startNodeId, startPinId, endNodeId, endPinId);

        if (connectionId >= 0) {
            std::unordered_map<std::string, std::any> data;
            data["connectionId"] = connectionId;
            data["startNodeId"] = startNodeId;
            data["startPinId"] = startPinId;
            data["endNodeId"] = endNodeId;
            data["endPinId"] = endPinId;
            dispatchAction("connectionAdded", data);
        }

        return connectionId;
    }

    void NodeEditor::removeConnection(int connectionId) {
        NodeEditorCore::Connection *connection = m_editor.getConnection(connectionId);
        if (connection) {
            std::unordered_map<std::string, std::any> data;
            data["connectionId"] = connectionId;
            data["startNodeId"] = connection->startNodeId;
            data["startPinId"] = connection->startPinId;
            data["endNodeId"] = connection->endNodeId;
            data["endPinId"] = connection->endPinId;

            m_editor.removeConnection(connectionId);

            dispatchAction("connectionRemoved", data);
        } else {
            m_editor.removeConnection(connectionId);
        }
    }

    int NodeEditor::addGroup(const std::string &name, const Vec2 &position, const Vec2 &size) {
        return m_editor.addGroup(
            name,
            NodeEditorCore::convertToVec2(position),
            NodeEditorCore::convertToVec2(size)
        );
    }

    Group *NodeEditor::getGroup(int groupId) {
        NodeEditorCore::Group *internalGroup = m_editor.getGroup(groupId);
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

    void NodeEditor::setViewPosition(const Vec2 &position) {
        m_editor.setViewPosition(NodeEditorCore::convertToVec2(position));
    }

    Vec2 NodeEditor::getViewPosition() const {
        NodeEditorCore::Vec2 pos = m_editor.getViewPosition();
        return Vec2(pos.x, pos.y);
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

    void NodeEditor::evaluateNode(int nodeId, const NodeEvaluationContext &context) {
        Node *node = getNode(nodeId);
        if (node && m_nodeEvaluateCallback) {
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

    void NodeEditor::registerNodeType(const std::string &type, const std::string &category,
                                      const std::string &description, std::function<Node*(const Vec2 &)> builder) {
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

    SerializedState NodeEditor::serialize() const {
        SerializedState state;

        for (const auto &node: m_editor.getNodes()) {
            SerializedNode serializedNode;
            serializedNode.id = node.id;
            serializedNode.name = node.name;
            serializedNode.type = node.type;
            serializedNode.position = NodeEditorCore::convertToANEVec2(node.position);
            serializedNode.size = NodeEditorCore::convertToANEVec2(node.size);
            serializedNode.isSubgraph = node.isSubgraph;
            serializedNode.subgraphId = node.subgraphId;

            for (const auto &pin: node.inputs) {
                std::unordered_map<std::string, std::any> serializedPin;
                serializedPin["id"] = pin.id;
                serializedPin["name"] = pin.name;
                serializedPin["type"] = static_cast<int>(pin.type);
                serializedPin["shape"] = static_cast<int>(pin.shape);
                serializedNode.inputs.push_back(serializedPin);
            }

            for (const auto &pin: node.outputs) {
                std::unordered_map<std::string, std::any> serializedPin;
                serializedPin["id"] = pin.id;
                serializedPin["name"] = pin.name;
                serializedPin["type"] = static_cast<int>(pin.type);
                serializedPin["shape"] = static_cast<int>(pin.shape);
                serializedNode.outputs.push_back(serializedPin);
            }

            for (const auto &pair: node.metadata.attributes) {
                serializedNode.metadata[pair.first] = pair.second;
            }

            state.nodes.push_back(serializedNode);
        }

        for (const auto &connection: m_editor.getConnections()) {
            SerializedConnection serializedConnection;
            serializedConnection.id = connection.id;
            serializedConnection.startNodeId = connection.startNodeId;
            serializedConnection.startPinId = connection.startPinId;
            serializedConnection.endNodeId = connection.endNodeId;
            serializedConnection.endPinId = connection.endPinId;

            for (const auto &pair: connection.metadata.attributes) {
                serializedConnection.metadata[pair.first] = pair.second;
            }

            state.connections.push_back(serializedConnection);
        }

        for (const auto &pair: m_subgraphs) {
            const auto &subgraph = pair.second;

            SerializedSubgraph serializedSubgraph;
            serializedSubgraph.id = subgraph->id;
            serializedSubgraph.name = subgraph->name;
            serializedSubgraph.nodeIds = subgraph->nodeIds;
            serializedSubgraph.connectionIds = subgraph->connectionIds;
            serializedSubgraph.groupIds = subgraph->groupIds;
            serializedSubgraph.interfaceInputs = subgraph->interfaceInputs;
            serializedSubgraph.interfaceOutputs = subgraph->interfaceOutputs;
            serializedSubgraph.viewPosition = subgraph->viewPosition;
            serializedSubgraph.viewScale = subgraph->viewScale;

            for (const auto &metaPair: subgraph->metadata.attributes) {
                serializedSubgraph.metadata[metaPair.first] = metaPair.second;
            }

            state.subgraphs.push_back(serializedSubgraph);
        }

        return state;
    }

    void NodeEditor::deserialize(const SerializedState &state) {
        for (const auto &node: m_editor.getNodes()) {
            m_editor.removeNode(node.id);
        }

        m_subgraphs.clear();

        for (const auto &serializedNode: state.nodes) {
            int nodeId = m_editor.addNode(serializedNode.name, serializedNode.type,
                                          NodeEditorCore::convertToVec2(serializedNode.position));
            Node *node = getNode(nodeId);
            if (node) {
                node->setAsSubgraph(serializedNode.isSubgraph, serializedNode.subgraphId);

                for (const auto &serializedPin: serializedNode.inputs) {
                    m_editor.addPin(
                        nodeId,
                        std::any_cast<std::string>(serializedPin.at("name")),
                        true,
                        NodeEditorCore::convertToPinType(
                            static_cast<PinType>(std::any_cast<int>(serializedPin.at("type")))),
                        NodeEditorCore::convertToPinShape(
                            static_cast<PinShape>(std::any_cast<int>(serializedPin.at("shape"))))
                    );
                }

                for (const auto &serializedPin: serializedNode.outputs) {
                    m_editor.addPin(
                        nodeId,
                        std::any_cast<std::string>(serializedPin.at("name")),
                        false,
                        NodeEditorCore::convertToPinType(
                            static_cast<PinType>(std::any_cast<int>(serializedPin.at("type")))),
                        NodeEditorCore::convertToPinShape(
                            static_cast<PinShape>(std::any_cast<int>(serializedPin.at("shape"))))
                    );
                }

                for (const auto &pair: serializedNode.metadata) {
                    node->metadata.setAttribute(pair.first, pair.second);
                }
            }
        }

        for (const auto &serializedConnection: state.connections) {
            int connectionId = m_editor.addConnection(
                serializedConnection.startNodeId,
                serializedConnection.startPinId,
                serializedConnection.endNodeId,
                serializedConnection.endPinId
            );

            if (connectionId >= 0) {
                NodeEditorCore::Connection *connection = m_editor.getConnection(connectionId);
                if (connection) {
                    for (const auto &pair: serializedConnection.metadata) {
                        connection->metadata.setAttribute(pair.first, pair.second);
                    }
                }
            }
        }

        for (const auto &serializedSubgraph: state.subgraphs) {
            auto subgraph = std::make_shared<Subgraph>(serializedSubgraph.id, serializedSubgraph.name);
            subgraph->nodeIds = serializedSubgraph.nodeIds;
            subgraph->connectionIds = serializedSubgraph.connectionIds;
            subgraph->groupIds = serializedSubgraph.groupIds;
            subgraph->interfaceInputs = serializedSubgraph.interfaceInputs;
            subgraph->interfaceOutputs = serializedSubgraph.interfaceOutputs;
            subgraph->viewPosition = serializedSubgraph.viewPosition;
            subgraph->viewScale = serializedSubgraph.viewScale;

            for (const auto &pair: serializedSubgraph.metadata) {
                subgraph->metadata.setAttribute(pair.first, pair.second);
            }

            m_subgraphs[serializedSubgraph.id] = subgraph;
        }
    }

    int NodeEditor::createSubgraph(const std::string &name) {
        int subgraphId = Subgraph::nextId++;
        auto subgraph = std::make_shared<Subgraph>(subgraphId, name);
        m_subgraphs[subgraphId] = subgraph;
        return subgraphId;
    }

    Subgraph *NodeEditor::getSubgraph(int subgraphId) {
        auto it = m_subgraphs.find(subgraphId);
        if (it != m_subgraphs.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    void NodeEditor::removeSubgraph(int subgraphId) {
        auto it = m_subgraphs.find(subgraphId);
        if (it == m_subgraphs.end()) {
            return;
        }

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

        m_subgraphs.erase(it);
    }

    Node *NodeEditor::createSubgraphNode(int subgraphId, const std::string &name, const Vec2 &position) {
        if (m_subgraphs.find(subgraphId) == m_subgraphs.end()) {
            return nullptr;
        }

        int nodeId = addNode(name.empty() ? m_subgraphs[subgraphId]->name : name, "Subgraph", position);
        Node *node = getNode(nodeId);
        if (node) {
            node->setAsSubgraph(true, subgraphId);

            if (!m_subgraphs[subgraphId]->iconSymbol.empty()) {
                node->setIconSymbol(m_subgraphs[subgraphId]->iconSymbol);
            } else {
                node->setIconSymbol("S");
            }
        }
        return node;
    }

    bool NodeEditor::enterSubgraph(int subgraphId) {
        if (m_subgraphs.find(subgraphId) == m_subgraphs.end()) {
            return false;
        }

        saveSubgraphViewState(m_currentSubgraphId);

        std::unordered_map<std::string, std::any> data;
        data["previousSubgraphId"] = m_currentSubgraphId;
        data["subgraphId"] = subgraphId;

        if (m_currentSubgraphId >= 0) {
            m_subgraphStack.push(m_currentSubgraphId);
        }
        m_currentSubgraphId = subgraphId;
        m_editor.setCurrentSubgraphId(subgraphId);

        restoreSubgraphViewState(subgraphId);

        dispatchAction("enterSubgraph", data);

        return true;
    }


    bool NodeEditor::exitSubgraph() {
        if (m_currentSubgraphId < 0) {
            return false;
        }

        saveSubgraphViewState(m_currentSubgraphId);

        std::unordered_map<std::string, std::any> data;
        data["previousSubgraphId"] = m_currentSubgraphId;

        int parentSubgraphId = -1;
        if (!m_subgraphStack.empty()) {
            parentSubgraphId = m_subgraphStack.top();
            m_subgraphStack.pop();
        }

        data["subgraphId"] = parentSubgraphId;

        m_currentSubgraphId = parentSubgraphId;
        m_editor.setCurrentSubgraphId(parentSubgraphId);

        restoreSubgraphViewState(m_currentSubgraphId);

        dispatchAction("exitSubgraph", data);

        return true;
    }

    int NodeEditor::getCurrentSubgraphId() const {
        return m_currentSubgraphId;
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

    void NodeEditor::exposeNodeInput(int nodeId, int pinId) {
        if (m_currentSubgraphId < 0) {
            return;
        }

        auto it = m_subgraphs.find(m_currentSubgraphId);
        if (it != m_subgraphs.end()) {
            it->second->exposeInput(nodeId, pinId);
        }
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

    void NodeEditor::unexposeNodeInput(int nodeId, int pinId) {
        if (m_currentSubgraphId < 0) {
            return;
        }

        auto it = m_subgraphs.find(m_currentSubgraphId);
        if (it != m_subgraphs.end()) {
            it->second->unexposeInput(nodeId, pinId);
        }
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

        it->second->addNode(nodeId);
        NodeEditorCore::Node *internalNode = m_editor.getNode(nodeId);
        if (internalNode) {
            internalNode->setSubgraphId(subgraphId);
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

    void NodeEditor::addConnectionToSubgraph(int connectionId, int subgraphId) {
        auto it = m_subgraphs.find(subgraphId);
        if (it == m_subgraphs.end()) {
            return;
        }

        it->second->addConnection(connectionId);
        NodeEditorCore::Connection *connection = m_editor.getConnection(connectionId);
        if (connection) {
            connection->setSubgraphId(subgraphId);
        }
    }

    void NodeEditor::removeConnectionFromSubgraph(int connectionId, int subgraphId) {
        auto it = m_subgraphs.find(subgraphId);
        if (it == m_subgraphs.end()) {
            return;
        }

        it->second->removeConnection(connectionId);
        NodeEditorCore::Connection *connection = m_editor.getConnection(connectionId);
        if (connection) {
            connection->setSubgraphId(-1);
        }
    }

    std::vector<int> NodeEditor::getNodesInSubgraph(int subgraphId) const {
        auto it = m_subgraphs.find(subgraphId);
        if (it == m_subgraphs.end()) {
            return {};
        }

        return it->second->nodeIds;
    }

    std::vector<int> NodeEditor::getConnectionsInSubgraph(int subgraphId) const {
        auto it = m_subgraphs.find(subgraphId);
        if (it == m_subgraphs.end()) {
            return {};
        }

        return it->second->connectionIds;
    }

    NodeEditor::ConnectionInfo NodeEditor::getConnectionInfo(int connectionId) const {
        ConnectionInfo info;
        info.id = connectionId;
        info.startNodeId = -1;
        info.endNodeId = -1;

        const NodeEditorCore::Connection *connection = m_editor.getConnection(connectionId);
        if (connection) {
            info.startNodeId = connection->startNodeId;
            info.endNodeId = connection->endNodeId;
        }

        return info;
    }
}
