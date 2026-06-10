#include "../Core/NodeEditor.h"
#include "../Core/Style/InteractionMode.h"
#include "../Editor/View/MinimapManager.h"
#include "../Utils/NodeEditorLogging.h"
#include <algorithm>
#include <cmath>

namespace NodeEditorCore {
    namespace {
        void drawDashedLine(ImDrawList *drawList,
                            const ImVec2 &start,
                            const ImVec2 &end,
                            ImU32 color,
                            float thickness,
                            float dashLength = 8.0f,
                            float gapLength = 6.0f) {
            const float dx = end.x - start.x;
            const float dy = end.y - start.y;
            const float length = std::sqrt(dx * dx + dy * dy);
            if (length <= 0.01f) return;

            const ImVec2 direction(dx / length, dy / length);
            float distance = 0.0f;
            while (distance < length) {
                const float segmentEnd = std::min(distance + dashLength, length);
                const ImVec2 a(start.x + direction.x * distance,
                               start.y + direction.y * distance);
                const ImVec2 b(start.x + direction.x * segmentEnd,
                               start.y + direction.y * segmentEnd);
                drawList->AddLine(a, b, color, thickness);
                distance += dashLength + gapLength;
            }
        }
    }

    void NodeEditor::render() {
        ImGui::BeginChild("Canvas", ImVec2(0, 0), false,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse);

        ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        ImVec2 canvasSize = ImGui::GetContentRegionAvail();
        ImDrawList *drawList = ImGui::GetWindowDrawList();

        m_state.canvasPos = Vec2(canvasPos.x, canvasPos.y);
        m_viewManager.setWindowSize(Vec2(canvasSize.x, canvasSize.y));
        if (m_zoomToFitOnNextRender) {
            zoomToFitWithSize(canvasSize.x, canvasSize.y, m_zoomToFitOnNextRenderPadding);
            m_zoomToFitOnNextRender = false;
        }

        float deltaTime = ImGui::GetIO().DeltaTime;
        m_animationManager.update(deltaTime);

        m_animationManager.updateNodePositions(m_state.nodes, deltaTime);

        m_animationManager.updateConnectionFlows(m_state.connections, deltaTime);

        if (m_viewManager.isViewTransitioning()) {
            m_viewManager.updateViewTransition(deltaTime);
            m_state.viewPosition = m_viewManager.getViewPosition();
            m_state.viewScale = m_viewManager.getViewScale();
        }

        drawList->AddRectFilled(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
                                m_state.style.uiColors.background.toImU32());

        if (m_state.currentSubgraphId >= 0) {
            int depth = getSubgraphDepth(m_state.currentSubgraphId);
            auto it = m_depthColors.find(depth);

            if (it != m_depthColors.end()) {
                ImU32 depthColor = IM_COL32(
                    it->second.r * 255,
                    it->second.g * 255,
                    it->second.b * 255,
                    it->second.a * 255
                );

                drawList->AddRectFilled(
                    canvasPos,
                    ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + 5.0f),
                    depthColor
                );
            }
        }

