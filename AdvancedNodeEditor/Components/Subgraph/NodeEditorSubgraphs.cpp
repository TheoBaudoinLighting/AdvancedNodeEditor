#include "../../../AdvancedNodeEditor/NodeEditor.h"
#include <algorithm>

namespace NodeEditorCore {
    int NodeEditor::createSubgraph(const std::string &name, const UUID &uuid) {
        int subgraphId = m_state.nextGroupId++;

        auto subgraph = std::make_shared<Subgraph>();
        subgraph->id = subgraphId;
        subgraph->name = name;
        subgraph->uuid = uuid.empty() ? generateUUID() : uuid;

        m_subgraphs[subgraphId] = subgraph;

        Vec2 inputPos(100.0f, 200.0f);
        Vec2 outputPos(500.0f, 200.0f);

        int inputNodeId = addNode("Input", "Input", inputPos);
        int outputNodeId = addNode("Output", "Output", outputPos);

        Node* inputNode = getNode(inputNodeId);
        Node* outputNode = getNode(outputNodeId);

        if (inputNode) {
            inputNode->isProtected = true;
            addPin(inputNodeId, "flow", false, PinType::Blue);
        }

        if (outputNode) {
            outputNode->isProtected = true;
            addPin(outputNodeId, "flow", true, PinType::Blue);
        }

        addNodeToSubgraph(inputNodeId, subgraphId);
        addNodeToSubgraph(outputNodeId, subgraphId);

        subgraph->metadata.setAttribute("inputNodeId", inputNodeId);
        subgraph->metadata.setAttribute("outputNodeId", outputNodeId);

        return subgraphId;
    }

    bool NodeEditor::enterSubgraphByUUID(const UUID &uuid) {
        int subgraphId = getSubgraphId(uuid);
        if (subgraphId == -1) return false;
        return enterSubgraph(subgraphId);
    }

    bool NodeEditor::enterSubgraph(int subgraphId) {
        auto it = m_subgraphs.find(subgraphId);
        if (it == m_subgraphs.end()) {
            return false;
        }

        m_subgraphStack.push(m_state.currentSubgraphId);

        m_state.currentSubgraphId = subgraphId;
        m_state.currentSubgraphUuid = it->second->uuid;

        Event event(EventType::SubgraphEntered);
        event.subgraphId = subgraphId;

        return true;
    }

    bool NodeEditor::exitSubgraph() {
        if (m_state.currentSubgraphId < 0) return false;

        if (!m_subgraphStack.empty()) {
            int previousSubgraphId = m_state.currentSubgraphId;
            m_state.currentSubgraphId = m_subgraphStack.top();
            m_subgraphStack.pop();

            if (m_state.currentSubgraphId >= 0) {
                Subgraph *currentSubgraph = getSubgraph(m_state.currentSubgraphId);
                if (currentSubgraph) {
                    m_state.currentSubgraphUuid = currentSubgraph->uuid;
                }
            } else {
                m_state.currentSubgraphUuid = "";
            }

            return true;
        } else {
            m_state.currentSubgraphId = -1;
            m_state.currentSubgraphUuid = "";
            return true;
        }
    }

    int NodeEditor::getSubgraphId(const UUID &uuid) const {
        for (const auto &pair: m_subgraphs) {
            if (pair.second->uuid == uuid) {
                return pair.first;
            }
        }
        return -1;
    }

