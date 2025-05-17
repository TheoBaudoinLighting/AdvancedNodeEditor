#include "../NodeEditor.h"
#include <algorithm>

namespace NodeEditorCore {
    void NodeEditor::drawConnections(ImDrawList *drawList, const ImVec2 &canvasPos) {
    std::vector<Connection> visibleConnections;
    int currentSubgraphId = m_state.currentSubgraphId;

    for (const auto &connection: m_state.connections) {
        const Node *startNode = getNode(connection.startNodeId);
        const Node *endNode = getNode(connection.endNodeId);

        if (!startNode || !endNode) continue;

        bool startInCurrentSubgraph = (currentSubgraphId >= 0 && startNode->getSubgraphId() == currentSubgraphId) ||
                                     (currentSubgraphId == -1 && startNode->getSubgraphId() == -1);

        bool endInCurrentSubgraph = (currentSubgraphId >= 0 && endNode->getSubgraphId() == currentSubgraphId) ||
                                    (currentSubgraphId == -1 && endNode->getSubgraphId() == -1);

        if (startInCurrentSubgraph && endInCurrentSubgraph) {
            visibleConnections.push_back(connection);
        }
    }

    for (const auto &connection: visibleConnections) {
        const Node *startNode = getNode(connection.startNodeId);
        const Node *endNode = getNode(connection.endNodeId);

        if (!startNode || !endNode) continue;

        const Pin *apiStartPin = getPin(connection.startNodeId, connection.startPinId);
        const Pin *apiEndPin = getPin(connection.endNodeId, connection.endPinId);

        if (!apiStartPin || !apiEndPin) continue;

        const Pin *startPinInternal = startNode->findPin(connection.startPinId);
        const Pin *endPinInternal = endNode->findPin(connection.endPinId);

        if (!startPinInternal || !endPinInternal) continue;

        ImVec2 p1 = getPinPos(*startNode, *apiStartPin, canvasPos);
        ImVec2 p2 = getPinPos(*endNode, *apiEndPin, canvasPos);

        std::string startPinType = pinTypeToString(startPinInternal->type);
        std::string endPinType = pinTypeToString(endPinInternal->type);

        const internal::PinColors &startPinColors = m_state.style.pinColors.count(startPinType)
                                                        ? m_state.style.pinColors.at(startPinType)
                                                        : m_state.style.pinColors.at("Default");

        const internal::PinColors &endPinColors = m_state.style.pinColors.count(endPinType)
                                                      ? m_state.style.pinColors.at(endPinType)
                                                      : m_state.style.pinColors.at("Default");

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

        m_connectionStyleManager.drawConnection(
            drawList, p1, p2,
            apiStartPin->isInput, apiEndPin->isInput,
            connection.selected, m_state.hoveredConnectionId == connection.id,
            startCol, endCol, m_state.viewScale
        );
    }

    if (m_state.connecting && m_state.connectingNodeId != -1 && m_state.connectingPinId != -1) {
        drawDragConnection(drawList, canvasPos);
    }
}
}
