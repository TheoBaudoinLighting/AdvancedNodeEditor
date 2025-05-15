#include "MinimapManager.h"
#include <algorithm>
#include <cmath>

#include "imgui_internal.h"

namespace NodeEditorCore {

MinimapManager::MinimapManager()
    : m_viewMin(-1000.0f, -1000.0f)
    , m_viewMax(1000.0f, 1000.0f)
    , m_viewPosition(0.0f, 0.0f)
    , m_viewScale(1.0f)
    , m_dragging(false)
    , m_dragStart(0.0f, 0.0f)
{
}

MinimapManager::~MinimapManager() = default;

void MinimapManager::setConfig(const MinimapConfig& config) {
    m_config = config;
}

MinimapManager::MinimapConfig& MinimapManager::getConfig() {
    return m_config;
}

const MinimapManager::MinimapConfig& MinimapManager::getConfig() const {
    return m_config;
}

void MinimapManager::setViewBounds(const Vec2& min, const Vec2& max) {
    m_viewMin = min;
    m_viewMax = max;
}

void MinimapManager::setViewPosition(const Vec2& position) {
    m_viewPosition = position;
}

void MinimapManager::setViewScale(float scale) {
    m_viewScale = scale;
}

void MinimapManager::setNodePositionProvider(NodePositionProvider provider) {
    m_nodePositionProvider = provider;
}

void MinimapManager::setViewportChangeCallback(ViewportChangeCallback callback) {
    m_viewportChangeCallback = callback;
}

ImVec2 MinimapManager::graphToMinimap(const Vec2& graphPos, const ImVec2& minimapPos, const ImVec2& minimapSize) const {
    Vec2 graphExtent = m_viewMax - m_viewMin;
    float graphWidth = graphExtent.x;
    float graphHeight = graphExtent.y;

    float x = minimapPos.x + m_config.padding + (graphPos.x - m_viewMin.x) / graphWidth * (minimapSize.x - 2 * m_config.padding);
    float y = minimapPos.y + m_config.padding + (graphPos.y - m_viewMin.y) / graphHeight * (minimapSize.y - 2 * m_config.padding);

    return ImVec2(x, y);
}

Vec2 MinimapManager::minimapToGraph(const ImVec2& minimapPos, const ImVec2& mapPos, const ImVec2& mapSize) const {
    float relX = minimapPos.x - mapPos.x - m_config.padding;
    float relY = minimapPos.y - mapPos.y - m_config.padding;

    Vec2 graphExtent = m_viewMax - m_viewMin;
    float graphWidth = graphExtent.x;
    float graphHeight = graphExtent.y;

    float x = m_viewMin.x + relX / (mapSize.x - 2 * m_config.padding) * graphWidth;
    float y = m_viewMin.y + relY / (mapSize.y - 2 * m_config.padding) * graphHeight;

    return Vec2(x, y);
}

bool MinimapManager::handleInteraction(const ImVec2& minimapPos, const ImVec2& minimapSize) {
    if (!m_config.interactable) return false;

    ImVec2 mousePos = ImGui::GetIO().MousePos;
    bool mouseInMinimap =
        mousePos.x >= minimapPos.x &&
        mousePos.x <= minimapPos.x + minimapSize.x &&
        mousePos.y >= minimapPos.y &&
        mousePos.y <= minimapPos.y + minimapSize.y;

    if (mouseInMinimap && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        m_dragging = true;
        m_dragStart = Vec2(mousePos.x, mousePos.y);

        if (m_viewportChangeCallback) {
            Vec2 graphPos = minimapToGraph(mousePos, minimapPos, minimapSize);
            m_viewportChangeCallback(graphPos);
        }

        return true;
    }

    if (m_dragging && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        if (m_viewportChangeCallback) {
            Vec2 graphPos = minimapToGraph(mousePos, minimapPos, minimapSize);
            m_viewportChangeCallback(graphPos);
        }
        return true;
    }

    if (m_dragging && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        m_dragging = false;
        return true;
    }

    return false;
}

bool MinimapManager::draw(ImDrawList* drawList, const ImVec2& canvasPos, const ImVec2& canvasSize) {
    if (!m_config.interactable) return false;

    ImVec2 minimapPos = ImVec2(canvasPos.x + m_config.position.x, canvasPos.y + m_config.position.y);
    ImVec2 minimapSize = ImVec2(m_config.size.x, m_config.size.y);

    drawList->AddRectFilled(
        minimapPos,
        ImVec2(minimapPos.x + minimapSize.x, minimapPos.y + minimapSize.y),
        ImGui::ColorConvertFloat4ToU32(ImVec4(m_config.backgroundColor.r, m_config.backgroundColor.g, 
            m_config.backgroundColor.b, m_config.backgroundColor.a * m_config.opacity)),
        4.0f
    );

    if (m_config.showBorder) {
        drawList->AddRect(
            minimapPos,
            ImVec2(minimapPos.x + minimapSize.x, minimapPos.y + minimapSize.y),
            ImGui::ColorConvertFloat4ToU32(ImVec4(m_config.borderColor.r, m_config.borderColor.g, 
                m_config.borderColor.b, m_config.borderColor.a * m_config.opacity)),
            4.0f,
            ImDrawFlags_None,
            1.5f
        );
    }

    if (m_nodePositionProvider) {
        auto nodePositions = m_nodePositionProvider();
        ImU32 nodeColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.7f, 0.7f, 0.7f, 0.7f * m_config.opacity));

