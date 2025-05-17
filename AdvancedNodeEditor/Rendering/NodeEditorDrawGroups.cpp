#include "../NodeEditor.h"
#include <algorithm>

namespace NodeEditorCore {
    void NodeEditor::drawGroups(ImDrawList *drawList, const ImVec2 &canvasPos) {
        std::vector<Group> visibleGroups;
        int currentSubgraphId = m_state.currentSubgraphId;

        for (const auto &group: m_state.groups) {
            if ((currentSubgraphId == -1 && group.getSubgraphId() == -1) ||
                (currentSubgraphId >= 0 && group.getSubgraphId() == currentSubgraphId)) {
                visibleGroups.push_back(group);
            }
        }

        for (const auto &group: visibleGroups) {
            ImVec2 groupPos = canvasToScreen(group.position).toImVec2();
            ImVec2 groupSize = Vec2(group.size.x * m_state.viewScale, group.size.y * m_state.viewScale).toImVec2();

            ImU32 baseColor = IM_COL32(60, 60, 70, 200);
            ImU32 borderColor = IM_COL32(80, 80, 90, 200);
            ImU32 titleColor = IM_COL32(220, 220, 240, 255);
            float titleHeight = 20.0f * m_state.viewScale;

            drawList->AddRectFilled(
                groupPos,
                ImVec2(groupPos.x + groupSize.x, groupPos.y + groupSize.y),
                baseColor, 4.0f
            );

            drawList->AddRect(
                groupPos,
                ImVec2(groupPos.x + groupSize.x, groupPos.y + groupSize.y),
                borderColor, 4.0f, 0, 1.5f
            );

            drawList->AddRectFilled(
                groupPos,
                ImVec2(groupPos.x + groupSize.x, groupPos.y + titleHeight),
                IM_COL32(50, 50, 60, 230), 4.0f, ImDrawFlags_RoundCornersTop
            );

            ImVec2 textSize = ImGui::CalcTextSize(group.name.c_str());
            drawList->AddText(
                ImVec2(groupPos.x + (groupSize.x - textSize.x) * 0.5f, groupPos.y + (titleHeight - textSize.y) * 0.5f),
                titleColor, group.name.c_str()
            );
        }
    }
}
