#include "../NodeEditor.h"
#include <algorithm>

namespace NodeEditorCore {
    static inline ImVec2 ImLerpVec2(const ImVec2& a, const ImVec2& b, float t) {
        return ImVec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
    }

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

            auto& connAnimState = m_animationManager.getConnectionAnimationState(connection.id);

            if (connAnimState.flowSpeed > 0.0f) {
                std::vector<ImVec2> pathPoints;
                ConnectionStyleManager::ConnectionStyle style = m_connectionStyleManager.getDefaultStyle();

                switch (style) {
                    case ConnectionStyleManager::ConnectionStyle::Bezier: {
                        const float distance = std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
                        const float tension = m_connectionStyleManager.getConfig().curveTension;
                        const float cpDistance = distance * tension;

                        ImVec2 cp1, cp2;
                        if (apiStartPin->isInput) {
                            cp1 = ImVec2(p1.x, p1.y - cpDistance);
                        } else {
                            cp1 = ImVec2(p1.x, p1.y + cpDistance);
                        }

                        if (apiEndPin->isInput) {
                            cp2 = ImVec2(p2.x, p2.y - cpDistance);
                        } else {
                            cp2 = ImVec2(p2.x, p2.y + cpDistance);
                        }

                        const int particleCount = 5;
                        for (int i = 0; i < particleCount; i++) {
                            float t = (connAnimState.flowAnimation + (float)i / particleCount) -
                                    std::floor(connAnimState.flowAnimation + (float)i / particleCount);

                            ImVec2 particlePos = ImBezierCubicCalc(p1, cp1, cp2, p2, t);
                            pathPoints.push_back(particlePos);
                        }
                        break;
                    }

                    case ConnectionStyleManager::ConnectionStyle::StraightLine: {
                        const int particleCount = 5;
                        for (int i = 0; i < particleCount; i++) {
                            float t = (connAnimState.flowAnimation + (float)i / particleCount) -
                                    std::floor(connAnimState.flowAnimation + (float)i / particleCount);

                            ImVec2 particlePos = ImLerpVec2(p1, p2, t);
                            pathPoints.push_back(particlePos);
                        }
                        break;
                    }

                    case ConnectionStyleManager::ConnectionStyle::AngleLine: {
                        ImVec2 middle = ImVec2(p2.x, p1.y);

                        const int particleCount = 5;
                        for (int i = 0; i < particleCount; i++) {
                            float t = (connAnimState.flowAnimation + (float)i / particleCount) -
                                    std::floor(connAnimState.flowAnimation + (float)i / particleCount);

                            ImVec2 particlePos;
                            if (t < 0.5f) {
                                particlePos = ImLerpVec2(p1, middle, t * 2.0f);
                            } else {
                                particlePos = ImLerpVec2(middle, p2, (t - 0.5f) * 2.0f);
                            }
                            pathPoints.push_back(particlePos);
                        }
                        break;
                    }

                    case ConnectionStyleManager::ConnectionStyle::MetroLine: {
                        float dx = p2.x - p1.x;
                        float dy = p2.y - p1.y;

                        ImVec2 middle1, middle2;
                        if (std::abs(dx) > std::abs(dy)) {
                            middle1 = ImVec2(p1.x + dx * 0.5f, p1.y);
                            middle2 = ImVec2(p1.x + dx * 0.5f, p2.y);
                        } else {
                            middle1 = ImVec2(p1.x, p1.y + dy * 0.5f);
                            middle2 = ImVec2(p2.x, p1.y + dy * 0.5f);
                        }

                        std::vector<ImVec2> metroPoints = {p1, middle1, middle2, p2};

                        const int particleCount = 5;
                        for (int i = 0; i < particleCount; i++) {
                            float t = (connAnimState.flowAnimation + (float)i / particleCount) -
                                    std::floor(connAnimState.flowAnimation + (float)i / particleCount);

                            float segmentLength = 1.0f / (metroPoints.size() - 1);
                            int segment = std::min(static_cast<int>(t / segmentLength),
                                                static_cast<int>(metroPoints.size()) - 2);

                            float segmentT = (t - segment * segmentLength) / segmentLength;
                            ImVec2 particlePos = ImLerpVec2(metroPoints[segment], metroPoints[segment + 1], segmentT);

                            pathPoints.push_back(particlePos);
                        }
                        break;
                    }

                    default:
                        const int particleCount = 5;
                        for (int i = 0; i < particleCount; i++) {
                            float t = (connAnimState.flowAnimation + (float)i / particleCount) -
                                    std::floor(connAnimState.flowAnimation + (float)i / particleCount);

                            ImVec2 particlePos = ImLerpVec2(p1, p2, t);
                            pathPoints.push_back(particlePos);
                        }
                        break;
                }

                for (size_t i = 0; i < pathPoints.size(); i++) {
                    float t = static_cast<float>(i) / pathPoints.size();

                    ImVec4 startColVec4 = ImGui::ColorConvertU32ToFloat4(startCol.toImU32());
                    ImVec4 endColVec4 = ImGui::ColorConvertU32ToFloat4(endCol.toImU32());
                    ImVec4 particleColor = ImVec4(
                        startColVec4.x * (1.0f - t) + endColVec4.x * t,
                        startColVec4.y * (1.0f - t) + endColVec4.y * t,
                        startColVec4.z * (1.0f - t) + endColVec4.z * t,
                        (startColVec4.w * (1.0f - t) + endColVec4.w * t) * 1.5f
                    );

                    float pulseScale = std::sin(t * 3.14159f * 2.0f) * 0.3f + 1.0f;
                    float particleSize = 3.5f * m_state.viewScale * pulseScale;

                    drawList->AddCircleFilled(
                        pathPoints[i],
                        particleSize,
                        ImGui::ColorConvertFloat4ToU32(particleColor)
                    );

                    drawList->AddCircle(
                        pathPoints[i],
                        particleSize * 1.5f,
                        ImGui::ColorConvertFloat4ToU32(ImVec4(particleColor.x, particleColor.y, particleColor.z, particleColor.w * 0.5f))
                    );
                }
            }
        }

        if (m_state.connecting && m_state.connectingNodeId != -1 && m_state.connectingPinId != -1) {
            drawDragConnection(drawList, canvasPos);
        }
    }
}