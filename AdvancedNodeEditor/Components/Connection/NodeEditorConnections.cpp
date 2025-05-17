#include "../../NodeEditor.h"
#include <algorithm>
#include <iostream>

namespace NodeEditorCore {
    void NodeEditor::removeConnection(int connectionId) {
        auto it = std::find_if(m_state.connections.begin(), m_state.connections.end(),
                               [connectionId](const Connection &conn) { return conn.id == connectionId; });

        if (it != m_state.connections.end()) {
            Node *startNode = getNode(it->startNodeId);
            Node *endNode = getNode(it->endNodeId);

            Pin *startPinInternal = startNode ? startNode->findPin(it->startPinId) : nullptr;
            Pin *endPinInternal = endNode ? endNode->findPin(it->endPinId) : nullptr;

            bool startPinConnected = false;
            bool endPinConnected = false;

            for (const auto &conn: m_state.connections) {
                if (conn.id == connectionId) continue;

                if (conn.startNodeId == it->startNodeId && conn.startPinId == it->startPinId) {
                    startPinConnected = true;
                }

                if (conn.endNodeId == it->endNodeId && conn.endPinId == it->endPinId) {
                    endPinConnected = true;
                }
            }

            if (startPinInternal && !startPinConnected) startPinInternal->connected = false;
            if (endPinInternal && !endPinConnected) endPinInternal->connected = false;

            UUID connectionUuid = it->uuid;

            if (m_state.connectionRemovedCallback) {
                m_state.connectionRemovedCallback(connectionId, connectionUuid);
            }

            m_state.connections.erase(it);
            updateConnectionUuidMap();
        }
    }

    Connection *NodeEditor::getConnection(int connectionId) {
        for (auto &connection: m_state.connections) {
            if (connection.id == connectionId) {
                return &connection;
            }
        }

        return nullptr;
    }

    const Connection *NodeEditor::getConnection(int connectionId) const {
        for (const auto &connection: m_state.connections) {
            if (connection.id == connectionId) {
                return &connection;
            }
        }
        return nullptr;
    }

    const std::vector<Connection> &NodeEditor::getConnections() const {
        if (m_state.connections.empty()) {
            bool connectedPinsFound = false;

            for (const auto &node: m_state.nodes) {
                for (const auto &pin: node.inputs) {
                    if (pin.connected) {
                        connectedPinsFound = true;
                    }
                }

                for (const auto &pin: node.outputs) {
                    if (pin.connected) {
                        connectedPinsFound = true;
                    }
                }
            }

            if (connectedPinsFound) {
            }
        }

        return m_state.connections;
    }

    bool NodeEditor::isConnected(int nodeId, int pinId) const {
        for (const auto &conn: m_state.connections) {
            if ((conn.startNodeId == nodeId && conn.startPinId == pinId) ||
                (conn.endNodeId == nodeId && conn.endPinId == pinId)) {
                return true;
            }
        }
        return false;
    }

    bool NodeEditor::isConnectedByUUID(const UUID &nodeUuid, const UUID &pinUuid) const {
        for (const auto &conn: m_state.connections) {
            if ((conn.startNodeUuid == nodeUuid && conn.startPinUuid == pinUuid) ||
                (conn.endNodeUuid == nodeUuid && conn.endPinUuid == pinUuid)) {
                return true;
            }
        }
        return false;
    }

    bool NodeEditor::doesConnectionExist(int startNodeId, int startPinId, int endNodeId, int endPinId) const {
        return std::any_of(m_state.connections.begin(), m_state.connections.end(),
                           [=](const Connection &conn) {
                               return conn.startNodeId == startNodeId &&
                                      conn.startPinId == startPinId &&
                                      conn.endNodeId == endNodeId &&
                                      conn.endPinId == endPinId;
                           });
    }

    bool NodeEditor::doesConnectionExistByUUID(const UUID &startNodeUuid, const UUID &startPinUuid,
                                               const UUID &endNodeUuid, const UUID &endPinUuid) const {
        return std::any_of(m_state.connections.begin(), m_state.connections.end(),
                           [&](const Connection &conn) {
                               return conn.startNodeUuid == startNodeUuid &&
                                      conn.startPinUuid == startPinUuid &&
                                      conn.endNodeUuid == endNodeUuid &&
                                      conn.endPinUuid == endPinUuid;
                           });
    }

