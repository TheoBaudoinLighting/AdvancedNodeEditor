#include "../Core/NodeEditor.h"
#include "../Core/Style/InteractionMode.h"
#include <algorithm>
#include <cmath>

namespace NodeEditorCore {

int NodeEditor::addReroute(int connectionId, const Vec2& position, int insertIndex) {
    int rerouteId = m_nextRerouteId++;

    std::vector<Reroute> connectionReroutes = getReroutesForConnection(connectionId);

    if (insertIndex == -1) {
        insertIndex = connectionReroutes.size();
    }

    for (auto& reroute : m_reroutes) {
        if (reroute.connectionId == connectionId && reroute.index >= insertIndex) {
            reroute.index++;
        }
    }

    Reroute newReroute(rerouteId, connectionId, position, insertIndex);
    m_reroutes.push_back(newReroute);

    printf("Reroute %d created for connection %d at (%.1f, %.1f) index %d\n",
           rerouteId, connectionId, position.x, position.y, insertIndex);

    return rerouteId;
}

void NodeEditor::removeReroute(int rerouteId) {
    auto it = std::find_if(m_reroutes.begin(), m_reroutes.end(),
        [rerouteId](const Reroute& r) { return r.id == rerouteId; });

    if (it != m_reroutes.end()) {
        int connectionId = it->connectionId;
        int removedIndex = it->index;

        m_reroutes.erase(it);

        for (auto& reroute : m_reroutes) {
            if (reroute.connectionId == connectionId && reroute.index > removedIndex) {
                reroute.index--;
            }
        }

        printf("Reroute %d removed\n", rerouteId);
    }
}

void NodeEditor::removeAllReroutesFromConnection(int connectionId) {
    size_t count = m_reroutes.size();
    m_reroutes.erase(
        std::remove_if(m_reroutes.begin(), m_reroutes.end(),
            [connectionId](const Reroute& r) { return r.connectionId == connectionId; }),
        m_reroutes.end()
    );

    printf("Removed %zu reroutes from connection %d\n", count - m_reroutes.size(), connectionId);
}

std::vector<Reroute> NodeEditor::getReroutesForConnection(int connectionId) const {
    std::vector<Reroute> result;

    for (const auto& reroute : m_reroutes) {
        if (reroute.connectionId == connectionId) {
            result.push_back(reroute);
        }
    }

    std::sort(result.begin(), result.end(),
        [](const Reroute& a, const Reroute& b) { return a.index < b.index; });

    return result;
}

Reroute* NodeEditor::getReroute(int rerouteId) {
    auto it = std::find_if(m_reroutes.begin(), m_reroutes.end(),
        [rerouteId](const Reroute& r) { return r.id == rerouteId; });
    return (it != m_reroutes.end()) ? &(*it) : nullptr;
}

const Reroute* NodeEditor::getReroute(int rerouteId) const {
    auto it = std::find_if(m_reroutes.begin(), m_reroutes.end(),
        [rerouteId](const Reroute& r) { return r.id == rerouteId; });
    return (it != m_reroutes.end()) ? &(*it) : nullptr;
}

void NodeEditor::drawReroutes(ImDrawList* drawList, const ImVec2& canvasPos) {
    for (const auto& reroute : m_reroutes) {
        const Connection* connection = getConnection(reroute.connectionId);
        if (!connection) continue;

        const Node* startNode = getNode(connection->startNodeId);
        const Node* endNode = getNode(connection->endNodeId);
        if (!startNode || !endNode) continue;

        if (!isNodeInCurrentSubgraph(*startNode) || !isNodeInCurrentSubgraph(*endNode)) continue;

        drawSingleReroute(drawList, reroute, canvasPos);
    }

    if (m_debugMode) {
        drawRerouteDebugInfo(drawList, canvasPos);
    }
}

void NodeEditor::drawSingleReroute(ImDrawList* drawList, const Reroute& reroute, const ImVec2& canvasPos) {
    Vec2 screenPos = canvasToScreen(reroute.position);
    ImVec2 center = screenPos.toImVec2();

    float scale = 1.0f;
    if (reroute.selected) {
        scale = m_rerouteStyle.selectedScale;
    } else if (reroute.hoveredInner || reroute.hoveredOuter) {
        scale = m_rerouteStyle.hoverScale;
    }

    float outerRadius = m_rerouteStyle.outerRadius * m_state.viewScale * scale;
    float innerRadius = m_rerouteStyle.innerRadius * m_state.viewScale * scale;

    ImU32 outerColor = m_rerouteStyle.outerColor;
    ImU32 innerColor = m_rerouteStyle.innerColor;

    if (reroute.selected) {
        outerColor = m_rerouteStyle.selectedColor;
        innerColor = m_rerouteStyle.selectedColor;
    } else if (reroute.hoveredOuter) {
        outerColor = m_rerouteStyle.hoveredColor;
    } else if (reroute.hoveredInner) {
        innerColor = m_rerouteStyle.hoveredColor;
    }

    drawList->AddCircleFilled(center, outerRadius, outerColor);
    drawList->AddCircle(center, outerRadius, m_rerouteStyle.borderColor, 0, m_rerouteStyle.borderWidth * m_state.viewScale);

    drawList->AddCircleFilled(center, innerRadius, innerColor);
    drawList->AddCircle(center, innerRadius, m_rerouteStyle.borderColor, 0, m_rerouteStyle.borderWidth * m_state.viewScale);
}

void NodeEditor::drawRerouteDebugInfo(ImDrawList* drawList, const ImVec2& canvasPos) {
    for (const auto& reroute : m_reroutes) {
        Vec2 screenPos = canvasToScreen(reroute.position);
        ImVec2 center = screenPos.toImVec2();

        char debugText[64];
        sprintf(debugText, "R%d[%d]", reroute.id, reroute.index);
        ImVec2 textPos = ImVec2(center.x + 15, center.y - 8);
        drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), debugText);

