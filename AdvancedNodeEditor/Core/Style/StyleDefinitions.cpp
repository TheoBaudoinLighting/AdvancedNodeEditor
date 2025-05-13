#include "StyleDefinitions.h"

namespace ANE {

EditorStyle::EditorStyle() {
    backgroundColor = Color(0.10f, 0.11f, 0.12f, 1.00f);
    gridColor = Color(0.16f, 0.17f, 0.18f, 0.50f);
    selectionColor = Color(0.00f, 0.44f, 0.80f, 0.30f);
    
    gridSpacing = 20.0f;
    nodeRounding = 5.0f;
    pinRadius = 4.0f;
    connectionThickness = 2.5f;
}

}

namespace NodeEditorCore {
    NodeEditorStyle::NodeEditorStyle()
        : gridSpacing(16.0f)
        , nodeRounding(4.0f)
        , pinRadius(3.3f)
        , connectionThickness(2.5f) {
        uiColors.background = Color(0.15f, 0.15f, 0.17f, 1.00f);
        uiColors.grid = Color(0.23f, 0.23f, 0.26f, 0.314f);
        uiColors.selection = Color(0.70f, 0.80f, 1.00f, 0.392f);
        uiColors.menuBar = Color(0.20f, 0.20f, 0.23f, 1.00f);
        uiColors.tooltipBg = Color(0.27f, 0.29f, 0.32f, 0.941f);
        uiColors.tooltipText = Color(0.95f, 0.95f, 0.97f, 1.00f);

        internal::NodeColors geometryColors;
        geometryColors.baseColor = Color(0.180f, 0.220f, 0.250f, 1.0f);
        geometryColors.headerColor = Color(0.160f, 0.180f, 0.210f, 0.7f);
        geometryColors.accentColor = Color(0.650f, 0.800f, 0.850f, 1.0f);
        geometryColors.borderColor = Color(0.220f, 0.260f, 0.300f, 1.0f);
        geometryColors.selectedColor = Color(0.500f, 0.650f, 0.780f, 1.0f);
        geometryColors.hoveredColor = Color(0.450f, 0.550f, 0.700f, 1.0f);
        geometryColors.glowColor = Color(0.250f, 0.300f, 0.350f, 0.235f);
        nodeColors["Geometry"] = geometryColors;

        internal::NodeColors materialColors;
        materialColors.baseColor = Color(0.230f, 0.200f, 0.220f, 1.0f);
        materialColors.headerColor = Color(0.190f, 0.170f, 0.180f, 0.7f);
        materialColors.accentColor = Color(0.950f, 0.780f, 0.800f, 1.0f);
        materialColors.borderColor = Color(0.250f, 0.230f, 0.240f, 1.0f);
        materialColors.selectedColor = Color(0.780f, 0.650f, 0.700f, 1.0f);
        materialColors.hoveredColor = Color(0.700f, 0.580f, 0.650f, 1.0f);
        materialColors.glowColor = Color(0.300f, 0.250f, 0.280f, 0.235f);
        nodeColors["Material"] = materialColors;

        internal::NodeColors mathColors;
        mathColors.baseColor = Color(0.210f, 0.190f, 0.240f, 1.0f);
        mathColors.headerColor = Color(0.180f, 0.160f, 0.200f, 0.7f);
        mathColors.accentColor = Color(0.800f, 0.750f, 0.950f, 1.0f);
        mathColors.borderColor = Color(0.240f, 0.220f, 0.270f, 1.0f);
        mathColors.selectedColor = Color(0.680f, 0.620f, 0.780f, 1.0f);
        mathColors.hoveredColor = Color(0.600f, 0.550f, 0.700f, 1.0f);
        mathColors.glowColor = Color(0.270f, 0.250f, 0.300f, 0.235f);
        nodeColors["Math"] = mathColors;

        internal::NodeColors subgraphColors;
        subgraphColors.baseColor = Color(0.180f, 0.240f, 0.200f, 1.0f);
        subgraphColors.headerColor = Color(0.160f, 0.210f, 0.180f, 0.7f);
        subgraphColors.accentColor = Color(0.600f, 0.850f, 0.700f, 1.0f);
        subgraphColors.borderColor = Color(0.220f, 0.300f, 0.250f, 1.0f);
        subgraphColors.selectedColor = Color(0.450f, 0.700f, 0.550f, 1.0f);
        subgraphColors.hoveredColor = Color(0.400f, 0.650f, 0.500f, 1.0f);
        subgraphColors.glowColor = Color(0.250f, 0.350f, 0.300f, 0.235f);
        nodeColors["Subgraph"] = subgraphColors;

        internal::NodeColors yellowColors;
        yellowColors.baseColor = Color(0.250f, 0.240f, 0.180f, 1.0f);
        yellowColors.headerColor = Color(0.210f, 0.200f, 0.150f, 0.7f);
        yellowColors.accentColor = Color(0.980f, 0.960f, 0.700f, 1.0f);
        yellowColors.borderColor = Color(0.280f, 0.270f, 0.200f, 1.0f);
        yellowColors.selectedColor = Color(0.850f, 0.830f, 0.580f, 1.0f);
        yellowColors.hoveredColor = Color(0.750f, 0.740f, 0.520f, 1.0f);
        yellowColors.glowColor = Color(0.300f, 0.290f, 0.210f, 0.235f);
        nodeColors["Yellow"] = yellowColors;

        internal::NodeColors defaultColors;
        defaultColors.baseColor = Color(0.200f, 0.200f, 0.200f, 1.0f);
        defaultColors.headerColor = Color(0.170f, 0.170f, 0.170f, 0.7f);
        defaultColors.accentColor = Color(0.800f, 0.800f, 0.800f, 1.0f);
        defaultColors.borderColor = Color(0.250f, 0.250f, 0.250f, 1.0f);
        defaultColors.selectedColor = Color(0.700f, 0.700f, 0.700f, 1.0f);
        defaultColors.hoveredColor = Color(0.600f, 0.600f, 0.600f, 1.0f);
        defaultColors.glowColor = Color(0.300f, 0.300f, 0.300f, 0.235f);
        nodeColors["Default"] = defaultColors;

        internal::PinColors blueColors;
        blueColors.color = Color(0.2f, 0.4f, 0.9f, 1.0f);
        blueColors.hoverColor = Color(0.3f, 0.5f, 1.0f, 1.0f);
        blueColors.connectedColor = Color(0.4f, 0.6f, 1.0f, 1.0f);
        pinColors["Blue"] = blueColors;

        internal::PinColors redColors;
        redColors.color = Color(0.9f, 0.3f, 0.3f, 1.0f);
        redColors.hoverColor = Color(1.0f, 0.4f, 0.4f, 1.0f);
        redColors.connectedColor = Color(1.0f, 0.5f, 0.5f, 1.0f);
        pinColors["Red"] = redColors;

        internal::PinColors greenColors;
        greenColors.color = Color(0.3f, 0.8f, 0.3f, 1.0f);
        greenColors.hoverColor = Color(0.4f, 0.9f, 0.4f, 1.0f);
        greenColors.connectedColor = Color(0.5f, 1.0f, 0.5f, 1.0f);
        pinColors["Green"] = greenColors;

        internal::PinColors yellowColor;
        yellowColor.color = Color(0.95f, 0.95f, 0.3f, 1.0f);
        yellowColor.hoverColor = Color(1.0f, 1.0f, 0.4f, 1.0f);
        yellowColor.connectedColor = Color(1.0f, 1.0f, 0.5f, 1.0f);
        pinColors["Yellow"] = yellowColor;

        internal::PinColors purpleColors;
        purpleColors.color = Color(0.8f, 0.3f, 0.8f, 1.0f);
        purpleColors.hoverColor = Color(0.9f, 0.4f, 0.9f, 1.0f);
        purpleColors.connectedColor = Color(1.0f, 0.5f, 1.0f, 1.0f);
        pinColors["Purple"] = purpleColors;

        internal::PinColors cyanColors;
        cyanColors.color = Color(0.3f, 0.8f, 0.9f, 1.0f);
        cyanColors.hoverColor = Color(0.4f, 0.9f, 1.0f, 1.0f);
        cyanColors.connectedColor = Color(0.5f, 1.0f, 1.0f, 1.0f);
        pinColors["Cyan"] = cyanColors;

        internal::PinColors orangeColors;
        orangeColors.color = Color(0.9f, 0.6f, 0.3f, 1.0f);
        orangeColors.hoverColor = Color(1.0f, 0.7f, 0.4f, 1.0f);
        orangeColors.connectedColor = Color(1.0f, 0.8f, 0.5f, 1.0f);
        pinColors["Orange"] = orangeColors;
    }
}