    Node* NodeEditor::createSubgraphNode(int subgraphId, const std::string& name, const Vec2& position, const UUID& uuid) {
        Subgraph* subgraph = getSubgraph(subgraphId);
        if (!subgraph) {
            return nullptr;
        }

        // Création du nœud représentant le subgraph
        int nodeId = addNode(name, "Subgraph", position, uuid);
        Node* node = getNode(nodeId);

        if (!node) {
            return nullptr;
        }

        node->isSubgraph = true;
        node->subgraphId = subgraphId;
        node->subgraphUuid = subgraph->uuid;

        // Récupérer les nœuds Input et Output du subgraph
        int inputNodeId = subgraph->metadata.getAttribute<int>("inputNodeId", -1);
        int outputNodeId = subgraph->metadata.getAttribute<int>("outputNodeId", -1);

        Node* inputNode = getNode(inputNodeId);
        Node* outputNode = getNode(outputNodeId);

        // Si les nœuds input/output existent, créer les pins correspondants sur le nœud du subgraph
        if (inputNode) {
            for (const auto& pin : inputNode->outputs) {
                // Créer un pin d'entrée correspondant sur le nœud du subgraph
                int newPinId = addPin(node->id, pin.name, true, static_cast<PinType>(pin.type));

                // Stocker la relation entre le pin du subgraph et le pin du nœud input
                int interfaceId = (inputNodeId << 16) | pin.id;
                subgraph->interfaceInputs.push_back(interfaceId);
            }
        }

        if (outputNode) {
            for (const auto& pin : outputNode->inputs) {
                // Créer un pin de sortie correspondant sur le nœud du subgraph
                int newPinId = addPin(node->id, pin.name, false, static_cast<PinType>(pin.type));

                // Stocker la relation entre le pin du subgraph et le pin du nœud output
                int interfaceId = (outputNodeId << 16) | pin.id;
                subgraph->interfaceOutputs.push_back(interfaceId);
            }
        }

        return node;
    }

    UUID NodeEditor::getSubgraphUUID(int subgraphId) const {
        auto it = m_subgraphs.find(subgraphId);
        if (it != m_subgraphs.end()) {
            return it->second->uuid;
        }

        for (const auto &node: m_state.nodes) {
            if (node.isSubgraph && node.subgraphId == subgraphId) {
                return node.subgraphUuid;
            }
        }

        return "";
    }

    UUID NodeEditor::createSubgraphWithUUID(const std::string &name) {
        UUID uuid = generateUUID();
        int subgraphId = createSubgraph(name, uuid);

        auto it = m_subgraphs.find(subgraphId);
        if (it != m_subgraphs.end()) {
            return it->second->uuid;
        }

        return uuid;
    }

    std::vector<int> NodeEditor::getNodesInSubgraph(int subgraphId) const {
        auto it = m_subgraphs.find(subgraphId);
        if (it == m_subgraphs.end()) return {};

        return it->second->nodeIds;
    }

    std::vector<int> NodeEditor::getConnectionsInSubgraph(int subgraphId) const {
        auto it = m_subgraphs.find(subgraphId);
        if (it == m_subgraphs.end()) {
            return {};
        }

        return it->second->connectionIds;
    }

    void NodeEditor::addNodeToSubgraph(int nodeId, int subgraphId) {
        auto it = m_subgraphs.find(subgraphId);
        if (it == m_subgraphs.end()) return;

        Subgraph *subgraph = it->second.get();
        Node *node = getNode(nodeId);
        if (!node) return;

        node->setSubgraphId(subgraphId);

        if (std::find(subgraph->nodeIds.begin(), subgraph->nodeIds.end(), nodeId) == subgraph->nodeIds.end()) {
            subgraph->nodeIds.push_back(nodeId);
        }
    }

    void NodeEditor::removeNodeFromSubgraph(int nodeId, int subgraphId) {
        auto it = m_subgraphs.find(subgraphId);
        if (it == m_subgraphs.end()) return;

        Subgraph *subgraph = it->second.get();
        Node *node = getNode(nodeId);
        if (!node || node->getSubgraphId() != subgraphId) return;

        node->setSubgraphId(-1);

        subgraph->nodeIds.erase(
            std::remove(subgraph->nodeIds.begin(), subgraph->nodeIds.end(), nodeId),
            subgraph->nodeIds.end()
        );
    }

    void NodeEditor::addConnectionToSubgraph(int connectionId, int subgraphId) {
        auto it = m_subgraphs.find(subgraphId);
        if (it == m_subgraphs.end()) {
            return;
        }

        Subgraph *subgraph = it->second.get();

        Connection *connection = getConnection(connectionId);
        if (!connection) {
            return;
        }

        if (std::find(subgraph->connectionIds.begin(), subgraph->connectionIds.end(), connectionId)
            == subgraph->connectionIds.end()) {
            subgraph->connectionIds.push_back(connectionId);
        }

        connection->subgraphId = subgraphId;
        connection->metadata.setAttribute("subgraphId", subgraphId);
    }

