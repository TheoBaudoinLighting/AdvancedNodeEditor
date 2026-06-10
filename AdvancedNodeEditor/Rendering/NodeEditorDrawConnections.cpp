#include "../Core/NodeEditor.h"
#include <algorithm>
#include <cmath>

namespace NodeEditorCore {
    void NodeEditor::drawConnections(ImDrawList *drawList, const ImVec2 &canvasPos) {
        if (!drawList) return;

        std::vector<Connection> visibleConnections = getVisibleConnections();

        for (const auto &connection: visibleConnections) {
            drawSingleConnection(drawList, connection, canvasPos);
        }

        if (m_state.connecting && m_state.connectingNodeId != -1 && m_state.connectingPinId != -1) {
            drawDragConnection(drawList, canvasPos);
        }
    }

    std::vector<Connection> NodeEditor::getVisibleConnections() const {
        std::vector<Connection> visibleConnections;
        const int currentSubgraphId = m_state.currentSubgraphId;

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

        return visibleConnections;
    }

    void NodeEditor::drawSingleConnection(ImDrawList *drawList, const Connection &connection, const ImVec2 &canvasPos) {
        const Node *startNode = getNode(connection.startNodeId);
        const Node *endNode = getNode(connection.endNodeId);

        if (!startNode || !endNode) return;

        const Pin *startPin = startNode->findPin(connection.startPinId);
        const Pin *endPin = endNode->findPin(connection.endPinId);

        if (!startPin || !endPin) return;

        Pin apiStartPin;
        apiStartPin.id = startPin->id;
        apiStartPin.name = startPin->name;
        apiStartPin.isInput = startPin->isInput;
        apiStartPin.type = static_cast<PinType>(startPin->type);
        apiStartPin.shape = static_cast<PinShape>(startPin->shape);

        Pin apiEndPin;
        apiEndPin.id = endPin->id;
        apiEndPin.name = endPin->name;
        apiEndPin.isInput = endPin->isInput;
        apiEndPin.type = static_cast<PinType>(endPin->type);
        apiEndPin.shape = static_cast<PinShape>(endPin->shape);

        ImVec2 p1 = getPinPos(*startNode, apiStartPin, canvasPos);
        ImVec2 p2 = getPinPos(*endNode, apiEndPin, canvasPos);

        Color startCol = getPinConnectionColor(*startPin);
        Color endCol = getPinConnectionColor(*endPin);

        std::vector<Reroute> reroutes = getReroutesForConnection(connection.id);

        if (reroutes.empty()) {
            drawConnectionLine(drawList, p1, p2, connection, apiStartPin, apiEndPin, startCol, endCol);
        } else {
            drawConnectionWithReroutes(drawList, connection, p1, p2, startCol, endCol);
        }

        drawConnectionAnimation(drawList, p1, p2, connection, apiStartPin, apiEndPin, startCol, endCol);
    }

    Color NodeEditor::getPinConnectionColor(const Pin &pin) const {
        std::string pinType = pinTypeToString(pin.type);

        const internal::PinColors &pinColors = m_state.style.pinColors.count(pinType)
                                                   ? m_state.style.pinColors.at(pinType)
                                                   : m_state.style.pinColors.at("Default");

        return Color(
            pinColors.connected.r,
            pinColors.connected.g,
            pinColors.connected.b,
            pinColors.connected.a * 0.8f
        );
    }

    void NodeEditor::drawConnectionLine(ImDrawList *drawList, const ImVec2 &p1, const ImVec2 &p2,
                                        const Connection &connection, const Pin &startPin, const Pin &endPin,
                                        const Color &startCol, const Color &endCol) {
        m_connectionStyleManager.drawConnection(
            drawList, p1, p2,
            startPin.isInput, endPin.isInput,
            connection.selected, m_state.hoveredConnectionId == connection.id,
            startCol, endCol, m_state.viewScale
        );
    }

    void NodeEditor::drawConnectionAnimation(ImDrawList *drawList, const ImVec2 &p1, const ImVec2 &p2,
                                             const Connection &connection, const Pin &startPin, const Pin &endPin,
                                             const Color &startCol, const Color &endCol) {
        auto &connAnimState = m_animationManager.getConnectionAnimationState(connection.id);

        if (connAnimState.flowSpeed <= 0.0f) return;

        std::vector<ImVec2> pathPoints;
        std::vector<Reroute> reroutes = getReroutesForConnection(connection.id);

        if (reroutes.empty()) {
            pathPoints = calculateAnimationPath(p1, p2, startPin, endPin, connAnimState);
        } else {
            updateConnectionAnimationWithReroutes(connection, p1, p2, connAnimState, pathPoints);
        }

        if (pathPoints.empty()) return;

        renderAnimationParticles(drawList, pathPoints, startCol, endCol);
    }

