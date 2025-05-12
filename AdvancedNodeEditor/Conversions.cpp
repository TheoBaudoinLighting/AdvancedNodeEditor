// Conversions.cpp
#include "Conversions.h"

#include "StyleDefinitions.h"

namespace NodeEditorCore {

NodeEditorStyle convertToInternalStyle(const ANE::EditorStyle& style) {
    NodeEditorStyle internalStyle;

    internalStyle.uiColors.background = Color(style.backgroundColor.r, style.backgroundColor.g, style.backgroundColor.b, style.backgroundColor.a);
    internalStyle.uiColors.grid = Color(style.gridColor.r, style.gridColor.g, style.gridColor.b, style.gridColor.a);
    internalStyle.uiColors.selection = Color(style.selectionColor.r, style.selectionColor.g, style.selectionColor.b, style.selectionColor.a);

    for (const auto& pair : style.nodeStyles) {
        internal::NodeColors nodeColors;
        nodeColors.baseColor = Color(pair.second.baseColor.r, pair.second.baseColor.g, pair.second.baseColor.b, pair.second.baseColor.a);
        nodeColors.headerColor = Color(pair.second.headerColor.r, pair.second.headerColor.g, pair.second.headerColor.b, pair.second.headerColor.a);
        nodeColors.accentColor = Color(pair.second.accentColor.r, pair.second.accentColor.g, pair.second.accentColor.b, pair.second.accentColor.a);
        nodeColors.borderColor = Color(pair.second.borderColor.r, pair.second.borderColor.g, pair.second.borderColor.b, pair.second.borderColor.a);
        nodeColors.selectedColor = Color(pair.second.selectedColor.r, pair.second.selectedColor.g, pair.second.selectedColor.b, pair.second.selectedColor.a);
        nodeColors.hoveredColor = Color(pair.second.hoveredColor.r, pair.second.hoveredColor.g, pair.second.hoveredColor.b, pair.second.hoveredColor.a);
        nodeColors.glowColor = Color(pair.second.glowColor.r, pair.second.glowColor.g, pair.second.glowColor.b, pair.second.glowColor.a);

        internalStyle.nodeColors[pair.first] = nodeColors;
    }

    for (const auto& pair : style.pinStyles) {
        internal::PinColors pinColors;
        pinColors.color = Color(pair.second.color.r, pair.second.color.g, pair.second.color.b, pair.second.color.a);
        pinColors.hoverColor = Color(pair.second.hoverColor.r, pair.second.hoverColor.g, pair.second.hoverColor.b, pair.second.hoverColor.a);
        pinColors.connectedColor = Color(pair.second.connectedColor.r, pair.second.connectedColor.g, pair.second.connectedColor.b, pair.second.connectedColor.a);

        internalStyle.pinColors[pair.first] = pinColors;
    }

    internalStyle.connectionColors.baseColor = Color(style.connectionStyle.baseColor.r, style.connectionStyle.baseColor.g, style.connectionStyle.baseColor.b, style.connectionStyle.baseColor.a);
    internalStyle.connectionColors.selectedColor = Color(style.connectionStyle.selectedColor.r, style.connectionStyle.selectedColor.g, style.connectionStyle.selectedColor.b, style.connectionStyle.selectedColor.a);
    internalStyle.connectionColors.hoveredColor = Color(style.connectionStyle.hoveredColor.r, style.connectionStyle.hoveredColor.g, style.connectionStyle.hoveredColor.b, style.connectionStyle.hoveredColor.a);
    internalStyle.connectionColors.validColor = Color(style.connectionStyle.validColor.r, style.connectionStyle.validColor.g, style.connectionStyle.validColor.b, style.connectionStyle.validColor.a);
    internalStyle.connectionColors.invalidColor = Color(style.connectionStyle.invalidColor.r, style.connectionStyle.invalidColor.g, style.connectionStyle.invalidColor.b, style.connectionStyle.invalidColor.a);

    internalStyle.gridSpacing = style.gridSpacing;
    internalStyle.nodeRounding = style.nodeRounding;
    internalStyle.pinRadius = style.pinRadius;
    internalStyle.connectionThickness = style.connectionThickness;

    return internalStyle;
}

ANE::EditorStyle convertToAPIStyle(const NodeEditorStyle& internalStyle) {
    ANE::EditorStyle style;

    style.backgroundColor = ANE::Color(internalStyle.uiColors.background.r, internalStyle.uiColors.background.g, internalStyle.uiColors.background.b, internalStyle.uiColors.background.a);
    style.gridColor = ANE::Color(internalStyle.uiColors.grid.r, internalStyle.uiColors.grid.g, internalStyle.uiColors.grid.b, internalStyle.uiColors.grid.a);
    style.selectionColor = ANE::Color(internalStyle.uiColors.selection.r, internalStyle.uiColors.selection.g, internalStyle.uiColors.selection.b, internalStyle.uiColors.selection.a);

    for (const auto& pair : internalStyle.nodeColors) {
        ANE::NodeStyle nodeStyle;
        nodeStyle.baseColor = ANE::Color(pair.second.baseColor.r, pair.second.baseColor.g, pair.second.baseColor.b, pair.second.baseColor.a);
        nodeStyle.headerColor = ANE::Color(pair.second.headerColor.r, pair.second.headerColor.g, pair.second.headerColor.b, pair.second.headerColor.a);
        nodeStyle.accentColor = ANE::Color(pair.second.accentColor.r, pair.second.accentColor.g, pair.second.accentColor.b, pair.second.accentColor.a);
        nodeStyle.borderColor = ANE::Color(pair.second.borderColor.r, pair.second.borderColor.g, pair.second.borderColor.b, pair.second.borderColor.a);
        nodeStyle.selectedColor = ANE::Color(pair.second.selectedColor.r, pair.second.selectedColor.g, pair.second.selectedColor.b, pair.second.selectedColor.a);
        nodeStyle.hoveredColor = ANE::Color(pair.second.hoveredColor.r, pair.second.hoveredColor.g, pair.second.hoveredColor.b, pair.second.hoveredColor.a);
        nodeStyle.glowColor = ANE::Color(pair.second.glowColor.r, pair.second.glowColor.g, pair.second.glowColor.b, pair.second.glowColor.a);

        style.nodeStyles[pair.first] = nodeStyle;
    }

    for (const auto& pair : internalStyle.pinColors) {
        ANE::PinStyle pinStyle;
        pinStyle.color = ANE::Color(pair.second.color.r, pair.second.color.g, pair.second.color.b, pair.second.color.a);
        pinStyle.hoverColor = ANE::Color(pair.second.hoverColor.r, pair.second.hoverColor.g, pair.second.hoverColor.b, pair.second.hoverColor.a);
        pinStyle.connectedColor = ANE::Color(pair.second.connectedColor.r, pair.second.connectedColor.g, pair.second.connectedColor.b, pair.second.connectedColor.a);

        style.pinStyles[pair.first] = pinStyle;
    }

    style.connectionStyle.baseColor = ANE::Color(internalStyle.connectionColors.baseColor.r, internalStyle.connectionColors.baseColor.g, internalStyle.connectionColors.baseColor.b, internalStyle.connectionColors.baseColor.a);
    style.connectionStyle.selectedColor = ANE::Color(internalStyle.connectionColors.selectedColor.r, internalStyle.connectionColors.selectedColor.g, internalStyle.connectionColors.selectedColor.b, internalStyle.connectionColors.selectedColor.a);
    style.connectionStyle.hoveredColor = ANE::Color(internalStyle.connectionColors.hoveredColor.r, internalStyle.connectionColors.hoveredColor.g, internalStyle.connectionColors.hoveredColor.b, internalStyle.connectionColors.hoveredColor.a);
    style.connectionStyle.validColor = ANE::Color(internalStyle.connectionColors.validColor.r, internalStyle.connectionColors.validColor.g, internalStyle.connectionColors.validColor.b, internalStyle.connectionColors.validColor.a);
    style.connectionStyle.invalidColor = ANE::Color(internalStyle.connectionColors.invalidColor.r, internalStyle.connectionColors.invalidColor.g, internalStyle.connectionColors.invalidColor.b, internalStyle.connectionColors.invalidColor.a);

    style.gridSpacing = internalStyle.gridSpacing;
    style.nodeRounding = internalStyle.nodeRounding;
    style.pinRadius = internalStyle.pinRadius;
    style.connectionThickness = internalStyle.connectionThickness;

    return style;
}

}
