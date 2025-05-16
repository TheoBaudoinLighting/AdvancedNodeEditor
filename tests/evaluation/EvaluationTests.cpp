#include <gtest/gtest.h>
#include "../../AdvancedNodeEditor/NodeEditor.h"
#include "../../AdvancedNodeEditor/Evaluation/NodeEditorEvaluation.h"

using namespace NodeEditorCore;

class EvaluationTests : public ::testing::Test {
protected:
    NodeEditor editor;
    int node1Id, node2Id, node3Id;
    int pin1Id, pin2Id, pin3Id, pin4Id;
    int conn1Id, conn2Id;

    void SetUp() override {
        node1Id = editor.addNode("Input", "Math", Vec2(100, 100));
        node2Id = editor.addNode("Process", "Math", Vec2(300, 100));
        node3Id = editor.addNode("Output", "Math", Vec2(500, 100));

        pin1Id = editor.addPin(node1Id, "Value", false, PinType::Blue);
        pin2Id = editor.addPin(node2Id, "Input", true, PinType::Blue);
        pin3Id = editor.addPin(node2Id, "Result", false, PinType::Green);
        pin4Id = editor.addPin(node3Id, "Result", true, PinType::Green);

        conn1Id = editor.addConnection(node1Id, pin1Id, node2Id, pin2Id);
        conn2Id = editor.addConnection(node2Id, pin3Id, node3Id, pin4Id);
    }
};

TEST_F(EvaluationTests, GetEvaluationOrder) {
    std::vector<int> order = editor.getEvaluationOrder();

    ASSERT_EQ(order.size(), 3);

    EXPECT_EQ(order[0], node1Id);
    EXPECT_EQ(order[1], node2Id);
    EXPECT_EQ(order[2], node3Id);
}

TEST_F(EvaluationTests, GetInputConnections) {
    std::vector<NodeEvaluator::ConnectionInfo> connections = editor.getInputConnections(node2Id);

    ASSERT_EQ(connections.size(), 1);
    EXPECT_EQ(connections[0].sourceNodeId, node1Id);
    EXPECT_EQ(connections[0].sourcePinId, pin1Id);
    EXPECT_EQ(connections[0].targetNodeId, node2Id);
    EXPECT_EQ(connections[0].targetPinId, pin2Id);
}

TEST_F(EvaluationTests, GetOutputConnections) {
    std::vector<NodeEvaluator::ConnectionInfo> connections = editor.getOutputConnections(node2Id);

    ASSERT_EQ(connections.size(), 1);
    EXPECT_EQ(connections[0].sourceNodeId, node2Id);
    EXPECT_EQ(connections[0].sourcePinId, pin3Id);
    EXPECT_EQ(connections[0].targetNodeId, node3Id);
    EXPECT_EQ(connections[0].targetPinId, pin4Id);
}

TEST_F(EvaluationTests, GetEvaluationOrderWithCycle) {
    int pin5Id = editor.addPin(node3Id, "Feedback", false, PinType::Red);
    int pin6Id = editor.addPin(node1Id, "Feedback", true, PinType::Red);
    int conn3Id = editor.addConnection(node3Id, pin5Id, node1Id, pin6Id);

    std::vector<int> order = editor.getEvaluationOrder();

    EXPECT_EQ(order.size(), 3);
}

TEST_F(EvaluationTests, OperationsWithUUIDs) {
    UUID node1Uuid = editor.getNodeUUID(node1Id);
    UUID node2Uuid = editor.getNodeUUID(node2Id);
    UUID node3Uuid = editor.getNodeUUID(node3Id);

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

    editor.addNodeToSubgraph(node1Id, subgraphId);
    editor.addNodeToSubgraph(node2Id, subgraphId);

    editor.setCurrentSubgraphId(subgraphId);

    std::vector<int> orderInSubgraph = editor.getEvaluationOrder();

    ASSERT_LE(orderInSubgraph.size(), 2);
}