    std::vector<ImVec2> NodeEditor::calculateAnimationPath(const ImVec2 &p1, const ImVec2 &p2,
                                                           const Pin &startPin, const Pin &endPin,
                                                           const ConnectionAnimationState &animState) const {
        std::vector<ImVec2> pathPoints;

        ConnectionStyleManager::ConnectionStyle style = m_connectionStyleManager.getDefaultStyle();
        const int particleCount = 5;

        switch (style) {
            case ConnectionStyleManager::ConnectionStyle::Bezier:
                pathPoints = calculateBezierAnimationPath(p1, p2, startPin, endPin, animState, particleCount);
                break;

            case ConnectionStyleManager::ConnectionStyle::StraightLine:
                pathPoints = calculateStraightAnimationPath(p1, p2, animState, particleCount);
                break;

            case ConnectionStyleManager::ConnectionStyle::AngleLine:
                pathPoints = calculateAngleAnimationPath(p1, p2, animState, particleCount);
                break;

            case ConnectionStyleManager::ConnectionStyle::MetroLine:
                pathPoints = calculateMetroAnimationPath(p1, p2, animState, particleCount);
                break;

            default:
                pathPoints = calculateStraightAnimationPath(p1, p2, animState, particleCount);
                break;
        }

        return pathPoints;
    }

    std::vector<ImVec2> NodeEditor::calculateBezierAnimationPath(const ImVec2 &p1, const ImVec2 &p2,
                                                                 const Pin &startPin, const Pin &endPin,
                                                                 const ConnectionAnimationState &animState,
                                                                 int particleCount) const {
        std::vector<ImVec2> pathPoints;

        const float distance = Math::distance(
            Vec2(p1.x, p1.y),
            Vec2(p2.x, p2.y));
        const float tension = m_connectionStyleManager.getConfig().curveTension;
        const float cpDistance = distance * tension;

        ImVec2 cp1, cp2;
        if (startPin.isInput) {
            cp1 = ImVec2(p1.x, p1.y - cpDistance);
        } else {
            cp1 = ImVec2(p1.x, p1.y + cpDistance);
        }

        if (endPin.isInput) {
            cp2 = ImVec2(p2.x, p2.y - cpDistance);
        } else {
            cp2 = ImVec2(p2.x, p2.y + cpDistance);
        }

        for (int i = 0; i < particleCount; i++) {
            float t = (animState.flowAnimation + (float) i / particleCount) -
                      std::floor(animState.flowAnimation + (float) i / particleCount);

            float u = 1.0f - t;
            float w1 = u * u * u;
            float w2 = 3 * u * u * t;
            float w3 = 3 * u * t * t;
            float w4 = t * t * t;

            ImVec2 particlePos(
                w1 * p1.x + w2 * cp1.x + w3 * cp2.x + w4 * p2.x,
                w1 * p1.y + w2 * cp1.y + w3 * cp2.y + w4 * p2.y
            );

            pathPoints.push_back(particlePos);
        }

        return pathPoints;
    }

    std::vector<ImVec2> NodeEditor::calculateStraightAnimationPath(const ImVec2 &p1, const ImVec2 &p2,
                                                                   const ConnectionAnimationState &animState,
                                                                   int particleCount) const {
        std::vector<ImVec2> pathPoints;

        for (int i = 0; i < particleCount; i++) {
            float t = (animState.flowAnimation + (float) i / particleCount) -
                      std::floor(animState.flowAnimation + (float) i / particleCount);

            ImVec2 particlePos = ImVec2(
                p1.x + (p2.x - p1.x) * t,
                p1.y + (p2.y - p1.y) * t
            );
            pathPoints.push_back(particlePos);
        }

        return pathPoints;
    }