        char stateText[32];
        sprintf(stateText, "%s%s%s",
                reroute.selected ? "S" : "",
                reroute.hoveredInner ? "I" : "",
                reroute.hoveredOuter ? "O" : "");
        ImVec2 statePos = ImVec2(center.x + 15, center.y + 8);
        drawList->AddText(statePos, IM_COL32(200, 200, 200, 255), stateText);
    }

    char countsText[128];
    sprintf(countsText, "Total reroutes: %zu", m_reroutes.size());
    drawList->AddText(ImVec2(10, 350), IM_COL32(255, 255, 255, 255), countsText);
}

void NodeEditor::updateRerouteHover(const ImVec2& mousePos, const ImVec2& canvasPos) {
    m_hoveredRerouteId = -1;
    m_rerouteHitZone = RerouteHitZone::None;

    for (auto& reroute : m_reroutes) {
        reroute.hoveredInner = false;
        reroute.hoveredOuter = false;

        RerouteHitZone hitZone = getRerouteHitZone(reroute, mousePos, canvasPos);

        if (hitZone != RerouteHitZone::None) {
            m_hoveredRerouteId = reroute.id;
            m_rerouteHitZone = hitZone;

            if (hitZone == RerouteHitZone::Inner) {
                reroute.hoveredInner = true;
            } else if (hitZone == RerouteHitZone::Outer) {
                reroute.hoveredOuter = true;
            }
            break;
        }
    }
}

void NodeEditor::processRerouteDrag(const ImVec2& mousePos) {
    if (m_activeRerouteId == -1) return;

    Reroute* reroute = getReroute(m_activeRerouteId);
    if (!reroute) return;

    Vec2 newCanvasPos = screenToCanvas(Vec2(mousePos.x, mousePos.y));
    reroute->position = newCanvasPos;
}

RerouteHitZone NodeEditor::getRerouteHitZone(const Reroute& reroute, const ImVec2& mousePos, const ImVec2& canvasPos) const {
    Vec2 rerouteScreenPos = canvasToScreen(reroute.position);

    float dx = mousePos.x - rerouteScreenPos.x;
    float dy = mousePos.y - rerouteScreenPos.y;
    float distance = sqrt(dx * dx + dy * dy);

    float outerRadius = m_rerouteStyle.outerRadius * m_state.viewScale;
    float innerRadius = m_rerouteStyle.innerRadius * m_state.viewScale;

    if (distance <= innerRadius) {
        return RerouteHitZone::Inner;
    } else if (distance <= outerRadius) {
        return RerouteHitZone::Outer;
    }

    return RerouteHitZone::None;
}

int NodeEditor::findRerouteAtPosition(const ImVec2& mousePos, const ImVec2& canvasPos, RerouteHitZone& hitZone) const {
    for (const auto& reroute : m_reroutes) {
        hitZone = getRerouteHitZone(reroute, mousePos, canvasPos);
        if (hitZone != RerouteHitZone::None) {
            return reroute.id;
        }
    }
    hitZone = RerouteHitZone::None;
    return -1;
}

