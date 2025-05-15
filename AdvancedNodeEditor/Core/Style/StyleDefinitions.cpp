#include "StyleDefinitions.h"

namespace NodeEditorCore {

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
        geometryColors.base = Color(0.180f, 0.220f, 0.250f, 1.0f);
        geometryColors.header = Color(0.160f, 0.180f, 0.210f, 0.7f);
        geometryColors.accent = Color(0.650f, 0.800f, 0.850f, 1.0f);
        geometryColors.border = Color(0.220f, 0.260f, 0.300f, 1.0f);
        geometryColors.selected = Color(0.500f, 0.650f, 0.780f, 1.0f);
        geometryColors.hovered = Color(0.450f, 0.550f, 0.700f, 1.0f);
        geometryColors.glow = Color(0.250f, 0.300f, 0.350f, 0.235f);
        nodeColors["Geometry"] = geometryColors;

        internal::NodeColors materialColors;
        materialColors.base = Color(0.230f, 0.200f, 0.220f, 1.0f);
        materialColors.header = Color(0.190f, 0.170f, 0.180f, 0.7f);
        materialColors.accent = Color(0.950f, 0.780f, 0.800f, 1.0f);
        materialColors.border = Color(0.250f, 0.230f, 0.240f, 1.0f);
        materialColors.selected = Color(0.780f, 0.650f, 0.700f, 1.0f);
        materialColors.hovered = Color(0.700f, 0.580f, 0.650f, 1.0f);
        materialColors.glow = Color(0.300f, 0.250f, 0.280f, 0.235f);
        nodeColors["Material"] = materialColors;

        internal::NodeColors mathColors;
        mathColors.base = Color(0.210f, 0.190f, 0.240f, 1.0f);
        mathColors.header = Color(0.180f, 0.160f, 0.200f, 0.7f);
        mathColors.accent = Color(0.800f, 0.750f, 0.950f, 1.0f);
        mathColors.border = Color(0.240f, 0.220f, 0.270f, 1.0f);
        mathColors.selected = Color(0.680f, 0.620f, 0.780f, 1.0f);
        mathColors.hovered = Color(0.600f, 0.550f, 0.700f, 1.0f);
        mathColors.glow = Color(0.270f, 0.250f, 0.300f, 0.235f);
        nodeColors["Math"] = mathColors;

        internal::NodeColors subgraphColors;
        subgraphColors.base = Color(0.180f, 0.240f, 0.200f, 1.0f);
        subgraphColors.header = Color(0.160f, 0.210f, 0.180f, 0.7f);
        subgraphColors.accent = Color(0.600f, 0.850f, 0.700f, 1.0f);
        subgraphColors.border = Color(0.220f, 0.300f, 0.250f, 1.0f);
        subgraphColors.selected = Color(0.450f, 0.700f, 0.550f, 1.0f);
        subgraphColors.hovered = Color(0.400f, 0.650f, 0.500f, 1.0f);
        subgraphColors.glow = Color(0.250f, 0.350f, 0.300f, 0.235f);
        nodeColors["Subgraph"] = subgraphColors;

        internal::NodeColors yellowColors;
        yellowColors.base = Color(0.250f, 0.240f, 0.180f, 1.0f);
        yellowColors.header = Color(0.210f, 0.200f, 0.150f, 0.7f);
        yellowColors.accent = Color(0.980f, 0.960f, 0.700f, 1.0f);
        yellowColors.border = Color(0.280f, 0.270f, 0.200f, 1.0f);
        yellowColors.selected = Color(0.850f, 0.830f, 0.580f, 1.0f);
        yellowColors.hovered = Color(0.750f, 0.740f, 0.520f, 1.0f);
        yellowColors.glow = Color(0.300f, 0.290f, 0.210f, 0.235f);
        nodeColors["Yellow"] = yellowColors;

        internal::NodeColors defaultColors;
        defaultColors.base = Color(0.200f, 0.200f, 0.200f, 1.0f);
        defaultColors.header = Color(0.170f, 0.170f, 0.170f, 0.7f);
        defaultColors.accent = Color(0.800f, 0.800f, 0.800f, 1.0f);
        defaultColors.border = Color(0.250f, 0.250f, 0.250f, 1.0f);
        defaultColors.selected = Color(0.700f, 0.700f, 0.700f, 1.0f);
        defaultColors.hovered = Color(0.600f, 0.600f, 0.600f, 1.0f);
        defaultColors.glow = Color(0.300f, 0.300f, 0.300f, 0.235f);
        nodeColors["Default"] = defaultColors;

        internal::PinColors blueColors;
        blueColors.base = Color(0.2f, 0.4f, 0.9f, 1.0f);
        blueColors.hover = Color(0.3f, 0.5f, 1.0f, 1.0f);
        blueColors.connected = Color(0.4f, 0.6f, 1.0f, 1.0f);
        pinColors["Blue"] = blueColors;

        internal::PinColors redColors;
        redColors.base = Color(0.9f, 0.3f, 0.3f, 1.0f);
        redColors.hover = Color(1.0f, 0.4f, 0.4f, 1.0f);
        redColors.connected = Color(1.0f, 0.5f, 0.5f, 1.0f);
        pinColors["Red"] = redColors;

        internal::PinColors greenColors;
        greenColors.base = Color(0.3f, 0.8f, 0.3f, 1.0f);
        greenColors.hover = Color(0.4f, 0.9f, 0.4f, 1.0f);
        greenColors.connected = Color(0.5f, 1.0f, 0.5f, 1.0f);
        pinColors["Green"] = greenColors;

        internal::PinColors yellowColor;
        yellowColor.base = Color(0.95f, 0.95f, 0.3f, 1.0f);
        yellowColor.hover = Color(1.0f, 1.0f, 0.4f, 1.0f);
        yellowColor.connected = Color(1.0f, 1.0f, 0.5f, 1.0f);
        pinColors["Yellow"] = yellowColor;

        internal::PinColors purpleColors;
        purpleColors.base = Color(0.8f, 0.3f, 0.8f, 1.0f);
        purpleColors.hover = Color(0.9f, 0.4f, 0.9f, 1.0f);
        purpleColors.connected = Color(1.0f, 0.5f, 1.0f, 1.0f);
        pinColors["Purple"] = purpleColors;

        internal::PinColors cyanColors;
        cyanColors.base = Color(0.3f, 0.8f, 0.9f, 1.0f);
        cyanColors.hover = Color(0.4f, 0.9f, 1.0f, 1.0f);
        cyanColors.connected = Color(0.5f, 1.0f, 1.0f, 1.0f);
        pinColors["Cyan"] = cyanColors;

        internal::PinColors orangeColors;
        orangeColors.base = Color(0.9f, 0.6f, 0.3f, 1.0f);
        orangeColors.hover = Color(1.0f, 0.7f, 0.4f, 1.0f);
        orangeColors.connected = Color(1.0f, 0.8f, 0.5f, 1.0f);
        pinColors["Orange"] = orangeColors;
    }
}