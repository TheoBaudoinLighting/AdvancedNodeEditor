#include "../NodeEditor.h"
#include "../Core/Conversions/Conversions.h"

namespace ANE {

    NodeEvaluator::ConnectionInfo::ConnectionInfo()
        : connectionId(-1), connectionUuid(""),
          sourceNodeId(-1), sourceNodeUuid(""), sourcePinId(-1), sourcePinUuid(""),
          targetNodeId(-1), targetNodeUuid(""), targetPinId(-1), targetPinUuid("") {
    }

    std::vector<NodeEvaluator::ConnectionInfo> NodeEvaluator::getInputConnections(NodeEditor& editor, int nodeId) {
        return editor.getInputConnections(nodeId);
    }

    std::vector<NodeEvaluator::ConnectionInfo> NodeEvaluator::getInputConnectionsByUUID(NodeEditor& editor, const UUID& nodeUuid) {
        return editor.getInputConnectionsByUUID(nodeUuid);
    }

    std::vector<NodeEvaluator::ConnectionInfo> NodeEvaluator::getOutputConnections(NodeEditor& editor, int nodeId) {
        return editor.getOutputConnections(nodeId);
    }

    std::vector<NodeEvaluator::ConnectionInfo> NodeEvaluator::getOutputConnectionsByUUID(NodeEditor& editor, const UUID& nodeUuid) {
        return editor.getOutputConnectionsByUUID(nodeUuid);
    }

    std::vector<int> NodeEvaluator::getEvaluationOrder(NodeEditor& editor) {
        return editor.getEvaluationOrder();
    }

    std::vector<UUID> NodeEvaluator::getEvaluationOrderUUIDs(NodeEditor& editor) {
        return editor.getEvaluationOrderUUIDs();
    }

    NodeEvaluator::Connection* NodeEvaluator::getConnection(int connectionId) {
        static Connection result;
        
        for (const auto& connection : m_editor.getConnections()) {
            if (connection.id == connectionId) {
                result.id = connection.id;
                result.startNodeId = connection.startNodeId;
                result.startPinId = connection.startPinId;
                result.endNodeId = connection.endNodeId;
                result.endPinId = connection.endPinId;
                return &result;
            }
        }
        return nullptr;
    }

    int NodeEvaluator::getCurrentSubgraphId() const {
        return 0;
    }

    bool NodeEvaluator::isConnectionInSubgraph(int connectionId, int subgraphId) const {
        const auto& connections = m_editor.getConnections();
        for (const auto& connection : connections) {
            if (connection.id == connectionId) {
                const auto& nodes = m_editor.getNodes();
                for (const auto& node : nodes) {
                    if ((node.id == connection.startNodeId || node.id == connection.endNodeId) && 
                        node.subgraphId != subgraphId) {
                        return false;
                    }
                }
                return true;
            }
        }
        return false;
    }
}

namespace NodeEditorCore {
    
    ANE::UUID NodeEditor::getNodeUUID(int nodeId) const {
        for (const auto& node : getNodes()) {
            if (node.id == nodeId) {
                return node.uuid;
            }
        }
        return "";
    }

    int NodeEditor::getNodeId(const ANE::UUID& uuid) const {
        for (const auto& node : getNodes()) {
            if (node.uuid == uuid) {
                return node.id;
            }
        }
        return -1;
    }

    NodeEditorCore::Node* NodeEditor::getNodeByUUID(const ANE::UUID& uuid) {
        int nodeId = getNodeId(uuid);
        if (nodeId == -1) return nullptr;
        return getNode(nodeId);
    }

    const NodeEditorCore::Node* NodeEditor::getNodeByUUID(const ANE::UUID& uuid) const {
        int nodeId = getNodeId(uuid);
        if (nodeId == -1) return nullptr;
        return getNode(nodeId);
    }

    void NodeEditor::removeNodeByUUID(const ANE::UUID& uuid) {
        int nodeId = getNodeId(uuid);
        if (nodeId != -1) {
            removeNode(nodeId);
        }
    }

    void NodeEditor::updateNodeUuidMap() {
        State& state = *(State*)this;
        state.nodeUuidMap.clear();
        for (auto& node : state.nodes) {
            state.nodeUuidMap[node.uuid] = &node;
        }
    }

    ANE::UUID NodeEditor::getPinUUID(int nodeId, int pinId) const {
        const Node* node = getNode(nodeId);
        if (!node) return "";
        
        for (const auto& pin : node->inputs) {
            if (pin.id == pinId) {
                return pin.uuid;
            }
        }
        
        for (const auto& pin : node->outputs) {
            if (pin.id == pinId) {
                return pin.uuid;
            }
        }
        
        return "";
    }