    bool NodeEditor::isConnectionInSubgraph(int connectionId, int subgraphId) const {
        if (connectionId < 0) return false;

        auto it = m_subgraphs.find(subgraphId);
        if (it == m_subgraphs.end()) {
            return false;
        }

        const Subgraph *subgraph = it->second.get();

        if (std::find(subgraph->connectionIds.begin(), subgraph->connectionIds.end(), connectionId)
            != subgraph->connectionIds.end()) {
            return true;
        }

        for (const auto &connection: m_state.connections) {
            if (connection.id == connectionId) {
                if (connection.subgraphId == subgraphId ||
                    connection.metadata.getAttribute<int>("subgraphId", -1) == subgraphId) {
                    return true;
                }
                break;
            }
        }

        return false;
    }

    void NodeEditor::removeConnectionFromSubgraph(int connectionId, int subgraphId) {
        auto it = m_subgraphs.find(subgraphId);
        if (it == m_subgraphs.end()) return;

        Subgraph *subgraph = it->second.get();
        Connection *connection = getConnection(connectionId);
        if (connection) {
            connection->subgraphId = -1;
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

    bool NodeEditor::isNodeInCurrentSubgraph(const Node &node) const {
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

    bool NodeEditor::isSubgraphContainer(int nodeId) const {
        const Node *node = getNode(nodeId);
        return node ? node->isSubgraph : false;
    }

    int NodeEditor::getSubgraphFromNode(int nodeId) const {
        const Node *node = getNode(nodeId);
        return (node && node->isSubgraph) ? node->subgraphId : -1;
    }

    int NodeEditor::getNodeSubgraph(int nodeId) const {
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

    int NodeEditor::getSubgraphDepth(int subgraphId) const {
        if (subgraphId < 0) return 0;

        int depth = 0;
        int currentId = subgraphId;

        while (currentId >= 0) {
            auto it = m_subgraphs.find(currentId);
            if (it == m_subgraphs.end()) break;

            const Subgraph *sg = it->second.get();
            if (!sg) break;

            depth++;
            currentId = sg->parentSubgraphId;
        }

        return depth;
    }

    Subgraph *NodeEditor::getSubgraph(int subgraphId) {
        auto it = m_subgraphs.find(subgraphId);
        if (it != m_subgraphs.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    Subgraph *NodeEditor::getSubgraphByUUID(const UUID &uuid) {
        for (auto &pair: m_subgraphs) {
            if (pair.second->uuid == uuid) {
                return pair.second.get();
            }
        }
        return nullptr;
    }

    void NodeEditor::removeSubgraph(int subgraphId) {
        m_subgraphs.erase(subgraphId);
    }

    void NodeEditor::debugSubgraph(int subgraphId) {
    }

    // Nouvelle méthode pour ajouter un pin au nœud Input dans un subgraph
    int NodeEditor::addInputPinToSubgraph(int subgraphId, const std::string& name, PinType type) {
        Subgraph* subgraph = getSubgraph(subgraphId);
        if (!subgraph) return -1;

        int inputNodeId = subgraph->metadata.getAttribute<int>("inputNodeId", -1);
        if (inputNodeId == -1) return -1;

        Node* inputNode = getNode(inputNodeId);
        if (!inputNode) return -1;

        // Ajouter un pin de sortie au nœud Input
        int pinId = addPin(inputNodeId, name, false, type);
        if (pinId == -1) return -1;

        // Mettre à jour les interfaces du subgraph
        int interfaceId = (inputNodeId << 16) | pinId;
        subgraph->interfaceInputs.push_back(interfaceId);

        // Si un nœud de subgraph existe déjà dans le parent, mettre à jour ses pins
        for (const auto& node : m_state.nodes) {
            if (node.isSubgraph && node.subgraphId == subgraphId) {
                addPin(node.id, name, true, type);
            }
        }

        return pinId;
    }

    // Nouvelle méthode pour ajouter un pin au nœud Output dans un subgraph
    int NodeEditor::addOutputPinToSubgraph(int subgraphId, const std::string& name, PinType type) {
        Subgraph* subgraph = getSubgraph(subgraphId);
        if (!subgraph) return -1;

        int outputNodeId = subgraph->metadata.getAttribute<int>("outputNodeId", -1);
        if (outputNodeId == -1) return -1;

        Node* outputNode = getNode(outputNodeId);
        if (!outputNode) return -1;

        // Ajouter un pin d'entrée au nœud Output
        int pinId = addPin(outputNodeId, name, true, type);
        if (pinId == -1) return -1;

        // Mettre à jour les interfaces du subgraph
        int interfaceId = (outputNodeId << 16) | pinId;
        subgraph->interfaceOutputs.push_back(interfaceId);

        // Si un nœud de subgraph existe déjà dans le parent, mettre à jour ses pins
        for (const auto& node : m_state.nodes) {
            if (node.isSubgraph && node.subgraphId == subgraphId) {
                addPin(node.id, name, false, type);
            }
        }

        return pinId;
    }


    void NodeEditor::synchronizeSubgraphConnections(int subgraphId, int subgraphNodeId) {
        auto it = m_subgraphs.find(subgraphId);
        if (it == m_subgraphs.end()) return;

        Subgraph* subgraph = it->second.get();
        Node* subgraphNode = getNode(subgraphNodeId);
        if (!subgraphNode) return;

        // Récupérer les nœuds Input et Output
        int inputNodeId = subgraph->metadata.getAttribute<int>("inputNodeId", -1);
        int outputNodeId = subgraph->metadata.getAttribute<int>("outputNodeId", -1);

        Node* inputNode = getNode(inputNodeId);
        Node* outputNode = getNode(outputNodeId);

        if (!inputNode || !outputNode) return;

        // Parcourir toutes les connexions vers le nœud subgraph
        for (const auto& conn : m_state.connections) {
            // Si c'est une connexion vers une entrée du nœud subgraph
            if (conn.endNodeId == subgraphNodeId) {
                Pin* subgraphPin = subgraphNode->findPin(conn.endPinId);
                if (!subgraphPin) continue;

                // Trouver le pin correspondant sur le nœud Input
                for (auto& pin : inputNode->outputs) {
                    if (pin.name == subgraphPin->name) {
                        // Créer une connexion vers le pin de sortie du nœud Input
                        addConnection(conn.startNodeId, conn.startPinId, inputNodeId, pin.id);
                        break;
                    }
                }
            }
            // Si c'est une connexion depuis une sortie du nœud subgraph
            else if (conn.startNodeId == subgraphNodeId) {
                Pin* subgraphPin = subgraphNode->findPin(conn.startPinId);
                if (!subgraphPin) continue;

                // Trouver le pin correspondant sur le nœud Output
                for (auto& pin : outputNode->inputs) {
                    if (pin.name == subgraphPin->name) {
                        // Créer une connexion depuis le pin d'entrée du nœud Output
                        addConnection(outputNodeId, pin.id, conn.endNodeId, conn.endPinId);
                        break;
                    }
                }
            }
        }
    }

    void NodeEditor::handleSubgraphConnections(int connectionId) {
        Connection* connection = getConnection(connectionId);
        if (!connection) return;

        Node* startNode = getNode(connection->startNodeId);
        Node* endNode = getNode(connection->endNodeId);

        if (!startNode || !endNode) return;

        if (endNode->isSubgraph) {
            Pin* endPin = endNode->findPin(connection->endPinId);
            if (!endPin) return;

            Subgraph* subgraph = getSubgraph(endNode->subgraphId);
            if (!subgraph) return;

            int inputNodeId = subgraph->metadata.getAttribute<int>("inputNodeId", -1);
            Node* inputNode = getNode(inputNodeId);
            if (!inputNode) return;

            for (auto& pin : inputNode->outputs) {
                if (pin.name == endPin->name) {
                    addConnectionToSubgraph(
                        addConnection(connection->startNodeId, connection->startPinId, inputNodeId, pin.id),
                        endNode->subgraphId
                    );
                    break;
                }
            }
        }
        else if (startNode->isSubgraph) {
            Pin* startPin = startNode->findPin(connection->startPinId);
            if (!startPin) return;

            Subgraph* subgraph = getSubgraph(startNode->subgraphId);
            if (!subgraph) return;

            int outputNodeId = subgraph->metadata.getAttribute<int>("outputNodeId", -1);
            Node* outputNode = getNode(outputNodeId);
            if (!outputNode) return;

            for (auto& pin : outputNode->inputs) {
                if (pin.name == startPin->name) {
                    addConnectionToSubgraph(
                        addConnection(outputNodeId, pin.id, connection->endNodeId, connection->endPinId),
                        startNode->subgraphId
                    );
                    break;
                }
            }
        }
    }
}