#include "ConnectionStyleManager.h"
#include <algorithm>
#include <cmath>
#include "../../Editor/View/NodeBoundingBoxManager.h"

#include "imgui_internal.h"

namespace NodeEditorCore {
    ConnectionStyleManager::ConnectionStyleManager()
        : m_boundingBoxCheck(nullptr) {
    }

    ConnectionStyleManager::~ConnectionStyleManager() = default;

    void ConnectionStyleManager::setConfig(const ConnectionConfig &config) {
        m_config = config;
    }

    ConnectionStyleManager::ConnectionConfig &ConnectionStyleManager::getConfig() {
        return m_config;
    }

    const ConnectionStyleManager::ConnectionConfig &ConnectionStyleManager::getConfig() const {
        return m_config;
    }

    void ConnectionStyleManager::setDefaultStyle(ConnectionStyle style) {
        m_config.style = style;
    }

    ConnectionStyleManager::ConnectionStyle ConnectionStyleManager::getDefaultStyle() const {
        return m_config.style;
    }

    void ConnectionStyleManager::addCustomConnectionDrawer(
        const std::string &name,
        std::function<void(ImDrawList *, const ImVec2 &, const ImVec2 &, bool, bool, const Color &, const Color &,
                           float)> drawer) {
        m_customDrawers[name] = drawer;
    }

    void ConnectionStyleManager::drawConnection(
        ImDrawList *drawList, const ImVec2 &startPos, const ImVec2 &endPos,
        bool isStartInput, bool isEndInput,
        bool selected, bool hovered, const Color &startCol, const Color &endCol, float scale) {
        switch (m_config.style) {
            case ConnectionStyle::Bezier:
                drawBezierConnection(drawList, startPos, endPos, isStartInput, isEndInput,
                                     selected, hovered, startCol, endCol, scale);
                break;

            case ConnectionStyle::StraightLine:
                drawStraightConnection(drawList, startPos, endPos,
                                       selected, hovered, startCol, endCol, scale);
                break;

            case ConnectionStyle::AngleLine:
                drawAngleConnection(drawList, startPos, endPos,
                                    selected, hovered, startCol, endCol, scale);
                break;

            case ConnectionStyle::MetroLine:
                drawMetroConnection(drawList, startPos, endPos,
                                    selected, hovered, startCol, endCol, scale);
                break;

            case ConnectionStyle::Custom:
                drawBezierConnection(drawList, startPos, endPos, isStartInput, isEndInput,
                                     selected, hovered, startCol, endCol, scale);
                break;
        }
    }

    void ConnectionStyleManager::setBoundingBoxFunction(std::function<bool(ImVec2, ImVec2)> func) {
        m_boundingBoxCheck = func;
    }

