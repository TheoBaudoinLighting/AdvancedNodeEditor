#include "../Core/NodeEditor.h"
#include <algorithm>

namespace {
    constexpr float kPinVisualRadiusScale = 1.2f;
    constexpr float kPinHoverEffectRadiusScale = 1.9f;
    constexpr float kPinHitRadiusScale = 3.0f;
}

namespace NodeEditorCore {
    std::string NodeEditor::pinTypeToString(PinType type) const {
        if (PinStyleCatalog::HasPinType(type)) {
            return PinStyleCatalog::ForPinType(type).styleKey;
        }
        return "Default";
    }

    float NodeEditor::getNodeHeaderHeight() const {
        return 14.0f * m_state.viewScale;
    }

    NodeEditor::NodeHeaderButtonLayout NodeEditor::getNodeHeaderButtonLayout(const ImVec2 &nodePos,
                                                                             const ImVec2 &nodeSize) const {
        const float headerHeight = getNodeHeaderHeight();
        const float buttonWidth = headerHeight * 0.75f;
        const float buttonHeight = headerHeight;
        const float separatorWidth = 1.0f * m_state.viewScale;
        const float rightSideX = nodePos.x + nodeSize.x - 3.0f * buttonWidth - 2.0f * separatorWidth;

        NodeHeaderButtonLayout layout{};
        layout.rightSideX = rightSideX;
        layout.buttonWidth = buttonWidth;
        layout.buttonHeight = buttonHeight;
        layout.separatorWidth = separatorWidth;

        layout.disableMin = ImVec2(rightSideX, nodePos.y);
        layout.disableMax = ImVec2(rightSideX + buttonWidth, nodePos.y + buttonHeight);

        layout.templateMin = ImVec2(rightSideX + buttonWidth + separatorWidth, nodePos.y);
        layout.templateMax = ImVec2(rightSideX + 2.0f * buttonWidth + separatorWidth, nodePos.y + buttonHeight);

        layout.flagMin = ImVec2(rightSideX + 2.0f * buttonWidth + 2.0f * separatorWidth, nodePos.y);
        layout.flagMax = ImVec2(rightSideX + 3.0f * buttonWidth + 2.0f * separatorWidth, nodePos.y + buttonHeight);

        return layout;
    }

    bool NodeEditor::nodeHasHeaderButtons(const Node &node) const {
        if (node.type == "GraphInputNode" || node.type == "GraphOutputNode" ||
            node.type == "Input" || node.type == "Output") {
            return false;
        }

        if (m_state.currentSubgraphId >= 0) {
            const Subgraph *subgraph = getSubgraph(m_state.currentSubgraphId);
            if (subgraph) {
                const int inputNodeId = subgraph->metadata.getAttribute<int>("inputNodeId", -1);
                const int outputNodeId = subgraph->metadata.getAttribute<int>("outputNodeId", -1);
                if (node.id == inputNodeId || node.id == outputNodeId) {
                    return false;
                }
            }
        }

        return true;
    }

    float NodeEditor::getPinVisualRadius() const {
        return m_state.style.pinRadius * m_state.viewScale * kPinVisualRadiusScale;
    }

    float NodeEditor::getPinHitRadius() const {
        return m_state.style.pinRadius * m_state.viewScale * kPinHitRadiusScale;
    }

    bool NodeEditor::isPointInPinHitArea(const ImVec2 &point, const ImVec2 &pinPos) const {
        const float hitRadius = getPinHitRadius();
        const float dx = point.x - pinPos.x;
        const float dy = point.y - pinPos.y;

        return (dx * dx + dy * dy) <= (hitRadius * hitRadius);
    }

    ImU32 NodeEditor::ImLerpColor(ImU32 col_a, ImU32 col_b, float t) {
        ImVec4 a = ImGui::ColorConvertU32ToFloat4(col_a);
        ImVec4 b = ImGui::ColorConvertU32ToFloat4(col_b);

        return ImGui::ColorConvertFloat4ToU32(ImVec4(
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t,
            a.z + (b.z - a.z) * t,
            a.w + (b.w - a.w) * t
        ));
    }

