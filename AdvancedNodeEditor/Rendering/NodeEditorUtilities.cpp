#include "../NodeEditor.h"
#include <algorithm>

namespace NodeEditorCore {
    std::string NodeEditor::pinTypeToString(PinType type) const {
        switch (type) {
            case PinType::Blue: return "Blue";
            case PinType::Red: return "Red";
            case PinType::Green: return "Green";
            case PinType::Yellow: return "Yellow";
            case PinType::Purple: return "Purple";
            case PinType::Cyan: return "Cyan";
            case PinType::Orange: return "Orange";
            case PinType::White: return "White";
            case PinType::Black: return "Black";
            case PinType::Gray: return "Gray";
            case PinType::Custom: return "Custom";
            default: return "Default";
        }
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

            float hoverRadius = radius * 1.9f;

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
