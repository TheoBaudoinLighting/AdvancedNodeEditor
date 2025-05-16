#include <gtest/gtest.h>
#include "../../AdvancedNodeEditor/NodeEditor.h"
#include "../../AdvancedNodeEditor/Evaluation/NodeEditorEvaluation.h"

using namespace NodeEditorCore;

class EvaluationDebugTests : public ::testing::Test {
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

TEST_F(EvaluationDebugTests, ConnectionsExist) {
    const auto& connections = editor.getConnections();
    std::cout << "=== Connexions dans l'éditeur ===" << std::endl;
    EXPECT_EQ(connections.size(), 2) << "L'éditeur devrait avoir 2 connexions";

    std::cout << "=== getInputConnections pour le noeud 2 ===" << std::endl;
    auto inputConns = editor.getInputConnections(node2Id);
    std::cout << "Nombre de connexions d'entrée: " << inputConns.size() << std::endl;

    for (const auto& conn : inputConns) {
        std::cout << "Connexion trouvée: ID: " << conn.connectionId
                  << ", De: Node " << conn.sourceNodeId
                  << ", Pin " << conn.sourcePinId
                  << ", Vers: Node " << conn.targetNodeId
                  << ", Pin " << conn.targetPinId << std::endl;
    }

    ASSERT_EQ(inputConns.size(), 1);
    EXPECT_EQ(inputConns[0].sourceNodeId, node1Id);
    EXPECT_EQ(inputConns[0].sourcePinId, pin1Id);
    EXPECT_EQ(inputConns[0].targetNodeId, node2Id);
    EXPECT_EQ(inputConns[0].targetPinId, pin2Id);

    std::cout << "=== getOutputConnections pour le noeud 2 ===" << std::endl;
    auto outputConns = editor.getOutputConnections(node2Id);
    std::cout << "Nombre de connexions de sortie: " << outputConns.size() << std::endl;

    for (const auto& conn : outputConns) {
        std::cout << "Connexion trouvée: ID: " << conn.connectionId
                  << ", De: Node " << conn.sourceNodeId
                  << ", Pin " << conn.sourcePinId
                  << ", Vers: Node " << conn.targetNodeId
                  << ", Pin " << conn.targetPinId << std::endl;
    }

    ASSERT_EQ(outputConns.size(), 1);
    EXPECT_EQ(outputConns[0].sourceNodeId, node2Id);
    EXPECT_EQ(outputConns[0].sourcePinId, pin3Id);
    EXPECT_EQ(outputConns[0].targetNodeId, node3Id);
    EXPECT_EQ(outputConns[0].targetPinId, pin4Id);
}