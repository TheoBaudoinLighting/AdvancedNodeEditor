#include "NodeEditor.h"
#include <algorithm>

namespace NodeEditorCore {

void NodeEditor::drawConnections(ImDrawList* drawList, const ImVec2& canvasPos) {
    for (const auto& connection : m_state.connections) {
        const Node* startNode = getNode(connection.startNodeId);
        const Node* endNode = getNode(connection.endNodeId);
        const Pin* startPin = getPin(connection.startNodeId, connection.startPinId);
        const Pin* endPin = getPin(connection.endNodeId, connection.endPinId);
        
        if (!startNode || !endNode || !startPin || !endPin) continue;
        
        ImVec2 p1 = getPinPos(*startNode, *startPin, canvasPos);
        ImVec2 p2 = getPinPos(*endNode, *endPin, canvasPos);
        
        std::string startPinType = pinTypeToString(startPin->type);
        std::string endPinType = pinTypeToString(endPin->type);
        
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

void NodeEditor::drawDragConnection(ImDrawList* drawList, const ImVec2& canvasPos) {
    const Node* node = getNode(m_state.connectingNodeId);
    if (!node) return;
    
    const Pin* pin = nullptr;
    for (const auto& p : node->outputs) {
        if (p.id == m_state.connectingPinId) {
            pin = &p;
            break;
        }
    }
    
    if (!pin) {
        for (const auto& p : node->inputs) {
            if (p.id == m_state.connectingPinId) {
                pin = &p;
                break;
            }
        }
    }
    
    if (!pin) return;
    
    ImVec2 p1 = getPinPos(*node, *pin, canvasPos);
    ImVec2 p2 = ImGui::GetMousePos();
    
    std::string pinTypeName = pinTypeToString(pin->type);
    const internal::PinColors& pinColors = m_state.style.pinColors.count(pinTypeName) ?
                               m_state.style.pinColors.at(pinTypeName) :
                               m_state.style.pinColors.at("Default");
    
    ImU32 dragColor = IM_COL32(
        pinColors.color.r * 255,
        pinColors.color.g * 255,
        pinColors.color.b * 255,
        pinColors.color.a * 255 * 0.8f
    );
    
    ImU32 outerColor = IM_COL32(40, 44, 52, 100);
    float thickness = m_state.style.connectionThickness * m_state.viewScale;
    
    float distance = std::abs(p2.y - p1.y);
    float cpOffset = std::max(50.0f, distance * 0.5f);
    
    ImVec2 cp1 = ImVec2(p1.x, p1.y + cpOffset);
    ImVec2 cp2 = ImVec2(p2.x, p2.y - cpOffset);
    
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
        std::min(static_cast<int>(pinColors.color.r * 255 + 50), 255),
        std::min(static_cast<int>(pinColors.color.g * 255 + 50), 255),
        std::min(static_cast<int>(pinColors.color.b * 255 + 50), 255),
        180
    );
    
    drawList->AddCircleFilled(p1, glowRadius, glowColor);
}

void NodeEditor::drawGroups(ImDrawList* drawList, const ImVec2& canvasPos) {
    for (const auto& group : m_state.groups) {
        ImVec2 groupPos = canvasToScreen(group.position).toImVec2();
        ImVec2 groupSize = Vec2(group.size.x * m_state.viewScale, group.size.y * m_state.viewScale).toImVec2();
        
        float headerHeight = 30.0f * m_state.viewScale;
        
        ImU32 fillColor = IM_COL32(group.color.r * 255, group.color.g * 255, group.color.b * 255, group.color.a * 255);
        ImU32 borderColor = group.selected ? 
                         IM_COL32(255, 255, 255, 100) : 
                         IM_COL32(group.color.r * 255 * 0.8f, group.color.g * 255 * 0.8f, group.color.b * 255 * 0.8f, 100);
        
        ImU32 headerColor = IM_COL32(
            group.color.r * 255 * 1.2f,
            group.color.g * 255 * 1.2f,
            group.color.b * 255 * 1.2f,
            group.color.a * 255
        );
        
        drawList->AddRectFilled(
            groupPos,
            ImVec2(groupPos.x + groupSize.x, groupPos.y + groupSize.y),
            fillColor,
            4.0f
        );
        
        drawList->AddRectFilled(
            groupPos,
            ImVec2(groupPos.x + groupSize.x, groupPos.y + headerHeight),
            headerColor,
            4.0f,
            ImDrawFlags_RoundCornersTop
        );
        
        drawList->AddRect(
            groupPos,
            ImVec2(groupPos.x + groupSize.x, groupPos.y + groupSize.y),
            borderColor,
            4.0f,
            0,
            1.5f
        );
        
        ImVec2 textSize = ImGui::CalcTextSize(group.name.c_str());
        float textPosX = groupPos.x + 10.0f;
        float textPosY = groupPos.y + (headerHeight - textSize.y) * 0.5f;
        
        drawList->AddText(
            ImVec2(textPosX, textPosY),
            IM_COL32(255, 255, 255, 220),
            group.name.c_str()
        );
    }
}

void NodeEditor::drawPinShape(ImDrawList* drawList, const ImVec2& center, float radius, PinShape shape, ImU32 fillColor, ImU32 borderColor, float borderThickness, bool isHovered) {
    if (isHovered) {
        ImU32 hoverEffectColor = IM_COL32(255, 255, 255, 100);
        
        float hoverRadius = radius * 1.6f;
        
        switch (shape) {
            case PinShape::Square:
                drawList->AddRectFilled(
                    ImVec2(center.x - hoverRadius, center.y - hoverRadius),
                    ImVec2(center.x + hoverRadius, center.y + hoverRadius),
                    hoverEffectColor
                );
                break;
                
            case PinShape::Triangle:
                {
                    ImVec2 p1(center.x, center.y - hoverRadius);
                    ImVec2 p2(center.x - hoverRadius, center.y + hoverRadius);
                    ImVec2 p3(center.x + hoverRadius, center.y + hoverRadius);
                    
                    drawList->AddTriangleFilled(p1, p2, p3, hoverEffectColor);
                }
                break;
                
            case PinShape::Diamond:
                {
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
            
        case PinShape::Triangle:
            {
                ImVec2 p1(center.x, center.y - radius);
                ImVec2 p2(center.x - radius, center.y + radius);
                ImVec2 p3(center.x + radius, center.y + radius);
                
                drawList->AddTriangleFilled(p1, p2, p3, fillColor);
                drawList->AddTriangle(p1, p2, p3, borderColor, borderThickness);
            }
            break;
            
        case PinShape::Diamond:
            {
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

void NodeEditor::drawNodePins(ImDrawList* drawList, const Node& node, const ImVec2& nodePos, const ImVec2& nodeSize, const ImVec2& canvasPos) {
    float pinRadius = m_state.style.pinRadius * m_state.viewScale;
    
    for (size_t i = 0; i < node.inputs.size(); ++i) {
        const auto& pin = node.inputs[i];
        ImVec2 pinPos = getPinPos(node, pin, canvasPos);
        
        std::string pinTypeName = pinTypeToString(pin.type);
        const internal::PinColors& pinColors = m_state.style.pinColors.count(pinTypeName) ?
                                   m_state.style.pinColors.at(pinTypeName) :
                                   m_state.style.pinColors.at("Default");
        
        ImU32 pinColor = IM_COL32(
            pinColors.color.r * 255,
            pinColors.color.g * 255,
            pinColors.color.b * 255,
            pinColors.color.a * 255
        );
        
        ImU32 pinOutlineColor = IM_COL32(80, 80, 90, 180);
        float pinOutlineThickness = 1.0f;
        
        ImVec2 mousePos = ImGui::GetMousePos();
        float dx = mousePos.x - pinPos.x;
        float dy = mousePos.y - pinPos.y;
        bool pinHovered = (dx*dx + dy*dy) <= (pinRadius * pinRadius * 2.0f);
        
        if (pin.connected) {
            pinColor = IM_COL32(
                pinColors.connectedColor.r * 255,
                pinColors.connectedColor.g * 255,
                pinColors.connectedColor.b * 255,
                pinColors.connectedColor.a * 255
            );
            pinOutlineColor = IM_COL32(255, 255, 255, 100);
            pinOutlineThickness = 1.5f;
        }
        else if (pinHovered) {
            pinColor = IM_COL32(
                pinColors.hoverColor.r * 255,
                pinColors.hoverColor.g * 255,
                pinColors.hoverColor.b * 255,
                pinColors.hoverColor.a * 255
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
        
        drawPinShape(drawList, pinPos, pinRadius, pin.shape, pinColor, pinOutlineColor, pinOutlineThickness, pinHovered);
    }
    
    for (size_t i = 0; i < node.outputs.size(); ++i) {
        const auto& pin = node.outputs[i];
        ImVec2 pinPos = getPinPos(node, pin, canvasPos);
        
        std::string pinTypeName = pinTypeToString(pin.type);
        const internal::PinColors& pinColors = m_state.style.pinColors.count(pinTypeName) ?
                                   m_state.style.pinColors.at(pinTypeName) :
                                   m_state.style.pinColors.at("Default");
        
        ImU32 pinColor = IM_COL32(
            pinColors.color.r * 255,
            pinColors.color.g * 255,
            pinColors.color.b * 255,
            pinColors.color.a * 255
        );
        
        ImU32 pinOutlineColor = IM_COL32(80, 80, 90, 180);
        float pinOutlineThickness = 1.0f;
        
        ImVec2 mousePos = ImGui::GetMousePos();
        float dx = mousePos.x - pinPos.x;
        float dy = mousePos.y - pinPos.y;
        bool pinHovered = (dx*dx + dy*dy) <= (pinRadius * pinRadius * 2.0f);
        
        if (pin.connected) {
            pinColor = IM_COL32(
                pinColors.connectedColor.r * 255,
                pinColors.connectedColor.g * 255,
                pinColors.connectedColor.b * 255,
                pinColors.connectedColor.a * 255
            );
            pinOutlineColor = IM_COL32(255, 255, 255, 100);
            pinOutlineThickness = 1.5f;
        }
        else if (pinHovered) {
            pinColor = IM_COL32(
                pinColors.hoverColor.r * 255,
                pinColors.hoverColor.g * 255,
                pinColors.hoverColor.b * 255,
                pinColors.hoverColor.a * 255
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
        
        drawPinShape(drawList, pinPos, pinRadius, pin.shape, pinColor, pinOutlineColor, pinOutlineThickness, pinHovered);
    }
}

}