    bool NodeEditor::canCreateConnection(const Pin &startPin, const Pin &endPin) const {
        std::cout << "=== DÉBUT VÉRIFICATION DE CONNEXION ===" << std::endl;
        std::cout << "Pin départ: id=" << startPin.id << ", nom='" << startPin.name 
                  << "', isInput=" << (startPin.isInput ? "true" : "false") 
                  << ", type=" << static_cast<int>(startPin.type)
                  << ", shape=" << static_cast<int>(startPin.shape) << std::endl;
        
        std::cout << "Pin arrivée: id=" << endPin.id << ", nom='" << endPin.name 
                  << "', isInput=" << (endPin.isInput ? "true" : "false") 
                  << ", type=" << static_cast<int>(endPin.type)
                  << ", shape=" << static_cast<int>(endPin.shape) << std::endl;
        
        if (startPin.isInput == endPin.isInput) {
            std::cout << "ERREUR: Impossible de connecter deux pins du même type (entrée-entrée ou sortie-sortie)" << std::endl;
            std::cout << "=== FIN VÉRIFICATION: ÉCHEC (pins de même direction) ===" << std::endl;
            return false;
        }

        const Pin &outputPin = startPin.isInput ? endPin : startPin;
        const Pin &inputPin = startPin.isInput ? startPin : endPin;
        
        std::cout << "Après réorganisation: outputPin=" << (startPin.isInput ? "endPin" : "startPin")
                  << ", inputPin=" << (startPin.isInput ? "startPin" : "endPin") << std::endl;

        if (m_state.canConnectCallback) {
            std::cout << "Exécution du callback canConnect..." << std::endl;
            bool result = m_state.canConnectCallback(outputPin, inputPin);
            if (!result) {
                std::cout << "ERREUR: Le callback canConnect a refusé la connexion" << std::endl;
                std::cout << "=== FIN VÉRIFICATION: ÉCHEC (callback) ===" << std::endl;
                return false;
            }
            std::cout << "Callback: connexion autorisée" << std::endl;
        } else {
            std::cout << "Aucun callback canConnect enregistré" << std::endl;
        }

        bool typeCompatible =
            static_cast<PinType>(outputPin.type) == static_cast<PinType>(inputPin.type) ||
            static_cast<PinType>(outputPin.type) == PinType::Blue ||
            static_cast<PinType>(inputPin.type) == PinType::Blue;

        std::cout << "Vérification de compatibilité des types:" << std::endl;
        std::cout << "  - output type=" << static_cast<int>(outputPin.type) << std::endl;
        std::cout << "  - input type=" << static_cast<int>(inputPin.type) << std::endl;
        std::cout << "  - PinType::Blue=" << static_cast<int>(PinType::Blue) << std::endl;
        std::cout << "  - Résultat: " << (typeCompatible ? "COMPATIBLE" : "INCOMPATIBLE") << std::endl;

        if (!typeCompatible) {
            std::cout << "ERREUR: Types incompatibles - output type=" << static_cast<int>(outputPin.type) 
                      << ", input type=" << static_cast<int>(inputPin.type) << std::endl;
            std::cout << "=== FIN VÉRIFICATION: ÉCHEC (types incompatibles) ===" << std::endl;
        } else {
            std::cout << "=== FIN VÉRIFICATION: SUCCÈS ===" << std::endl;
        }

        return typeCompatible;
    }

