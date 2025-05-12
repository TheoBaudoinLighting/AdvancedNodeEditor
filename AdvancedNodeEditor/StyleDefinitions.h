#ifndef STYLE_DEFINITIONS_H
#define STYLE_DEFINITIONS_H

#include "CoreTypes.h"
#include <unordered_map>

namespace ANE {
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