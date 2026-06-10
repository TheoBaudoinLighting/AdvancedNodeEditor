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

        // Base values at 96 DPI (scale = 1.0). Multiply by DPI scale before use.
        float gridSpacing;
        float nodeRounding;
        float pinRadius;
        float connectionThickness;
        Vec2  defaultNodeSize;   // canvas units for a new node

        NodeEditorStyle();

        // Returns a copy of this style with all metric fields scaled by `dpiScale`.
        NodeEditorStyle scaledForDpi(float dpiScale) const {
            NodeEditorStyle s = *this;
            s.gridSpacing        = gridSpacing        * dpiScale;
            s.nodeRounding       = nodeRounding       * dpiScale;
            s.pinRadius          = pinRadius          * dpiScale;
            s.connectionThickness= connectionThickness* dpiScale;
            s.defaultNodeSize    = Vec2(defaultNodeSize.x * dpiScale,
                                       defaultNodeSize.y * dpiScale);
            return s;
        }
    };
}

#endif