    int NodeEditor::addConnection(int startNodeId, int startPinId, int endNodeId, int endPinId, const UUID &uuid) {
        std::cout << "=== DÉBUT CRÉATION DE CONNEXION ===" << std::endl;
        std::cout << "Tentative de connexion: startNode=" << startNodeId << ", startPin=" << startPinId
                  << ", endNode=" << endNodeId << ", endPin=" << endPinId << std::endl;
     
        if (doesConnectionExist(startNodeId, startPinId, endNodeId, endPinId)) {
            std::cout << "ERREUR: La connexion existe déjà" << std::endl;
            std::cout << "=== FIN CRÉATION: ÉCHEC (connexion existante) ===" << std::endl;
            return -1;
        }

        Node *startNode = getNode(startNodeId);
        Node *endNode = getNode(endNodeId);

        if (!startNode) {
            std::cout << "ERREUR: Nœud de départ (ID " << startNodeId << ") introuvable" << std::endl;
            std::cout << "=== FIN CRÉATION: ÉCHEC (nœud départ introuvable) ===" << std::endl;
            return -1;
        }
        if (!endNode) {
            std::cout << "ERREUR: Nœud d'arrivée (ID " << endNodeId << ") introuvable" << std::endl;
            std::cout << "=== FIN CRÉATION: ÉCHEC (nœud arrivée introuvable) ===" << std::endl;
            return -1;
        }

        // Afficher les pins du nœud de départ
        std::cout << "Pins du nœud de départ (ID " << startNodeId << "):" << std::endl;
        std::cout << "  Inputs:" << std::endl;
        for (const auto& pin : startNode->inputs) {
            std::cout << "    ID=" << pin.id << ", Nom='" << pin.name << "'" << std::endl;
        }
        std::cout << "  Outputs:" << std::endl;
        for (const auto& pin : startNode->outputs) {
            std::cout << "    ID=" << pin.id << ", Nom='" << pin.name << "'" << std::endl;
        }

        // Afficher les pins du nœud d'arrivée
        std::cout << "Pins du nœud d'arrivée (ID " << endNodeId << "):" << std::endl;
        std::cout << "  Inputs:" << std::endl;
        for (const auto& pin : endNode->inputs) {
            std::cout << "    ID=" << pin.id << ", Nom='" << pin.name << "'" << std::endl;
        }
        std::cout << "  Outputs:" << std::endl;
        for (const auto& pin : endNode->outputs) {
            std::cout << "    ID=" << pin.id << ", Nom='" << pin.name << "'" << std::endl;
        }

        Pin *startPinInternal = startNode->findPin(startPinId);
        Pin *endPinInternal = endNode->findPin(endPinId);

        if (!startPinInternal) {
            std::cout << "ERREUR: Pin de départ (ID " << startPinId << ") introuvable dans le nœud " << startNodeId << std::endl;
            std::cout << "=== FIN CRÉATION: ÉCHEC (pin départ introuvable) ===" << std::endl;
            return -1;
        }
        if (!endPinInternal) {
            std::cout << "ERREUR: Pin d'arrivée (ID " << endPinId << ") introuvable dans le nœud " << endNodeId << std::endl;
            std::cout << "=== FIN CRÉATION: ÉCHEC (pin arrivée introuvable) ===" << std::endl;
            return -1;
        }

        std::cout << "Pins trouvés:" << std::endl;

        if (startPinInternal->isInput || !endPinInternal->isInput) {
            std::cout << "ERREUR: Direction des pins invalide - Pin départ (isInput=" 
                      << (startPinInternal->isInput ? "true" : "false") 
                      << "), Pin arrivée (isInput=" 
                      << (endPinInternal->isInput ? "true" : "false") << ")" << std::endl;
            std::cout << "=== FIN CRÉATION: ÉCHEC (direction pins invalide) ===" << std::endl;
            return -1;
        }

        Pin startPin, endPin;

        startPin.id = startPinInternal->id;
        startPin.uuid = startPinInternal->uuid;
        startPin.name = startPinInternal->name;
        startPin.isInput = startPinInternal->isInput;
        startPin.type = static_cast<PinType>(startPinInternal->type);
        startPin.shape = static_cast<PinShape>(startPinInternal->shape);

        endPin.id = endPinInternal->id;
        endPin.uuid = endPinInternal->uuid;
        endPin.name = endPinInternal->name;
        endPin.isInput = endPinInternal->isInput;
        endPin.type = static_cast<PinType>(endPinInternal->type);
        endPin.shape = static_cast<PinShape>(endPinInternal->shape);

        std::cout << "Vérification de compatibilité via canCreateConnection..." << std::endl;
        if (!canCreateConnection(startPin, endPin)) {
            std::cout << "ERREUR: La vérification canCreateConnection a échoué" << std::endl;
            std::cout << "=== FIN CRÉATION: ÉCHEC (canCreateConnection) ===" << std::endl;
            return -1;
        }
        std::cout << "canCreateConnection: OK" << std::endl;

        int connectionId = m_state.nextConnectionId++;
        Connection connection(connectionId, startNodeId, startPinId, endNodeId, endPinId);

        connection.uuid = uuid.empty() ? generateUUID() : uuid;
        std::cout << "ID de connexion attribué: " << connectionId << ", UUID: " << connection.uuid << std::endl;

        connection.startNodeUuid = startNode->uuid;
        connection.startPinUuid = startPinInternal->uuid;
        connection.endNodeUuid = endNode->uuid;
        connection.endPinUuid = endPinInternal->uuid;

        if (startNode->getSubgraphId() == endNode->getSubgraphId() && startNode->getSubgraphId() >= 0) {
            connection.subgraphId = startNode->getSubgraphId();
            std::cout << "Les nœuds sont dans le même sous-graphe (ID: " << connection.subgraphId << ")" << std::endl;
        } else {
            connection.subgraphId = -1;
            std::cout << "Les nœuds sont dans des sous-graphes différents ou dans le graphe principal" << std::endl;
        }

        startPinInternal->connected = true;
        endPinInternal->connected = true;

        m_state.connections.push_back(connection);
        std::cout << "Connexion ajoutée à la liste des connexions" << std::endl;

        updateConnectionUuidMap();

        if (m_state.connectionCreatedCallback) {
            std::cout << "Appel du callback de création de connexion" << std::endl;
            m_state.connectionCreatedCallback(connectionId, connection.uuid);
        }

        if (m_state.currentSubgraphId >= 0) {
            if (startNode->getSubgraphId() == m_state.currentSubgraphId &&
                endNode->getSubgraphId() == m_state.currentSubgraphId) {
                std::cout << "Ajout de la connexion au sous-graphe courant (ID: " << m_state.currentSubgraphId << ")" << std::endl;
                addConnectionToSubgraph(connectionId, m_state.currentSubgraphId);
            }
        }

        std::cout << "=== FIN CRÉATION: SUCCÈS (ID: " << connectionId << ") ===" << std::endl;
        return connectionId;
    }

