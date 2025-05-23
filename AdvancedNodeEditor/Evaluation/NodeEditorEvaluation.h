#ifndef NODE_EDITOR_EVALUATION_H
#define NODE_EDITOR_EVALUATION_H

#include <algorithm>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <string>
#include "../Core/Types/CoreTypes.h"

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

        struct Connection {
            int id;
            int startNodeId;
            int startPinId;
            int endNodeId;
            int endPinId;
        };

        NodeEvaluator(NodeEditorCore::NodeEditor &editor) : m_editor(editor) {
        }

        std::vector<int> getEvaluationOrder();

        static std::vector<int> getEvaluationOrder(NodeEditor &editor);

        static std::vector<UUID> getEvaluationOrderUUIDs(NodeEditor &editor);

    private:
        NodeEditorCore::NodeEditor &m_editor;

        Connection *getConnection(int connectionId);

        int getCurrentSubgraphId() const;

        bool isConnectionInSubgraph(int connectionId, int subgraphId) const;
    };
}
#endif
