#ifndef STYLE_DEFINITIONS_H
#define STYLE_DEFINITIONS_H

#include "../Types/CoreTypes.h"
#include <unordered_map>

namespace NodeEditorCore {
    struct NodeStyle {
        Color baseColor;
        Color headerColor;
        Color accentColor;
        Color borderColor;
        Color selectedColor;
        Color hoveredColor;
        Color glowColor;
    };

    struct PinStyle {
        Color color;
        Color hoverColor;
        Color connectedColor;
    };

    struct ConnectionStyle {
        Color baseColor;
        Color selectedColor;
        Color hoveredColor;
        Color validColor;
        Color invalidColor;
    };

    struct EditorStyle {
        Color backgroundColor;
        Color gridColor;
        Color selectionColor;

        std::unordered_map<std::string, NodeStyle> nodeStyles;
        std::unordered_map<std::string, PinStyle> pinStyles;
        ConnectionStyle connectionStyle;

        float gridSpacing;
        float nodeRounding;
        float pinRadius;
        float connectionThickness;

        EditorStyle();
    };
}

namespace NodeEditorCore {
    namespace internal {
        struct UIColors {
            Color background;
            Color grid;
            Color selection;
            Color menuBar;
            Color tooltipBg;
            Color tooltipText;
        };

        struct NodeColors {
            Color base;
            Color header;
            Color accent;
            Color border;
            Color selected;
            Color hovered;
            Color glow;
        };

        struct PinColors {
            Color base;
            Color hover;
            Color connected;
        };

        struct ConnectionColors {
            Color baseColor;
            Color selectedColor;
            Color hoveredColor;
            Color validColor;
            Color invalidColor;
        };
    }

    struct NodeEditorStyle {
        internal::UIColors uiColors;
        std::unordered_map<std::string, internal::NodeColors> nodeColors;
        std::unordered_map<std::string, internal::PinColors> pinColors;
        internal::ConnectionColors connectionColors;

        float gridSpacing;
        float nodeRounding;
        float pinRadius;
        float connectionThickness;

        NodeEditorStyle();
    };
}

#endif