    void NodeEditor::createConnection(int startNodeId, int startPinId, int endNodeId, int endPinId) {
        std::cout << "=== ENTRÉE DANS createConnection ===" << std::endl;
        
        const Pin *apiStartPin = getPin(startNodeId, startPinId);
        const Pin *apiEndPin = getPin(endNodeId, endPinId);

        if (!apiStartPin || !apiEndPin) {
            std::cout << "ERREUR createConnection: Un ou plusieurs pins introuvables" << std::endl;
            std::cout << "  apiStartPin: " << (apiStartPin ? "trouvé" : "introuvable") << std::endl;
            std::cout << "  apiEndPin: " << (apiEndPin ? "trouvé" : "introuvable") << std::endl;
            std::cout << "=== SORTIE DE createConnection: ÉCHEC (pins introuvables) ===" << std::endl;
            return;
        }

        std::cout << "Tentative de connexion entre:" << std::endl;
        std::cout << "  Départ: Nœud " << startNodeId << ", Pin " << startPinId 
                  << " ('" << apiStartPin->name << "', isInput=" << (apiStartPin->isInput ? "true" : "false") 
                  << ", type=" << static_cast<int>(apiStartPin->type) << ")" << std::endl;
        std::cout << "  Arrivée: Nœud " << endNodeId << ", Pin " << endPinId 
                  << " ('" << apiEndPin->name << "', isInput=" << (apiEndPin->isInput ? "true" : "false") 
                  << ", type=" << static_cast<int>(apiEndPin->type) << ")" << std::endl;

        // Garantir que nous connectons toujours une sortie vers une entrée
        if (apiStartPin->isInput && !apiEndPin->isInput) {
            std::cout << "Échange des pins pour assurer sortie → entrée" << std::endl;
            std::swap(startNodeId, endNodeId);
            std::swap(startPinId, endPinId);
            std::cout << "Après échange: startNode=" << startNodeId << ", startPin=" << startPinId 
                      << ", endNode=" << endNodeId << ", endPin=" << endPinId << std::endl;
        } else if (apiStartPin->isInput && apiEndPin->isInput) {
            std::cout << "ERREUR: Tentative de connecter deux entrées" << std::endl;
            std::cout << "=== SORTIE DE createConnection: ÉCHEC (deux entrées) ===" << std::endl;
            return;
        } else if (!apiStartPin->isInput && !apiEndPin->isInput) {
            std::cout << "ERREUR: Tentative de connecter deux sorties" << std::endl;
            std::cout << "=== SORTIE DE createConnection: ÉCHEC (deux sorties) ===" << std::endl;
            return;
        }

        int connectionId = addConnection(startNodeId, startPinId, endNodeId, endPinId, "");
        if (connectionId != -1) {
            std::cout << "Connexion créée avec succès, ID: " << connectionId << std::endl;
            std::cout << "=== SORTIE DE createConnection: SUCCÈS ===" << std::endl;
        } else {
            std::cout << "Échec de la création de connexion" << std::endl;
            std::cout << "=== SORTIE DE createConnection: ÉCHEC (addConnection a échoué) ===" << std::endl;
        }
    }

