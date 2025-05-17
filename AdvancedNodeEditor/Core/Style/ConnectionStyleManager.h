#ifndef CONNECTION_STYLE_MANAGER_H
#define CONNECTION_STYLE_MANAGER_H

#include "../../Core/Types/CoreTypes.h"
#include <imgui.h>
#include <functional>
#include <unordered_map>
#include <string>
#include <memory>

namespace NodeEditorCore {
    class NodeBoundingBoxManager;

    class ConnectionStyleManager {
    public:
        enum class ConnectionStyle {
            Bezier,
            StraightLine,
            AngleLine,
            MetroLine,
            Custom
        };

        struct ConnectionConfig {
            ConnectionStyle style;
            float thickness;
            Color startColor;
            Color endColor;
            Color selectedColor;
            Color hoveredColor;
            float curveTension;
            bool useGradient;
            bool drawShadow;
            bool drawHighlight;
            bool avoidNodes;
            float cornerRadius;

            ConnectionConfig()
                : style(ConnectionStyle::Bezier)
                  , thickness(2.5f)
                  , startColor(0.3f, 0.6f, 0.9f, 1.0f)
                  , endColor(0.3f, 0.6f, 0.9f, 1.0f)
                  , selectedColor(1.0f, 0.6f, 0.2f, 1.0f)
                  , hoveredColor(0.7f, 0.8f, 1.0f, 1.0f)
                  , curveTension(0.5f)
                  , useGradient(true)
                  , drawShadow(true)
                  , drawHighlight(true)
                  , avoidNodes(false)
                  , cornerRadius(5.0f) {
            }
        };

        ConnectionStyleManager();

        ~ConnectionStyleManager();

        void setConfig(const ConnectionConfig &config);

        ConnectionConfig &getConfig();

        const ConnectionConfig &getConfig() const;

        void setDefaultStyle(ConnectionStyle style);

        ConnectionStyle getDefaultStyle() const;

        void addCustomConnectionDrawer(const std::string &name,
                                       std::function<void(ImDrawList *, const ImVec2 &, const ImVec2 &,
                                                          bool, bool, const Color &, const Color &, float)> drawer);

        void drawConnection(ImDrawList *drawList,
                            const ImVec2 &startPos, const ImVec2 &endPos,
                            bool isStartInput, bool isEndInput,
                            bool selected, bool hovered,
                            const Color &startCol, const Color &endCol,
                            float scale = 1.0f);

        void setBoundingBoxFunction(std::function<bool(ImVec2, ImVec2)> func);

        void setBoundingBoxManager(std::shared_ptr<NodeBoundingBoxManager> manager);

    private:
        ConnectionConfig m_config;
        std::unordered_map<std::string, std::function<void(ImDrawList *, const ImVec2 &, const ImVec2 &,
                                                           bool, bool, const Color &, const Color &,
                                                           float)> > m_customDrawers;

        std::function<bool(ImVec2, ImVec2)> m_boundingBoxCheck;

        void drawBezierConnection(ImDrawList *drawList, const ImVec2 &start, const ImVec2 &end,
                                  bool isStartInput, bool isEndInput,
                                  bool selected, bool hovered, const Color &startCol, const Color &endCol, float scale);

        void drawStraightConnection(ImDrawList *drawList, const ImVec2 &start, const ImVec2 &end,
                                    bool selected, bool hovered, const Color &startCol, const Color &endCol,
                                    float scale);

        void drawAngleConnection(ImDrawList *drawList, const ImVec2 &start, const ImVec2 &end,
                                 bool selected, bool hovered, const Color &startCol, const Color &endCol, float scale);

        void drawMetroConnection(ImDrawList *drawList, const ImVec2 &start, const ImVec2 &end,
                                 bool selected, bool hovered, const Color &startCol, const Color &endCol, float scale);

        ImVec2 findPathAroundNodes(const ImVec2 &start, const ImVec2 &end);

        bool hasNodeIntersection(const ImVec2 &start, const ImVec2 &end);

        std::shared_ptr<NodeBoundingBoxManager> m_boundingBoxManager;
    };
} // namespace NodeEditorCore

#endif // CONNECTION_STYLE_MANAGER_H
