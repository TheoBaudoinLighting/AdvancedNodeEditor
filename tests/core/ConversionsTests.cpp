#include <gtest/gtest.h>
#include "../../AdvancedNodeEditor/Core/Conversions/Conversions.h"

using namespace NodeEditorCore;

TEST(ConversionsTests, VectorConversion) {
    Vec2 original(10.0f, 20.0f);
    Vec2 converted = convertToVec2(original);
    
    EXPECT_FLOAT_EQ(converted.x, 10.0f);
    EXPECT_FLOAT_EQ(converted.y, 20.0f);
    
    Vec2 aneConverted = convertToANEVec2(original);
    EXPECT_FLOAT_EQ(aneConverted.x, 10.0f);
    EXPECT_FLOAT_EQ(aneConverted.y, 20.0f);
}

TEST(ConversionsTests, ColorConversion) {
    Color original(0.1f, 0.2f, 0.3f, 0.4f);
    Color converted = convertToColor(original);
    
    EXPECT_FLOAT_EQ(converted.r, 0.1f);
    EXPECT_FLOAT_EQ(converted.g, 0.2f);
    EXPECT_FLOAT_EQ(converted.b, 0.3f);
    EXPECT_FLOAT_EQ(converted.a, 0.4f);
    
    Color aneConverted = convertToANEColor(original);
    EXPECT_FLOAT_EQ(aneConverted.r, 0.1f);
    EXPECT_FLOAT_EQ(aneConverted.g, 0.2f);
    EXPECT_FLOAT_EQ(aneConverted.b, 0.3f);
    EXPECT_FLOAT_EQ(aneConverted.a, 0.4f);
}

TEST(ConversionsTests, PinTypeConversion) {
    PinType original = PinType::Blue;
    PinType converted = convertToPinType(original);
    
    EXPECT_EQ(converted, PinType::Blue);
    
    PinType aneConverted = convertToANEPinType(original);
    EXPECT_EQ(aneConverted, PinType::Blue);
}

TEST(ConversionsTests, PinShapeConversion) {
    PinShape original = PinShape::Circle;
    PinShape converted = convertToPinShape(original);
    
    EXPECT_EQ(converted, PinShape::Circle);
    
    PinShape aneConverted = convertToANEPinShape(original);
    EXPECT_EQ(aneConverted, PinShape::Circle);
}

TEST(ConversionsTests, StyleConversion) {
    EditorStyle original;
    original.backgroundColor = Color(0.1f, 0.2f, 0.3f, 0.4f);
    original.gridColor = Color(0.5f, 0.6f, 0.7f, 0.8f);
    original.gridSpacing = 20.0f;
    original.nodeRounding = 5.0f;
    
    NodeStyle nodeStyle;
    nodeStyle.baseColor = Color(0.1f, 0.1f, 0.1f, 1.0f);
    nodeStyle.headerColor = Color(0.2f, 0.2f, 0.2f, 1.0f);
    original.nodeStyles["Default"] = nodeStyle;
    
    PinStyle pinStyle;
    pinStyle.color = Color(0.3f, 0.3f, 0.3f, 1.0f);
    pinStyle.hoverColor = Color(0.4f, 0.4f, 0.4f, 1.0f);
    original.pinStyles["Blue"] = pinStyle;
    
    NodeEditorStyle internal = convertToInternalStyle(original);
    
    EXPECT_FLOAT_EQ(internal.uiColors.background.r, 0.1f);
    EXPECT_FLOAT_EQ(internal.uiColors.background.g, 0.2f);
    EXPECT_FLOAT_EQ(internal.uiColors.background.b, 0.3f);
    EXPECT_FLOAT_EQ(internal.uiColors.background.a, 0.4f);
    
    EXPECT_FLOAT_EQ(internal.uiColors.grid.r, 0.5f);
    EXPECT_FLOAT_EQ(internal.uiColors.grid.g, 0.6f);
    EXPECT_FLOAT_EQ(internal.uiColors.grid.b, 0.7f);
    EXPECT_FLOAT_EQ(internal.uiColors.grid.a, 0.8f);
    
    EXPECT_FLOAT_EQ(internal.gridSpacing, 20.0f);
    EXPECT_FLOAT_EQ(internal.nodeRounding, 5.0f);
    
    EXPECT_TRUE(internal.nodeColors.find("Default") != internal.nodeColors.end());
    EXPECT_FLOAT_EQ(internal.nodeColors["Default"].base.r, 0.1f);
    EXPECT_FLOAT_EQ(internal.nodeColors["Default"].base.g, 0.1f);
    EXPECT_FLOAT_EQ(internal.nodeColors["Default"].base.b, 0.1f);
    EXPECT_FLOAT_EQ(internal.nodeColors["Default"].base.a, 1.0f);
    
    EXPECT_TRUE(internal.pinColors.find("Blue") != internal.pinColors.end());
    EXPECT_FLOAT_EQ(internal.pinColors["Blue"].base.r, 0.3f);
    EXPECT_FLOAT_EQ(internal.pinColors["Blue"].base.g, 0.3f);
    EXPECT_FLOAT_EQ(internal.pinColors["Blue"].base.b, 0.3f);
    EXPECT_FLOAT_EQ(internal.pinColors["Blue"].base.a, 1.0f);
    
    EditorStyle converted = convertToAPIStyle(internal);
    
    EXPECT_FLOAT_EQ(converted.backgroundColor.r, 0.1f);
    EXPECT_FLOAT_EQ(converted.backgroundColor.g, 0.2f);
    EXPECT_FLOAT_EQ(converted.backgroundColor.b, 0.3f);
    EXPECT_FLOAT_EQ(converted.backgroundColor.a, 0.4f);
    
    EXPECT_FLOAT_EQ(converted.gridSpacing, 20.0f);
    EXPECT_FLOAT_EQ(converted.nodeRounding, 5.0f);
    
    EXPECT_TRUE(converted.nodeStyles.find("Default") != converted.nodeStyles.end());
    EXPECT_FLOAT_EQ(converted.nodeStyles["Default"].baseColor.r, 0.1f);
    EXPECT_FLOAT_EQ(converted.nodeStyles["Default"].baseColor.g, 0.1f);
    EXPECT_FLOAT_EQ(converted.nodeStyles["Default"].baseColor.b, 0.1f);
    
    EXPECT_TRUE(converted.pinStyles.find("Blue") != converted.pinStyles.end());
    EXPECT_FLOAT_EQ(converted.pinStyles["Blue"].color.r, 0.3f);
    EXPECT_FLOAT_EQ(converted.pinStyles["Blue"].color.g, 0.3f);
    EXPECT_FLOAT_EQ(converted.pinStyles["Blue"].color.b, 0.3f);
}