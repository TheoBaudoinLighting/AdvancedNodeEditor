#include "Conversions.h"

#include "../Style/StyleDefinitions.h"

namespace NodeEditorCore {
    NodeEditorStyle convertToInternalStyle(const EditorStyle &style) {
        NodeEditorStyle internalStyle;

        internalStyle.uiColors.background = Color(style.backgroundColor.r, style.backgroundColor.g,
                                                  style.backgroundColor.b, style.backgroundColor.a);
        internalStyle.uiColors.grid = Color(style.gridColor.r, style.gridColor.g, style.gridColor.b, style.gridColor.a);
        internalStyle.uiColors.selection = Color(style.selectionColor.r, style.selectionColor.g, style.selectionColor.b,
                                                 style.selectionColor.a);

        for (const auto &pair: style.nodeStyles) {
            internal::NodeColors nodeColors;
            nodeColors.base = Color(pair.second.baseColor.r, pair.second.baseColor.g, pair.second.baseColor.b,
                                    pair.second.baseColor.a);
            nodeColors.header = Color(pair.second.headerColor.r, pair.second.headerColor.g, pair.second.headerColor.b,
                                      pair.second.headerColor.a);
            nodeColors.accent = Color(pair.second.accentColor.r, pair.second.accentColor.g, pair.second.accentColor.b,
                                      pair.second.accentColor.a);
            nodeColors.border = Color(pair.second.borderColor.r, pair.second.borderColor.g, pair.second.borderColor.b,
                                      pair.second.borderColor.a);
            nodeColors.selected = Color(pair.second.selectedColor.r, pair.second.selectedColor.g,
                                        pair.second.selectedColor.b, pair.second.selectedColor.a);
            nodeColors.hovered = Color(pair.second.hoveredColor.r, pair.second.hoveredColor.g,
                                       pair.second.hoveredColor.b, pair.second.hoveredColor.a);
            nodeColors.glow = Color(pair.second.glowColor.r, pair.second.glowColor.g, pair.second.glowColor.b,
                                    pair.second.glowColor.a);

            internalStyle.nodeColors[pair.first] = nodeColors;
        }

        for (const auto &pair: style.pinStyles) {
            internal::PinColors pinColors;
            pinColors.base = Color(pair.second.color.r, pair.second.color.g, pair.second.color.b, pair.second.color.a);
            pinColors.hover = Color(pair.second.hoverColor.r, pair.second.hoverColor.g, pair.second.hoverColor.b,
                                    pair.second.hoverColor.a);
            pinColors.connected = Color(pair.second.connectedColor.r, pair.second.connectedColor.g,
                                        pair.second.connectedColor.b, pair.second.connectedColor.a);

            internalStyle.pinColors[pair.first] = pinColors;
        }

        internalStyle.connectionColors.baseColor = Color(style.connectionStyle.baseColor.r,
                                                         style.connectionStyle.baseColor.g,
                                                         style.connectionStyle.baseColor.b,
                                                         style.connectionStyle.baseColor.a);
        internalStyle.connectionColors.selectedColor = Color(style.connectionStyle.selectedColor.r,
                                                             style.connectionStyle.selectedColor.g,
                                                             style.connectionStyle.selectedColor.b,
                                                             style.connectionStyle.selectedColor.a);
        internalStyle.connectionColors.hoveredColor = Color(style.connectionStyle.hoveredColor.r,
                                                            style.connectionStyle.hoveredColor.g,
                                                            style.connectionStyle.hoveredColor.b,
                                                            style.connectionStyle.hoveredColor.a);
        internalStyle.connectionColors.validColor = Color(style.connectionStyle.validColor.r,
                                                          style.connectionStyle.validColor.g,
                                                          style.connectionStyle.validColor.b,
                                                          style.connectionStyle.validColor.a);
        internalStyle.connectionColors.invalidColor = Color(style.connectionStyle.invalidColor.r,
                                                            style.connectionStyle.invalidColor.g,
                                                            style.connectionStyle.invalidColor.b,
                                                            style.connectionStyle.invalidColor.a);

        internalStyle.gridSpacing = style.gridSpacing;
        internalStyle.nodeRounding = style.nodeRounding;
        internalStyle.pinRadius = style.pinRadius;
        internalStyle.connectionThickness = style.connectionThickness;

        return internalStyle;
    }

