#ifndef NODE_EDITOR_EVALUATION_H
#define NODE_EDITOR_EVALUATION_H

#include <algorithm>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <string>
#include "../Core/Types/UuidTypes.h"

namespace NodeEditorCore {
    class NodeEditor;
}

namespace ANE {
    class NodeEditor;

    namespace NodeEvaluator {

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

        std::vector<ConnectionInfo> getInputConnections(NodeEditor& editor, int nodeId);
        std::vector<ConnectionInfo> getInputConnectionsByUUID(NodeEditor& editor, const UUID& nodeUuid);

        std::vector<ConnectionInfo> getOutputConnections(NodeEditor& editor, int nodeId);
        std::vector<ConnectionInfo> getOutputConnectionsByUUID(NodeEditor& editor, const UUID& nodeUuid);

        std::vector<int> getEvaluationOrder(NodeEditor& editor);
        std::vector<UUID> getEvaluationOrderUUIDs(NodeEditor& editor);

        class Evaluator {
        public:
            Evaluator(NodeEditor& editor) : m_editor(editor) {}
            
            std::vector<int> getEvaluationOrder() {
                return NodeEvaluator::getEvaluationOrder(m_editor);
            }
            
            std::vector<UUID> getEvaluationOrderUUIDs() {
                return NodeEvaluator::getEvaluationOrderUUIDs(m_editor);
            }
            
            std::vector<ConnectionInfo> getInputConnections(int nodeId) {
                return NodeEvaluator::getInputConnections(m_editor, nodeId);
            }
            
            std::vector<ConnectionInfo> getInputConnectionsByUUID(const UUID& nodeUuid) {
                return NodeEvaluator::getInputConnectionsByUUID(m_editor, nodeUuid);
            }
            
            std::vector<ConnectionInfo> getOutputConnections(int nodeId) {
                return NodeEvaluator::getOutputConnections(m_editor, nodeId);
            }
            
            std::vector<ConnectionInfo> getOutputConnectionsByUUID(const UUID& nodeUuid) {
                return NodeEvaluator::getOutputConnectionsByUUID(m_editor, nodeUuid);
            }
            
        private:
            NodeEditor& m_editor;
        };
    }
} // namespace ANE

#endif // NODE_EDITOR_EVALUATION_H