        for (const auto& nodePair : nodePositions) {
            ImVec2 nodePos = graphToMinimap(nodePair.first, minimapPos, minimapSize);
            Vec2 nodeSize = nodePair.second;

            float scaleX = minimapSize.x / (m_viewMax.x - m_viewMin.x);
            float scaleY = minimapSize.y / (m_viewMax.y - m_viewMin.y);
            float scaledWidth = nodeSize.x * scaleX;
            float scaledHeight = nodeSize.y * scaleY;

            drawList->AddRectFilled(
                nodePos,
                ImVec2(nodePos.x + scaledWidth, nodePos.y + scaledHeight),
                nodeColor
            );
        }
    }

    ImVec2 viewSize = canvasSize;
    float viewRatioX = viewSize.x / m_viewScale;
    float viewRatioY = viewSize.y / m_viewScale;

    ImVec2 viewRectMin = graphToMinimap(Vec2(
        -m_viewPosition.x / m_viewScale,
        -m_viewPosition.y / m_viewScale
    ), minimapPos, minimapSize);

    ImVec2 viewRectMax = graphToMinimap(Vec2(
        (-m_viewPosition.x + viewSize.x) / m_viewScale,
        (-m_viewPosition.y + viewSize.y) / m_viewScale
    ), minimapPos, minimapSize);

    viewRectMin.x = ImClamp(viewRectMin.x, minimapPos.x, minimapPos.x + minimapSize.x);
    viewRectMin.y = ImClamp(viewRectMin.y, minimapPos.y, minimapPos.y + minimapSize.y);
    viewRectMax.x = ImClamp(viewRectMax.x, minimapPos.x, minimapPos.x + minimapSize.x);
    viewRectMax.y = ImClamp(viewRectMax.y, minimapPos.y, minimapPos.y + minimapSize.y);

    drawList->AddRect(
        viewRectMin,
        viewRectMax,
        ImGui::ColorConvertFloat4ToU32(ImVec4(m_config.viewportColor.r, m_config.viewportColor.g,
            m_config.viewportColor.b, m_config.viewportColor.a * m_config.opacity)),
        0.0f,
        ImDrawFlags_None,
        2.0f
    );

    bool interacted = false;
    if (m_config.interactable) {
        interacted = handleInteraction(minimapPos, minimapSize);
    }

    return interacted;
}

} // namespace NodeEditorCore