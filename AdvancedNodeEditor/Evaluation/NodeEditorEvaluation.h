#ifndef NODE_EDITOR_EVALUATION_H
#define NODE_EDITOR_EVALUATION_H

#include <algorithm>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>

namespace NodeEditorCore {
    class NodeEditor;

    class NodeEvaluator {
    public:
        struct ConnectionInfo {
            int connectionId;
            UUID connectionUuid;
            int sourceNodeId;
            UUID sourceNodeUuid;
            int sourcePinId;
            UUID sourcePinUuid;
            int targetNodeId;
            UUID targetNodeUuid;
            int targetPinId;
            UUID targetPinUuid;
            ConnectionInfo();
        };

        NodeEvaluator(NodeEditorCore::NodeEditor& editor) : m_editor(editor) {
        }

        std::vector<int> getEvaluationOrder() {
            std::vector<int> result;
            std::unordered_map<int, std::vector<int>> dependencyGraph;
            std::unordered_map<int, int> inDegree;

            for (int i = 0; i < 10000; i++) {
                auto connection = getConnection(i);
                if (!connection) continue;

                if (getCurrentSubgraphId() >= 0 &&
                    !isConnectionInSubgraph(connection->id, getCurrentSubgraphId())) {
                    continue;
                }

                dependencyGraph[connection->startNodeId].push_back(connection->endNodeId);
                inDegree[connection->endNodeId]++;

                if (dependencyGraph.find(connection->startNodeId) == dependencyGraph.end()) {
                    dependencyGraph[connection->startNodeId] = std::vector<int>();
                }

                if (dependencyGraph.find(connection->endNodeId) == dependencyGraph.end()) {
                    dependencyGraph[connection->endNodeId] = std::vector<int>();
                }
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

            if (result.size() != dependencyGraph.size()) {
                for (const auto& pair : dependencyGraph) {
                    if (std::find(result.begin(), result.end(), pair.first) == result.end()) {
                        result.push_back(pair.first);
                    }
                }
            }

            return result;
        }

        std::vector<ConnectionInfo> getInputConnections(int nodeId) {
            std::vector<ConnectionInfo> result;

            for (int i = 0; i < 10000; i++) {
                auto connection = getConnection(i);
                if (!connection) continue;

                if (connection->endNodeId == nodeId) {
                    ConnectionInfo info;
                    info.connectionId = connection->id;
                    info.sourceNodeId = connection->startNodeId;
                    info.sourcePinId = connection->startPinId;
                    info.targetNodeId = connection->endNodeId;
                    info.targetPinId = connection->endPinId;
                    result.push_back(info);
                }
            }

            return result;
        }

        std::vector<ConnectionInfo> getOutputConnections(int nodeId) {
            std::vector<ConnectionInfo> result;

            for (int i = 0; i < 10000; i++) {
                auto connection = getConnection(i);
                if (!connection) continue;

                if (connection->startNodeId == nodeId) {
                    ConnectionInfo info;
                    info.connectionId = connection->id;
                    info.sourceNodeId = connection->startNodeId;
                    info.sourcePinId = connection->startPinId;
                    info.targetNodeId = connection->endNodeId;
                    info.targetPinId = connection->endPinId;
                    result.push_back(info);
                }
            }

            return result;
        }

        std::vector<NodeEvaluator::ConnectionInfo> getInputConnections(NodeEditor &editor, int nodeId);

        std::vector<NodeEvaluator::ConnectionInfo> getInputConnectionsByUUID(NodeEditor &editor, const UUID &nodeUuid);

        std::vector<NodeEvaluator::ConnectionInfo> getOutputConnections(NodeEditor &editor, int nodeId);

        std::vector<NodeEvaluator::ConnectionInfo> getOutputConnectionsByUUID(NodeEditor &editor, const UUID &nodeUuid);

        std::vector<int> getEvaluationOrder(NodeEditor &editor);

        std::vector<UUID> getEvaluationOrderUUIDs(NodeEditor &editor);

    private:
        NodeEditorCore::NodeEditor& m_editor;

        struct Connection {
            int id;
            int startNodeId;
            int startPinId;
            int endNodeId;
            int endPinId;
        };

        Connection* getConnection(int id);
        int getCurrentSubgraphId() const;
        bool isConnectionInSubgraph(int connectionId, int subgraphId) const;
    };
} // namespace ANE

#endif // NODE_EDITOR_EVALUATION_H