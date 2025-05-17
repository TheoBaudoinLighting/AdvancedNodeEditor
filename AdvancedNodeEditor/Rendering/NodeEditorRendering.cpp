#include "../NodeEditor.h"
#include "../Core/Style/InteractionMode.h"
#include "../Editor/View/MinimapManager.h"
#include <algorithm>
#include <cmath>

namespace NodeEditorCore {
    void NodeEditor::render() {
        ImGui::BeginChild("Canvas", ImVec2(0, 0), false,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse);


        ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        ImVec2 canvasSize = ImGui::GetContentRegionAvail();
        ImDrawList *drawList = ImGui::GetWindowDrawList();

        m_viewManager.updateViewTransition(ImGui::GetIO().DeltaTime);

        if (m_viewManager.isViewTransitioning()) {
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

        if (ImGui::IsItemHovered() || ImGui::IsItemActive()) {
            processInteraction();
        }

        drawGrid(drawList, canvasPos);
        drawGroups(drawList, canvasPos);
        drawConnections(drawList, canvasPos);
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

        m_titleManager.setViewScale(m_state.viewScale);

        if (m_state.currentSubgraphId >= 0) {
            std::vector<std::string> path;
            int parentId = m_state.currentSubgraphId;

            while (parentId >= 0) {
                Subgraph *sg = getSubgraph(parentId);
                if (sg) {
                    path.insert(path.begin(), sg->name);
                    parentId = sg->parentSubgraphId;
                } else {
                    break;
                }
            }

            if (!path.empty()) {
                m_titleManager.setCurrentSubgraph(path.back(), path);
            }
        }

        m_titleManager.draw(drawList, canvasPos, canvasSize);

        if (isShowingSubgraphBreadcrumbs() && m_state.currentSubgraphId >= 0) {
            drawSubgraphBreadcrumbs(drawList, canvasPos);
        }

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

    void NodeEditor::drawGrid(ImDrawList *drawList, const ImVec2 &canvasPos) {
        const float GRID_STEP_MAJOR = 64.0f * m_state.viewScale;
        const float GRID_STEP_MINOR = 16.0f * m_state.viewScale;

        float intensityMultiplier = 1.0f;
        if (m_state.currentSubgraphId >= 0) {
            int depth = getSubgraphDepth(m_state.currentSubgraphId);
            intensityMultiplier = std::max(0.5f, 1.0f - depth * 0.1f);
        }

        ImU32 gridMinorColor = IM_COL32(
            m_state.style.uiColors.grid.r * 255 * 0.7f * intensityMultiplier,
            m_state.style.uiColors.grid.g * 255 * 0.7f * intensityMultiplier,
            m_state.style.uiColors.grid.b * 255 * 0.7f * intensityMultiplier,
            m_state.style.uiColors.grid.a * 255 * 0.2f
        );

        ImU32 gridMajorColor = IM_COL32(
            m_state.style.uiColors.grid.r * 255 * intensityMultiplier,
            m_state.style.uiColors.grid.g * 255 * intensityMultiplier,
            m_state.style.uiColors.grid.b * 255 * intensityMultiplier,
            m_state.style.uiColors.grid.a * 255 * 0.4f
        );

        ImVec2 windowSize = ImGui::GetWindowSize();

        for (float x = fmodf(m_state.viewPosition.x, GRID_STEP_MINOR); x < windowSize.x; x += GRID_STEP_MINOR) {
            if (fmodf(x - fmodf(m_state.viewPosition.x, GRID_STEP_MAJOR), GRID_STEP_MAJOR) != 0.0f) {
                drawList->AddLine(
                    ImVec2(canvasPos.x + x, canvasPos.y),
                    ImVec2(canvasPos.x + x, canvasPos.y + windowSize.y),
                    gridMinorColor, 0.5f
                );
            }
        }

        for (float y = fmodf(m_state.viewPosition.y, GRID_STEP_MINOR); y < windowSize.y; y += GRID_STEP_MINOR) {
            if (fmodf(y - fmodf(m_state.viewPosition.y, GRID_STEP_MAJOR), GRID_STEP_MAJOR) != 0.0f) {
                drawList->AddLine(
                    ImVec2(canvasPos.x, canvasPos.y + y),
                    ImVec2(canvasPos.x + windowSize.x, canvasPos.y + y),
                    gridMinorColor, 0.5f
                );
            }
        }

        for (float x = fmodf(m_state.viewPosition.x, GRID_STEP_MAJOR); x < windowSize.x; x += GRID_STEP_MAJOR) {
            drawList->AddLine(
                ImVec2(canvasPos.x + x, canvasPos.y),
                ImVec2(canvasPos.x + x, canvasPos.y + windowSize.y),
                gridMajorColor, 1.0f
            );
        }

        for (float y = fmodf(m_state.viewPosition.y, GRID_STEP_MAJOR); y < windowSize.y; y += GRID_STEP_MAJOR) {
            drawList->AddLine(
                ImVec2(canvasPos.x, canvasPos.y + y),
                ImVec2(canvasPos.x + windowSize.x, canvasPos.y + y),
                gridMajorColor, 1.0f
            );
        }

        const float fadeWidth = 50.0f;
        ImU32 fadeColor = IM_COL32(0, 0, 0, 30);

        for (float i = 0; i < fadeWidth; i++) {
            float alpha = 30.0f * (1.0f - i / fadeWidth);
            ImU32 currentFadeColor = IM_COL32(0, 0, 0, static_cast<int>(alpha));

            drawList->AddLine(
                ImVec2(canvasPos.x + i, canvasPos.y),
                ImVec2(canvasPos.x + i, canvasPos.y + windowSize.y),
                currentFadeColor, 1.0f
            );

            drawList->AddLine(
                ImVec2(canvasPos.x + windowSize.x - i, canvasPos.y),
                ImVec2(canvasPos.x + windowSize.x - i, canvasPos.y + windowSize.y),
                currentFadeColor, 1.0f
            );
        }

        for (float i = 0; i < fadeWidth; i++) {
            float alpha = 30.0f * (1.0f - i / fadeWidth);
            ImU32 currentFadeColor = IM_COL32(0, 0, 0, static_cast<int>(alpha));

            drawList->AddLine(
                ImVec2(canvasPos.x, canvasPos.y + i),
                ImVec2(canvasPos.x + windowSize.x, canvasPos.y + i),
                currentFadeColor, 1.0f
            );

            drawList->AddLine(
                ImVec2(canvasPos.x, canvasPos.y + windowSize.y - i),
                ImVec2(canvasPos.x + windowSize.x, canvasPos.y + windowSize.y - i),
                currentFadeColor, 1.0f
            );
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

    bool NodeEditor::isShowingSubgraphBreadcrumbs() const {
        return m_breadcrumbManager.getConfig().showSubgraphPath;
    }

    void NodeEditor::drawSubgraphBreadcrumbs(ImDrawList *drawList, const ImVec2 &canvasPos) {
        std::vector<std::string> path;
        int parentId = m_state.currentSubgraphId;

        while (parentId >= 0) {
            Subgraph *sg = getSubgraph(parentId);
            if (sg) {
                path.insert(path.begin(), sg->name);
                parentId = sg->parentSubgraphId;
            } else {
                break;
            }
        }

        if (path.empty()) return;

        m_breadcrumbManager.setViewScale(m_state.viewScale);
        m_breadcrumbManager.setCurrentSubgraph(path.back(), path);

        auto config = m_breadcrumbManager.getConfig();
        config.position = GraphTitleManager::TitlePosition::TopCenter;
        m_breadcrumbManager.setConfig(config);

        m_breadcrumbManager.draw(drawList, canvasPos, ImGui::GetWindowSize());
    }

    void NodeEditor::updateMinimapBounds() {
        Vec2 min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        Vec2 max(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

        bool hasNodes = false;

        for (const auto &node: m_state.nodes) {
            if (!isNodeInCurrentSubgraph(node)) continue;

            min.x = std::min(min.x, node.position.x);
            min.y = std::min(min.y, node.position.y);
            max.x = std::max(max.x, node.position.x + node.size.x);
            max.y = std::max(max.y, node.position.y + node.size.y);

            hasNodes = true;
        }

        float margin = 200.0f;

        if (hasNodes) {
            min.x -= margin;
            min.y -= margin;
            max.x += margin;
            max.y += margin;
        } else {
            min = Vec2(-1000.0f, -1000.0f);
            max = Vec2(1000.0f, 1000.0f);
        }

        m_minimapManager.setViewBounds(min, max);
    }
}