    ANE::Pin* NodeEditor::getPinByUUID(const ANE::UUID& nodeUuid, const ANE::UUID& pinUuid) {
        static ANE::Pin convertedPin;
        
        Node* node = getNodeByUUID(nodeUuid);
        if (!node) return nullptr;
        
        for (auto& pin : node->inputs) {
            if (pin.uuid == pinUuid) {
                convertedPin.id = pin.id;
                convertedPin.uuid = pin.uuid;
                convertedPin.name = pin.name;
                convertedPin.isInput = pin.isInput;
                convertedPin.type = ANE::PinType(int(pin.type));
                convertedPin.shape = ANE::PinShape(int(pin.shape));
                return &convertedPin;
            }
        }
        
        for (auto& pin : node->outputs) {
            if (pin.uuid == pinUuid) {
                convertedPin.id = pin.id;
                convertedPin.uuid = pin.uuid;
                convertedPin.name = pin.name;
                convertedPin.isInput = pin.isInput;
                convertedPin.type = ANE::PinType(int(pin.type));
                convertedPin.shape = ANE::PinShape(int(pin.shape));
                return &convertedPin;
            }
        }
        
        return nullptr;
    }

    const ANE::Pin* NodeEditor::getPinByUUID(const ANE::UUID& nodeUuid, const ANE::UUID& pinUuid) const {
        static ANE::Pin convertedPin;
        
        const Node* node = getNodeByUUID(nodeUuid);
        if (!node) return nullptr;
        
        for (const auto& pin : node->inputs) {
            if (pin.uuid == pinUuid) {
                convertedPin.id = pin.id;
                convertedPin.uuid = pin.uuid;
                convertedPin.name = pin.name;
                convertedPin.isInput = pin.isInput;
                convertedPin.type = ANE::PinType(int(pin.type));
                convertedPin.shape = ANE::PinShape(int(pin.shape));
                return &convertedPin;
            }
        }
        
        for (const auto& pin : node->outputs) {
            if (pin.uuid == pinUuid) {
                convertedPin.id = pin.id;
                convertedPin.uuid = pin.uuid;
                convertedPin.name = pin.name;
                convertedPin.isInput = pin.isInput;
                convertedPin.type = ANE::PinType(int(pin.type));
                convertedPin.shape = ANE::PinShape(int(pin.shape));
                return &convertedPin;
            }
        }
        
        return nullptr;
    }

    ANE::UUID NodeEditor::getConnectionUUID(int connectionId) const {
        for (const auto& connection : getConnections()) {
            if (connection.id == connectionId) {
                return connection.uuid;
            }
        }
        return "";
    }

    NodeEditorCore::Connection* NodeEditor::getConnectionByUUID(const ANE::UUID& uuid) {
        for (auto& connection : m_state.connections) {
            if (connection.uuid == uuid) {
                return &connection;
            }
        }
        return nullptr;
    }

    const NodeEditorCore::Connection* NodeEditor::getConnectionByUUID(const ANE::UUID& uuid) const {
        for (const auto& connection : getConnections()) {
            if (connection.uuid == uuid) {
                return &connection;
            }
        }
        return nullptr;
    }

    void NodeEditor::removeConnectionByUUID(const ANE::UUID& uuid) {
        for (const auto& connection : getConnections()) {
            if (connection.uuid == uuid) {
                removeConnection(connection.id);
                break;
            }
        }
    }

    int NodeEditor::addConnectionByUUID(const ANE::UUID& startNodeUuid, const ANE::UUID& startPinUuid,
                                     const ANE::UUID& endNodeUuid, const ANE::UUID& endPinUuid, const ANE::UUID& uuid) {
        int startNodeId = getNodeId(startNodeUuid);
        int endNodeId = getNodeId(endNodeUuid);
        
        if (startNodeId == -1 || endNodeId == -1) return -1;
        
        const Node* startNode = getNode(startNodeId);
        const Node* endNode = getNode(endNodeId);
        
        if (!startNode || !endNode) return -1;
        
        int startPinId = -1;
        int endPinId = -1;
        
        for (const auto& pin : startNode->outputs) {
            if (pin.uuid == startPinUuid) {
                startPinId = pin.id;
                break;
            }
        }
        
        for (const auto& pin : endNode->inputs) {
            if (pin.uuid == endPinUuid) {
                endPinId = pin.id;
                break;
            }
        }
        
        if (startPinId == -1 || endPinId == -1) return -1;
        
        return addConnection(startNodeId, startPinId, endNodeId, endPinId, uuid);
    }