    ImVec2 NodeEditor::ImBezierCubicCalc(const ImVec2 &p1, const ImVec2 &p2, const ImVec2 &p3, const ImVec2 &p4,
                                         float t) {
        float u = 1.0f - t;
        float tt = t * t;
        float uu = u * u;
        float uuu = uu * u;
        float ttt = tt * t;

        ImVec2 result = ImVec2(
            uuu * p1.x + 3 * uu * t * p2.x + 3 * u * tt * p3.x + ttt * p4.x,
            uuu * p1.y + 3 * uu * t * p2.y + 3 * u * tt * p3.y + ttt * p4.y
        );
        return result;
    }

    void NodeEditor::drawPinShape(ImDrawList *drawList, const ImVec2 &center, float radius, PinShape shape,
                                  ImU32 fillColor, ImU32 borderColor, float borderThickness, bool isHovered) {
        if (isHovered) {
            ImU32 hoverEffectColor = IM_COL32(255, 255, 255, 100);

            float hoverRadius = radius * kPinHoverEffectRadiusScale;

            switch (shape) {
                case PinShape::Square:
                    drawList->AddRectFilled(
                        ImVec2(center.x - hoverRadius, center.y - hoverRadius),
                        ImVec2(center.x + hoverRadius, center.y + hoverRadius),
                        hoverEffectColor
                    );
                    break;

                case PinShape::Triangle: {
                    ImVec2 p1(center.x, center.y - hoverRadius);
                    ImVec2 p2(center.x - hoverRadius, center.y + hoverRadius);
                    ImVec2 p3(center.x + hoverRadius, center.y + hoverRadius);

                    drawList->AddTriangleFilled(p1, p2, p3, hoverEffectColor);
                }
                break;

                case PinShape::Diamond: {
                    ImVec2 p1(center.x, center.y - hoverRadius);
                    ImVec2 p2(center.x - hoverRadius, center.y);
                    ImVec2 p3(center.x, center.y + hoverRadius);
                    ImVec2 p4(center.x + hoverRadius, center.y);

                    drawList->AddQuadFilled(p1, p2, p3, p4, hoverEffectColor);
                }
                break;

                default:
                    drawList->AddCircleFilled(center, hoverRadius, hoverEffectColor);
                    break;
            }
        }

        borderThickness = std::max(1.5f, borderThickness);

        switch (shape) {
            case PinShape::Square:
                drawList->AddRectFilled(
                    ImVec2(center.x - radius, center.y - radius),
                    ImVec2(center.x + radius, center.y + radius),
                    fillColor
                );
                drawList->AddRect(
                    ImVec2(center.x - radius, center.y - radius),
                    ImVec2(center.x + radius, center.y + radius),
                    borderColor, 0, 0, borderThickness
                );
                break;

            case PinShape::Triangle: {
                ImVec2 p1(center.x, center.y - radius);
                ImVec2 p2(center.x - radius, center.y + radius);
                ImVec2 p3(center.x + radius, center.y + radius);

                drawList->AddTriangleFilled(p1, p2, p3, fillColor);
                drawList->AddTriangle(p1, p2, p3, borderColor, borderThickness);
            }
            break;

            case PinShape::Diamond: {
                ImVec2 p1(center.x, center.y - radius);
                ImVec2 p2(center.x - radius, center.y);
                ImVec2 p3(center.x, center.y + radius);
                ImVec2 p4(center.x + radius, center.y);

                drawList->AddQuadFilled(p1, p2, p3, p4, fillColor);
                drawList->AddQuad(p1, p2, p3, p4, borderColor, borderThickness);
            }
            break;

            default:
                drawList->AddCircleFilled(center, radius, fillColor);
                drawList->AddCircle(center, radius, borderColor, 0, borderThickness);
                break;
        }
    }
}
