#include "NodeEditor.h"

namespace ANE {

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

    std::vector<int> NodeEditor::getEvaluationOrder() const {
        NodeEvaluator evaluator(m_editor);
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

} 