float NodeEditor::getDistanceToConnection(const Connection& connection, const ImVec2& mousePos, const ImVec2& canvasPos, int& insertIndex) const {
    const Node* startNode = getNode(connection.startNodeId);
    const Node* endNode = getNode(connection.endNodeId);

    if (!startNode || !endNode) {
        return FLT_MAX;
    }

    const Pin* startPin = startNode->findPin(connection.startPinId);
    const Pin* endPin = endNode->findPin(connection.endPinId);

    if (!startPin || !endPin) {
        return FLT_MAX;
    }

    Pin apiStartPin;
    apiStartPin.id = startPin->id;
    apiStartPin.isInput = startPin->isInput;
    apiStartPin.type = static_cast<PinType>(startPin->type);

    Pin apiEndPin;
    apiEndPin.id = endPin->id;
    apiEndPin.isInput = endPin->isInput;
    apiEndPin.type = static_cast<PinType>(endPin->type);

    ImVec2 p1 = getPinPos(*startNode, apiStartPin, canvasPos);
    ImVec2 p2 = getPinPos(*endNode, apiEndPin, canvasPos);

    std::vector<Reroute> reroutes = getReroutesForConnection(connection.id);
    std::vector<ImVec2> pathPoints;

    pathPoints.push_back(p1);
    for (const auto& reroute : reroutes) {
        Vec2 rerouteScreenPos = canvasToScreen(reroute.position);
        pathPoints.push_back(rerouteScreenPos.toImVec2());
    }
    pathPoints.push_back(p2);

    float minDistance = FLT_MAX;
    insertIndex = 0;

    for (size_t i = 0; i < pathPoints.size() - 1; i++) {
        ImVec2 segStart = pathPoints[i];
        ImVec2 segEnd = pathPoints[i + 1];

        float dx = segEnd.x - segStart.x;
        float dy = segEnd.y - segStart.y;
        float length2 = dx * dx + dy * dy;

        if (length2 > 0.0001f) {
            float t = ((mousePos.x - segStart.x) * dx + (mousePos.y - segStart.y) * dy) / length2;
            t = std::max(0.0f, std::min(1.0f, t));

            ImVec2 closest = ImVec2(segStart.x + t * dx, segStart.y + t * dy);

            dx = mousePos.x - closest.x;
            dy = mousePos.y - closest.y;
            float distance = sqrt(dx * dx + dy * dy);

            if (distance < minDistance) {
                minDistance = distance;
                insertIndex = static_cast<int>(i);
            }
        }
    }

    return minDistance;
}

void NodeEditor::selectReroute(int rerouteId, bool append) {
    if (!append) {
        deselectAllReroutes();
    }

    Reroute* reroute = getReroute(rerouteId);
    if (reroute) {
        reroute->selected = true;
        printf("Selected reroute %d\n", rerouteId);
    }
}

void NodeEditor::deselectReroute(int rerouteId) {
    Reroute* reroute = getReroute(rerouteId);
    if (reroute) {
        reroute->selected = false;
        printf("Deselected reroute %d\n", rerouteId);
    }
}

void NodeEditor::deselectAllReroutes() {
    for (auto& reroute : m_reroutes) {
        reroute.selected = false;
    }
}

std::vector<int> NodeEditor::getSelectedReroutes() const {
    std::vector<int> selected;
    for (const auto& reroute : m_reroutes) {
        if (reroute.selected) {
            selected.push_back(reroute.id);
        }
    }
    return selected;
}

void NodeEditor::setRerouteStyle(const RerouteStyle& style) {
    m_rerouteStyle = style;
}

const RerouteStyle& NodeEditor::getRerouteStyle() const {
    return m_rerouteStyle;
}

void NodeEditor::startRerouteConnection(int rerouteId, const ImVec2& mousePos) {
    Reroute* reroute = getReroute(rerouteId);
    if (!reroute) return;

    const Connection* connection = getConnection(reroute->connectionId);
    if (!connection) return;

    m_state.interactionMode = InteractionMode::DragConnection;
    m_state.connecting = true;
    m_connectingFromReroute = true;
    m_connectingRerouteId = rerouteId;
    m_state.dragStart = Vec2(mousePos.x, mousePos.y);

    printf("Started connection from reroute %d\n", rerouteId);

    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
}

std::vector<ImVec2> NodeEditor::getConnectionPathWithReroutes(const Connection& connection, const ImVec2& p1, const ImVec2& p2) const {
    std::vector<ImVec2> pathPoints;
    std::vector<Reroute> reroutes = getReroutesForConnection(connection.id);

    pathPoints.push_back(p1);

    for (const auto& reroute : reroutes) {
        Vec2 rerouteScreenPos = canvasToScreen(reroute.position);
        pathPoints.push_back(rerouteScreenPos.toImVec2());
    }

    pathPoints.push_back(p2);

    return pathPoints;
}