    void ConnectionStyleManager::drawBezierConnection(
        ImDrawList *drawList, const ImVec2 &start, const ImVec2 &end,
        bool isStartInput, bool isEndInput,
        bool selected, bool hovered, const Color &startCol, const Color &endCol, float scale) {
        const float distance = std::sqrt(std::pow(end.x - start.x, 2) + std::pow(end.y - start.y, 2));
        const float tension = m_config.curveTension;
        const float cpDistance = distance * tension;

        ImVec2 cp1, cp2;

        if (isStartInput) {
            cp1 = ImVec2(start.x, start.y - cpDistance);
        } else {
            cp1 = ImVec2(start.x, start.y + cpDistance);
        }

        if (isEndInput) {
            cp2 = ImVec2(end.x, end.y - cpDistance);
        } else {
            cp2 = ImVec2(end.x, end.y + cpDistance);
        }

        const float thickness = m_config.thickness * scale;

        ImU32 startColor = ImColor(startCol.r, startCol.g, startCol.b, startCol.a);
        ImU32 endColor = ImColor(endCol.r, endCol.g, endCol.b, endCol.a);

        if (selected) {
            startColor = ImColor(m_config.selectedColor.r, m_config.selectedColor.g,
                                 m_config.selectedColor.b, m_config.selectedColor.a);
            endColor = startColor;
        } else if (hovered) {
            startColor = ImColor(m_config.hoveredColor.r, m_config.hoveredColor.g,
                                 m_config.hoveredColor.b, m_config.hoveredColor.a);
            endColor = startColor;
        }

        if (m_config.drawShadow) {
            ImU32 shadowColor = IM_COL32(0, 0, 0, 40);
            drawList->AddBezierCubic(
                ImVec2(start.x + 3, start.y + 3),
                ImVec2(cp1.x + 3, cp1.y + 3),
                ImVec2(cp2.x + 3, cp2.y + 3),
                ImVec2(end.x + 3, end.y + 3),
                shadowColor, thickness
            );
        }

        if (!m_config.useGradient && startColor == endColor) {
            drawList->AddBezierCubic(start, cp1, cp2, end, startColor, thickness);

            if (m_config.drawHighlight && (selected || hovered)) {
                ImU32 highlightColor = IM_COL32(255, 255, 255, 100);
                drawList->AddBezierCubic(start, cp1, cp2, end, highlightColor, thickness * 0.5f);
            }
        } else {
            const int segments = 20;

            for (int i = 0; i < segments; i++) {
                float t0 = static_cast<float>(i) / segments;
                float t1 = static_cast<float>(i + 1) / segments;

                ImVec2 pos0 = ImBezierCubicCalc(start, cp1, cp2, end, t0);
                ImVec2 pos1 = ImBezierCubicCalc(start, cp1, cp2, end, t1);

                float tMid = (t0 + t1) * 0.5f;
                ImU32 segmentColor = ImLerp(startColor, endColor, tMid);

                drawList->AddLine(pos0, pos1, segmentColor, thickness);

                if (m_config.drawHighlight && (selected || hovered)) {
                    ImU32 highlightColor = IM_COL32(255, 255, 255, 100);
                    drawList->AddLine(pos0, pos1, highlightColor, thickness * 0.5f);
                }
            }
        }

        float endpointRadius = thickness * 0.8f;
        drawList->AddCircleFilled(start, endpointRadius, startColor);
        drawList->AddCircleFilled(end, endpointRadius, endColor);
    }

    void ConnectionStyleManager::drawStraightConnection(
        ImDrawList *drawList, const ImVec2 &start, const ImVec2 &end,
        bool selected, bool hovered, const Color &startCol, const Color &endCol, float scale) {
        const float thickness = m_config.thickness * scale;

        ImU32 startColor = ImColor(startCol.r, startCol.g, startCol.b, startCol.a);
        ImU32 endColor = ImColor(endCol.r, endCol.g, endCol.b, endCol.a);

        if (selected) {
            startColor = ImColor(m_config.selectedColor.r, m_config.selectedColor.g,
                                 m_config.selectedColor.b, m_config.selectedColor.a);
            endColor = startColor;
        } else if (hovered) {
            startColor = ImColor(m_config.hoveredColor.r, m_config.hoveredColor.g,
                                 m_config.hoveredColor.b, m_config.hoveredColor.a);
            endColor = startColor;
        }

        if (m_config.drawShadow) {
            ImU32 shadowColor = IM_COL32(0, 0, 0, 40);
            drawList->AddLine(
                ImVec2(start.x + 3, start.y + 3),
                ImVec2(end.x + 3, end.y + 3),
                shadowColor, thickness
            );
        }

        if (!m_config.useGradient && startColor == endColor) {
            drawList->AddLine(start, end, startColor, thickness);

            if (m_config.drawHighlight && (selected || hovered)) {
                ImU32 highlightColor = IM_COL32(255, 255, 255, 100);
                drawList->AddLine(start, end, highlightColor, thickness * 0.5f);
            }
        } else {
            const int segments = 20;

            for (int i = 0; i < segments; i++) {
                float t0 = static_cast<float>(i) / segments;
                float t1 = static_cast<float>(i + 1) / segments;

                ImVec2 pos0 = ImLerp(start, end, t0);
                ImVec2 pos1 = ImLerp(start, end, t1);

                float tMid = (t0 + t1) * 0.5f;
                ImU32 segmentColor = ImLerp(startColor, endColor, tMid);

                drawList->AddLine(pos0, pos1, segmentColor, thickness);

                if (m_config.drawHighlight && (selected || hovered)) {
                    ImU32 highlightColor = IM_COL32(255, 255, 255, 100);
                    drawList->AddLine(pos0, pos1, highlightColor, thickness * 0.5f);
                }
            }
        }

        float endpointRadius = thickness * 0.8f;
        drawList->AddCircleFilled(start, endpointRadius, startColor);
        drawList->AddCircleFilled(end, endpointRadius, endColor);
    }

