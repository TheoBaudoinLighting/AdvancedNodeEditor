#include "../NodeEditor.h"
#include <algorithm>

namespace NodeEditorCore {
    void NodeEditor::drawConnections(ImDrawList* drawList, const ImVec2& canvasPos) {
        std::vector<Connection> visibleConnections;
        int currentSubgraphId = m_state.currentSubgraphId;

        if (currentSubgraphId >= 0) {
            std::vector<int> connectionIds = getConnectionsInSubgraph(currentSubgraphId);
            std::unordered_set<int> connectionIdSet(connectionIds.begin(), connectionIds.end());

            for (const auto& connection : m_state.connections) {
                if (connectionIdSet.find(connection.id) != connectionIdSet.end()) {
                    visibleConnections.push_back(connection);
                }
            }
        } else {
            for (const auto& connection : m_state.connections) {
                if (connection.getSubgraphId() == -1) {
                    visibleConnections.push_back(connection);
                }
            }
        }

        for (const auto& connection : visibleConnections) {
        const Node* startNode = getNode(connection.startNodeId);
        const Node* endNode = getNode(connection.endNodeId);

        if (!startNode || !endNode) continue;

        const Pin* apiStartPin = getPin(connection.startNodeId, connection.startPinId);
        const Pin* apiEndPin = getPin(connection.endNodeId, connection.endPinId);

        if (!apiStartPin || !apiEndPin) continue;

        const Pin* startPinInternal = startNode->findPin(connection.startPinId);
        const Pin* endPinInternal = endNode->findPin(connection.endPinId);

        if (!startPinInternal || !endPinInternal) continue;

        ImVec2 p1 = getPinPos(*startNode, *apiStartPin, canvasPos);
        ImVec2 p2 = getPinPos(*endNode, *apiEndPin, canvasPos);

        std::string startPinType = pinTypeToString(startPinInternal->type);
        std::string endPinType = pinTypeToString(endPinInternal->type);

        // Obtenir les couleurs de pins
        const internal::PinColors& startPinColors = m_state.style.pinColors.count(startPinType) ?
                                         m_state.style.pinColors.at(startPinType) :
                                         m_state.style.pinColors.at("Default");

        const internal::PinColors& endPinColors = m_state.style.pinColors.count(endPinType) ?
                                       m_state.style.pinColors.at(endPinType) :
                                       m_state.style.pinColors.at("Default");

        Color startCol(
            startPinColors.connected.r,
            startPinColors.connected.g,
            startPinColors.connected.b,
            startPinColors.connected.a * 0.8f
        );

        Color endCol(
            endPinColors.connected.r,
            endPinColors.connected.g,
            endPinColors.connected.b,
            endPinColors.connected.a * 0.8f
        );

        // Utiliser le ConnectionStyleManager pour dessiner la connexion
        m_connectionStyleManager.drawConnection(
            drawList, p1, p2,
            connection.selected, m_state.hoveredConnectionId == connection.id,
            startCol, endCol, m_state.viewScale
        );
    }

        if (m_state.connecting && m_state.connectingNodeId != -1 && m_state.connectingPinId != -1) {
            drawDragConnection(drawList, canvasPos);
        }
    }
}