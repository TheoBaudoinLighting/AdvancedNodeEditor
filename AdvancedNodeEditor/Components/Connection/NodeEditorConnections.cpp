#include "../../NodeEditor.h"
#include <algorithm>

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
        if (startPin.isInput == endPin.isInput) {
            return false;
        }

        const Pin &outputPin = startPin.isInput ? endPin : startPin;
        const Pin &inputPin = startPin.isInput ? startPin : endPin;

        if (m_state.canConnectCallback) {
            bool result = m_state.canConnectCallback(outputPin, inputPin);
            if (!result) {
                return false;
            }
        }

        bool typeCompatible =
            static_cast<PinType>(outputPin.type) == static_cast<PinType>(inputPin.type) ||
            static_cast<PinType>(outputPin.type) == PinType::Blue ||
            static_cast<PinType>(inputPin.type) == PinType::Blue;

        return typeCompatible;
    }

    int NodeEditor::addConnection(int startNodeId, int startPinId, int endNodeId, int endPinId, const UUID &uuid) {

        if (doesConnectionExist(startNodeId, startPinId, endNodeId, endPinId)) {
            return -1;
        }

        Node *startNode = getNode(startNodeId);
        Node *endNode = getNode(endNodeId);

        if (!startNode) {
            return -1;
        }
        if (!endNode) {
            return -1;
        }

        Pin *startPinInternal = startNode->findPin(startPinId);
        Pin *endPinInternal = endNode->findPin(endPinId);

        if (!startPinInternal) {
            return -1;
        }
        if (!endPinInternal) {
            return -1;
        }

        if (startPinInternal->isInput || !endPinInternal->isInput) {
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

        if (!canCreateConnection(startPin, endPin)) {
            return -1;
        }

        int connectionId = m_state.nextConnectionId++;
        Connection connection(connectionId, startNodeId, startPinId, endNodeId, endPinId);

        connection.uuid = uuid.empty() ? generateUUID() : uuid;

        connection.startNodeUuid = startNode->uuid;
        connection.startPinUuid = startPinInternal->uuid;
        connection.endNodeUuid = endNode->uuid;
        connection.endPinUuid = endPinInternal->uuid;

        if (startNode->getSubgraphId() == endNode->getSubgraphId() && startNode->getSubgraphId() >= 0) {
            connection.subgraphId = startNode->getSubgraphId();
        } else {
            connection.subgraphId = -1;
        }

        startPinInternal->connected = true;
        endPinInternal->connected = true;

        m_state.connections.push_back(connection);

        updateConnectionUuidMap();

        if (m_state.connectionCreatedCallback) {
            m_state.connectionCreatedCallback(connectionId, connection.uuid);
        }

        if (m_state.currentSubgraphId >= 0) {
            if (startNode->getSubgraphId() == m_state.currentSubgraphId &&
                endNode->getSubgraphId() == m_state.currentSubgraphId) {
                addConnectionToSubgraph(connectionId, m_state.currentSubgraphId);
            }
        }

        return connectionId;
    }

    void NodeEditor::createConnection(int startNodeId, int startPinId, int endNodeId, int endPinId) {

        const Pin *apiStartPin = getPin(startNodeId, startPinId);
        const Pin *apiEndPin = getPin(endNodeId, endPinId);

        if (!apiStartPin || !apiEndPin) {
            return;
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