    void ConnectionStyleManager::drawAngleConnection(
        ImDrawList *drawList, const ImVec2 &start, const ImVec2 &end,
        bool selected, bool hovered, const Color &startCol, const Color &endCol, float scale) {
        const float thickness = m_config.thickness * scale;

        ImU32 startColor = ImColor(startCol.r, startCol.g, startCol.b, startCol.a);
        ImU32 endColor = ImColor(endCol.r, endCol.g, endCol.b, endCol.a);

        if (selected) {
            startColor = ImColor(m_config.selectedColor.r, m_config.selectedColor.g,
                                 m_config.selectedColor.b, m_config.selectedColor.a);
            endColor = startColor;
        } else if (hovered) {
            startColor = ImColor(m_config.hoveredColor.r, m_config.hoveredColor.g,
                                 m_config.hoveredColor.b, m_config.hoveredColor.a);
            endColor = startColor;
        }

        ImVec2 middle = ImVec2(end.x, start.y);

        if (m_config.drawShadow) {
            ImU32 shadowColor = IM_COL32(0, 0, 0, 40);
            drawList->AddLine(
                ImVec2(start.x + 3, start.y + 3),
                ImVec2(middle.x + 3, middle.y + 3),
                shadowColor, thickness
            );
            drawList->AddLine(
                ImVec2(middle.x + 3, middle.y + 3),
                ImVec2(end.x + 3, end.y + 3),
                shadowColor, thickness
            );
        }

        if (!m_config.useGradient && startColor == endColor) {
            drawList->AddLine(start, middle, startColor, thickness);
            drawList->AddLine(middle, end, startColor, thickness);

            if (m_config.drawHighlight && (selected || hovered)) {
                ImU32 highlightColor = IM_COL32(255, 255, 255, 100);
                drawList->AddLine(start, middle, highlightColor, thickness * 0.5f);
                drawList->AddLine(middle, end, highlightColor, thickness * 0.5f);
            }
        } else {
            const int segments = 10;

            for (int i = 0; i < segments; i++) {
                float t0 = static_cast<float>(i) / segments;
                float t1 = static_cast<float>(i + 1) / segments;

                ImVec2 pos0 = ImLerp(start, middle, t0);
                ImVec2 pos1 = ImLerp(start, middle, t1);

                float tMid = (t0 + t1) * 0.5f;
                ImU32 segmentColor = ImLerp(startColor, endColor, tMid * 0.5f);

                drawList->AddLine(pos0, pos1, segmentColor, thickness);

                if (m_config.drawHighlight && (selected || hovered)) {
                    ImU32 highlightColor = IM_COL32(255, 255, 255, 100);
                    drawList->AddLine(pos0, pos1, highlightColor, thickness * 0.5f);
                }
            }

            for (int i = 0; i < segments; i++) {
                float t0 = static_cast<float>(i) / segments;
                float t1 = static_cast<float>(i + 1) / segments;

                ImVec2 pos0 = ImLerp(middle, end, t0);
                ImVec2 pos1 = ImLerp(middle, end, t1);

                float tMid = (t0 + t1) * 0.5f;
                ImU32 segmentColor = ImLerp(startColor, endColor, 0.5f + tMid * 0.5f);

                drawList->AddLine(pos0, pos1, segmentColor, thickness);

                if (m_config.drawHighlight && (selected || hovered)) {
                    ImU32 highlightColor = IM_COL32(255, 255, 255, 100);
                    drawList->AddLine(pos0, pos1, highlightColor, thickness * 0.5f);
                }
            }
        }

        if (m_config.cornerRadius > 0.0f) {
            float radius = m_config.cornerRadius * scale;

            ImVec2 cornerStart, cornerEnd;

            if (end.x > start.x) {
                if (end.y > start.y) {
                    cornerStart = ImVec2(middle.x - radius, middle.y);
                    cornerEnd = ImVec2(middle.x, middle.y + radius);
                } else {
                    cornerStart = ImVec2(middle.x - radius, middle.y);
                    cornerEnd = ImVec2(middle.x, middle.y - radius);
                }
            } else {
                if (end.y > start.y) {
                    cornerStart = ImVec2(middle.x + radius, middle.y);
                    cornerEnd = ImVec2(middle.x, middle.y + radius);
                } else {
                    cornerStart = ImVec2(middle.x + radius, middle.y);
                    cornerEnd = ImVec2(middle.x, middle.y - radius);
                }
            }

            ImU32 midColor = ImLerp(startColor, endColor, 0.5f);
            drawList->AddBezierQuadratic(cornerStart, middle, cornerEnd, midColor, thickness);
        }

        float endpointRadius = thickness * 0.8f;
        drawList->AddCircleFilled(start, endpointRadius, startColor);
        drawList->AddCircleFilled(end, endpointRadius, endColor);
    }

