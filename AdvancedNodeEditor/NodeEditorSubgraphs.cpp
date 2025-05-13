#include "NodeEditor.h"
#include <algorithm>

namespace NodeEditorCore {

    std::vector<int> NodeEditor::getNodesInSubgraph(int subgraphId) const {
        std::vector<int> result;

        for (const auto& node : m_state.nodes) {
            if (node.getSubgraphId() == subgraphId) {
                result.push_back(node.id);
            }
        }

        return result;
    }

    std::vector<int> NodeEditor::getConnectionsInSubgraph(int subgraphId) const {
        std::vector<int> result;

        for (const auto& connection : m_state.connections) {
            if (connection.getSubgraphId() == subgraphId) {
                result.push_back(connection.id);
            }
        }

        return result;
    }

    void NodeEditor::addNodeToSubgraph(int nodeId, int subgraphId) {
        Node* node = getNode(nodeId);
        if (!node) return;

        node->setSubgraphId(subgraphId);
    }

    void NodeEditor::removeNodeFromSubgraph(int nodeId, int subgraphId) {
        Node* node = getNode(nodeId);
        if (!node || node->getSubgraphId() != subgraphId) return;

        node->setSubgraphId(-1);
    }

    void NodeEditor::addConnectionToSubgraph(int connectionId, int subgraphId) {
        Connection* connection = getConnection(connectionId);
        if (!connection) return;

        connection->setSubgraphId(subgraphId);
    }

    void NodeEditor::removeConnectionFromSubgraph(int connectionId, int subgraphId) {
        Connection* connection = getConnection(connectionId);
        if (!connection || connection->getSubgraphId() != subgraphId) return;

        connection->setSubgraphId(-1);
    }

    void NodeEditor::saveSubgraphViewState(int subgraphId) {
    }

    void NodeEditor::restoreSubgraphViewState(int subgraphId) {
    }

    void NodeEditor::setCurrentSubgraphId(int subgraphId) {
        m_state.currentSubgraphId = subgraphId;
    }

    int NodeEditor::getCurrentSubgraphId() const {
        return m_state.currentSubgraphId;
    }

}