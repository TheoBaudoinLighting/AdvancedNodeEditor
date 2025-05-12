#include "NodeEditor.h"
#include <algorithm>
#include <cmath>

namespace NodeEditorCore {

void NodeEditor::render() {
    ImGui::BeginChild("Canvas", ImVec2(0, 0), false, 
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse);
    
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    drawList->AddRectFilled(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), 
                          m_state.style.uiColors.background.toImU32());
    
    ImGui::InvisibleButton("canvas", canvasSize);
    
    if (ImGui::IsItemHovered() || ImGui::IsItemActive()) {
        processInteraction();
    }
    
    drawGrid(drawList, canvasPos);
    drawGroups(drawList, canvasPos);
    drawConnections(drawList, canvasPos);
    drawNodes(drawList, canvasPos);
    
    if (m_state.boxSelecting) {
        drawBoxSelection(drawList);
        processBoxSelection(canvasPos);
    }
    
    ImGui::EndChild();
}

void NodeEditor::drawGrid(ImDrawList* drawList, const ImVec2& canvasPos) {
    const float GRID_STEP_MAJOR = 64.0f * m_state.viewScale;
    const float GRID_STEP_MINOR = 16.0f * m_state.viewScale;
    
    ImU32 gridMinorColor = IM_COL32(
        m_state.style.uiColors.grid.r * 255 * 0.7f,
        m_state.style.uiColors.grid.g * 255 * 0.7f,
        m_state.style.uiColors.grid.b * 255 * 0.7f,
        m_state.style.uiColors.grid.a * 255 * 0.2f
    );

    ImU32 gridMajorColor = IM_COL32(
        m_state.style.uiColors.grid.r * 255,
        m_state.style.uiColors.grid.g * 255,
        m_state.style.uiColors.grid.b * 255,
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

void NodeEditor::drawBoxSelection(ImDrawList* drawList) {
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

}
