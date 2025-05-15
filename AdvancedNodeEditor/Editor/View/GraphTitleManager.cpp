#include "GraphTitleManager.h"
#include <imgui.h>
#include <algorithm>
#include <sstream>

namespace NodeEditorCore {

inline ImU32 ImLerp(ImU32 col_a, ImU32 col_b, float t)
{
    float a_r = (float)((col_a >> 0) & 0xFF);
    float a_g = (float)((col_a >> 8) & 0xFF);
    float a_b = (float)((col_a >> 16) & 0xFF);
    float a_a = (float)((col_a >> 24) & 0xFF);
    
    float b_r = (float)((col_b >> 0) & 0xFF);
    float b_g = (float)((col_b >> 8) & 0xFF);
    float b_b = (float)((col_b >> 16) & 0xFF);
    float b_a = (float)((col_b >> 24) & 0xFF);
    
    float r = a_r + (b_r - a_r) * t;
    float g = a_g + (b_g - a_g) * t;
    float b = a_b + (b_b - a_b) * t;
    float a = a_a + (b_a - a_a) * t;
    
    return IM_COL32((int)r, (int)g, (int)b, (int)a);
}

GraphTitleManager::GraphTitleManager()
    : m_viewScale(1.0f)
{
}

GraphTitleManager::~GraphTitleManager() = default;

void GraphTitleManager::setTitle(const std::string& title) {
    m_config.text = title;
}

std::string GraphTitleManager::getTitle() const {
    return m_config.text;
}

void GraphTitleManager::setConfig(const TitleConfig& config) {
    m_config = config;
}

GraphTitleManager::TitleConfig& GraphTitleManager::getConfig() {
    return m_config;
}

const GraphTitleManager::TitleConfig& GraphTitleManager::getConfig() const {
    return m_config;
}

void GraphTitleManager::setViewScale(float scale) {
    m_viewScale = scale;
}

void GraphTitleManager::setCurrentSubgraph(const std::string& name, const std::vector<std::string>& path) {
    m_currentSubgraph = name;
    m_subgraphPath = path;
}

void GraphTitleManager::draw(ImDrawList* drawList, const ImVec2& canvasPos, const ImVec2& canvasSize) {
    std::string titleText = m_config.text;

    if (m_config.showSubgraphPath && !m_currentSubgraph.empty()) {
        std::stringstream pathStr;

        if (!m_subgraphPath.empty()) {
            for (size_t i = 0; i < m_subgraphPath.size(); i++) {
                pathStr << m_subgraphPath[i];
                if (i < m_subgraphPath.size() - 1) {
                    pathStr << " > ";
                }
            }
            pathStr << " > ";
        }

        pathStr << m_currentSubgraph;
        titleText = pathStr.str();
    }

    float fontSize = m_config.fontSize;
    if (m_config.adaptToZoom) {
        fontSize *= std::max(0.5f, std::min(1.5f, m_viewScale));
    }

    ImFont* currentFont = ImGui::GetFont();
    float fontScale = fontSize / ImGui::GetFontSize();

    ImVec2 textSize = currentFont->CalcTextSizeA(
        fontSize, FLT_MAX, -1.0f, titleText.c_str());

    ImVec2 padding(m_config.padding.x, m_config.padding.y);
    ImVec2 pos = calculateTitlePosition(canvasPos, canvasSize, textSize);
    ImVec2 backgroundSize = ImVec2(textSize.x + padding.x * 2.0f, textSize.y + padding.y * 2.0f);

    drawTitleBackground(drawList, pos, backgroundSize);

    ImU32 textColor = IM_COL32(
        m_config.textColor.r * 255.0f,
        m_config.textColor.g * 255.0f,
        m_config.textColor.b * 255.0f,
        m_config.textColor.a * 255.0f * m_config.opacity
    );

    ImVec2 textPos = ImVec2(
        pos.x + padding.x,
        pos.y + padding.y
    );

    drawList->AddText(currentFont, fontSize, textPos, textColor, titleText.c_str());
}

ImVec2 GraphTitleManager::calculateTitlePosition(const ImVec2& canvasPos, const ImVec2& canvasSize, const ImVec2& textSize) {
    ImVec2 padding(m_config.padding.x, m_config.padding.y);
    ImVec2 result = canvasPos;
    float margin = 10.0f;

    switch (m_config.position) {
        case TitlePosition::TopLeft:
            result.x += margin;
            result.y += margin;
            break;

        case TitlePosition::TopCenter:
            result.x += (canvasSize.x - textSize.x - padding.x * 2.0f) * 0.5f;
            result.y += margin;
            break;

        case TitlePosition::TopRight:
            result.x += canvasSize.x - textSize.x - padding.x * 2.0f - margin;
            result.y += margin;
            break;

        case TitlePosition::BottomLeft:
            result.x += margin;
            result.y += canvasSize.y - textSize.y - padding.y * 2.0f - margin;
            break;

        case TitlePosition::BottomCenter:
            result.x += (canvasSize.x - textSize.x - padding.x * 2.0f) * 0.5f;
            result.y += canvasSize.y - textSize.y - padding.y * 2.0f - margin;
            break;

        case TitlePosition::BottomRight:
            result.x += canvasSize.x - textSize.x - padding.x * 2.0f - margin;
            result.y += canvasSize.y - textSize.y - padding.y * 2.0f - margin;
            break;

        case TitlePosition::Center:
            result.x += (canvasSize.x - textSize.x - padding.x * 2.0f) * 0.5f;
            result.y += (canvasSize.y - textSize.y - padding.y * 2.0f) * 0.5f;
            break;

        case TitlePosition::Custom:
            result.x += m_config.customPosition.x;
            result.y += m_config.customPosition.y;
            break;
    }

    return result;
}

void GraphTitleManager::drawTitleBackground(ImDrawList* drawList, const ImVec2& pos, const ImVec2& size) {
    ImU32 bgColor = IM_COL32(
        m_config.backgroundColor.r * 255.0f,
        m_config.backgroundColor.g * 255.0f,
        m_config.backgroundColor.b * 255.0f,
        m_config.backgroundColor.a * 255.0f * m_config.opacity
    );

    ImU32 borderColor = IM_COL32(
        m_config.borderColor.r * 255.0f,
        m_config.borderColor.g * 255.0f,
        m_config.borderColor.b * 255.0f,
        m_config.borderColor.a * 255.0f * m_config.opacity
    );

    float cornerRadius = 0.0f;
    float borderThickness = 1.0f;

    switch (m_config.style) {
        case TitleStyle::Default:
            cornerRadius = 3.0f;
            drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), bgColor, cornerRadius);
            drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), borderColor, cornerRadius, 0, borderThickness);
            break;

        case TitleStyle::Minimal:
            drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), bgColor, 0.0f);
            break;

        case TitleStyle::Bordered:
            cornerRadius = 0.0f;
            borderThickness = 2.0f;
            drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), borderColor, cornerRadius, 0, borderThickness);
            break;

        case TitleStyle::Filled:
            cornerRadius = 5.0f;
            drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), bgColor, cornerRadius);
            break;

        case TitleStyle::FilledTransparent:
            cornerRadius = 5.0f;
            {
                ImU32 transparentBgColor = IM_COL32(
                    m_config.backgroundColor.r * 255.0f,
                    m_config.backgroundColor.g * 255.0f,
                    m_config.backgroundColor.b * 255.0f,
                    m_config.backgroundColor.a * 255.0f * 0.7f * m_config.opacity
                );
                drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), transparentBgColor, cornerRadius);
            }
            break;

        case TitleStyle::Houdini:
            cornerRadius = 2.0f;
            {
                ImU32 houdiniGradientTop = IM_COL32(80, 80, 90, 200 * m_config.opacity);
                ImU32 houdiniGradientBottom = IM_COL32(40, 40, 50, 200 * m_config.opacity);

                const int gradientSteps = 10;
                float stepHeight = size.y / gradientSteps;

                for (int i = 0; i < gradientSteps; i++) {
                    float t = static_cast<float>(i) / (gradientSteps - 1);
                    ImU32 color = ImLerp(houdiniGradientTop, houdiniGradientBottom, t);

                    ImVec2 stepPos = ImVec2(pos.x, pos.y + i * stepHeight);
                    ImVec2 stepSize = ImVec2(pos.x + size.x, pos.y + (i + 1) * stepHeight);

                    ImDrawFlags flags = ImDrawFlags_RoundCornersNone;
                    if (i == 0) flags = ImDrawFlags_RoundCornersTop;
                    else if (i == gradientSteps - 1) flags = ImDrawFlags_RoundCornersBottom;

                    drawList->AddRectFilled(stepPos, stepSize, color, cornerRadius, flags);
                }

                drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), borderColor, cornerRadius, 0, 1.0f);
            }
            break;

        case TitleStyle::Unreal:
            {
                ImU32 unrealColor = IM_COL32(50, 50, 60, 220 * m_config.opacity);
                ImU32 unrealHeaderColor = IM_COL32(30, 90, 130, 255 * m_config.opacity);
                float headerHeight = size.y * 0.3f;

                drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), unrealColor, 0.0f);

                drawList->AddRectFilled(
                    pos,
                    ImVec2(pos.x + size.x, pos.y + headerHeight),
                    unrealHeaderColor, 0.0f);

                ImU32 reflectionColor = IM_COL32(255, 255, 255, 20 * m_config.opacity);
                drawList->AddLine(
                    ImVec2(pos.x, pos.y + headerHeight + 1),
                    ImVec2(pos.x + size.x, pos.y + headerHeight + 1),
                    reflectionColor, 1.0f);
            }
            break;
    }
}

} // namespace NodeEditorCore