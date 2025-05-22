#include "../Core/NodeEditor.h"
#include "../Core/Style/InteractionMode.h"
#include <algorithm>
#include <cmath>

namespace NodeEditorCore {

std::pair<ImVec2, ImVec2> NodeEditor::calculateBezierControlPoints(
    const ImVec2& segStart, const ImVec2& segEnd,
    bool segmentStartInput, bool segmentEndInput,
    float tension) const {

    const float distance = std::sqrt(std::pow(segEnd.x - segStart.x, 2) + std::pow(segEnd.y - segStart.y, 2));
    const float cpDistance = distance * tension;

    ImVec2 cp1, cp2;

    if (segmentStartInput) {
        cp1 = ImVec2(segStart.x, segStart.y - cpDistance);
    } else {
        cp1 = ImVec2(segStart.x, segStart.y + cpDistance);
    }

    if (segmentEndInput) {
        cp2 = ImVec2(segEnd.x, segEnd.y - cpDistance);
    } else {
        cp2 = ImVec2(segEnd.x, segEnd.y + cpDistance);
    }

    return {cp1, cp2};
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

    std::vector<ImVec2> pathPoints = getConnectionPathWithReroutesForDetection(connection, canvasPos);

    ConnectionStyleManager::ConnectionStyle style = m_connectionStyleManager.getDefaultStyle();
    float minDistance = FLT_MAX;
    insertIndex = 0;

    const float tension = m_connectionStyleManager.getConfig().curveTension;

    for (size_t i = 0; i < pathPoints.size() - 1; i++) {
        ImVec2 segStart = pathPoints[i];
        ImVec2 segEnd = pathPoints[i + 1];
        float segmentDistance = FLT_MAX;

        bool segmentStartInput, segmentEndInput;

        if (i == 0) {
            segmentStartInput = startPin->isInput;
        } else {
            segmentStartInput = false;
        }

        if (i == pathPoints.size() - 2) {
            segmentEndInput = endPin->isInput;
        } else {
            segmentEndInput = true;
        }

        switch (style) {
            case ConnectionStyleManager::ConnectionStyle::Bezier: {
                auto [cp1, cp2] = calculateBezierControlPoints(segStart, segEnd, segmentStartInput, segmentEndInput, tension);
                segmentDistance = getDistanceToBezierCubic(mousePos, segStart, cp1, cp2, segEnd);
                break;
            }

            case ConnectionStyleManager::ConnectionStyle::AngleLine: {
                ImVec2 middle = ImVec2(segEnd.x, segStart.y);
                float dist1 = getDistanceToLineSegment(mousePos, segStart, middle);
                float dist2 = getDistanceToLineSegment(mousePos, middle, segEnd);
                segmentDistance = std::min(dist1, dist2);
                break;
            }

            case ConnectionStyleManager::ConnectionStyle::MetroLine: {
                float dx = segEnd.x - segStart.x;
                float dy = segEnd.y - segStart.y;

                ImVec2 middle1, middle2;
                if (std::abs(dx) > std::abs(dy)) {
                    middle1 = ImVec2(segStart.x + dx * 0.5f, segStart.y);
                    middle2 = ImVec2(segStart.x + dx * 0.5f, segEnd.y);
                } else {
                    middle1 = ImVec2(segStart.x, segStart.y + dy * 0.5f);
                    middle2 = ImVec2(segEnd.x, segStart.y + dy * 0.5f);
                }

                float dist1 = getDistanceToLineSegment(mousePos, segStart, middle1);
                float dist2 = getDistanceToLineSegment(mousePos, middle1, middle2);
                float dist3 = getDistanceToLineSegment(mousePos, middle2, segEnd);
                segmentDistance = std::min({dist1, dist2, dist3});
                break;
            }

            default:
                segmentDistance = getDistanceToLineSegment(mousePos, segStart, segEnd);
                break;
        }

        if (segmentDistance < minDistance) {
            minDistance = segmentDistance;
            insertIndex = static_cast<int>(i);
        }
    }

    return minDistance;
}

float NodeEditor::getDistanceToBezierCubic(const ImVec2& point, const ImVec2& p0, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3) const {
    float minDistance = FLT_MAX;

    const int samples = 50;

    ImVec2 prev = p0;
    for (int i = 1; i <= samples; i++) {
        float t = static_cast<float>(i) / samples;

        float u = 1.0f - t;
        float w1 = u * u * u;
        float w2 = 3 * u * u * t;
        float w3 = 3 * u * t * t;
        float w4 = t * t * t;

        ImVec2 current(
            w1 * p0.x + w2 * p1.x + w3 * p2.x + w4 * p3.x,
            w1 * p0.y + w2 * p1.y + w3 * p2.y + w4 * p3.y
        );

        float dist = getDistanceToLineSegment(point, prev, current);
        minDistance = std::min(minDistance, dist);

        prev = current;
    }

    return minDistance;
}

float NodeEditor::getDistanceToLineSegment(const ImVec2& point, const ImVec2& lineStart, const ImVec2& lineEnd) const {
    float dx = lineEnd.x - lineStart.x;
    float dy = lineEnd.y - lineStart.y;
    float length2 = dx * dx + dy * dy;

    if (length2 <= 0.0001f) {
        dx = point.x - lineStart.x;
        dy = point.y - lineStart.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    float t = ((point.x - lineStart.x) * dx + (point.y - lineStart.y) * dy) / length2;
    t = std::max(0.0f, std::min(1.0f, t)); // Clamper sur le segment

    float projX = lineStart.x + t * dx;
    float projY = lineStart.y + t * dy;

    dx = point.x - projX;
    dy = point.y - projY;
    return std::sqrt(dx * dx + dy * dy);
}

void NodeEditor::processRerouteInteraction(const ImVec2& mousePos) {
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        RerouteHitZone hitZone;
        int rerouteId = findRerouteAtPosition(mousePos, ImVec2(0, 0), hitZone);

        if (rerouteId != -1) {
            Reroute* reroute = getReroute(rerouteId);
            if (!reroute) return;

            if (hitZone == RerouteHitZone::Inner) {
                if (ImGui::GetIO().KeyCtrl) {
                    selectReroute(rerouteId, true);
                } else {
                    deselectAllReroutes();
                    selectReroute(rerouteId);
                }

                m_activeRerouteId = rerouteId;
                m_state.interactionMode = InteractionMode::DragReroute;
                m_state.dragStart = Vec2(mousePos.x, mousePos.y);
            } else if (hitZone == RerouteHitZone::Outer) {
                startRerouteConnection(rerouteId, mousePos);
            }
            return;
        }

        for (const auto& connection : m_state.connections) {
            if (!isNodeInCurrentSubgraph(*getNode(connection.startNodeId)) ||
                !isNodeInCurrentSubgraph(*getNode(connection.endNodeId))) continue;

            int insertIndex;
            float distance = getDistanceToConnection(connection, mousePos, ImVec2(0, 0), insertIndex);

            const float clickThreshold = 12.0f * m_state.viewScale;

            if (distance <= clickThreshold) {
                Vec2 canvasPos = screenToCanvas(Vec2(mousePos.x, mousePos.y));
                int newRerouteId = addReroute(connection.id, canvasPos, insertIndex);

                selectReroute(newRerouteId);
                m_activeRerouteId = newRerouteId;
                m_state.interactionMode = InteractionMode::DragReroute;
                m_state.dragStart = Vec2(mousePos.x, mousePos.y);

                return;
            }
        }
    }

    if (m_state.interactionMode == InteractionMode::DragReroute && m_activeRerouteId != -1) {
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            processRerouteDrag(mousePos);
        }

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            m_activeRerouteId = -1;
            m_state.interactionMode = InteractionMode::None;
            ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
        }
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Delete) || ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
        std::vector<int> selectedReroutes = getSelectedReroutes();
        for (int rerouteId : selectedReroutes) {
            removeReroute(rerouteId);
        }
    }
}

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
    }
}

