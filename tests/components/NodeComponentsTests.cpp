#include <gtest/gtest.h>
#include "../../AdvancedNodeEditor/Core/Types/CoreTypes.h"

using namespace NodeEditorCore;

TEST(NodeComponentsTests, NodeCreation) {
    Node node(1, "TestNode", "TestType", Vec2(100.0f, 100.0f));
    
    EXPECT_EQ(node.id, 1);
    EXPECT_EQ(node.name, "TestNode");
    EXPECT_EQ(node.type, "TestType");
    EXPECT_EQ(node.position.x, 100.0f);
    EXPECT_EQ(node.position.y, 100.0f);
    EXPECT_FALSE(node.selected);
    EXPECT_FALSE(node.disabled);
    EXPECT_EQ(node.groupId, -1);
    EXPECT_FALSE(node.isTemplate);
    EXPECT_FALSE(node.isCurrentFlag);
    EXPECT_EQ(node.labelPosition, NodeLabelPosition::Right);
    EXPECT_FALSE(node.isSubgraph);
    EXPECT_EQ(node.subgraphId, -1);
}

TEST(NodeComponentsTests, NodeWithExistingUuid) {
    UUID existingUuid = "12345678-1234-1234-1234-123456789abc";
    Node node(existingUuid, 1, "TestNode", "TestType", Vec2(100.0f, 100.0f));
    
    EXPECT_EQ(node.id, 1);
    EXPECT_EQ(node.uuid, existingUuid);
    EXPECT_EQ(node.name, "TestNode");
    EXPECT_EQ(node.type, "TestType");
}

TEST(NodeComponentsTests, PinCreation) {
    Pin pin(1, "TestPin", true, PinType::Blue);
    
    EXPECT_EQ(pin.id, 1);
    EXPECT_EQ(pin.name, "TestPin");
    EXPECT_TRUE(pin.isInput);
    EXPECT_EQ(pin.type, PinType::Blue);
    EXPECT_EQ(pin.shape, PinShape::Circle);
    EXPECT_FALSE(pin.connected);
}

TEST(NodeComponentsTests, PinWithShapeAndUuid) {
    UUID existingUuid = "12345678-1234-1234-1234-123456789abc";
    Pin pin(existingUuid, 1, "TestPin", false, PinType::Red, PinShape::Square);
    
    EXPECT_EQ(pin.id, 1);
    EXPECT_EQ(pin.uuid, existingUuid);
    EXPECT_EQ(pin.name, "TestPin");
    EXPECT_FALSE(pin.isInput);
    EXPECT_EQ(pin.type, PinType::Red);
    EXPECT_EQ(pin.shape, PinShape::Square);
}

TEST(NodeComponentsTests, NodeFindPin) {
    Node node(1, "TestNode", "TestType", Vec2(100.0f, 100.0f));
    
    Pin inputPin(1, "Input", true, PinType::Blue);
    Pin outputPin(2, "Output", false, PinType::Red);
    
    node.inputs.push_back(inputPin);
    node.outputs.push_back(outputPin);
    
    Pin* foundInputPin = node.findPin(1);
    ASSERT_NE(foundInputPin, nullptr);
    EXPECT_EQ(foundInputPin->id, 1);
    EXPECT_EQ(foundInputPin->name, "Input");
    
    Pin* foundOutputPin = node.findPin(2);
    ASSERT_NE(foundOutputPin, nullptr);
    EXPECT_EQ(foundOutputPin->id, 2);
    EXPECT_EQ(foundOutputPin->name, "Output");
    
    Pin* notFoundPin = node.findPin(3);
    EXPECT_EQ(notFoundPin, nullptr);
}

TEST(NodeComponentsTests, NodeFindPinByUUID) {
    Node node(1, "TestNode", "TestType", Vec2(100.0f, 100.0f));
    
    UUID inputUuid = "12345678-1234-1234-1234-123456789abc";
    UUID outputUuid = "98765432-9876-9876-9876-987654321fed";
    
    Pin inputPin(inputUuid, 1, "Input", true, PinType::Blue, PinShape::Circle);
    Pin outputPin(outputUuid, 2, "Output", false, PinType::Red, PinShape::Square);
    
    node.inputs.push_back(inputPin);
    node.outputs.push_back(outputPin);
    
    Pin* foundInputPin = node.findPinByUUID(inputUuid);
    ASSERT_NE(foundInputPin, nullptr);
    EXPECT_EQ(foundInputPin->id, 1);
    EXPECT_EQ(foundInputPin->name, "Input");
    
    Pin* foundOutputPin = node.findPinByUUID(outputUuid);
    ASSERT_NE(foundOutputPin, nullptr);
    EXPECT_EQ(foundOutputPin->id, 2);
    EXPECT_EQ(foundOutputPin->name, "Output");
    
    Pin* notFoundPin = node.findPinByUUID("non-existent-uuid");
    EXPECT_EQ(notFoundPin, nullptr);
}

TEST(NodeComponentsTests, NodeMetadata) {
    Node node(1, "TestNode", "TestType", Vec2(100.0f, 100.0f));
    
    node.setMetadata("intValue", 42);
    node.setMetadata("floatValue", 3.14f);
    node.setMetadata("stringValue", std::string("test"));
    
    EXPECT_EQ(node.getMetadata<int>("intValue"), 42);
    EXPECT_EQ(node.getMetadata<float>("floatValue"), 3.14f);
    EXPECT_EQ(node.getMetadata<std::string>("stringValue"), "test");
    
    EXPECT_EQ(node.getMetadata<int>("nonExistent", 100), 100);
    EXPECT_EQ(node.getMetadata<std::string>("nonExistent", "default"), "default");
}

TEST(NodeComponentsTests, PinMetadata) {
    Pin pin(1, "TestPin", true, PinType::Blue);
    
    pin.setMetadata("intValue", 42);
    pin.setMetadata("floatValue", 3.14f);
    pin.setMetadata("stringValue", std::string("test"));
    
    EXPECT_EQ(pin.getMetadata<int>("intValue"), 42);
    EXPECT_EQ(pin.getMetadata<float>("floatValue"), 3.14f);
    EXPECT_EQ(pin.getMetadata<std::string>("stringValue"), "test");
    
    EXPECT_EQ(pin.getMetadata<int>("nonExistent", 100), 100);
    EXPECT_EQ(pin.getMetadata<std::string>("nonExistent", "default"), "default");
}

TEST(NodeComponentsTests, SubgraphOperations) {
    Node node;
    
    node.setSubgraphId(5);
    EXPECT_EQ(node.getSubgraphId(), 5);
    
    node.setAsSubgraph(true, 10, "subgraph-uuid");
    EXPECT_TRUE(node.isSubgraph);
    EXPECT_EQ(node.subgraphId, 10);
    EXPECT_EQ(node.subgraphUuid, "subgraph-uuid");
}