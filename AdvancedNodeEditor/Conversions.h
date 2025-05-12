#ifndef CONVERSIONS_H
#define CONVERSIONS_H

#include "ANETypes.h"
#include "CoreTypes.h"
#include "StyleDefinitions.h"

namespace NodeEditorCore {

    inline Vec2 convertToVec2(const ANE::Vec2& v) { return Vec2(v.x, v.y); }
    inline Color convertToColor(const ANE::Color& c) { return Color(c.r, c.g, c.b, c.a); }
    inline PinType convertToPinType(ANE::PinType t) { return static_cast<PinType>(t); }
    inline PinShape convertToPinShape(ANE::PinShape s) { return static_cast<PinShape>(s); }
    inline NodeLabelPosition convertToNodeLabelPosition(ANE::NodeLabelPosition p) { return static_cast<NodeLabelPosition>(p); }
    inline GroupStyle convertToGroupStyle(ANE::GroupStyle s) { return static_cast<GroupStyle>(s); }

    inline ANE::Vec2 convertToANEVec2(const Vec2& v) { return ANE::Vec2(v.x, v.y); }
    inline ANE::Color convertToANEColor(const Color& c) { return ANE::Color(c.r, c.g, c.b, c.a); }
    inline ANE::PinType convertToANEPinType(PinType t) { return static_cast<ANE::PinType>(t); }
    inline ANE::PinShape convertToANEPinShape(PinShape s) { return static_cast<ANE::PinShape>(s); }
    inline ANE::NodeLabelPosition convertToANENodeLabelPosition(NodeLabelPosition p) { return static_cast<ANE::NodeLabelPosition>(p); }
    inline ANE::GroupStyle convertToANEGroupStyle(GroupStyle s) { return static_cast<ANE::GroupStyle>(s); }

    NodeEditorStyle convertToInternalStyle(const ANE::EditorStyle& style);
    ANE::EditorStyle convertToAPIStyle(const NodeEditorStyle& internalStyle);

}

#endif