    std::vector<ImVec2> NodeEditor::calculateAngleAnimationPath(const ImVec2 &p1, const ImVec2 &p2,
                                                                const ConnectionAnimationState &animState,
                                                                int particleCount) const {
        std::vector<ImVec2> pathPoints;
        ImVec2 middle = ImVec2(p2.x, p1.y);

        for (int i = 0; i < particleCount; i++) {
            float t = (animState.flowAnimation + (float) i / particleCount) -
                      std::floor(animState.flowAnimation + (float) i / particleCount);

            ImVec2 particlePos;
            if (t < 0.5f) {
                float segmentT = t * 2.0f;
                particlePos = ImVec2(
                    p1.x + (middle.x - p1.x) * segmentT,
                    p1.y + (middle.y - p1.y) * segmentT
                );
            } else {
                float segmentT = (t - 0.5f) * 2.0f;
                particlePos = ImVec2(
                    middle.x + (p2.x - middle.x) * segmentT,
                    middle.y + (p2.y - middle.y) * segmentT
                );
            }
            pathPoints.push_back(particlePos);
        }

        return pathPoints;
    }

    std::vector<ImVec2> NodeEditor::calculateMetroAnimationPath(const ImVec2 &p1, const ImVec2 &p2,
                                                                const ConnectionAnimationState &animState,
                                                                int particleCount) const {
        std::vector<ImVec2> pathPoints;

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

        std::vector metroPoints = {p1, middle1, middle2, p2};

        for (int i = 0; i < particleCount; i++) {
            float t = (animState.flowAnimation + (float) i / particleCount) -
                      std::floor(animState.flowAnimation + (float) i / particleCount);

            float segmentLength = 1.0f / (metroPoints.size() - 1);
            int segment = std::min(static_cast<int>(t / segmentLength),
                                   static_cast<int>(metroPoints.size()) - 2);

            float segmentT = (t - segment * segmentLength) / segmentLength;

            ImVec2 particlePos = ImVec2(
                metroPoints[segment].x + (metroPoints[segment + 1].x - metroPoints[segment].x) * segmentT,
                metroPoints[segment].y + (metroPoints[segment + 1].y - metroPoints[segment].y) * segmentT
            );

            pathPoints.push_back(particlePos);
        }

        return pathPoints;
    }

    void NodeEditor::renderAnimationParticles(ImDrawList *drawList, const std::vector<ImVec2> &pathPoints,
                                              const Color &startCol, const Color &endCol) {
        if (pathPoints.empty()) return;

        ImVec4 startColVec4 = ImGui::ColorConvertU32ToFloat4(startCol.toImU32());
        ImVec4 endColVec4 = ImGui::ColorConvertU32ToFloat4(endCol.toImU32());

        for (size_t i = 0; i < pathPoints.size(); i++) {
            float t = static_cast<float>(i) / pathPoints.size();

            ImVec4 particleColor = ImVec4(
                startColVec4.x * (1.0f - t) + endColVec4.x * t,
                startColVec4.y * (1.0f - t) + endColVec4.y * t,
                startColVec4.z * (1.0f - t) + endColVec4.z * t,
                (startColVec4.w * (1.0f - t) + endColVec4.w * t) * 1.5f
            );

            float pulseScale = std::sin(t * Math::TWO_PI) * 0.3f + 1.0f;
            float particleSize = 3.5f * m_state.viewScale * pulseScale;

            drawList->AddCircleFilled(
                pathPoints[i],
                particleSize,
                ImGui::ColorConvertFloat4ToU32(particleColor)
            );

            drawList->AddCircle(
                pathPoints[i],
                particleSize * 1.5f,
                ImGui::ColorConvertFloat4ToU32(ImVec4(
                    particleColor.x,
                    particleColor.y,
                    particleColor.z,
                    particleColor.w * 0.5f
                ))
            );
        }
    }

