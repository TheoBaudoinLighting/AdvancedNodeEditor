#include "../Core/NodeEditor.h"
#include <algorithm>

namespace NodeEditorCore {
    void NodeEditor::drawNodes(ImDrawList *drawList, const ImVec2 &canvasPos) {
    std::vector<Node*> visibleNodes;
    int currentSubgraphId = m_state.currentSubgraphId;

    for (auto &node : m_state.nodes) {
        bool shouldDraw = false;

        if (currentSubgraphId >= 0) {
            shouldDraw = (node.getSubgraphId() == currentSubgraphId);
        } else {
            shouldDraw = (node.getSubgraphId() == -1);
        }

        if (shouldDraw) {
            visibleNodes.push_back(&node);
        }
    }

    std::stable_sort(visibleNodes.begin(), visibleNodes.end(),
        [](const Node *a, const Node *b) {
            if (a->selected && !b->selected) return false;
            if (!a->selected && b->selected) return true;
            return false;
        });

    for (Node* nodePtr : visibleNodes) {
        const Node& node = *nodePtr;
        bool isInputNode = false;
        bool isOutputNode = false;

        if (currentSubgraphId >= 0) {
            Subgraph *subgraph = getSubgraph(currentSubgraphId);
            if (subgraph) {
                int inputNodeId = subgraph->metadata.getAttribute<int>("inputNodeId", -1);
                int outputNodeId = subgraph->metadata.getAttribute<int>("outputNodeId", -1);

                isInputNode = (node.id == inputNodeId);
                isOutputNode = (node.id == outputNodeId);
            }
        }

        ImVec2 nodePos = canvasToScreen(node.position).toImVec2();
        ImVec2 nodeSize = Vec2(node.size.x * m_state.viewScale, node.size.y * m_state.viewScale).toImVec2();

        const float cornerRadius = 4.0f * m_state.viewScale;
        const float headerHeight = 14.0f * m_state.viewScale;
        const float accentLineHeight = 1.0f * m_state.viewScale;

        bool isHovered = m_state.hoveredNodeId == node.id;

        auto& nodeAnimState = m_animationManager.getNodeAnimationState(node.id);
        m_animationManager.setNodeHovered(node.id, isHovered);

        float scaleFactor = nodeAnimState.hoverScaleFactor;
        ImVec2 nodeSizeOriginal = nodeSize;
        ImVec2 nodePosOriginal = nodePos;

        if (scaleFactor != 1.0f) {
            float scaleOffsetX = (nodeSize.x * scaleFactor - nodeSize.x) * 0.5f;
            float scaleOffsetY = (nodeSize.y * scaleFactor - nodeSize.y) * 0.5f;
            nodeSize.x *= scaleFactor;
            nodeSize.y *= scaleFactor;
            nodePos.x -= scaleOffsetX;
            nodePos.y -= scaleOffsetY;
        }

        float executionPulseIntensity = 0.0f;
        if (nodeAnimState.executionPulse > 0.0f) {
            executionPulseIntensity = std::sin(nodeAnimState.executionPulse * 3.14159f * 2.0f) * 0.5f + 0.5f;
        }

        ImU32 baseColor, headerColor, accentColor, borderColor, selectedColor, hoveredColor, glowColor;

        if (isInputNode) {
            baseColor = IM_COL32(30, 80, 30, 230);
            headerColor = IM_COL32(20, 60, 20, 230);
            accentColor = IM_COL32(80, 180, 80, 255);
            borderColor = IM_COL32(40, 100, 40, 200);
            selectedColor = IM_COL32(100, 200, 100, 200);
            hoveredColor = IM_COL32(60, 150, 60, 180);
            glowColor = IM_COL32(40, 120, 40, 120);
        } else if (isOutputNode) {
            baseColor = IM_COL32(80, 30, 30, 230);
            headerColor = IM_COL32(60, 20, 20, 230);
            accentColor = IM_COL32(180, 80, 80, 255);
            borderColor = IM_COL32(100, 40, 40, 200);
            selectedColor = IM_COL32(200, 100, 100, 200);
            hoveredColor = IM_COL32(150, 60, 60, 180);
            glowColor = IM_COL32(120, 40, 40, 120);
        } else {
            const internal::NodeColors &nodeColors = m_state.style.nodeColors.count(node.type)
                                                         ? m_state.style.nodeColors.at(node.type)
                                                         : m_state.style.nodeColors.at("Default");

            baseColor = IM_COL32(
                nodeColors.base.r * 255,
                nodeColors.base.g * 255,
                nodeColors.base.b * 255,
                nodeColors.base.a * 255
            );

            headerColor = IM_COL32(
                nodeColors.header.r * 255,
                nodeColors.header.g * 255,
                nodeColors.header.b * 255,
                nodeColors.header.a * 255
            );

            accentColor = IM_COL32(
                nodeColors.accent.r * 255,
                nodeColors.accent.g * 255,
                nodeColors.accent.b * 255,
                nodeColors.accent.a * 255
            );

            borderColor = IM_COL32(
                nodeColors.border.r * 255,
                nodeColors.border.g * 255,
                nodeColors.border.b * 255,
                nodeColors.border.a * 255
            );

            selectedColor = IM_COL32(
                nodeColors.selected.r * 255,
                nodeColors.selected.g * 255,
                nodeColors.selected.b * 255,
                nodeColors.selected.a * 255
            );

            hoveredColor = IM_COL32(
                nodeColors.hovered.r * 255,
                nodeColors.hovered.g * 255,
                nodeColors.hovered.b * 255,
                nodeColors.hovered.a * 255
            );

            glowColor = IM_COL32(
                nodeColors.glow.r * 255,
                nodeColors.glow.g * 255,
                nodeColors.glow.b * 255,
                nodeColors.glow.a * 255
            );
        }

        if (executionPulseIntensity > 0.0f) {
            ImVec4 baseColorVec4 = ImGui::ColorConvertU32ToFloat4(baseColor);
            ImVec4 accentColorVec4 = ImGui::ColorConvertU32ToFloat4(accentColor);

            baseColorVec4.x = std::min(baseColorVec4.x + executionPulseIntensity * 0.2f, 1.0f);
            baseColorVec4.y = std::min(baseColorVec4.y + executionPulseIntensity * 0.2f, 1.0f);
            baseColorVec4.z = std::min(baseColorVec4.z + executionPulseIntensity * 0.2f, 1.0f);

            accentColorVec4.x = std::min(accentColorVec4.x + executionPulseIntensity * 0.3f, 1.0f);
            accentColorVec4.y = std::min(accentColorVec4.y + executionPulseIntensity * 0.3f, 1.0f);
            accentColorVec4.z = std::min(accentColorVec4.z + executionPulseIntensity * 0.3f, 1.0f);

            baseColor = ImGui::ColorConvertFloat4ToU32(baseColorVec4);
            accentColor = ImGui::ColorConvertFloat4ToU32(accentColorVec4);
        }

        if (node.disabled) {
            baseColor = IM_COL32(40, 40, 40, 180);
            headerColor = IM_COL32(30, 30, 35, 180);
            accentColor = IM_COL32(70, 70, 80, 150);
            borderColor = IM_COL32(60, 60, 60, 180);
        }

        bool isSelectable = isNodeSelectableForDelete(node.id);
        ImU32 actualSelectedColor = isSelectable ? selectedColor : IM_COL32(100, 100, 100, 150);

        if (node.selected || isHovered) {
            float glowSize = node.selected ? 8.0f : 6.0f;

            ImU32 enhancedGlowColor = node.selected
                                          ? IM_COL32(
                                              (actualSelectedColor >> 0) & 0xFF,
                                              (actualSelectedColor >> 8) & 0xFF,
                                              (actualSelectedColor >> 16) & 0xFF,
                                              100
                                          )
                                          : IM_COL32(
                                              (hoveredColor >> 0) & 0xFF,
                                              (hoveredColor >> 8) & 0xFF,
                                              (hoveredColor >> 16) & 0xFF,
                                              80
                                          );

            drawList->AddRectFilled(
                ImVec2(nodePos.x - glowSize, nodePos.y - glowSize),
                ImVec2(nodePos.x + nodeSize.x + glowSize, nodePos.y + nodeSize.y + glowSize),
                enhancedGlowColor, cornerRadius + glowSize
            );
        }

        {
            const float shadowOffset = 4.0f;
            const float shadowFeather = 10.0f;
            const ImU32 shadowColor = IM_COL32(0, 0, 0, 40);

            drawList->AddRectFilled(
                ImVec2(nodePos.x + shadowOffset, nodePos.y + shadowOffset),
                ImVec2(nodePos.x + nodeSize.x + shadowOffset, nodePos.y + nodeSize.y + shadowOffset),
                shadowColor, cornerRadius, ImDrawFlags_RoundCornersAll
            );

            for (float i = 1; i <= shadowFeather; i++) {
                float alpha = 40 * (1.0f - i / shadowFeather);
                ImU32 featherColor = IM_COL32(0, 0, 0, static_cast<int>(alpha));

                drawList->AddRect(
                    ImVec2(nodePos.x + shadowOffset - i, nodePos.y + shadowOffset - i),
                    ImVec2(nodePos.x + nodeSize.x + shadowOffset + i, nodePos.y + nodeSize.y + shadowOffset + i),
                    featherColor, cornerRadius + i, 0, 1.0f
                );
            }
        }

        ImVec4 baseColorVec4 = ImGui::ColorConvertU32ToFloat4(baseColor);

        ImVec4 saturatedBorderColorVec4 = baseColorVec4;
        saturatedBorderColorVec4.x = std::min(saturatedBorderColorVec4.x * 2.0f, 1.0f);
        saturatedBorderColorVec4.y = std::min(saturatedBorderColorVec4.y * 2.0f, 1.0f);
        saturatedBorderColorVec4.z = std::min(saturatedBorderColorVec4.z * 2.0f, 1.0f);
        saturatedBorderColorVec4.w = 0.85f;
        ImU32 saturatedBorderColor = ImGui::ColorConvertFloat4ToU32(saturatedBorderColorVec4);

        ImU32 activeBorderColor = node.selected ? actualSelectedColor : saturatedBorderColor;

        float outlineOffset = 1.0f;
        float borderThickness = node.selected ? 2.0f : 1.5f;

        drawList->AddRect(
            ImVec2(nodePos.x - outlineOffset, nodePos.y - outlineOffset),
            ImVec2(nodePos.x + nodeSize.x + outlineOffset, nodePos.y + nodeSize.y + outlineOffset),
            activeBorderColor, cornerRadius + outlineOffset, 0, borderThickness
        );

        const int segments = 3;
        for (int segment = 0; segment < segments; segment++) {
            float t = static_cast<float>(segment) / (segments - 1);
            ImVec4 gradientBorderColor = saturatedBorderColorVec4;
            gradientBorderColor.x = std::min(gradientBorderColor.x * (1.3f - t * 0.3f), 1.0f);
            gradientBorderColor.y = std::min(gradientBorderColor.y * (1.3f - t * 0.3f), 1.0f);
            gradientBorderColor.z = std::min(gradientBorderColor.z * (1.3f - t * 0.3f), 1.0f);
            ImU32 segmentBorderColor = node.selected ? actualSelectedColor : ImGui::ColorConvertFloat4ToU32(gradientBorderColor);

            float offset = outlineOffset + segment * (borderThickness / segments) / 2.0f;

            drawList->AddRect(
                ImVec2(nodePos.x - offset, nodePos.y - offset),
                ImVec2(nodePos.x + nodeSize.x + offset, nodePos.y + nodeSize.y + offset),
                segmentBorderColor, cornerRadius + offset, 0, borderThickness / (segments * 1.5f)
            );
        }

        const int gradientSteps = 10;

        for (int y = 0; y < gradientSteps; y++) {
            for (int x = 0; x < gradientSteps; x++) {
                float tx = static_cast<float>(x) / (gradientSteps - 1);
                float ty = static_cast<float>(y) / (gradientSteps - 1);

                float diagonal = (tx + ty) / 2.0f;

                ImU32 gradientColor = ImGui::ColorConvertFloat4ToU32(ImVec4(
                    baseColorVec4.x * (1.25f - diagonal * 0.5f),
                    baseColorVec4.y * (1.25f - diagonal * 0.5f),
                    baseColorVec4.z * (1.25f - diagonal * 0.5f),
                    baseColorVec4.w
                ));

                float xStart = nodePos.x + (nodeSize.x * x) / gradientSteps;
                float yStart = nodePos.y + (nodeSize.y * y) / gradientSteps;
                float xEnd = nodePos.x + (nodeSize.x * (x + 1)) / gradientSteps;
                float yEnd = nodePos.y + (nodeSize.y * (y + 1)) / gradientSteps;

                ImDrawFlags cornerFlags = ImDrawFlags_RoundCornersNone;

                if (x == 0 && y == 0)
                    cornerFlags = ImDrawFlags_RoundCornersTopLeft;
                else if (x == gradientSteps - 1 && y == 0)
                    cornerFlags = ImDrawFlags_RoundCornersTopRight;
                else if (x == 0 && y == gradientSteps - 1)
                    cornerFlags = ImDrawFlags_RoundCornersBottomLeft;
                else if (x == gradientSteps - 1 && y == gradientSteps - 1)
                    cornerFlags = ImDrawFlags_RoundCornersBottomRight;

                if (cornerFlags == ImDrawFlags_RoundCornersNone) {
                    drawList->AddRectFilled(
                        ImVec2(xStart, yStart),
                        ImVec2(xEnd, yEnd),
                        gradientColor
                    );
                } else {
                    drawList->AddRectFilled(
                        ImVec2(xStart, yStart),
                        ImVec2(xEnd, yEnd),
                        gradientColor, cornerRadius, cornerFlags
                    );
                }
            }
        }

        drawList->AddRectFilled(
            nodePos,
            ImVec2(nodePos.x + nodeSize.x, nodePos.y + headerHeight),
            headerColor, cornerRadius, ImDrawFlags_RoundCornersTop
        );

        float buttonWidth = headerHeight * 0.75f;
        float buttonHeight = headerHeight;
        float separatorWidth = 1.0f * m_state.viewScale;
        float rightSideX = nodePos.x + nodeSize.x - 3 * buttonWidth - 2 * separatorWidth;

        ImVec2 disableBarMin = ImVec2(rightSideX, nodePos.y);
        ImVec2 disableBarMax = ImVec2(rightSideX + buttonWidth, nodePos.y + buttonHeight);

        ImVec2 templateBarMin = ImVec2(rightSideX + buttonWidth + separatorWidth, nodePos.y);
        ImVec2 templateBarMax = ImVec2(rightSideX + 2 * buttonWidth + separatorWidth, nodePos.y + buttonHeight);

        ImVec2 flagBarMin = ImVec2(rightSideX + 2 * buttonWidth + 2 * separatorWidth, nodePos.y);
        ImVec2 flagBarMax = ImVec2(rightSideX + 3 * buttonWidth + 2 * separatorWidth, nodePos.y + buttonHeight);

        ImVec2 mousePos = ImGui::GetIO().MousePos;
        bool hoverDisableBar = mousePos.x >= disableBarMin.x && mousePos.x <= disableBarMax.x &&
                               mousePos.y >= disableBarMin.y && mousePos.y <= disableBarMax.y;

        bool hoverTemplateBar = mousePos.x >= templateBarMin.x && mousePos.x <= templateBarMax.x &&
                                mousePos.y >= templateBarMin.y && mousePos.y <= templateBarMax.y;

        bool hoverFlagBar = mousePos.x >= flagBarMin.x && mousePos.x <= flagBarMax.x &&
                            mousePos.y >= flagBarMin.y && mousePos.y <= flagBarMax.y;

        ImU32 disableColorBase = IM_COL32(100, 90, 30, hoverDisableBar ? 200 : 140);
        ImU32 templateColorBase = IM_COL32(80, 60, 120, hoverTemplateBar ? 200 : 140);
        ImU32 flagColorBase = IM_COL32(40, 80, 130, hoverFlagBar ? 200 : 140);

        ImU32 disableColorActive = IM_COL32(255, 230, 150, 255);
        ImU32 templateColorActive = IM_COL32(210, 170, 255, 255);
        ImU32 flagColorActive = IM_COL32(150, 200, 255, 255);

        ImU32 disableColor = node.disabled
                                 ? disableColorActive
                                 : (hoverDisableBar ? disableColorBase : headerColor);
        ImU32 templateColor = node.isTemplate
                                  ? templateColorActive
                                  : (hoverTemplateBar ? templateColorBase : headerColor);
        ImU32 flagColor = node.isCurrentFlag ? flagColorActive : (hoverFlagBar ? flagColorBase : headerColor);

        if (!isInputNode && !isOutputNode) {
            drawList->AddRectFilled(
                ImVec2(rightSideX - separatorWidth, nodePos.y),
                ImVec2(rightSideX, nodePos.y + buttonHeight),
                accentColor
            );

            drawList->AddRectFilled(
                disableBarMin,
                disableBarMax,
                disableColor
            );

            drawList->AddRectFilled(
                ImVec2(rightSideX + buttonWidth, nodePos.y),
                ImVec2(rightSideX + buttonWidth + separatorWidth, nodePos.y + buttonHeight),
                accentColor
            );

            drawList->AddRectFilled(
                templateBarMin,
                templateBarMax,
                templateColor
            );

            drawList->AddRectFilled(
                ImVec2(rightSideX + 2 * buttonWidth + separatorWidth, nodePos.y),
                ImVec2(rightSideX + 2 * buttonWidth + 2 * separatorWidth, nodePos.y + buttonHeight),
                accentColor
            );

            drawList->AddRectFilled(
                flagBarMin,
                flagBarMax,
                flagColor,
                cornerRadius, ImDrawFlags_RoundCornersTopRight
            );
        }

        float accentLineY = nodePos.y + nodeSize.y / 2.0f - accentLineHeight / 2.0f;
        drawList->AddRectFilled(
            ImVec2(nodePos.x, accentLineY),
            ImVec2(nodePos.x + nodeSize.x, accentLineY + accentLineHeight),
            accentColor
        );

        float cornerPinRadius = 3.0f * m_state.viewScale;
        float cornerPinX = nodePos.x + nodeSize.x - cornerPinRadius - 3.0f;
        float cornerPinY = nodePos.y + nodeSize.y - cornerPinRadius - 3.0f;

        ImVec4 accentVec4 = ImGui::ColorConvertU32ToFloat4(accentColor);
        accentVec4.x *= 0.8f;
        accentVec4.y *= 0.8f;
        accentVec4.z *= 0.8f;
        ImU32 cornerPinColor = ImGui::ColorConvertFloat4ToU32(accentVec4);

        drawList->AddCircleFilled(
            ImVec2(cornerPinX, cornerPinY),
            cornerPinRadius,
            cornerPinColor
        );

        drawList->AddCircle(
            ImVec2(cornerPinX, cornerPinY),
            cornerPinRadius,
            IM_COL32(255, 255, 255, 100),
            0,
            1.0f
        );

        float highlightThickness = 1.0f;
        ImU32 highlightColor = IM_COL32(255, 255, 255, 30);
        drawList->AddLine(
            ImVec2(nodePos.x + cornerRadius, nodePos.y + highlightThickness / 2.0f),
            ImVec2(nodePos.x + nodeSize.x - cornerRadius, nodePos.y + highlightThickness / 2.0f),
            highlightColor,
            highlightThickness
        );

        if (nodeAnimState.justConnected && nodeAnimState.connectionGlow > 0.0f) {
            std::string pinTypeName = pinTypeToString(static_cast<PinType>(nodeAnimState.lastConnectedPinType));
            const internal::PinColors &pinColors = m_state.style.pinColors.count(pinTypeName)
                                                ? m_state.style.pinColors.at(pinTypeName)
                                                : m_state.style.pinColors.at("Default");

            ImU32 pinGlowColor = IM_COL32(
                pinColors.connected.r * 255,
                pinColors.connected.g * 255,
                pinColors.connected.b * 255,
                static_cast<int>(255 * (1.0f - nodeAnimState.connectionGlow))
            );

            const float orbitRadius = 10.0f * m_state.viewScale;
            const float centralRadius = std::max(nodeSize.x, nodeSize.y) / 2.0f + orbitRadius;

            ImVec2 nodeCenter = ImVec2(
                nodePos.x + nodeSize.x / 2.0f,
                nodePos.y + nodeSize.y / 2.0f
            );

            const int particleCount = 3;
            const float particleSize = 4.0f * m_state.viewScale;

            for (int i = 0; i < particleCount; i++) {
                float angle = nodeAnimState.connectionGlowAngle + (i * 6.28318f / particleCount);
                ImVec2 particlePos = ImVec2(
                    nodeCenter.x + std::cos(angle) * centralRadius,
                    nodeCenter.y + std::sin(angle) * centralRadius
                );

                drawList->AddCircleFilled(
                    particlePos,
                    particleSize,
                    pinGlowColor
                );

                const int trailCount = 5;
                for (int t = 1; t <= trailCount; t++) {
                    float trailAngle = angle - (t * 0.15f);
                    float fadeOut = 1.0f - (static_cast<float>(t) / trailCount);

                    ImVec2 trailPos = ImVec2(
                        nodeCenter.x + std::cos(trailAngle) * centralRadius,
                        nodeCenter.y + std::sin(trailAngle) * centralRadius
                    );

                    ImU32 trailColor = IM_COL32(
                        pinColors.connected.r * 255,
                        pinColors.connected.g * 255,
                        pinColors.connected.b * 255,
                        static_cast<int>(255 * fadeOut * (1.0f - nodeAnimState.connectionGlow))
                    );

                    drawList->AddCircleFilled(
                        trailPos,
                        particleSize * fadeOut,
                        trailColor
                    );
                }
            }

            drawList->AddCircle(
                nodeCenter,
                centralRadius,
                pinGlowColor,
                32,
                2.0f * m_state.viewScale
            );
        }

        if (node.labelPosition != NodeLabelPosition::None) {
            float textPosY = nodePos.y + nodeSize.y * 0.5f - ImGui::GetFontSize() * 0.5f;
            float textPosX;

            if (node.labelPosition == NodeLabelPosition::Right) {
                textPosX = nodePos.x + nodeSize.x + 5.0f * m_state.viewScale;
            } else {
                float textWidth = ImGui::CalcTextSize(node.name.c_str()).x;
                textPosX = nodePos.x - textWidth - 5.0f * m_state.viewScale;
            }

            ImU32 textColor = IM_COL32(
                180, 180, 185, node.disabled ? 128 : 220
            );

            drawList->AddText(
                ImVec2(textPosX, textPosY),
                textColor,
                node.name.c_str()
            );
        }

        if (!node.iconSymbol.empty()) {
            float iconCenterX = nodePos.x + 8.0f * m_state.viewScale;
            float iconCenterY = nodePos.y + 8.0f * m_state.viewScale;

            float iconCircleRadius = 5.5f * m_state.viewScale;
            float iconInnerRadius = 5.0f * m_state.viewScale;
            float iconHighlightRadius = 3.0f * m_state.viewScale;

            ImVec2 iconTextSize = ImGui::CalcTextSize(node.iconSymbol.c_str());
            float textScaleFactor = m_state.viewScale;

            drawList->AddCircleFilled(
                ImVec2(iconCenterX + 0.8f * m_state.viewScale, iconCenterY + 0.8f * m_state.viewScale),
                iconCircleRadius,
                IM_COL32(15, 18, 25, node.disabled ? 100 : 180)
            );

            drawList->AddCircleFilled(
                ImVec2(iconCenterX, iconCenterY),
                iconInnerRadius,
                IM_COL32(25, 30, 40, node.disabled ? 128 : 255)
            );

            drawList->AddCircle(
                ImVec2(iconCenterX - 1.0f * m_state.viewScale, iconCenterY - 1.0f * m_state.viewScale),
                iconHighlightRadius,
                IM_COL32(100, 120, 140, node.disabled ? 60 : 90),
                0,
                1.0f
            );

            drawList->AddText(
                ImVec2(iconCenterX - iconTextSize.x * 0.5f + 0.8f * m_state.viewScale,
                       iconCenterY - iconTextSize.y * 0.5f + 0.8f * m_state.viewScale),
                IM_COL32(0, 0, 0, node.disabled ? 80 : 160),
                node.iconSymbol.c_str()
            );

            drawList->AddText(
                ImVec2(iconCenterX - iconTextSize.x * 0.5f,
                       iconCenterY - iconTextSize.y * 0.5f),
                node.disabled ? IM_COL32(100, 100, 110, 128) : accentColor,
                node.iconSymbol.c_str()
            );
        }

        if (isInputNode || isOutputNode) {
            float iconSize = 15.0f * m_state.viewScale;
            float iconX = nodePos.x + nodeSize.x - iconSize - 5.0f * m_state.viewScale;
            float iconY = nodePos.y + 5.0f * m_state.viewScale;

            ImU32 iconColor = isInputNode ? IM_COL32(120, 255, 120, 255) : IM_COL32(255, 120, 120, 255);

            if (isInputNode) {
                drawList->AddTriangleFilled(
                    ImVec2(iconX, iconY),
                    ImVec2(iconX + iconSize, iconY + iconSize / 2),
                    ImVec2(iconX, iconY + iconSize),
                    iconColor
                );
            } else {
                drawList->AddTriangleFilled(
                    ImVec2(iconX + iconSize, iconY),
                    ImVec2(iconX, iconY + iconSize / 2),
                    ImVec2(iconX + iconSize, iconY + iconSize),
                    iconColor
                );
            }

            float labelX = nodePos.x + 5.0f * m_state.viewScale;
            float labelY = nodePos.y + 5.0f * m_state.viewScale;
            ImU32 labelColor = IM_COL32(220, 220, 220, 255);

            std::string nodeTypeLabel = isInputNode ? "INPUT" : "OUTPUT";
            drawList->AddText(
                ImVec2(labelX, labelY),
                labelColor,
                nodeTypeLabel.c_str()
            );
        }

        drawNodePins(drawList, node, nodePos, nodeSize, canvasPos);
    }
}

    bool NodeEditor::isNodeSelectableForDelete(int nodeId) const {
        for (const auto &subgraphPair: m_subgraphs) {
            int inputNodeId = subgraphPair.second->metadata.getAttribute<int>("inputNodeId", -1);
            int outputNodeId = subgraphPair.second->metadata.getAttribute<int>("outputNodeId", -1);

            if (nodeId == inputNodeId || nodeId == outputNodeId) {
                return false;
            }
        }

        const Node *node = getNode(nodeId);
        if (node && node->isProtected) {
            return false;
        }

        return true;
    }
}