std::vector<ImVec2> NodeEditor::getConnectionPathWithReroutesForDetection(const Connection& connection, const ImVec2& canvasPos) const {
    const Node* startNode = getNode(connection.startNodeId);
    const Node* endNode = getNode(connection.endNodeId);

    if (!startNode || !endNode) return {};

    const Pin* startPin = startNode->findPin(connection.startPinId);
    const Pin* endPin = endNode->findPin(connection.endPinId);

    if (!startPin || !endPin) return {};

    Pin apiStartPin;
    apiStartPin.id = startPin->id;
    apiStartPin.isInput = startPin->isInput;
    apiStartPin.type = static_cast<PinType>(startPin->type);

    Pin apiEndPin;
    apiEndPin.id = endPin->id;
    apiEndPin.isInput = endPin->isInput;
    apiEndPin.type = static_cast<PinType>(endPin->type);

    ImVec2 p1 = getPinPos(*startNode, apiStartPin, canvasPos);
    ImVec2 p2 = getPinPos(*endNode, apiEndPin, canvasPos);

    return getConnectionPathWithReroutes(connection, p1, p2);
}

void NodeEditor::drawConnectionWithReroutes(ImDrawList* drawList, const Connection& connection,
                                          const ImVec2& p1, const ImVec2& p2,
                                          const Color& startCol, const Color& endCol) {
    std::vector<ImVec2> pathPoints = getConnectionPathWithReroutes(connection, p1, p2);

    if (pathPoints.size() < 2) return;

    bool isSelected = connection.selected;
    bool isHovered = m_state.hoveredConnectionId == connection.id;

    for (size_t i = 0; i < pathPoints.size() - 1; i++) {
        ImVec2 segmentStart = pathPoints[i];
        ImVec2 segmentEnd = pathPoints[i + 1];

        Color segmentStartCol = startCol;
        Color segmentEndCol = endCol;

        if (pathPoints.size() > 2) {
            float t = (float)i / (pathPoints.size() - 1);
            segmentStartCol = Color(
                startCol.r * (1.0f - t) + endCol.r * t,
                startCol.g * (1.0f - t) + endCol.g * t,
                startCol.b * (1.0f - t) + endCol.b * t,
                startCol.a * (1.0f - t) + endCol.a * t
            );

            t = (float)(i + 1) / (pathPoints.size() - 1);
            segmentEndCol = Color(
                startCol.r * (1.0f - t) + endCol.r * t,
                startCol.g * (1.0f - t) + endCol.g * t,
                startCol.b * (1.0f - t) + endCol.b * t,
                startCol.a * (1.0f - t) + endCol.a * t
            );
        }

        m_connectionStyleManager.drawConnection(
            drawList, segmentStart, segmentEnd,
            false, true,
            isSelected, isHovered,
            segmentStartCol, segmentEndCol, m_state.viewScale
        );
    }
}

void NodeEditor::updateConnectionAnimationWithReroutes(const Connection& connection, const ImVec2& p1, const ImVec2& p2,
                                                      const ConnectionAnimationState& animState,
                                                      std::vector<ImVec2>& particlePoints) const {
    std::vector<ImVec2> pathPoints = getConnectionPathWithReroutes(connection, p1, p2);

    if (pathPoints.size() < 2) return;

    float totalLength = 0.0f;
    std::vector<float> segmentLengths;

    for (size_t i = 0; i < pathPoints.size() - 1; i++) {
        float dx = pathPoints[i + 1].x - pathPoints[i].x;
        float dy = pathPoints[i + 1].y - pathPoints[i].y;
        float length = sqrt(dx * dx + dy * dy);
        segmentLengths.push_back(length);
        totalLength += length;
    }

    const int particleCount = 5;
    particlePoints.clear();

    for (int i = 0; i < particleCount; i++) {
        float t = (animState.flowAnimation + (float)i / particleCount);
        t = t - std::floor(t);

        float targetDistance = t * totalLength;
        float currentDistance = 0.0f;

        for (size_t j = 0; j < segmentLengths.size(); j++) {
            if (currentDistance + segmentLengths[j] >= targetDistance) {
                float segmentT = (targetDistance - currentDistance) / segmentLengths[j];

                ImVec2 particlePos = ImVec2(
                    pathPoints[j].x + (pathPoints[j + 1].x - pathPoints[j].x) * segmentT,
                    pathPoints[j].y + (pathPoints[j + 1].y - pathPoints[j].y) * segmentT
                );

                particlePoints.push_back(particlePos);
                break;
            }
            currentDistance += segmentLengths[j];
        }
    }
}

}