    NodeEditorCore::Group* NodeEditor::getGroupByUUID(const ANE::UUID& uuid) {
        for (int i = 0; i < 10000; i++) {
            Group* group = getGroup(i);
            if (group && group->uuid == uuid) {
                return group;
            }
        }
        return nullptr;
    }

    void NodeEditor::removeGroupByUUID(const ANE::UUID& uuid) {
        for (int i = 0; i < 10000; i++) {
            const Group* group = getGroup(i);
            if (group && group->uuid == uuid) {
                removeGroup(group->id);
                break;
            }
        }
    }

    void NodeEditor::addNodeToGroupByUUID(const ANE::UUID& nodeUuid, const ANE::UUID& groupUuid) {
        int nodeId = getNodeId(nodeUuid);
        if (nodeId == -1) return;
        
        Group* group = getGroupByUUID(groupUuid);
        if (group) {
            addNodeToGroup(nodeId, group->id);
        }
    }

    void NodeEditor::selectNodeByUUID(const ANE::UUID& uuid, bool append) {
        int nodeId = getNodeId(uuid);
        if (nodeId != -1) {
            selectNode(nodeId, append);
        }
    }

    std::vector<ANE::UUID> NodeEditor::getSelectedNodeUUIDs() const {
        std::vector<ANE::UUID> result;
        std::vector<int> selectedNodes = getSelectedNodes();
        
        for (int nodeId : selectedNodes) {
            result.push_back(getNodeUUID(nodeId));
        }
        
        return result;
    }
    
    std::vector<int> NodeEditor::getEvaluationOrder() const {
        ANE::NodeEvaluator evaluator(const_cast<NodeEditor&>(*this));
        return evaluator.getEvaluationOrder();
    }

    std::vector<ANE::UUID> NodeEditor::getEvaluationOrderUUIDs() const {
        std::vector<ANE::UUID> result;
        std::vector<int> order = getEvaluationOrder();
        
        for (int nodeId : order) {
            result.push_back(getNodeUUID(nodeId));
        }
        
        return result;
    }

    std::vector<ANE::NodeEvaluator::ConnectionInfo> NodeEditor::getInputConnections(int nodeId) {
        std::vector<ANE::NodeEvaluator::ConnectionInfo> result;
        
        for (const auto& connection : getConnections()) {
            if (connection.endNodeId == nodeId) {
                ANE::NodeEvaluator::ConnectionInfo info;
                info.connectionId = connection.id;
                info.connectionUuid = getConnectionUUID(connection.id);
                info.sourceNodeId = connection.startNodeId;
                info.sourceNodeUuid = getNodeUUID(connection.startNodeId);
                info.sourcePinId = connection.startPinId;
                info.sourcePinUuid = getPinUUID(connection.startNodeId, connection.startPinId);
                info.targetNodeId = connection.endNodeId;
                info.targetNodeUuid = getNodeUUID(connection.endNodeId);
                info.targetPinId = connection.endPinId;
                info.targetPinUuid = getPinUUID(connection.endNodeId, connection.endPinId);
                result.push_back(info);
            }
        }
        
        return result;
    }

    std::vector<ANE::NodeEvaluator::ConnectionInfo> NodeEditor::getInputConnectionsByUUID(const ANE::UUID& nodeUuid) {
        int nodeId = getNodeId(nodeUuid);
        return getInputConnections(nodeId);
    }

    std::vector<ANE::NodeEvaluator::ConnectionInfo> NodeEditor::getOutputConnections(int nodeId) {
        std::vector<ANE::NodeEvaluator::ConnectionInfo> result;
        
        for (const auto& connection : getConnections()) {
            if (connection.startNodeId == nodeId) {
                ANE::NodeEvaluator::ConnectionInfo info;
                info.connectionId = connection.id;
                info.connectionUuid = getConnectionUUID(connection.id);
                info.sourceNodeId = connection.startNodeId;
                info.sourceNodeUuid = getNodeUUID(connection.startNodeId);
                info.sourcePinId = connection.startPinId;
                info.sourcePinUuid = getPinUUID(connection.startNodeId, connection.startPinId);
                info.targetNodeId = connection.endNodeId;
                info.targetNodeUuid = getNodeUUID(connection.endNodeId);
                info.targetPinId = connection.endPinId;
                info.targetPinUuid = getPinUUID(connection.endNodeId, connection.endPinId);
                result.push_back(info);
            }
        }
        
        return result;
    }

    std::vector<ANE::NodeEvaluator::ConnectionInfo> NodeEditor::getOutputConnectionsByUUID(const ANE::UUID& nodeUuid) {
        int nodeId = getNodeId(nodeUuid);
        return getOutputConnections(nodeId);
    }
}