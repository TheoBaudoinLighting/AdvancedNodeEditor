// GraphTitleManager.h
#ifndef GRAPH_TITLE_MANAGER_H
#define GRAPH_TITLE_MANAGER_H

#include "../../Core/Types/CoreTypes.h"
#include <string>
#include <functional>

namespace NodeEditorCore {

class GraphTitleManager {
public:
    enum class TitlePosition {
        TopLeft,
        TopCenter,
        TopRight,
        BottomLeft,
        BottomCenter,
        BottomRight,
        Center,
        Custom
    };

    enum class TitleStyle {
        Default,
        Minimal,
        Bordered,
        Filled,
        FilledTransparent,
        Houdini,
        Unreal
    };

    struct TitleConfig {
        std::string text;
        TitlePosition position;
        TitleStyle style;
        Color textColor;
        Color backgroundColor;
        Color borderColor;
        Vec2 customPosition;
        Vec2 padding;
        float fontSize;
        bool showSubgraphPath;
        bool adaptToZoom;
        float opacity;

        TitleConfig()
            : text("Graph")
            , position(TitlePosition::TopRight)
            , style(TitleStyle::Houdini)
            , textColor(1.0f, 1.0f, 1.0f, 1.0f)
            , backgroundColor(0.2f, 0.2f, 0.2f, 0.7f)
            , borderColor(0.4f, 0.4f, 0.4f, 1.0f)
            , customPosition(0.0f, 0.0f)
            , padding(10.0f, 5.0f)
            , fontSize(20.0f)
            , showSubgraphPath(true)
            , adaptToZoom(true)
            , opacity(1.0f)
        {}
    };

    GraphTitleManager();
    ~GraphTitleManager();

    void setTitle(const std::string& title);
    std::string getTitle() const;

    void setConfig(const TitleConfig& config);
    TitleConfig& getConfig();
    const TitleConfig& getConfig() const;

    void setViewScale(float scale);
    void setCurrentSubgraph(const std::string& name, const std::vector<std::string>& path);

    void draw(ImDrawList* drawList, const ImVec2& canvasPos, const ImVec2& canvasSize);

private:
    TitleConfig m_config;
    float m_viewScale;
    std::string m_currentSubgraph;
    std::vector<std::string> m_subgraphPath;

    ImVec2 calculateTitlePosition(const ImVec2& canvasPos, const ImVec2& canvasSize, const ImVec2& textSize);
    void drawTitleBackground(ImDrawList* drawList, const ImVec2& pos, const ImVec2& size);
};

} // namespace NodeEditorCore

#endif // GRAPH_TITLE_MANAGER_H