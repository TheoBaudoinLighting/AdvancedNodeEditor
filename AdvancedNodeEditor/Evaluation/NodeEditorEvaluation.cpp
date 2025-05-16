#include "../NodeEditor.h"
#include "../Core/Conversions/Conversions.h"

namespace NodeEditorCore {

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
    
    UUID NodeEditor::getNodeUUID(int nodeId) const {
        for (const auto& node : getNodes()) {
            if (node.id == nodeId) {
                return node.uuid;
            }
        }
        return "";
    }

    int NodeEditor::getNodeId(const UUID& uuid) const {
        for (const auto& node : getNodes()) {
            if (node.uuid == uuid) {
                return node.id;
            }
        }
        return -1;
    }

    NodeEditorCore::Node* NodeEditor::getNodeByUUID(const UUID& uuid) {
        int nodeId = getNodeId(uuid);
        if (nodeId == -1) return nullptr;
        return getNode(nodeId);
    }

    const NodeEditorCore::Node* NodeEditor::getNodeByUUID(const UUID& uuid) const {
        int nodeId = getNodeId(uuid);
        if (nodeId == -1) return nullptr;
        return getNode(nodeId);
    }

    void NodeEditor::removeNodeByUUID(const UUID& uuid) {
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

    UUID NodeEditor::getPinUUID(int nodeId, int pinId) const {
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

    Pin* NodeEditor::getPinByUUID(const UUID& nodeUuid, const UUID& pinUuid) {
        Node* node = getNodeByUUID(nodeUuid);
        if (!node) return nullptr;

        for (auto& pin : node->inputs) {
            if (pin.uuid == pinUuid) {
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
            if (pin.uuid == pinUuid) {
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

    const Pin* NodeEditor::getPinByUUID(const UUID& nodeUuid, const UUID& pinUuid) const {
        const Node* node = getNodeByUUID(nodeUuid);
        if (!node) return nullptr;

        for (const auto& pin : node->inputs) {
            if (pin.uuid == pinUuid) {
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
            if (pin.uuid == pinUuid) {
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

    UUID NodeEditor::getConnectionUUID(int connectionId) const {
        for (const auto& connection : getConnections()) {
            if (connection.id == connectionId) {
                return connection.uuid;
            }
        }
        return "";
    }

    void NodeEditor::removeConnectionByUUID(const UUID& uuid) {
        for (const auto& connection : getConnections()) {
            if (connection.uuid == uuid) {
                removeConnection(connection.id);
                break;
            }
        }
    }

    NodeEditorCore::Group* NodeEditor::getGroupByUUID(const UUID& uuid) {
        for (int i = 0; i < 10000; i++) {
            Group* group = getGroup(i);
            if (group && group->uuid == uuid) {
                return group;
            }
        }
        return nullptr;
    }

    void NodeEditor::removeGroupByUUID(const UUID& uuid) {
        for (int i = 0; i < 10000; i++) {
            const Group* group = getGroup(i);
            if (group && group->uuid == uuid) {
                removeGroup(group->id);
                break;
            }
        }
    }

    void NodeEditor::addNodeToGroupByUUID(const UUID& nodeUuid, const UUID& groupUuid) {
        int nodeId = getNodeId(nodeUuid);
        if (nodeId == -1) return;
        
        Group* group = getGroupByUUID(groupUuid);
        if (group) {
            addNodeToGroup(nodeId, group->id);
        }
    }

    void NodeEditor::selectNodeByUUID(const UUID& uuid, bool append) {
        int nodeId = getNodeId(uuid);
        if (nodeId != -1) {
            selectNode(nodeId, append);
        }
    }

    std::vector<UUID> NodeEditor::getSelectedNodeUUIDs() const {
        std::vector<UUID> result;
        std::vector<int> selectedNodes = getSelectedNodes();
        
        for (int nodeId : selectedNodes) {
            result.push_back(getNodeUUID(nodeId));
        }
        
        return result;
    }
    
    std::vector<int> NodeEditor::getEvaluationOrder() const {
        NodeEvaluator evaluator(const_cast<NodeEditor&>(*this));
        return evaluator.getEvaluationOrder();
    }

    std::vector<UUID> NodeEditor::getEvaluationOrderUUIDs() const {
        std::vector<UUID> result;
        std::vector<int> order = getEvaluationOrder();
        
        for (int nodeId : order) {
            result.push_back(getNodeUUID(nodeId));
        }
        
        return result;
    }

    std::vector<NodeEvaluator::ConnectionInfo> NodeEditor::getInputConnections(int nodeId) {
        std::vector<NodeEvaluator::ConnectionInfo> result;
        
        for (const auto& connection : getConnections()) {
            if (connection.endNodeId == nodeId) {
                NodeEvaluator::ConnectionInfo info;
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

    std::vector<NodeEvaluator::ConnectionInfo> NodeEditor::getInputConnectionsByUUID(const UUID& nodeUuid) {
        int nodeId = getNodeId(nodeUuid);
        return getInputConnections(nodeId);
    }

    std::vector<NodeEvaluator::ConnectionInfo> NodeEditor::getOutputConnections(int nodeId) {
        std::vector<NodeEvaluator::ConnectionInfo> result;
        
        for (const auto& connection : getConnections()) {
            if (connection.startNodeId == nodeId) {
                NodeEvaluator::ConnectionInfo info;
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

    std::vector<NodeEvaluator::ConnectionInfo> NodeEditor::getOutputConnectionsByUUID(const UUID& nodeUuid) {
        int nodeId = getNodeId(nodeUuid);
        return getOutputConnections(nodeId);
    }
}