    void NodeEditor::createConnectionByUUID(const UUID &startNodeUuid, const UUID &startPinUuid,
                                            const UUID &endNodeUuid, const UUID &endPinUuid) {
        const Pin *apiStartPin = getPinByUUID(startNodeUuid, startPinUuid);
        const Pin *apiEndPin = getPinByUUID(endNodeUuid, endPinUuid);

        if (!apiStartPin || !apiEndPin) return;

        if (apiStartPin->isInput) {
            addConnectionByUUID(endNodeUuid, endPinUuid, startNodeUuid, startPinUuid, "");
        } else {
            addConnectionByUUID(startNodeUuid, startPinUuid, endNodeUuid, endPinUuid, "");
        }
    }

    void NodeEditor::selectConnection(int connectionId, bool append) {
        if (!append) {
            deselectAllConnections();
        }

        Connection *connection = getConnection(connectionId);
        if (connection) {
            connection->selected = true;
        }
    }

    void NodeEditor::selectConnectionByUUID(const UUID &uuid, bool append) {
        Connection *connection = getConnectionByUUID(uuid);
        if (connection) {
            selectConnection(connection->id, append);
        }
    }

    void NodeEditor::deselectConnection(int connectionId) {
        Connection *connection = getConnection(connectionId);
        if (connection) {
            connection->selected = false;
        }
    }

    void NodeEditor::deselectConnectionByUUID(const UUID &uuid) {
        Connection *connection = getConnectionByUUID(uuid);
        if (connection) {
            connection->selected = false;
        }
    }

    void NodeEditor::deselectAllConnections() {
        for (auto it = m_state.connections.begin(); it != m_state.connections.end(); ++it) {
            it->selected = false;
        }
    }

    int NodeEditor::getConnectionId(const UUID &uuid) const {
        auto it = m_state.connectionUuidMap.find(uuid);
        return it != m_state.connectionUuidMap.end() ? it->second->id : -1;
    }

    int NodeEditor::addConnectionByUUID(const UUID &startNodeUuid, const UUID &startPinUuid,
                                        const UUID &endNodeUuid, const UUID &endPinUuid, const UUID &uuid) {
        int startNodeId = getNodeId(startNodeUuid);
        int endNodeId = getNodeId(endNodeUuid);

        if (startNodeId == -1 || endNodeId == -1) return -1;

        Node *startNode = getNode(startNodeId);
        Node *endNode = getNode(endNodeId);

        if (!startNode || !endNode) return -1;

        int startPinId = -1;
        int endPinId = -1;

        for (const auto &pin: startNode->outputs) {
            if (pin.uuid == startPinUuid) {
                startPinId = pin.id;
                break;
            }
        }

        for (const auto &pin: endNode->inputs) {
            if (pin.uuid == endPinUuid) {
                endPinId = pin.id;
                break;
            }
        }

        if (startPinId == -1 || endPinId == -1) return -1;

        return addConnection(startNodeId, startPinId, endNodeId, endPinId, uuid);
    }

    UUID NodeEditor::addConnectionWithUUID(int startNodeId, int startPinId, int endNodeId, int endPinId) {
        int connectionId = addConnection(startNodeId, startPinId, endNodeId, endPinId);
        if (connectionId == -1) return "";
        return getConnectionUUID(connectionId);
    }

    UUID NodeEditor::addConnectionWithUUIDByUUID(const UUID &startNodeUuid, const UUID &startPinUuid,
                                                 const UUID &endNodeUuid, const UUID &endPinUuid) {
        int connectionId = addConnectionByUUID(startNodeUuid, startPinUuid, endNodeUuid, endPinUuid, "");
        if (connectionId == -1) return "";
        return getConnectionUUID(connectionId);
    }

    Connection *NodeEditor::getConnectionByUUID(const UUID &uuid) {
        auto it = m_state.connectionUuidMap.find(uuid);
        if (it != m_state.connectionUuidMap.end()) {
            return it->second;
        }

        for (auto &connection: m_state.connections) {
            if (connection.uuid == uuid) {
                return &connection;
            }
        }
        return nullptr;
    }

    const Connection *NodeEditor::getConnectionByUUID(const UUID &uuid) const {
        auto it = m_state.connectionUuidMap.find(uuid);
        if (it != m_state.connectionUuidMap.end()) {
            return it->second;
        }

        for (const auto &connection: m_state.connections) {
            if (connection.uuid == uuid) {
                return &connection;
            }
        }
        return nullptr;
    }

}
