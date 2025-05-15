#include "../NodeEditor.h"
#include <algorithm>

namespace NodeEditorCore {

void NodeEditor::drawDragConnection(ImDrawList* drawList, const ImVec2& canvasPos) {
    const Node* node = getNode(m_state.connectingNodeId);
    if (!node) return;

    const Pin* pinInternal = node->findPin(m_state.connectingPinId);
    if (!pinInternal) return;

    const Pin* apiPin = getPin(m_state.connectingNodeId, m_state.connectingPinId);
    if (!apiPin) return;

    ImVec2 p1 = getPinPos(*node, *apiPin, canvasPos);
    ImVec2 p2;

    if (m_state.magnetPinNodeId != -1) {
        const Node* magnetNode = getNode(m_state.magnetPinNodeId);
        const Pin* magnetPin = getPin(m_state.magnetPinNodeId, m_state.magnetPinId);
        if (magnetNode && magnetPin) {
            p2 = getPinPos(*magnetNode, *magnetPin, canvasPos);
        } else {
            p2 = ImGui::GetMousePos();
        }
    } else {
        p2 = ImGui::GetMousePos();
    }

    std::string pinTypeName = pinTypeToString(pinInternal->type);
    const internal::PinColors& pinColors = m_state.style.pinColors.count(pinTypeName) ?
                               m_state.style.pinColors.at(pinTypeName) :
                               m_state.style.pinColors.at("Default");

    ImU32 dragColor;
    if (m_state.magnetPinNodeId != -1 && !m_state.canConnectToMagnetPin) {
        dragColor = IM_COL32(255, 50, 50, 200);
    } else {
        dragColor = IM_COL32(
            pinColors.base.r * 255,
            pinColors.base.g * 255,
            pinColors.base.b * 255,
            pinColors.base.a * 255 * 0.8f
        );
    }

    ImU32 outerColor = IM_COL32(40, 44, 52, 100);
    float thickness = m_state.style.connectionThickness * m_state.viewScale;

    float distance = std::abs(p2.y - p1.y);
    float cpOffset = std::max(50.0f, distance * 0.5f);

    ImVec2 cp1, cp2;
    if (pinInternal->isInput) {
        cp1 = ImVec2(p1.x, p1.y - cpOffset);
        cp2 = ImVec2(p2.x, p2.y + cpOffset);
    } else {
        cp1 = ImVec2(p1.x, p1.y + cpOffset);
        cp2 = ImVec2(p2.x, p2.y - cpOffset);
    }

    drawList->AddBezierCubic(p1, cp1, cp2, p2, outerColor, thickness + 1.5f);
    drawList->AddBezierCubic(p1, cp1, cp2, p2, dragColor, thickness);

    ImVec4 brightColor = ImGui::ColorConvertU32ToFloat4(dragColor);
    brightColor.x = std::min(brightColor.x + 0.3f, 1.0f);
    brightColor.y = std::min(brightColor.y + 0.3f, 1.0f);
    brightColor.z = std::min(brightColor.z + 0.3f, 1.0f);
    brightColor.w = 0.7f;

    ImU32 brightDragColor = ImGui::ColorConvertFloat4ToU32(brightColor);
    drawList->AddBezierCubic(p1, cp1, cp2, p2, brightDragColor, thickness * 0.4f);

    const float glowRadius = 2.5f * m_state.viewScale;
    ImU32 glowColor = IM_COL32(
        std::min(static_cast<int>(pinColors.base.r * 255 + 50), 255),
        std::min(static_cast<int>(pinColors.base.g * 255 + 50), 255),
        std::min(static_cast<int>(pinColors.base.b * 255 + 50), 255),
        180
    );

    drawList->AddCircleFilled(p1, glowRadius, glowColor);

    // Dessiner une croix au milieu si connexion invalide
    if (m_state.magnetPinNodeId != -1 && !m_state.canConnectToMagnetPin) {
        // Calculer le point du milieu de la Bézier
        ImVec2 midPoint = ImBezierCubicCalc(p1, cp1, cp2, p2, 0.5f);

        float crossSize = 8.0f * m_state.viewScale;
        float crossThickness = 2.0f * m_state.viewScale;
        ImU32 crossColor = IM_COL32(255, 50, 50, 230);

        drawList->AddLine(
            ImVec2(midPoint.x - crossSize, midPoint.y - crossSize),
            ImVec2(midPoint.x + crossSize, midPoint.y + crossSize),
            crossColor, crossThickness
        );

        drawList->AddLine(
            ImVec2(midPoint.x - crossSize, midPoint.y + crossSize),
            ImVec2(midPoint.x + crossSize, midPoint.y - crossSize),
            crossColor, crossThickness
        );
    }
}

void NodeEditor::drawNodePins(ImDrawList* drawList, const Node& node, const ImVec2& nodePos, const ImVec2& nodeSize, const ImVec2& canvasPos) {
    float pinRadius = m_state.style.pinRadius * m_state.viewScale * 1.2f;

    for (size_t i = 0; i < node.inputs.size(); ++i) {
        const auto& pinInternal = node.inputs[i];

        Pin apiPin;
        apiPin.id = pinInternal.id;
        apiPin.name = pinInternal.name;
        apiPin.isInput = pinInternal.isInput;
        apiPin.type = static_cast<PinType>(pinInternal.type);
        apiPin.shape = static_cast<PinShape>(pinInternal.shape);

        ImVec2 pinPos = getPinPos(node, apiPin, canvasPos);

        std::string pinTypeName = pinTypeToString(pinInternal.type);
        const internal::PinColors& pinColors = m_state.style.pinColors.count(pinTypeName) ?
                                   m_state.style.pinColors.at(pinTypeName) :
                                   m_state.style.pinColors.at("Default");

        ImU32 pinColor = IM_COL32(
            pinColors.base.r * 255,
            pinColors.base.g * 255,
            pinColors.base.b * 255,
            pinColors.base.a * 255
        );

        ImU32 pinOutlineColor = IM_COL32(80, 80, 90, 180);
        float pinOutlineThickness = 1.0f;

        ImVec2 mousePos = ImGui::GetMousePos();
        float dx = mousePos.x - pinPos.x;
        float dy = mousePos.y - pinPos.y;
        bool pinHovered = (dx*dx + dy*dy) <= (pinRadius * pinRadius * 4.0f);

        if (pinInternal.connected) {
            pinColor = IM_COL32(
                pinColors.connected.r * 255,
                pinColors.connected.g * 255,
                pinColors.connected.b * 255,
                pinColors.connected.a * 255
            );
            pinOutlineColor = IM_COL32(255, 255, 255, 100);
            pinOutlineThickness = 1.5f;
        }
        else if (pinHovered) {
            pinColor = IM_COL32(
                pinColors.hover.r * 255,
                pinColors.hover.g * 255,
                pinColors.hover.b * 255,
                pinColors.hover.a * 255
            );
        }

        if (node.disabled) {
            ImVec4 colorVec = ImGui::ColorConvertU32ToFloat4(pinColor);
            colorVec.w *= 0.5f;
            pinColor = ImGui::ColorConvertFloat4ToU32(colorVec);

            ImVec4 outlineVec = ImGui::ColorConvertU32ToFloat4(pinOutlineColor);
            outlineVec.w *= 0.5f;
            pinOutlineColor = ImGui::ColorConvertFloat4ToU32(outlineVec);
        }

        drawPinShape(drawList, pinPos, pinRadius, pinInternal.shape, pinColor, pinOutlineColor, pinOutlineThickness, pinHovered);
    }

    for (size_t i = 0; i < node.outputs.size(); ++i) {
        const auto& pinInternal = node.outputs[i];

        Pin apiPin;
        apiPin.id = pinInternal.id;
        apiPin.name = pinInternal.name;
        apiPin.isInput = pinInternal.isInput;
        apiPin.type = static_cast<PinType>(pinInternal.type);
        apiPin.shape = static_cast<PinShape>(pinInternal.shape);

        ImVec2 pinPos = getPinPos(node, apiPin, canvasPos);

        std::string pinTypeName = pinTypeToString(pinInternal.type);
        const internal::PinColors& pinColors = m_state.style.pinColors.count(pinTypeName) ?
                                   m_state.style.pinColors.at(pinTypeName) :
                                   m_state.style.pinColors.at("Default");

        ImU32 pinColor = IM_COL32(
            pinColors.base.r * 255,
            pinColors.base.g * 255,
            pinColors.base.b * 255,
            pinColors.base.a * 255
        );

        ImU32 pinOutlineColor = IM_COL32(80, 80, 90, 180);
        float pinOutlineThickness = 1.0f;

        ImVec2 mousePos = ImGui::GetMousePos();
        float dx = mousePos.x - pinPos.x;
        float dy = mousePos.y - pinPos.y;
        bool pinHovered = (dx*dx + dy*dy) <= (pinRadius * pinRadius * 4.0f);

        if (pinInternal.connected) {
            pinColor = IM_COL32(
                pinColors.connected.r * 255,
                pinColors.connected.g * 255,
                pinColors.connected.b * 255,
                pinColors.connected.a * 255
            );
            pinOutlineColor = IM_COL32(255, 255, 255, 100);
            pinOutlineThickness = 1.5f;
        }
        else if (pinHovered) {
            pinColor = IM_COL32(
                pinColors.hover.r * 255,
                pinColors.hover.g * 255,
                pinColors.hover.b * 255,
                pinColors.hover.a * 255
            );
        }

        if (node.disabled) {
            ImVec4 colorVec = ImGui::ColorConvertU32ToFloat4(pinColor);
            colorVec.w *= 0.5f;
            pinColor = ImGui::ColorConvertFloat4ToU32(colorVec);

            ImVec4 outlineVec = ImGui::ColorConvertU32ToFloat4(pinOutlineColor);
            outlineVec.w *= 0.5f;
            pinOutlineColor = ImGui::ColorConvertFloat4ToU32(outlineVec);
        }

        drawPinShape(drawList, pinPos, pinRadius, pinInternal.shape, pinColor, pinOutlineColor, pinOutlineThickness, pinHovered);
    }
}

}