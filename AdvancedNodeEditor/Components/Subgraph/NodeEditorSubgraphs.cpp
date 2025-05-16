#include "../../../AdvancedNodeEditor/NodeEditor.h"
#include <algorithm>
#include <iostream>

namespace NodeEditorCore {
    int NodeEditor::createSubgraph(const std::string &name, const UUID &uuid) {
        int subgraphId = m_state.nextGroupId++;

        auto subgraph = std::make_shared<Subgraph>();
        subgraph->id = subgraphId;
        subgraph->name = name;
        subgraph->uuid = uuid.empty() ? generateUUID() : uuid;

        m_subgraphs[subgraphId] = subgraph;

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

        // Définir le nouveau sous-graphe actif
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
        std::cout << "Sous-graphe non trouvé: " << subgraphId << std::endl;
        return nullptr;
    }

    // Logs pour déboguer
    std::cout << "Création d'un nœud de sous-graphe pour: " << subgraph->name << " (ID: " << subgraphId << ")" << std::endl;
    std::cout << "Interfaces d'entrée: " << subgraph->interfaceInputs.size() << std::endl;
    std::cout << "Interfaces de sortie: " << subgraph->interfaceOutputs.size() << std::endl;

    for (int interfaceId : subgraph->interfaceInputs) {
        int nodeId = interfaceId >> 16;
        int pinId = interfaceId & 0xFFFF;
        std::cout << "  Interface d'entrée: nœud=" << nodeId << ", pin=" << pinId << std::endl;
    }

    for (int interfaceId : subgraph->interfaceOutputs) {
        int nodeId = interfaceId >> 16;
        int pinId = interfaceId & 0xFFFF;
        std::cout << "  Interface de sortie: nœud=" << nodeId << ", pin=" << pinId << std::endl;
    }

    int nodeId = addNode(name, "Subgraph", position, uuid);
    Node* node = getNode(nodeId);

    if (!node) {
        std::cout << "Échec de la création du nœud" << std::endl;
        return nullptr;
    }

    node->isSubgraph = true;
    node->subgraphId = subgraphId;
    node->subgraphUuid = subgraph->uuid;

    // Ajouter les pins basés sur les interfaces du sous-graphe
    int inputPinsAdded = 0;
    int outputPinsAdded = 0;

    // Pour chaque entrée du sous-graphe, créer un pin d'entrée correspondant
    for (int interfaceId : subgraph->interfaceInputs) {
        int nodeId = interfaceId >> 16;
        int pinId = interfaceId & 0xFFFF;

        const Node* interfaceNode = getNode(nodeId);
        if (interfaceNode) {
            // Trouver le pin correspondant
            const Pin* interfacePin = nullptr;
            for (const auto& pin : interfaceNode->inputs) {
                if (pin.id == pinId) {
                    interfacePin = &pin;
                    break;
                }
            }

            if (!interfacePin) {
                for (const auto& pin : interfaceNode->outputs) {
                    if (pin.id == pinId) {
                        interfacePin = &pin;
                        break;
                    }
                }
            }

            if (interfacePin) {
                int newPinId = addPin(node->id, interfacePin->name, true, static_cast<PinType>(interfacePin->type));
                if (newPinId >= 0) {
                    inputPinsAdded++;
                    std::cout << "  Pin d'entrée ajouté: " << newPinId << " (" << interfacePin->name << ")" << std::endl;
                } else {
                    std::cout << "  Échec de l'ajout du pin d'entrée" << std::endl;
                }
            } else {
                std::cout << "  Pin d'interface non trouvé: " << pinId << " dans le nœud " << nodeId << std::endl;
            }
        } else {
            std::cout << "  Nœud d'interface non trouvé: " << nodeId << std::endl;
        }
    }

    // Pour chaque sortie du sous-graphe, créer un pin de sortie correspondant
    for (int interfaceId : subgraph->interfaceOutputs) {
        int nodeId = interfaceId >> 16;
        int pinId = interfaceId & 0xFFFF;

        const Node* interfaceNode = getNode(nodeId);
        if (interfaceNode) {
            // Trouver le pin correspondant
            const Pin* interfacePin = nullptr;
            for (const auto& pin : interfaceNode->inputs) {
                if (pin.id == pinId) {
                    interfacePin = &pin;
                    break;
                }
            }

            if (!interfacePin) {
                for (const auto& pin : interfaceNode->outputs) {
                    if (pin.id == pinId) {
                        interfacePin = &pin;
                        break;
                    }
                }
            }

            if (interfacePin) {
                int newPinId = addPin(node->id, interfacePin->name, false, static_cast<PinType>(interfacePin->type));
                if (newPinId >= 0) {
                    outputPinsAdded++;
                    std::cout << "  Pin de sortie ajouté: " << newPinId << " (" << interfacePin->name << ")" << std::endl;
                } else {
                    std::cout << "  Échec de l'ajout du pin de sortie" << std::endl;
                }
            } else {
                std::cout << "  Pin d'interface non trouvé: " << pinId << " dans le nœud " << nodeId << std::endl;
            }
        } else {
            std::cout << "  Nœud d'interface non trouvé: " << nodeId << std::endl;
        }
    }

    std::cout << "Nœud de sous-graphe créé avec " << inputPinsAdded << " entrées et " << outputPinsAdded << " sorties" << std::endl;
    std::cout << "Nœud: inputs=" << node->inputs.size() << ", outputs=" << node->outputs.size() << std::endl;

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
        std::cout << "Debug du subgraph " << subgraphId << ":" << std::endl;

        auto it = m_subgraphs.find(subgraphId);
        if (it == m_subgraphs.end()) {
            std::cout << "  Subgraph non trouvé dans m_subgraphs" << std::endl;
            return;
        }

        Subgraph *subgraph = it->second.get();
        std::cout << "  Nom: " << subgraph->name << std::endl;

        std::cout << "  Noeuds: ";
        for (int nodeId: subgraph->nodeIds) {
            std::cout << nodeId << " ";
        }
        std::cout << std::endl;

        std::cout << "  Connexions: ";
        for (int connId: subgraph->connectionIds) {
            std::cout << connId << " ";

            Connection *conn = getConnection(connId);
            if (conn) {
                std::cout << "(trouvée, subgraphId=" << conn->subgraphId << ") ";
            } else {
                std::cout << "(non trouvée) ";
            }
        }
        std::cout << std::endl;

        std::cout << "  Toutes les connexions: " << std::endl;
        for (const auto &conn: m_state.connections) {
            std::cout << "    ID: " << conn.id
                    << ", de " << conn.startNodeId << ":" << conn.startPinId
                    << " à " << conn.endNodeId << ":" << conn.endPinId
                    << ", subgraphId=" << conn.subgraphId
                    << ", meta=" << conn.metadata.getAttribute<int>("subgraphId", -999)
                    << std::endl;
        }
    }
}
