#include "../NodeEditor.h"
#include "NodeEditorEvaluation.h"
#include "../Core/Conversions/Conversions.h"

namespace NodeEditorCore {

    NodeEvaluator::ConnectionInfo::ConnectionInfo()
        : connectionId(-1), connectionUuid(""),
          sourceNodeId(-1), sourceNodeUuid(""), sourcePinId(-1), sourcePinUuid(""),
          targetNodeId(-1), targetNodeUuid(""), targetPinId(-1), targetPinUuid("") {
    }

    std::vector<NodeEvaluator::ConnectionInfo> NodeEvaluator::getInputConnections(NodeEditor& editor, int nodeId) {
        std::vector<ConnectionInfo> result;

        for (const auto& connection : editor.getConnections()) {
            if (connection.endNodeId == nodeId) {
                ConnectionInfo info;
                info.connectionId = connection.id;
                info.connectionUuid = connection.uuid;
                info.sourceNodeId = connection.startNodeId;
                info.sourceNodeUuid = connection.startNodeUuid;
                info.sourcePinId = connection.startPinId;
                info.sourcePinUuid = connection.startPinUuid;
                info.targetNodeId = connection.endNodeId;
                info.targetNodeUuid = connection.endNodeUuid;
                info.targetPinId = connection.endPinId;
                info.targetPinUuid = connection.endPinUuid;
                result.push_back(info);
            }
        }

        return result;
    }

    std::vector<NodeEvaluator::ConnectionInfo> NodeEvaluator::getInputConnectionsByUUID(NodeEditor& editor, const UUID& nodeUuid) {
        int nodeId = editor.getNodeId(nodeUuid);
        if (nodeId == -1) return {};
        return getInputConnections(editor, nodeId);
    }

    std::vector<NodeEvaluator::ConnectionInfo> NodeEvaluator::getOutputConnections(NodeEditor& editor, int nodeId) {
        std::vector<ConnectionInfo> result;

        for (const auto& connection : editor.getConnections()) {
            if (connection.startNodeId == nodeId) {
                ConnectionInfo info;
                info.connectionId = connection.id;
                info.connectionUuid = connection.uuid;
                info.sourceNodeId = connection.startNodeId;
                info.sourceNodeUuid = connection.startNodeUuid;
                info.sourcePinId = connection.startPinId;
                info.sourcePinUuid = connection.startPinUuid;
                info.targetNodeId = connection.endNodeId;
                info.targetNodeUuid = connection.endNodeUuid;
                info.targetPinId = connection.endPinId;
                info.targetPinUuid = connection.endPinUuid;
                result.push_back(info);
            }
        }

        return result;
    }

    std::vector<NodeEvaluator::ConnectionInfo> NodeEvaluator::getOutputConnectionsByUUID(NodeEditor& editor, const UUID& nodeUuid) {
        int nodeId = editor.getNodeId(nodeUuid);
        if (nodeId == -1) return {};
        return getOutputConnections(editor, nodeId);
    }

    std::vector<int> NodeEvaluator::getEvaluationOrder(NodeEditor& editor) {
        NodeEvaluator evaluator(editor);
        return evaluator.getEvaluationOrder();
    }

    std::vector<UUID> NodeEvaluator::getEvaluationOrderUUIDs(NodeEditor& editor) {
        std::vector<UUID> result;
        std::vector<int> order = getEvaluationOrder(editor);

        for (int nodeId : order) {
            UUID uuid = editor.getNodeUUID(nodeId);
            if (!uuid.empty()) {
                result.push_back(uuid);
            }
        }

        return result;
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
        return m_editor.getCurrentSubgraphId();
    }

    bool NodeEvaluator::isConnectionInSubgraph(int connectionId, int subgraphId) const {
        if (connectionId < 0) return false;

        return m_editor.isConnectionInSubgraph(connectionId, subgraphId);
    }

    std::vector<int> NodeEvaluator::getEvaluationOrder() {
        std::vector<int> result;
        std::unordered_map<int, std::vector<int>> dependencyGraph;
        std::unordered_map<int, int> inDegree;

        // Construire le graphe de dépendances
        for (const auto& connection : m_editor.getConnections()) {
            int startNodeId = connection.startNodeId;
            int endNodeId = connection.endNodeId;

            // Ignorer les connexions qui ne sont pas dans le subgraph courant
            if (getCurrentSubgraphId() >= 0 && !isConnectionInSubgraph(connection.id, getCurrentSubgraphId())) {
                continue;
            }

            if (dependencyGraph.find(startNodeId) == dependencyGraph.end()) {
                dependencyGraph[startNodeId] = std::vector<int>();
            }

            if (dependencyGraph.find(endNodeId) == dependencyGraph.end()) {
                dependencyGraph[endNodeId] = std::vector<int>();
            }

            dependencyGraph[startNodeId].push_back(endNodeId);
            inDegree[endNodeId]++;
        }

        // Si aucune dépendance trouvée, ajouter tous les nœuds au résultat
        if (dependencyGraph.empty()) {
            for (const auto& node : m_editor.getNodes()) {
                if (getCurrentSubgraphId() < 0 || node.getSubgraphId() == getCurrentSubgraphId()) {
                    result.push_back(node.id);
                }
            }
            return result;
        }

        std::queue<int> q;
        for (const auto& pair : dependencyGraph) {
            if (inDegree[pair.first] == 0) {
                q.push(pair.first);
            }
        }

        while (!q.empty()) {
            int current = q.front();
            q.pop();
            result.push_back(current);

            for (int dependent : dependencyGraph[current]) {
                if (--inDegree[dependent] == 0) {
                    q.push(dependent);
                }
            }
        }

        std::unordered_set<int> visited(result.begin(), result.end());
        for (const auto& pair : dependencyGraph) {
            if (visited.find(pair.first) == visited.end()) {
                result.push_back(pair.first);
            }
        }

        return result;
    }

    std::vector<int> NodeEditor::getEvaluationOrder() const {
        NodeEvaluator evaluator(*const_cast<NodeEditor*>(this));
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
        return NodeEvaluator::getInputConnections(*this, nodeId);
    }

    std::vector<NodeEvaluator::ConnectionInfo> NodeEditor::getInputConnectionsByUUID(const UUID& nodeUuid) {
        return NodeEvaluator::getInputConnectionsByUUID(*this, nodeUuid);
    }

    std::vector<NodeEvaluator::ConnectionInfo> NodeEditor::getOutputConnections(int nodeId) {
        return NodeEvaluator::getOutputConnections(*this, nodeId);
    }

    std::vector<NodeEvaluator::ConnectionInfo> NodeEditor::getOutputConnectionsByUUID(const UUID& nodeUuid) {
        return NodeEvaluator::getOutputConnectionsByUUID(*this, nodeUuid);
    }





}