    void NodeEditor::drawConnectionWithReroutes(ImDrawList *drawList, const Connection &connection,
                                                const ImVec2 &p1, const ImVec2 &p2,
                                                const Color &startCol, const Color &endCol) {
        std::vector<Reroute> reroutes = getReroutesForConnection(connection.id);

        if (reroutes.empty()) {
            const Node *startNode = getNode(connection.startNodeId);
            const Node *endNode = getNode(connection.endNodeId);
            if (!startNode || !endNode) return;

            const Pin *startPin = startNode->findPin(connection.startPinId);
            const Pin *endPin = endNode->findPin(connection.endPinId);
            if (!startPin || !endPin) return;

            Pin apiStartPin;
            apiStartPin.id = startPin->id;
            apiStartPin.isInput = startPin->isInput;
            apiStartPin.type = static_cast<PinType>(startPin->type);

            Pin apiEndPin;
            apiEndPin.id = endPin->id;
            apiEndPin.isInput = endPin->isInput;
            apiEndPin.type = static_cast<PinType>(endPin->type);

            m_connectionStyleManager.drawConnection(
                drawList, p1, p2,
                apiStartPin.isInput, apiEndPin.isInput,
                connection.selected, m_state.hoveredConnectionId == connection.id,
                startCol, endCol, m_state.viewScale
            );
            return;
        }

        const Node *startNode = getNode(connection.startNodeId);
        const Node *endNode = getNode(connection.endNodeId);
        if (!startNode || !endNode) return;

        const Pin *startPin = startNode->findPin(connection.startPinId);
        const Pin *endPin = endNode->findPin(connection.endPinId);
        if (!startPin || !endPin) return;

        Pin apiStartPin;
        apiStartPin.id = startPin->id;
        apiStartPin.isInput = startPin->isInput;
        apiStartPin.type = static_cast<PinType>(startPin->type);

        Pin apiEndPin;
        apiEndPin.id = endPin->id;
        apiEndPin.isInput = endPin->isInput;
        apiEndPin.type = static_cast<PinType>(endPin->type);

        std::vector<ImVec2> pathPoints;
        pathPoints.push_back(p1);

        for (const auto &reroute: reroutes) {
            Vec2 rerouteScreenPos = canvasToScreen(reroute.position);
            pathPoints.push_back(rerouteScreenPos.toImVec2());
        }

        pathPoints.push_back(p2);

        bool isSelected = connection.selected;
        bool isHovered = m_state.hoveredConnectionId == connection.id;

        for (size_t i = 0; i < pathPoints.size() - 1; i++) {
            ImVec2 segmentStart = pathPoints[i];
            ImVec2 segmentEnd = pathPoints[i + 1];

            bool segmentStartInput, segmentEndInput;

            if (i == 0) {
                segmentStartInput = apiStartPin.isInput;
            } else {
                segmentStartInput = false;
            }

            if (i == pathPoints.size() - 2) {
                segmentEndInput = apiEndPin.isInput;
            } else {
                segmentEndInput = true;
            }

            Color segmentStartCol = startCol;
            Color segmentEndCol = endCol;

            if (pathPoints.size() > 2) {
                float t = static_cast<float>(i) / (pathPoints.size() - 1);
                segmentStartCol = Color(
                    startCol.r * (1.0f - t) + endCol.r * t,
                    startCol.g * (1.0f - t) + endCol.g * t,
                    startCol.b * (1.0f - t) + endCol.b * t,
                    startCol.a * (1.0f - t) + endCol.a * t
                );

                t = static_cast<float>(i + 1) / (pathPoints.size() - 1);
                segmentEndCol = Color(
                    startCol.r * (1.0f - t) + endCol.r * t,
                    startCol.g * (1.0f - t) + endCol.g * t,
                    startCol.b * (1.0f - t) + endCol.b * t,
                    startCol.a * (1.0f - t) + endCol.a * t
                );
            }

            m_connectionStyleManager.drawConnection(
                drawList, segmentStart, segmentEnd,
                segmentStartInput, segmentEndInput,
                isSelected, isHovered,
                segmentStartCol, segmentEndCol, m_state.viewScale
            );
        }
    }

