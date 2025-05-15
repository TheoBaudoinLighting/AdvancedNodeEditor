#include <gtest/gtest.h>
#include "../../AdvancedNodeEditor/NodeEditor.h"
#include "../../AdvancedNodeEditor/Evaluation/NodeEditorEvaluation.h"

using namespace NodeEditorCore;

class EvaluationTests : public ::testing::Test {
protected:
    NodeEditor editor;
    
    void SetUp() override {
        editor.addNode("Input", "Math", Vec2(100, 100));
        editor.addNode("Process", "Math", Vec2(300, 100));
        editor.addNode("Output", "Math", Vec2(500, 100));
        
        editor.addPin(1, "Value", false, PinType::Blue);
        editor.addPin(2, "Input", true, PinType::Blue);
        editor.addPin(2, "Result", false, PinType::Green);
        editor.addPin(3, "Result", true, PinType::Green);
        
        editor.addConnection(1, 1, 2, 1);
        editor.addConnection(2, 2, 3, 1);
    }
};

TEST_F(EvaluationTests, GetEvaluationOrder) {
    std::vector<int> order = editor.getEvaluationOrder();
    
    ASSERT_EQ(order.size(), 3);
    
    EXPECT_EQ(order[0], 1);
    EXPECT_EQ(order[1], 2);
    EXPECT_EQ(order[2], 3);
}

TEST_F(EvaluationTests, GetInputConnections) {
    std::vector<NodeEvaluator::ConnectionInfo> connections = editor.getInputConnections(2);
    
    ASSERT_EQ(connections.size(), 1);
    EXPECT_EQ(connections[0].sourceNodeId, 1);
    EXPECT_EQ(connections[0].sourcePinId, 1);
    EXPECT_EQ(connections[0].targetNodeId, 2);
    EXPECT_EQ(connections[0].targetPinId, 1);
}

TEST_F(EvaluationTests, GetOutputConnections) {
    std::vector<NodeEvaluator::ConnectionInfo> connections = editor.getOutputConnections(2);
    
    ASSERT_EQ(connections.size(), 1);
    EXPECT_EQ(connections[0].sourceNodeId, 2);
    EXPECT_EQ(connections[0].sourcePinId, 2);
    EXPECT_EQ(connections[0].targetNodeId, 3);
    EXPECT_EQ(connections[0].targetPinId, 1);
}

TEST_F(EvaluationTests, GetEvaluationOrderWithCycle) {
    editor.addPin(3, "Feedback", false, PinType::Red);
    editor.addPin(1, "Feedback", true, PinType::Red);
    editor.addConnection(3, 2, 1, 2);
    
    std::vector<int> order = editor.getEvaluationOrder();
    
    EXPECT_EQ(order.size(), 3);
}

TEST_F(EvaluationTests, OperationsWithUUIDs) {
    UUID node1Uuid = editor.getNodeUUID(1);
    UUID node2Uuid = editor.getNodeUUID(2);
    UUID node3Uuid = editor.getNodeUUID(3);
    
    std::vector<UUID> orderUuids = editor.getEvaluationOrderUUIDs();
    
    ASSERT_EQ(orderUuids.size(), 3);
    EXPECT_EQ(orderUuids[0], node1Uuid);
    EXPECT_EQ(orderUuids[1], node2Uuid);
    EXPECT_EQ(orderUuids[2], node3Uuid);
    
    std::vector<NodeEvaluator::ConnectionInfo> connections = editor.getInputConnectionsByUUID(node2Uuid);
    
    ASSERT_EQ(connections.size(), 1);
    EXPECT_EQ(connections[0].sourceNodeUuid, node1Uuid);
    EXPECT_EQ(connections[0].targetNodeUuid, node2Uuid);
}

TEST_F(EvaluationTests, SubgraphEvaluation) {
    int subgraphId = editor.createSubgraph("Subgraph");
    
    editor.addNodeToSubgraph(1, subgraphId);
    editor.addNodeToSubgraph(2, subgraphId);
    
    editor.setCurrentSubgraphId(subgraphId);
    
    std::vector<int> orderInSubgraph = editor.getEvaluationOrder();
    
    ASSERT_LE(orderInSubgraph.size(), 2);
}