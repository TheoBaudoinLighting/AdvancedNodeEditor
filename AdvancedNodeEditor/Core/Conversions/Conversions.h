#ifndef CONVERSIONS_H
#define CONVERSIONS_H

#include "../Types/CoreTypes.h"
#include "../Style/StyleDefinitions.h"

namespace NodeEditorCore {
    inline Vec2 convertToVec2(const Vec2 &v) { return Vec2(v.x, v.y); }
    inline Color convertToColor(const Color &c) { return Color(c.r, c.g, c.b, c.a); }
    inline PinType convertToPinType(PinType t) { return static_cast<PinType>(t); }
    inline PinShape convertToPinShape(PinShape s) { return static_cast<PinShape>(s); }

    inline NodeLabelPosition convertToNodeLabelPosition(NodeLabelPosition p) {
        return static_cast<NodeLabelPosition>(p);
    }

    inline GroupStyle convertToGroupStyle(GroupStyle s) { return static_cast<GroupStyle>(s); }

    inline Vec2 convertToANEVec2(const Vec2 &v) { return Vec2(v.x, v.y); }
    inline Color convertToANEColor(const Color &c) { return Color(c.r, c.g, c.b, c.a); }
    inline PinType convertToANEPinType(PinType t) { return static_cast<PinType>(t); }
    inline PinShape convertToANEPinShape(PinShape s) { return static_cast<PinShape>(s); }

    inline NodeLabelPosition convertToANENodeLabelPosition(NodeLabelPosition p) {
        return static_cast<NodeLabelPosition>(p);
    }

    inline GroupStyle convertToANEGroupStyle(GroupStyle s) { return static_cast<GroupStyle>(s); }

    NodeEditorStyle convertToInternalStyle(const EditorStyle &style);

    EditorStyle convertToAPIStyle(const NodeEditorStyle &internalStyle);
}

#endif
