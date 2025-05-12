#ifndef CORE_TYPES_H
#define CORE_TYPES_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <imgui.h>
#include "ANETypes.h"

namespace NodeEditorCore {

struct Vec2 {
    float x, y;
    Vec2() : x(0.0f), y(0.0f) {}
    Vec2(float x_, float y_) : x(x_), y(y_) {}

    Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
    Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
    Vec2 operator*(float scalar) const { return Vec2(x * scalar, y * scalar); }

    ImVec2 toImVec2() const { return ImVec2(x, y); }
    static Vec2 fromImVec2(const ImVec2& v) { return Vec2(v.x, v.y); }
};

struct Color {
    float r, g, b, a;
    Color() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
    Color(float r_, float g_, float b_, float a_ = 1.0f) : r(r_), g(g_), b(b_), a(a_) {}

    ImVec4 toImVec4() const { return ImVec4(r, g, b, a); }
    ImU32 toImU32() const { return IM_COL32(r * 255, g * 255, b * 255, a * 255); }
    static Color fromImVec4(const ImVec4& v) { return Color(v.x, v.y, v.z, v.w); }
};

enum class PinType {
    Flow,
    Bool,
    Int,
    Float,
    String,
    Vec2,
    Vec3,
    Vec4,
    Color,
    Object,
    Custom
};

enum class PinShape {
    Circle,
    Square,
    Triangle,
    Diamond
};

enum class GroupStyle {
    Default,
    Subtle,
    Bold,
    Wire,
    Ghost
};

enum class NodeLabelPosition {
    None,
    Left,
    Right
};

namespace internal {
    struct NodeColors {
        Color baseColor;
        Color headerColor;
        Color accentColor;
        Color borderColor;
        Color selectedColor;
        Color hoveredColor;
        Color glowColor;
    };

    struct PinColors {
        Color color;
        Color hoverColor;
        Color connectedColor;
    };

    struct ConnectionColors {
        Color baseColor;
        Color selectedColor;
        Color hoveredColor;
        Color validColor;
        Color invalidColor;
    };

    struct UIColors {
        Color background;
        Color grid;
        Color selection;
        Color menuBar;
        Color tooltipBg;
        Color tooltipText;
    };
}

}

#endif