#include "NodeEditor.h"
#include <algorithm>

namespace NodeEditorCore {

int NodeEditor::addConnection(int startNodeId, int startPinId, int endNodeId, int endPinId) {
    if (doesConnectionExist(startNodeId, startPinId, endNodeId, endPinId)) {
        return -1;
    }
    
    const Pin* startPin = getPin(startNodeId, startPinId);
    const Pin* endPin = getPin(endNodeId, endPinId);
    
    if (!startPin || !endPin) {
        return -1;
    }
    
    if (startPin->isInput || !endPin->isInput) {
        return -1;
    }
    
    if (!canCreateConnection(*startPin, *endPin)) {
        return -1;
    }
    
    int connectionId = m_state.nextConnectionId++;
    m_state.connections.emplace_back(connectionId, startNodeId, startPinId, endNodeId, endPinId);
    
    Pin* startPinMutable = getPin(startNodeId, startPinId);
    Pin* endPinMutable = getPin(endNodeId, endPinId);
    
    if (startPinMutable) startPinMutable->connected = true;
    if (endPinMutable) endPinMutable->connected = true;
    
    if (m_state.connectionCreatedCallback) {
        m_state.connectionCreatedCallback(connectionId);
    }
    
    return connectionId;
}

void NodeEditor::removeConnection(int connectionId) {
    auto it = std::find_if(m_state.connections.begin(), m_state.connections.end(),
                          [connectionId](const Connection& conn) { return conn.id == connectionId; });
    
    if (it != m_state.connections.end()) {
        Pin* startPin = getPin(it->startNodeId, it->startPinId);
        Pin* endPin = getPin(it->endNodeId, it->endPinId);
        
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
        
        if (startPin && !startPinConnected) startPin->connected = false;
        if (endPin && !endPinConnected) endPin->connected = false;
        
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

bool NodeEditor::canCreateConnection(const Pin& startPin, const Pin& endPin) const {
    if (startPin.isInput == endPin.isInput) {
        return false;
    }
    
    if (m_state.canConnectCallback) {
        const Pin& outputPin = startPin.isInput ? endPin : startPin;
        const Pin& inputPin = startPin.isInput ? startPin : endPin;
        return m_state.canConnectCallback(outputPin, inputPin);
    }
    
    return startPin.type == endPin.type || 
           startPin.type == PinType::Flow || 
           endPin.type == PinType::Flow;
}

void NodeEditor::createConnection(int startNodeId, int startPinId, int endNodeId, int endPinId) {
    const Pin* startPin = getPin(startNodeId, startPinId);
    const Pin* endPin = getPin(endNodeId, endPinId);
    
    if (!startPin || !endPin) return;
    
    if (startPin->isInput) {
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