#ifndef ANE_TYPES_H
#define ANE_TYPES_H

#include <string>

namespace ANE {

    struct Vec2 {
        float x, y;
        Vec2() : x(0.0f), y(0.0f) {}
        Vec2(float x_, float y_) : x(x_), y(y_) {}
    };

    struct Color {
        float r, g, b, a;
        Color() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
        Color(float r_, float g_, float b_, float a_ = 1.0f) : r(r_), g(g_), b(b_), a(a_) {}
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

    enum class NodeLabelPosition {
        None,
        Left,
        Right
    };

    enum class GroupStyle {
        Default,
        Subtle,
        Bold,
        Wire,
        Ghost
    };

}

#endif