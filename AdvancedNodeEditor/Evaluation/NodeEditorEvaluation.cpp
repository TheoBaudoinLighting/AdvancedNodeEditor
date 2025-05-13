#include "NodeEditorEvaluation.h"
#include "../NodeEditor.h"

namespace ANE {

    NodeEvaluator::Connection* NodeEvaluator::getConnection(int id) {
        static Connection connection;
        
        auto internalConnection = m_editor.getConnection(id);
        if (!internalConnection) return nullptr;
        
        connection.id = internalConnection->id;
        connection.startNodeId = internalConnection->startNodeId;
        connection.startPinId = internalConnection->startPinId;
        connection.endNodeId = internalConnection->endNodeId;
        connection.endPinId = internalConnection->endPinId;
        
        return &connection;
    }

    int NodeEvaluator::getCurrentSubgraphId() const {
        return m_editor.getCurrentSubgraphId();
    }

    bool NodeEvaluator::isConnectionInSubgraph(int connectionId, int subgraphId) const {
        auto connections = m_editor.getConnectionsInSubgraph(subgraphId);
        return std::find(connections.begin(), connections.end(), connectionId) != connections.end();
    }

} // namespace ANE