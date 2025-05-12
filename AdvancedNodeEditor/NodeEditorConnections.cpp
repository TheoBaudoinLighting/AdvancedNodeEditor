#include "NodeEditor.h"
#include <algorithm>

namespace NodeEditorCore {

int NodeEditor::addConnection(int startNodeId, int startPinId, int endNodeId, int endPinId) {
    if (doesConnectionExist(startNodeId, startPinId, endNodeId, endPinId)) {
        return -1;
    }

    const ANE::Pin* apiStartPin = getPin(startNodeId, startPinId);
    const ANE::Pin* apiEndPin = getPin(endNodeId, endPinId);

    if (!apiStartPin || !apiEndPin) {
        return -1;
    }

    if (apiStartPin->isInput || !apiEndPin->isInput) {
        return -1;
    }

    if (!canCreateConnection(*apiStartPin, *apiEndPin)) {
        return -1;
    }

    int connectionId = m_state.nextConnectionId++;
    m_state.connections.emplace_back(connectionId, startNodeId, startPinId, endNodeId, endPinId);

    Node* startNode = getNode(startNodeId);
    Node* endNode = getNode(endNodeId);
    Pin* startPinInternal = startNode ? startNode->findPin(startPinId) : nullptr;
    Pin* endPinInternal = endNode ? endNode->findPin(endPinId) : nullptr;

    if (startPinInternal) startPinInternal->connected = true;
    if (endPinInternal) endPinInternal->connected = true;

    if (m_state.connectionCreatedCallback) {
        m_state.connectionCreatedCallback(connectionId);
    }

    return connectionId;
}

void NodeEditor::removeConnection(int connectionId) {
    auto it = std::find_if(m_state.connections.begin(), m_state.connections.end(),
                          [connectionId](const Connection& conn) { return conn.id == connectionId; });

    if (it != m_state.connections.end()) {
        Node* startNode = getNode(it->startNodeId);
        Node* endNode = getNode(it->endNodeId);

        Pin* startPinInternal = startNode ? startNode->findPin(it->startPinId) : nullptr;
        Pin* endPinInternal = endNode ? endNode->findPin(it->endPinId) : nullptr;

        bool startPinConnected = false;
        bool endPinConnected = false;

        for (const auto& conn : m_state.connections) {
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

        if (m_state.connectionRemovedCallback) {
            m_state.connectionRemovedCallback(connectionId);
        }

        m_state.connections.erase(it);
    }
}

Connection* NodeEditor::getConnection(int connectionId) {
    auto it = std::find_if(m_state.connections.begin(), m_state.connections.end(),
                          [connectionId](const Connection& conn) { return conn.id == connectionId; });
    return it != m_state.connections.end() ? &(*it) : nullptr;
}

const Connection* NodeEditor::getConnection(int connectionId) const {
    auto it = std::find_if(m_state.connections.begin(), m_state.connections.end(),
                          [connectionId](const Connection& conn) { return conn.id == connectionId; });
    return it != m_state.connections.end() ? &(*it) : nullptr;
}

const std::vector<Connection>& NodeEditor::getConnections() const {
    return m_state.connections;
}

bool NodeEditor::isConnected(int nodeId, int pinId) const {
    for (const auto& conn : m_state.connections) {
        if ((conn.startNodeId == nodeId && conn.startPinId == pinId) ||
            (conn.endNodeId == nodeId && conn.endPinId == pinId)) {
            return true;
        }
    }
    return false;
}

bool NodeEditor::doesConnectionExist(int startNodeId, int startPinId, int endNodeId, int endPinId) const {
    return std::any_of(m_state.connections.begin(), m_state.connections.end(),
                     [=](const Connection& conn) {
                         return conn.startNodeId == startNodeId &&
                                conn.startPinId == startPinId &&
                                conn.endNodeId == endNodeId &&
                                conn.endPinId == endPinId;
                     });
}

bool NodeEditor::canCreateConnection(const ANE::Pin& startPin, const ANE::Pin& endPin) const {
    if (startPin.isInput == endPin.isInput) {
        return false;
    }

    if (m_state.canConnectCallback) {
        const ANE::Pin& outputPin = startPin.isInput ? endPin : startPin;
        const ANE::Pin& inputPin = startPin.isInput ? startPin : endPin;
        return m_state.canConnectCallback(outputPin, inputPin);
    }

    return static_cast<PinType>(startPin.type) == static_cast<PinType>(endPin.type) ||
           static_cast<PinType>(startPin.type) == PinType::Blue ||
           static_cast<PinType>(endPin.type) == PinType::Blue;
}

void NodeEditor::createConnection(int startNodeId, int startPinId, int endNodeId, int endPinId) {
    const ANE::Pin* apiStartPin = getPin(startNodeId, startPinId);
    const ANE::Pin* apiEndPin = getPin(endNodeId, endPinId);

    if (!apiStartPin || !apiEndPin) return;

    if (apiStartPin->isInput) {
        std::swap(startNodeId, endNodeId);
        std::swap(startPinId, endPinId);
    }
    
    addConnection(startNodeId, startPinId, endNodeId, endPinId);
}

void NodeEditor::selectConnection(int connectionId, bool append) {
    if (!append) {
        deselectAllConnections();
    }
    
    Connection* connection = getConnection(connectionId);
    if (connection) {
        connection->selected = true;
    }
}

void NodeEditor::deselectConnection(int connectionId) {
    Connection* connection = getConnection(connectionId);
    if (connection) {
        connection->selected = false;
    }
}

void NodeEditor::deselectAllConnections() {
    for (auto& connection : m_state.connections) {
        connection.selected = false;
    }
}

}