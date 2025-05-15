#include "../../NodeEditor.h"
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
        const Subgraph* subgraph = getSubgraph(subgraphId);
        if (!subgraph) return {};

        return subgraph->connectionIds;
    }

    void NodeEditor::addNodeToSubgraph(int nodeId, int subgraphId) {
        Node* node = getNode(nodeId);
        if (!node) return;

        node->setSubgraphId(subgraphId);

        auto subgraph = getSubgraph(subgraphId);
        if (subgraph) {
            if (std::find(subgraph->nodeIds.begin(), subgraph->nodeIds.end(), nodeId) == subgraph->nodeIds.end()) {
                subgraph->nodeIds.push_back(nodeId);
            }
        }
    }

    void NodeEditor::removeNodeFromSubgraph(int nodeId, int subgraphId) {
        Node* node = getNode(nodeId);
        if (!node || node->getSubgraphId() != subgraphId) return;

        node->setSubgraphId(-1);

        auto subgraph = getSubgraph(subgraphId);
        if (subgraph) {
            subgraph->nodeIds.erase(
                std::remove(subgraph->nodeIds.begin(), subgraph->nodeIds.end(), nodeId),
                subgraph->nodeIds.end()
            );
        }
    }

    void NodeEditor::addConnectionToSubgraph(int connectionId, int subgraphId) {
        auto subgraph = getSubgraph(subgraphId);
        if (!subgraph) return;

        Connection* connection = getConnection(connectionId);
        if (!connection) return;

        if (std::find(subgraph->connectionIds.begin(), subgraph->connectionIds.end(), connectionId) == subgraph->connectionIds.end()) {
            subgraph->connectionIds.push_back(connectionId);
        }

        connection->metadata.setAttribute("subgraphId", subgraphId);
    }

    bool NodeEditor::isConnectionInSubgraph(int connectionId, int subgraphId) const {
        const Subgraph* subgraph = getSubgraph(subgraphId);
        if (!subgraph) return false;

        return std::find(subgraph->connectionIds.begin(), subgraph->connectionIds.end(), connectionId) != subgraph->connectionIds.end();
    }

    void NodeEditor::removeConnectionFromSubgraph(int connectionId, int subgraphId) {
        Subgraph* subgraph = getSubgraph(subgraphId);
        if (!subgraph) return;

        Connection* connection = getConnection(connectionId);
        if (connection) {
            connection->metadata.setAttribute("subgraphId", -1);
        }

        subgraph->connectionIds.erase(
            std::remove(subgraph->connectionIds.begin(), subgraph->connectionIds.end(), connectionId),
            subgraph->connectionIds.end()
        );
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

    bool NodeEditor::isNodeInCurrentSubgraph(const Node& node) const {
        if (m_state.currentSubgraphId < 0) {
            return node.getSubgraphId() == -1;
        } else {
            return node.getSubgraphId() == m_state.currentSubgraphId;
        }
    }

    bool NodeEditor::isSubgraphContainer(const Node &node) const {
        return node.isSubgraph;
    }

    bool NodeEditor::isNodeInSubgraph(const Node &node, int subgraphId) const {
        return node.getSubgraphId() == subgraphId;
    }

    bool NodeEditorCore::NodeEditor::isSubgraphContainer(int nodeId) const {
        const Node *node = getNode(nodeId);
        return node ? node->isSubgraph : false;
    }

    int NodeEditorCore::NodeEditor::getSubgraphFromNode(int nodeId) const {
        const Node *node = getNode(nodeId);
        return (node && node->isSubgraph) ? node->subgraphId : -1;
    }

    int NodeEditorCore::NodeEditor::getNodeSubgraph(int nodeId) const {
        const Node *node = getNode(nodeId);
        return node ? node->getSubgraphId() : -1;
    }

    void NodeEditor::setSubgraphIdForNode(int nodeId, int subgraphId) {
        Node *node = getNode(nodeId);
        if (node) {
            node->metadata.setAttribute("subgraphId", subgraphId);
        }
    }

    void NodeEditor::setSubgraphUUIDForNode(int nodeId, const UUID &uuid) {
        Node *node = getNode(nodeId);
        if (node) {
            node->metadata.setAttribute("subgraphUuid", uuid);
        }
    }

    UUID NodeEditor::getSubgraphUUIDForNode(int nodeId) const {
        const Node *node = getNode(nodeId);
        if (node) {
            return node->metadata.getAttribute<UUID>("subgraphUuid", "");
        }
        return "";
    }

    int NodeEditor::getSubgraphIdForNode(int nodeId) const {
        const Node *node = getNode(nodeId);
        if (node) {
            return node->metadata.getAttribute<int>("subgraphId", -1);
        }
        return -1;
    }
}