void NodeEditor::removeAllReroutesFromConnection(int connectionId) {
    size_t count = m_reroutes.size();
    m_reroutes.erase(
        std::remove_if(m_reroutes.begin(), m_reroutes.end(),
            [connectionId](const Reroute& r) { return r.connectionId == connectionId; }),
        m_reroutes.end()
    );
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
    drawList->AddCircle(center, outerRadius, m_rerouteStyle.borderColor, 32, m_rerouteStyle.borderWidth * m_state.viewScale);

    drawList->AddCircleFilled(center, innerRadius, innerColor);
    drawList->AddCircle(center, innerRadius, m_rerouteStyle.borderColor, 32, m_rerouteStyle.borderWidth * m_state.viewScale);
}

void NodeEditor::drawRerouteDebugInfo(ImDrawList* drawList, const ImVec2& canvasPos) {
    for (const auto& reroute : m_reroutes) {
        Vec2 screenPos = canvasToScreen(reroute.position);
        ImVec2 center = screenPos.toImVec2();

        char debugText[64];
        sprintf(debugText, "R%d[%d]", reroute.id, reroute.index);
        ImVec2 textPos = ImVec2(center.x + 15, center.y - 8);
        drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), debugText);
    }
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

void NodeEditor::selectReroute(int rerouteId, bool append) {
    if (!append) {
        deselectAllReroutes();
    }

    Reroute* reroute = getReroute(rerouteId);
    if (reroute) {
        reroute->selected = true;
    }
}

void NodeEditor::deselectReroute(int rerouteId) {
    Reroute* reroute = getReroute(rerouteId);
    if (reroute) {
        reroute->selected = false;
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

    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
}

std::vector<ImVec2> NodeEditor::getConnectionPathWithReroutesForDetection(const Connection& connection, const ImVec2& canvasPos) const {
    const Node* startNode = getNode(connection.startNodeId);
    const Node* endNode = getNode(connection.endNodeId);

    if (!startNode || !endNode) {
        return {};
    }

    const Pin* startPin = startNode->findPin(connection.startPinId);
    const Pin* endPin = endNode->findPin(connection.endPinId);

    if (!startPin || !endPin) {
        return {};
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

}