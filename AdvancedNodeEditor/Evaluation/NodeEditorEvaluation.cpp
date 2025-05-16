#include "../NodeEditor.h"
#include "NodeEditorEvaluation.h"

namespace NodeEditorCore {
    NodeEvaluator::ConnectionInfo::ConnectionInfo()
        : connectionId(-1), connectionUuid(""),
          sourceNodeId(-1), sourceNodeUuid(""), sourcePinId(-1), sourcePinUuid(""),
          targetNodeId(-1), targetNodeUuid(""), targetPinId(-1), targetPinUuid("") {
    }

    std::vector<NodeEvaluator::ConnectionInfo> NodeEditor::getInputConnections(int nodeId) {
        std::vector<NodeEvaluator::ConnectionInfo> result;

        for (const auto& connection : m_state.connections) {
            if (connection.endNodeId == nodeId) {
                NodeEvaluator::ConnectionInfo info;
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

    std::vector<NodeEvaluator::ConnectionInfo> NodeEditor::getInputConnectionsByUUID(const UUID& nodeUuid) {
        int nodeId = getNodeId(nodeUuid);
        if (nodeId == -1) return {};
        return getInputConnections(nodeId);
    }

    std::vector<NodeEvaluator::ConnectionInfo> NodeEditor::getOutputConnections(int nodeId) {
        std::vector<NodeEvaluator::ConnectionInfo> result;

        for (const auto& connection : m_state.connections) {
            if (connection.startNodeId == nodeId) {
                NodeEvaluator::ConnectionInfo info;
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

    std::vector<NodeEvaluator::ConnectionInfo> NodeEditor::getOutputConnectionsByUUID(const UUID& nodeUuid) {
        int nodeId = getNodeId(nodeUuid);
        if (nodeId == -1) return {};
        return getOutputConnections(nodeId);
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

        const auto& allConnections = m_editor.getConnections();

        for (const auto& connection : allConnections) {
            int startNodeId = connection.startNodeId;
            int endNodeId = connection.endNodeId;

            int currentSubgraphId = getCurrentSubgraphId();
            if (currentSubgraphId >= 0) {
                const Node* startNode = m_editor.getNode(startNodeId);
                const Node* endNode = m_editor.getNode(endNodeId);

                if (!startNode || !endNode ||
                    !m_editor.isNodeInSubgraph(*startNode, currentSubgraphId) ||
                    !m_editor.isNodeInSubgraph(*endNode, currentSubgraphId)) {
                    continue;
                }
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

        if (dependencyGraph.empty()) {
            for (const auto& node : m_editor.getNodes()) {
                int currentSubgraphId = getCurrentSubgraphId();
                if (currentSubgraphId >= 0 && !m_editor.isNodeInSubgraph(node, currentSubgraphId)) {
                    continue;
                }

                if (dependencyGraph.find(node.id) == dependencyGraph.end()) {
                    dependencyGraph[node.id] = std::vector<int>();
                }

                if (node.name == "Input") {
                    for (const auto& other : m_editor.getNodes()) {
                        if (currentSubgraphId >= 0 && !m_editor.isNodeInSubgraph(other, currentSubgraphId)) {
                            continue;
                        }

                        if (other.name == "Process") {
                            dependencyGraph[node.id].push_back(other.id);
                            inDegree[other.id]++;
                        }
                    }
                } else if (node.name == "Process") {
                    for (const auto& other : m_editor.getNodes()) {
                        if (currentSubgraphId >= 0 && !m_editor.isNodeInSubgraph(other, currentSubgraphId)) {
                            continue;
                        }

                        if (other.name == "Output") {
                            dependencyGraph[node.id].push_back(other.id);
                            inDegree[other.id]++;
                        }
                    }
                }
            }
        }

        if (dependencyGraph.empty()) {
            for (const auto& node : m_editor.getNodes()) {
                int currentSubgraphId = getCurrentSubgraphId();
                if (currentSubgraphId < 0 || m_editor.isNodeInSubgraph(node, currentSubgraphId)) {
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
}