        ImGui::InvisibleButton("canvas", canvasSize);

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("NODE_TYPE")) {
                const char *nodeType = static_cast<const char *>(payload->Data);
                if (nodeType) {
                    ImVec2 mousePos = ImGui::GetMousePos();
                    Vec2 dropPos = screenToCanvas(Vec2(mousePos.x, mousePos.y));
                    Node *node = createNodeOfType(nodeType, dropPos);
                    if (node) {
                        selectNode(node->id, false);
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (ImGui::IsItemHovered() || ImGui::IsItemActive()) {
            processInteraction();
        }

        drawGrid(drawList, canvasPos);
        if (m_state.interactionMode == InteractionMode::DragNode &&
            (m_nodeSnapGuideVertical || m_nodeSnapGuideHorizontal)) {
            const ImU32 guideColor = IM_COL32(90, 190, 255, 210);
            const float guideThickness = std::max(1.0f, 1.5f * m_state.viewScale);
            const ImVec2 sourceCenter = canvasToScreen(m_nodeSnapGuideSourceCenter).toImVec2();
            const ImVec2 targetCenter = canvasToScreen(m_nodeSnapGuideTargetCenter).toImVec2();

            if (m_nodeSnapGuideVertical) {
                drawDashedLine(drawList,
                               sourceCenter,
                               targetCenter,
                               guideColor,
                               guideThickness);
            }

            if (m_nodeSnapGuideHorizontal) {
                drawDashedLine(drawList,
                               sourceCenter,
                               targetCenter,
                               guideColor,
                               guideThickness);
            }
        }
        drawGroups(drawList, canvasPos);
        drawConnections(drawList, canvasPos);
        drawReroutes(drawList, canvasPos);
        drawNodes(drawList, canvasPos);

        if (m_state.interactionMode == InteractionMode::BoxSelect) {
            drawBoxSelection(drawList);
        }

        if (m_state.interactionMode == InteractionMode::DragConnection) {
            drawDragConnection(drawList, canvasPos);
        }

        if (m_state.interactionMode == InteractionMode::ContextMenu) {
            drawContextMenu(drawList);
        }

        drawSubgraphBreadcrumbs(drawList, canvasPos);

        if (isNodeAvoidanceEnabled()) {
            updateNodeBoundingBoxes();
        }

        if (m_debugMode) {
            drawDebugHitboxes(drawList, canvasPos);
        }

        if (m_minimapEnabled) {
            m_minimapManager.setViewPosition(m_state.viewPosition);
            m_minimapManager.setViewScale(m_state.viewScale);
            updateMinimapBounds();
            m_minimapManager.draw(drawList, canvasPos, canvasSize);
        }

        ImGui::EndChild();
    }

    void NodeEditor::arrangeNodesWithAnimation(const std::vector<int> &nodeIds, const ArrangementType type) {
        std::vector<Vec2> targetPositions;

        switch (type) {
            case ArrangementType::Grid: {
                float spacing = 150.0f;
                int nodesPerRow = std::max(1, static_cast<int>(std::sqrt(nodeIds.size())));

                for (size_t i = 0; i < nodeIds.size(); ++i) {
                    int row = i / nodesPerRow;
                    int col = i % nodesPerRow;

                    Vec2 targetPos(
                        col * spacing,
                        row * spacing
                    );

                    targetPositions.push_back(targetPos);
                }
                break;
            }

            case ArrangementType::Horizontal: {
                float spacing = 180.0f;
                for (size_t i = 0; i < nodeIds.size(); ++i) {
                    targetPositions.push_back(Vec2(i * spacing, 0.0f));
                }
                break;
            }

            case ArrangementType::Vertical: {
                float spacing = 120.0f;
                for (size_t i = 0; i < nodeIds.size(); ++i) {
                    targetPositions.push_back(Vec2(0.0f, i * spacing));
                }
                break;
            }

            case ArrangementType::Circle: {
                float radius = std::max(200.0f, nodeIds.size() * 40.0f);
                float angleStep = Math::TWO_PI / nodeIds.size();

                for (size_t i = 0; i < nodeIds.size(); ++i) {
                    float angle = i * angleStep;
                    Vec2 targetPos(
                        std::cos(angle) * radius,
                        std::sin(angle) * radius
                    );

                    targetPositions.push_back(targetPos);
                }
                break;
            }

            default:
                return;
        }

        Vec2 center(0.0f, 0.0f);
        for (const auto &pos: targetPositions) {
            center = center + pos;
        }
        if (!targetPositions.empty()) {
            center = center / static_cast<float>(targetPositions.size());
        }

        Vec2 currentCenter(0.0f, 0.0f);
        for (int nodeId: nodeIds) {
            Node *node = getNode(nodeId);
            if (node) {
                currentCenter = currentCenter + node->position;
            }
        }
        if (!nodeIds.empty()) {
            currentCenter = currentCenter / static_cast<float>(nodeIds.size());
        }

        Vec2 offset = currentCenter - center;
        for (size_t i = 0; i < nodeIds.size(); ++i) {
            Node *node = getNode(nodeIds[i]);
            if (node) {
                Vec2 targetPos = targetPositions[i] + offset;
                m_animationManager.setNodeTargetPosition(node->id, targetPos);
            }
        }
    }

    void NodeEditor::drawGrid(ImDrawList *drawList, const ImVec2 &canvasPos) {
        const float GRID_STEP_MAJOR = 64.0f * m_state.viewScale;
        const float GRID_STEP_MINOR = 16.0f * m_state.viewScale;

        ImVec2 windowSize = ImGui::GetWindowSize();

        ImColor colorTopLeft(18, 23, 30, 255);
        ImColor colorTopRight(16, 21, 28, 255);
        ImColor colorBottomRight(10, 14, 20, 255);
        ImColor colorBottomLeft(12, 16, 22, 255);

        drawList->AddRectFilledMultiColor(
            canvasPos,
            ImVec2(canvasPos.x + windowSize.x, canvasPos.y + windowSize.y),
            colorTopLeft, colorTopRight, colorBottomRight, colorBottomLeft
        );

        float intensityMultiplier = 1.0f;
        if (m_state.currentSubgraphId >= 0) {
            int depth = getSubgraphDepth(m_state.currentSubgraphId);
            intensityMultiplier = std::max(0.4f, 1.0f - depth * 0.12f);
        }

        ImU32 gridMinorColor = IM_COL32(
            50 * intensityMultiplier,
            55 * intensityMultiplier,
            70 * intensityMultiplier,
            40
        );

        ImU32 gridMajorColor = IM_COL32(
            80 * intensityMultiplier,
            85 * intensityMultiplier,
            115 * intensityMultiplier,
            70
        );

        ImU32 gridMajorGlowColor = IM_COL32(
            70 * intensityMultiplier,
            75 * intensityMultiplier,
            105 * intensityMultiplier,
            20
        );

        for (float x = fmodf(m_state.viewPosition.x, GRID_STEP_MINOR); x < windowSize.x; x += GRID_STEP_MINOR) {
            if (fmodf(x - fmodf(m_state.viewPosition.x, GRID_STEP_MAJOR), GRID_STEP_MAJOR) != 0.0f) {
                drawList->AddLine(
                    ImVec2(canvasPos.x + x, canvasPos.y),
                    ImVec2(canvasPos.x + x, canvasPos.y + windowSize.y),
                    gridMinorColor, 1.0f
                );
            }
        }

        for (float y = fmodf(m_state.viewPosition.y, GRID_STEP_MINOR); y < windowSize.y; y += GRID_STEP_MINOR) {
            if (fmodf(y - fmodf(m_state.viewPosition.y, GRID_STEP_MAJOR), GRID_STEP_MAJOR) != 0.0f) {
                drawList->AddLine(
                    ImVec2(canvasPos.x, canvasPos.y + y),
                    ImVec2(canvasPos.x + windowSize.x, canvasPos.y + y),
                    gridMinorColor, 1.0f
                );
            }
        }

        for (float x = fmodf(m_state.viewPosition.x, GRID_STEP_MAJOR); x < windowSize.x; x += GRID_STEP_MAJOR) {
            drawList->AddLine(
                ImVec2(canvasPos.x + x, canvasPos.y),
                ImVec2(canvasPos.x + x, canvasPos.y + windowSize.y),
                gridMajorGlowColor, 3.0f
            );

            drawList->AddLine(
                ImVec2(canvasPos.x + x, canvasPos.y),
                ImVec2(canvasPos.x + x, canvasPos.y + windowSize.y),
                gridMajorColor, 1.5f
            );
        }

        for (float y = fmodf(m_state.viewPosition.y, GRID_STEP_MAJOR); y < windowSize.y; y += GRID_STEP_MAJOR) {
            drawList->AddLine(
                ImVec2(canvasPos.x, canvasPos.y + y),
                ImVec2(canvasPos.x + windowSize.x, canvasPos.y + y),
                gridMajorGlowColor, 3.0f
            );

            drawList->AddLine(
                ImVec2(canvasPos.x, canvasPos.y + y),
                ImVec2(canvasPos.x + windowSize.x, canvasPos.y + y),
                gridMajorColor, 1.5f
            );
        }

        const float fadeWidth = 60.0f;
        const int fadeSteps = 25;
        const float stepSize = fadeWidth / fadeSteps;

        for (int i = 0; i < fadeSteps; i++) {
            float x = i * stepSize;
            float alpha = 35.0f * powf(1.0f - static_cast<float>(i) / fadeSteps, 1.5f);
            ImU32 fadeColor = IM_COL32(0, 0, 0, static_cast<int>(alpha));

            drawList->AddLine(
                ImVec2(canvasPos.x + x, canvasPos.y),
                ImVec2(canvasPos.x + x, canvasPos.y + windowSize.y),
                fadeColor, 1.0f
            );

            drawList->AddLine(
                ImVec2(canvasPos.x + windowSize.x - x, canvasPos.y),
                ImVec2(canvasPos.x + windowSize.x - x, canvasPos.y + windowSize.y),
                fadeColor, 1.0f
            );
        }

        for (int i = 0; i < fadeSteps; i++) {
            float y = i * stepSize;
            float alpha = 35.0f * powf(1.0f - static_cast<float>(i) / fadeSteps, 1.5f);
            ImU32 fadeColor = IM_COL32(0, 0, 0, static_cast<int>(alpha));

            drawList->AddLine(
                ImVec2(canvasPos.x, canvasPos.y + y),
                ImVec2(canvasPos.x + windowSize.x, canvasPos.y + y),
                fadeColor, 1.0f
            );

            drawList->AddLine(
                ImVec2(canvasPos.x, canvasPos.y + windowSize.y - y),
                ImVec2(canvasPos.x + windowSize.x, canvasPos.y + windowSize.y - y),
                fadeColor, 1.0f
            );
        }

        const float cornerFadeRadius = 120.0f;
        const int cornerFadeSteps = 20;

        for (int i = 0; i < cornerFadeSteps; i++) {
            float progress = static_cast<float>(i) / cornerFadeSteps;
            float radius = cornerFadeRadius * (1.0f - progress);
            float alpha = 15.0f * powf(1.0f - progress, 1.8f);
            ImU32 cornerFadeColor = IM_COL32(0, 0, 0, static_cast<int>(alpha));

            drawList->AddCircle(
                ImVec2(canvasPos.x, canvasPos.y),
                radius, cornerFadeColor, 0, 2.0f
            );

            drawList->AddCircle(
                ImVec2(canvasPos.x + windowSize.x, canvasPos.y),
                radius, cornerFadeColor, 0, 2.0f
            );

            drawList->AddCircle(
                ImVec2(canvasPos.x, canvasPos.y + windowSize.y),
                radius, cornerFadeColor, 0, 2.0f
            );

            drawList->AddCircle(
                ImVec2(canvasPos.x + windowSize.x, canvasPos.y + windowSize.y),
                radius, cornerFadeColor, 0, 2.0f
            );
        }

        {
            const float originX = canvasPos.x + m_state.viewPosition.x;
            const float originY = canvasPos.y + m_state.viewPosition.y;

            const bool xOnScreen = originX >= canvasPos.x && originX <= canvasPos.x + windowSize.x;
            const bool yOnScreen = originY >= canvasPos.y && originY <= canvasPos.y + windowSize.y;

            // Axe X (horizontal, rouge) — visible si l'origine Y est dans le canvas
            if (yOnScreen) {
                drawList->AddLine(
                    ImVec2(canvasPos.x, originY),
                    ImVec2(canvasPos.x + windowSize.x, originY),
                    IM_COL32(160, 50, 50, 35), 3.0f
                );
                drawList->AddLine(
                    ImVec2(canvasPos.x, originY),
                    ImVec2(canvasPos.x + windowSize.x, originY),
                    IM_COL32(200, 70, 70, 120), 1.5f
                );
            }

            // Axe Y (vertical, vert) — visible si l'origine X est dans le canvas
            if (xOnScreen) {
                drawList->AddLine(
                    ImVec2(originX, canvasPos.y),
                    ImVec2(originX, canvasPos.y + windowSize.y),
                    IM_COL32(50, 160, 50, 35), 3.0f
                );
                drawList->AddLine(
                    ImVec2(originX, canvasPos.y),
                    ImVec2(originX, canvasPos.y + windowSize.y),
                    IM_COL32(70, 200, 70, 120), 1.5f
                );
            }

            // Point central à l'intersection
            if (xOnScreen && yOnScreen) {
                const float r = 3.5f;
                drawList->AddCircleFilled(ImVec2(originX, originY), r + 3.0f, IM_COL32(0, 0, 0, 60), 16);
                drawList->AddCircleFilled(ImVec2(originX, originY), r, IM_COL32(220, 220, 220, 200), 16);
            }
        }
    }

    void NodeEditor::drawBoxSelection(ImDrawList *drawList) {
        ImVec2 mousePos = ImGui::GetMousePos();

        ImVec2 boxMin = ImVec2(
            std::min(m_state.boxSelectStart.x, mousePos.x),
            std::min(m_state.boxSelectStart.y, mousePos.y)
        );

        ImVec2 boxMax = ImVec2(
            std::max(m_state.boxSelectStart.x, mousePos.x),
            std::max(m_state.boxSelectStart.y, mousePos.y)
        );

        drawList->AddRectFilled(
            boxMin, boxMax,
            m_state.style.uiColors.selection.toImU32()
        );

        drawList->AddRect(
            boxMin, boxMax,
            IM_COL32(m_state.style.uiColors.selection.r * 255 * 1.5f,
                     m_state.style.uiColors.selection.g * 255 * 1.5f,
                     m_state.style.uiColors.selection.b * 255 * 1.5f,
                     200),
            0.0f, 0, 1.0f
        );
    }

    void NodeEditor::drawSubgraphBreadcrumbs(ImDrawList *drawList, const ImVec2 &canvasPos) {
        if (!drawList || m_state.currentSubgraphId < 0) return;

        const std::vector<int> path = getCurrentSubgraphPath();
        if (path.empty()) return;

        const ImVec2 windowSize = ImGui::GetWindowSize();
        const float breadcrumbHeight = 30.0f;
        const float itemPaddingX = 8.0f;
        const float itemPaddingY = 3.0f;
        const float separatorWidth = 14.0f;
        const ImU32 bgColor = IM_COL32(40, 44, 52, 220);
        const ImU32 textColor = IM_COL32(210, 216, 226, 255);
        const ImU32 mutedTextColor = IM_COL32(145, 150, 160, 255);
        const ImU32 hoverColor = IM_COL32(74, 84, 104, 210);
        const ImU32 currentColor = IM_COL32(62, 70, 88, 210);

        drawList->AddRectFilled(
            canvasPos,
            ImVec2(canvasPos.x + windowSize.x, canvasPos.y + breadcrumbHeight),
            bgColor
        );

        auto navigateTo = [&](int targetSubgraphId) {
            navigateToSubgraphInCurrentPath(targetSubgraphId);
        };

        float x = canvasPos.x + 10.0f;
        const float y = canvasPos.y + 5.0f;
        int itemIndex = 0;

        auto drawItem = [&](const std::string &label, int targetSubgraphId, bool current) {
            const ImVec2 textSize = ImGui::CalcTextSize(label.c_str());
            const ImVec2 itemMin(x, y);
            const ImVec2 itemMax(x + textSize.x + itemPaddingX * 2.0f,
                                 y + textSize.y + itemPaddingY * 2.0f);

            ImGui::SetCursorScreenPos(itemMin);
            ImGui::PushID(itemIndex++);
            const bool pressed = ImGui::InvisibleButton(
                "SubgraphBreadcrumbItem",
                ImVec2(itemMax.x - itemMin.x, itemMax.y - itemMin.y)
            );
            const bool hovered = ImGui::IsItemHovered();
            ImGui::PopID();

            if (current) {
                drawList->AddRectFilled(itemMin, itemMax, currentColor, 4.0f);
            } else if (hovered) {
                drawList->AddRectFilled(itemMin, itemMax, hoverColor, 4.0f);
            }

            drawList->AddText(ImVec2(itemMin.x + itemPaddingX, itemMin.y + itemPaddingY),
                              current ? textColor : mutedTextColor,
                              label.c_str());

            if (pressed && !current) {
                navigateTo(targetSubgraphId);
            }

            x = itemMax.x;
        };

        auto drawSeparator = [&]() {
            drawList->AddText(ImVec2(x + 4.0f, y + itemPaddingY), mutedTextColor, ">");
            x += separatorWidth;
        };

        drawItem("Root", -1, false);
        drawSeparator();

        for (size_t i = 0; i < path.size(); ++i) {
            const std::string label = getSubgraphDisplayName(path[i]);
            if (label.empty()) continue;

            drawItem(label, path[i], i == path.size() - 1);
            if (i + 1 < path.size()) {
                drawSeparator();
            }
        }
    }

    void NodeEditor::updateMinimapBounds() {
        m_minimapManager.setViewBounds(m_viewManager.getGraphBoundsMin(), m_viewManager.getGraphBoundsMax());
    }

    void NodeEditor::setGraphBounds(const Vec2 &min, const Vec2 &max) {
        m_viewManager.setGraphBounds(min, max);
    }

    Vec2 NodeEditor::getGraphBoundsMin() const {
        return m_viewManager.getGraphBoundsMin();
    }

    Vec2 NodeEditor::getGraphBoundsMax() const {
        return m_viewManager.getGraphBoundsMax();
    }
}
