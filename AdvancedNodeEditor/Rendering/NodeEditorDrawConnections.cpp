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

            const ANE::Pin* apiStartPin = getPin(connection.startNodeId, connection.startPinId);
            const ANE::Pin* apiEndPin = getPin(connection.endNodeId, connection.endPinId);

            if (!apiStartPin || !apiEndPin) continue;

            const Pin* startPinInternal = startNode->findPin(connection.startPinId);
            const Pin* endPinInternal = endNode->findPin(connection.endPinId);

            if (!startPinInternal || !endPinInternal) continue;

            ImVec2 p1 = getPinPos(*startNode, *apiStartPin, canvasPos);
            ImVec2 p2 = getPinPos(*endNode, *apiEndPin, canvasPos);

            std::string startPinType = pinTypeToString(startPinInternal->type);
            std::string endPinType = pinTypeToString(endPinInternal->type);

            const internal::PinColors& startPinColors = m_state.style.pinColors.count(startPinType) ?
                                           m_state.style.pinColors.at(startPinType) :
                                           m_state.style.pinColors.at("Default");

            const internal::PinColors& endPinColors = m_state.style.pinColors.count(endPinType) ?
                                         m_state.style.pinColors.at(endPinType) :
                                         m_state.style.pinColors.at("Default");

            ImU32 startColor = IM_COL32(
                startPinColors.connectedColor.r * 255,
                startPinColors.connectedColor.g * 255,
                startPinColors.connectedColor.b * 255,
                startPinColors.connectedColor.a * 255 * 0.8f
            );

            ImU32 endColor = IM_COL32(
                endPinColors.connectedColor.r * 255,
                endPinColors.connectedColor.g * 255,
                endPinColors.connectedColor.b * 255,
                endPinColors.connectedColor.a * 255 * 0.8f
            );

            ImU32 outerColor = IM_COL32(40, 44, 52, 100);

            ImU32 coreColor = IM_COL32(
                (startPinColors.connectedColor.r + endPinColors.connectedColor.r) * 0.5f * 255 + 50,
                (startPinColors.connectedColor.g + endPinColors.connectedColor.g) * 0.5f * 255 + 50,
                (startPinColors.connectedColor.b + endPinColors.connectedColor.b) * 0.5f * 255 + 50,
                200
            );

            if (connection.selected) {
                startColor = IM_COL32(
                    m_state.style.connectionColors.selectedColor.r * 255,
                    m_state.style.connectionColors.selectedColor.g * 255,
                    m_state.style.connectionColors.selectedColor.b * 255,
                    m_state.style.connectionColors.selectedColor.a * 255
                );
                endColor = startColor;
                coreColor = IM_COL32(
                    m_state.style.connectionColors.selectedColor.r * 255 + 50,
                    m_state.style.connectionColors.selectedColor.g * 255 + 50,
                    m_state.style.connectionColors.selectedColor.b * 255 + 50,
                    m_state.style.connectionColors.selectedColor.a * 255
                );
                outerColor = IM_COL32(
                    m_state.style.connectionColors.selectedColor.r * 255 * 0.7f,
                    m_state.style.connectionColors.selectedColor.g * 255 * 0.7f,
                    m_state.style.connectionColors.selectedColor.b * 255 * 0.7f,
                    150
                );
            }

            float distance = std::abs(p2.y - p1.y);
            float controlPointYOffset = std::max(distance * 0.5f, 40.0f);

            ImVec2 cp1 = ImVec2(p1.x, p1.y + controlPointYOffset);
            ImVec2 cp2 = ImVec2(p2.x, p2.y - controlPointYOffset);

            const float outerThickness = 3.5f * m_state.viewScale;
            drawList->AddBezierCubic(p1, cp1, cp2, p2, outerColor, outerThickness);

            const float mainThickness = 2.0f * m_state.viewScale;
            const int lineSegments = 20;

            const float startSolidPct = 0.15f;
            const float endSolidPct = 0.15f;
            const float transitionPct = 0.7f;

            for (int i = 0; i < lineSegments; i++) {
                float t0 = static_cast<float>(i) / lineSegments;
                float t1 = static_cast<float>(i + 1) / lineSegments;

                ImVec2 pos0 = ImBezierCubicCalc(p1, cp1, cp2, p2, t0);
                ImVec2 pos1 = ImBezierCubicCalc(p1, cp1, cp2, p2, t1);

                ImU32 segmentColor;
                if (t0 < startSolidPct) {
                    segmentColor = startColor;
                }
                else if (t0 > (1.0f - endSolidPct)) {
                    segmentColor = endColor;
                }
                else {
                    float transitionPos = (t0 - startSolidPct) / transitionPct;
                    transitionPos = std::min(1.0f, std::max(0.0f, transitionPos));
                    segmentColor = ImLerpColor(startColor, endColor, transitionPos);
                }

                drawList->AddLine(pos0, pos1, segmentColor, mainThickness);
            }

            const float coreThickness = 0.8f * m_state.viewScale;

            for (int i = 0; i < lineSegments; i++) {
                float t0 = static_cast<float>(i) / lineSegments;
                float t1 = static_cast<float>(i + 1) / lineSegments;

                ImVec2 pos0 = ImBezierCubicCalc(p1, cp1, cp2, p2, t0);
                ImVec2 pos1 = ImBezierCubicCalc(p1, cp1, cp2, p2, t1);

                ImVec4 brightStartVec4 = ImGui::ColorConvertU32ToFloat4(startColor);
                ImVec4 brightEndVec4 = ImGui::ColorConvertU32ToFloat4(endColor);

                brightStartVec4.x = std::min(brightStartVec4.x + 0.3f, 1.0f);
                brightStartVec4.y = std::min(brightStartVec4.y + 0.3f, 1.0f);
                brightStartVec4.z = std::min(brightStartVec4.z + 0.3f, 1.0f);
                brightStartVec4.w = 0.7f;

                brightEndVec4.x = std::min(brightEndVec4.x + 0.3f, 1.0f);
                brightEndVec4.y = std::min(brightEndVec4.y + 0.3f, 1.0f);
                brightEndVec4.z = std::min(brightEndVec4.z + 0.3f, 1.0f);
                brightEndVec4.w = 0.7f;

                ImU32 brightStart = ImGui::ColorConvertFloat4ToU32(brightStartVec4);
                ImU32 brightEnd = ImGui::ColorConvertFloat4ToU32(brightEndVec4);

                ImU32 brightSegmentColor;
                if (t0 < startSolidPct) {
                    brightSegmentColor = brightStart;
                }
                else if (t0 > (1.0f - endSolidPct)) {
                    brightSegmentColor = brightEnd;
                }
                else {
                    float transitionPos = (t0 - startSolidPct) / transitionPct;
                    transitionPos = std::min(1.0f, std::max(0.0f, transitionPos));
                    brightSegmentColor = ImLerpColor(brightStart, brightEnd, transitionPos);
                }

                drawList->AddLine(pos0, pos1, brightSegmentColor, coreThickness);
            }

            const float glowRadius = 2.5f * m_state.viewScale;
            ImU32 startGlowColor = IM_COL32(
                std::min(static_cast<int>(startPinColors.connectedColor.r * 255 + 50), 255),
                std::min(static_cast<int>(startPinColors.connectedColor.g * 255 + 50), 255),
                std::min(static_cast<int>(startPinColors.connectedColor.b * 255 + 50), 255),
                180
            );

            ImU32 endGlowColor = IM_COL32(
                std::min(static_cast<int>(endPinColors.connectedColor.r * 255 + 50), 255),
                std::min(static_cast<int>(endPinColors.connectedColor.g * 255 + 50), 255),
                std::min(static_cast<int>(endPinColors.connectedColor.b * 255 + 50), 255),
                180
            );

            drawList->AddCircleFilled(p1, glowRadius, startGlowColor);
            drawList->AddCircleFilled(p2, glowRadius, endGlowColor);
        }

        if (m_state.connecting && m_state.connectingNodeId != -1 && m_state.connectingPinId != -1) {
            drawDragConnection(drawList, canvasPos);
        }
    }
}