    void ConnectionStyleManager::setBoundingBoxManager(std::shared_ptr<NodeBoundingBoxManager> manager) {
        m_boundingBoxManager = manager;
    }

    void ConnectionStyleManager::drawMetroConnection(
        ImDrawList *drawList, const ImVec2 &start, const ImVec2 &end,
        bool selected, bool hovered, const Color &startCol, const Color &endCol, float scale) {
        const float thickness = m_config.thickness * scale;
        const float cornerRadius = m_config.cornerRadius * scale;

        ImU32 startColor = ImColor(startCol.r, startCol.g, startCol.b, startCol.a);
        ImU32 endColor = ImColor(endCol.r, endCol.g, endCol.b, endCol.a);

        if (selected) {
            startColor = ImColor(m_config.selectedColor.r, m_config.selectedColor.g,
                                 m_config.selectedColor.b, m_config.selectedColor.a);
            endColor = startColor;
        } else if (hovered) {
            startColor = ImColor(m_config.hoveredColor.r, m_config.hoveredColor.g,
                                 m_config.hoveredColor.b, m_config.hoveredColor.a);
            endColor = startColor;
        }

        std::vector<ImVec2> points;

        if (m_config.avoidNodes && m_boundingBoxManager) {
            Vec2 startVec(start.x, start.y);
            Vec2 endVec(end.x, end.y);

            std::vector<Vec2> path = m_boundingBoxManager->findPathAroundNodes(startVec, endVec, 10.0f);

            for (const auto &point: path) {
                points.push_back(ImVec2(point.x, point.y));
            }
        } else {
            float dx = end.x - start.x;
            float dy = end.y - start.y;

            ImVec2 middle1, middle2;

            if (std::abs(dx) > std::abs(dy)) {
                middle1 = ImVec2(start.x + dx * 0.5f, start.y);
                middle2 = ImVec2(start.x + dx * 0.5f, end.y);
            } else {
                middle1 = ImVec2(start.x, start.y + dy * 0.5f);
                middle2 = ImVec2(end.x, start.y + dy * 0.5f);
            }

            points = {start, middle1, middle2, end};
        }

        if (points.size() < 2) {
            points = {start, end};
        }

        if (m_config.drawShadow) {
            ImU32 shadowColor = IM_COL32(0, 0, 0, 40);

            for (size_t i = 0; i < points.size() - 1; i++) {
                ImVec2 shadowStart = ImVec2(points[i].x + 3, points[i].y + 3);
                ImVec2 shadowEnd = ImVec2(points[i + 1].x + 3, points[i + 1].y + 3);

                drawList->AddLine(shadowStart, shadowEnd, shadowColor, thickness);
            }
        }

        if (!m_config.useGradient && startColor == endColor) {
            for (size_t i = 0; i < points.size() - 1; i++) {
                drawList->AddLine(points[i], points[i + 1], startColor, thickness);

                if (m_config.drawHighlight && (selected || hovered)) {
                    ImU32 highlightColor = IM_COL32(255, 255, 255, 100);
                    drawList->AddLine(points[i], points[i + 1], highlightColor, thickness * 0.5f);
                }
            }
        } else {
            for (size_t i = 0; i < points.size() - 1; i++) {
                float segmentProgress = static_cast<float>(i) / (points.size() - 2);
                float nextSegmentProgress = static_cast<float>(i + 1) / (points.size() - 2);

                const int segmentsPerLine = 5;

                for (int j = 0; j < segmentsPerLine; j++) {
                    float t0 = static_cast<float>(j) / segmentsPerLine;
                    float t1 = static_cast<float>(j + 1) / segmentsPerLine;

                    ImVec2 pos0 = ImLerp(points[i], points[i + 1], t0);
                    ImVec2 pos1 = ImLerp(points[i], points[i + 1], t1);

                    float overallT0 = segmentProgress + t0 / segmentsPerLine * (nextSegmentProgress - segmentProgress);
                    ImU32 segmentColor = ImLerp(startColor, endColor, overallT0);

                    drawList->AddLine(pos0, pos1, segmentColor, thickness);

                    if (m_config.drawHighlight && (selected || hovered)) {
                        ImU32 highlightColor = IM_COL32(255, 255, 255, 100);
                        drawList->AddLine(pos0, pos1, highlightColor, thickness * 0.5f);
                    }
                }
            }
        }

        if (cornerRadius > 0.0f && points.size() > 2) {
            for (size_t i = 1; i < points.size() - 1; i++) {
                float segmentProgress = static_cast<float>(i - 1) / (points.size() - 3);
                ImU32 cornerColor = ImLerp(startColor, endColor, segmentProgress);

                ImVec2 dir1 = ImVec2(points[i].x - points[i - 1].x, points[i].y - points[i - 1].y);
                ImVec2 dir2 = ImVec2(points[i + 1].x - points[i].x, points[i + 1].y - points[i].y);

                float len1 = std::sqrt(dir1.x * dir1.x + dir1.y * dir1.y);
                float len2 = std::sqrt(dir2.x * dir2.x + dir2.y * dir2.y);

                if (len1 < 0.0001f || len2 < 0.0001f) continue;

                dir1.x /= len1;
                dir1.y /= len1;
                dir2.x /= len2;
                dir2.y /= len2;

                ImVec2 cornerStart = ImVec2(
                    points[i].x - dir1.x * cornerRadius,
                    points[i].y - dir1.y * cornerRadius
                );

                ImVec2 cornerEnd = ImVec2(
                    points[i].x + dir2.x * cornerRadius,
                    points[i].y + dir2.y * cornerRadius
                );

                drawList->AddBezierQuadratic(
                    cornerStart,
                    points[i],
                    cornerEnd,
                    cornerColor, thickness
                );
            }
        }

        float endpointRadius = thickness * 0.8f;
        drawList->AddCircleFilled(start, endpointRadius, startColor);
        drawList->AddCircleFilled(end, endpointRadius, endColor);
    }

    ImVec2 ConnectionStyleManager::findPathAroundNodes(const ImVec2 &start, const ImVec2 &end) {
        if (!m_boundingBoxCheck) {
            return ImVec2((start.x + end.x) * 0.5f, (start.y + end.y) * 0.5f);
        }

        const ImVec2 directions[] = {
            ImVec2(0, -100),
            ImVec2(100, 0),
            ImVec2(0, 100),
            ImVec2(-100, 0)
        };

        for (const auto &dir: directions) {
            ImVec2 midPoint = ImVec2(start.x + dir.x, start.y + dir.y);

            if (!m_boundingBoxCheck(start, midPoint) && !m_boundingBoxCheck(midPoint, end)) {
                return midPoint;
            }
        }

        return ImVec2((start.x + end.x) * 0.5f, (start.y + end.y) * 0.5f);
    }

    bool ConnectionStyleManager::hasNodeIntersection(const ImVec2 &start, const ImVec2 &end) {
        if (!m_boundingBoxCheck) return false;
        return m_boundingBoxCheck(start, end);
    }
}