    void NodeEditor::updateConnectionAnimationWithReroutes(const Connection &connection, const ImVec2 &p1,
                                                           const ImVec2 &p2,
                                                           const ConnectionAnimationState &animState,
                                                           std::vector<ImVec2> &particlePoints) const {
        std::vector<ImVec2> pathPoints = getConnectionPathWithReroutes(connection, p1, p2);

        if (pathPoints.size() < 2) return;

        const Node *startNode = getNode(connection.startNodeId);
        const Node *endNode = getNode(connection.endNodeId);
        if (!startNode || !endNode) return;

        const Pin *startPin = startNode->findPin(connection.startPinId);
        const Pin *endPin = endNode->findPin(connection.endPinId);
        if (!startPin || !endPin) return;

        std::vector<ImVec2> sampledPath;
        auto appendPoint = [&sampledPath](const ImVec2 &point) {
            if (!sampledPath.empty()) {
                const ImVec2 &last = sampledPath.back();
                const float dx = point.x - last.x;
                const float dy = point.y - last.y;
                if ((dx * dx + dy * dy) <= 0.01f) {
                    return;
                }
            }
            sampledPath.push_back(point);
        };

        const auto style = m_connectionStyleManager.getDefaultStyle();
        const float tension = m_connectionStyleManager.getConfig().curveTension;
        constexpr int curveSamples = 16;

        for (size_t i = 0; i + 1 < pathPoints.size(); ++i) {
            const ImVec2 segmentStart = pathPoints[i];
            const ImVec2 segmentEnd = pathPoints[i + 1];
            const bool segmentStartInput = i == 0 ? startPin->isInput : false;
            const bool segmentEndInput = i + 2 == pathPoints.size() ? endPin->isInput : true;

            appendPoint(segmentStart);

            switch (style) {
                case ConnectionStyleManager::ConnectionStyle::Bezier: {
                    const auto [cp1, cp2] = calculateBezierControlPoints(
                        segmentStart, segmentEnd, segmentStartInput, segmentEndInput, tension);

                    for (int sample = 1; sample <= curveSamples; ++sample) {
                        const float t = static_cast<float>(sample) / static_cast<float>(curveSamples);
                        appendPoint(evaluateBezierCubic(segmentStart, cp1, cp2, segmentEnd, t));
                    }
                    break;
                }

                case ConnectionStyleManager::ConnectionStyle::AngleLine:
                    appendPoint(ImVec2(segmentEnd.x, segmentStart.y));
                    appendPoint(segmentEnd);
                    break;

                case ConnectionStyleManager::ConnectionStyle::MetroLine: {
                    const float dx = segmentEnd.x - segmentStart.x;
                    const float dy = segmentEnd.y - segmentStart.y;

                    if (std::abs(dx) > std::abs(dy)) {
                        appendPoint(ImVec2(segmentStart.x + dx * 0.5f, segmentStart.y));
                        appendPoint(ImVec2(segmentStart.x + dx * 0.5f, segmentEnd.y));
                    } else {
                        appendPoint(ImVec2(segmentStart.x, segmentStart.y + dy * 0.5f));
                        appendPoint(ImVec2(segmentEnd.x, segmentStart.y + dy * 0.5f));
                    }
                    appendPoint(segmentEnd);
                    break;
                }

                case ConnectionStyleManager::ConnectionStyle::StraightLine:
                default:
                    appendPoint(segmentEnd);
                    break;
            }
        }

        if (sampledPath.size() < 2) return;

        float totalLength = 0.0f;
        std::vector<float> segmentLengths;
        segmentLengths.reserve(sampledPath.size() - 1);

        for (size_t i = 0; i + 1 < sampledPath.size(); ++i) {
            const float dx = sampledPath[i + 1].x - sampledPath[i].x;
            const float dy = sampledPath[i + 1].y - sampledPath[i].y;
            const float length = Math::distance(
                Vec2(0, 0),
                Vec2(dx, dy));
            segmentLengths.push_back(length);
            totalLength += length;
        }

        if (totalLength <= 0.001f) return;

        const int particleCount = 5;
        particlePoints.clear();

        for (int i = 0; i < particleCount; i++) {
            float t = (animState.flowAnimation + (float) i / particleCount);
            t = t - std::floor(t);

            float targetDistance = t * totalLength;
            float currentDistance = 0.0f;

            for (size_t j = 0; j < segmentLengths.size(); j++) {
                if (segmentLengths[j] <= 0.001f) {
                    currentDistance += segmentLengths[j];
                    continue;
                }

                if (currentDistance + segmentLengths[j] >= targetDistance) {
                    float segmentT = (targetDistance - currentDistance) / segmentLengths[j];

                    ImVec2 particlePos = ImVec2(
                        sampledPath[j].x + (sampledPath[j + 1].x - sampledPath[j].x) * segmentT,
                        sampledPath[j].y + (sampledPath[j + 1].y - sampledPath[j].y) * segmentT
                    );

                    particlePoints.push_back(particlePos);
                    break;
                }
                currentDistance += segmentLengths[j];
            }

            if (particlePoints.size() <= static_cast<size_t>(i)) {
                particlePoints.push_back(sampledPath.back());
            }
        }
    }

    std::vector<ImVec2> NodeEditor::getConnectionPathWithReroutes(const Connection &connection, const ImVec2 &p1,
                                                                  const ImVec2 &p2) const {
        std::vector<ImVec2> pathPoints;
        std::vector<Reroute> reroutes = getReroutesForConnection(connection.id);

        pathPoints.push_back(p1);

        for (const auto &reroute: reroutes) {
            Vec2 rerouteScreenPos = canvasToScreen(reroute.position);
            pathPoints.push_back(rerouteScreenPos.toImVec2());
        }

        pathPoints.push_back(p2);

        return pathPoints;
    }
}