    EditorStyle convertToAPIStyle(const NodeEditorStyle &internalStyle) {
        EditorStyle style;

        style.backgroundColor = Color(internalStyle.uiColors.background.r, internalStyle.uiColors.background.g,
                                      internalStyle.uiColors.background.b, internalStyle.uiColors.background.a);
        style.gridColor = Color(internalStyle.uiColors.grid.r, internalStyle.uiColors.grid.g,
                                internalStyle.uiColors.grid.b, internalStyle.uiColors.grid.a);
        style.selectionColor = Color(internalStyle.uiColors.selection.r, internalStyle.uiColors.selection.g,
                                     internalStyle.uiColors.selection.b, internalStyle.uiColors.selection.a);

        for (const auto &pair: internalStyle.nodeColors) {
            NodeStyle nodeStyle;
            nodeStyle.baseColor = Color(pair.second.base.r, pair.second.base.g, pair.second.base.b, pair.second.base.a);
            nodeStyle.headerColor = Color(pair.second.header.r, pair.second.header.g, pair.second.header.b,
                                          pair.second.header.a);
            nodeStyle.accentColor = Color(pair.second.accent.r, pair.second.accent.g, pair.second.accent.b,
                                          pair.second.accent.a);
            nodeStyle.borderColor = Color(pair.second.border.r, pair.second.border.g, pair.second.border.b,
                                          pair.second.border.a);
            nodeStyle.selectedColor = Color(pair.second.selected.r, pair.second.selected.g, pair.second.selected.b,
                                            pair.second.selected.a);
            nodeStyle.hoveredColor = Color(pair.second.hovered.r, pair.second.hovered.g, pair.second.hovered.b,
                                           pair.second.hovered.a);
            nodeStyle.glowColor = Color(pair.second.glow.r, pair.second.glow.g, pair.second.glow.b, pair.second.glow.a);

            style.nodeStyles[pair.first] = nodeStyle;
        }

        for (const auto &pair: internalStyle.pinColors) {
            PinStyle pinStyle;
            pinStyle.color = Color(pair.second.base.r, pair.second.base.g, pair.second.base.b, pair.second.base.a);
            pinStyle.hoverColor = Color(pair.second.hover.r, pair.second.hover.g, pair.second.hover.b,
                                        pair.second.hover.a);
            pinStyle.connectedColor = Color(pair.second.connected.r, pair.second.connected.g, pair.second.connected.b,
                                            pair.second.connected.a);

            style.pinStyles[pair.first] = pinStyle;
        }

        style.connectionStyle.baseColor = Color(internalStyle.connectionColors.baseColor.r,
                                                internalStyle.connectionColors.baseColor.g,
                                                internalStyle.connectionColors.baseColor.b,
                                                internalStyle.connectionColors.baseColor.a);
        style.connectionStyle.selectedColor = Color(internalStyle.connectionColors.selectedColor.r,
                                                    internalStyle.connectionColors.selectedColor.g,
                                                    internalStyle.connectionColors.selectedColor.b,
                                                    internalStyle.connectionColors.selectedColor.a);
        style.connectionStyle.hoveredColor = Color(internalStyle.connectionColors.hoveredColor.r,
                                                   internalStyle.connectionColors.hoveredColor.g,
                                                   internalStyle.connectionColors.hoveredColor.b,
                                                   internalStyle.connectionColors.hoveredColor.a);
        style.connectionStyle.validColor = Color(internalStyle.connectionColors.validColor.r,
                                                 internalStyle.connectionColors.validColor.g,
                                                 internalStyle.connectionColors.validColor.b,
                                                 internalStyle.connectionColors.validColor.a);
        style.connectionStyle.invalidColor = Color(internalStyle.connectionColors.invalidColor.r,
                                                   internalStyle.connectionColors.invalidColor.g,
                                                   internalStyle.connectionColors.invalidColor.b,
                                                   internalStyle.connectionColors.invalidColor.a);

        style.gridSpacing = internalStyle.gridSpacing;
        style.nodeRounding = internalStyle.nodeRounding;
        style.pinRadius = internalStyle.pinRadius;
        style.connectionThickness = internalStyle.connectionThickness;

        return style;
    }
}
