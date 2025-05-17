#include <gtest/gtest.h>
#include "../../AdvancedNodeEditor/NodeEditor.h"

using namespace NodeEditorCore;

class SubgraphTests : public ::testing::Test {
protected:
    NodeEditor editor;
    
    void SetUp() override {
        int node1Id = editor.addNode("Node1", "Default", NodeEditorCore::Vec2(100, 100));
        int node2Id = editor.addNode("Node2", "Default", NodeEditorCore::Vec2(300, 100));

        std::cout << "Nœuds créés: " << node1Id << ", " << node2Id << std::endl;

        if (node1Id != 1 || node2Id != 2) {
            std::cout << "ERREUR: Les IDs des nœuds ne sont pas ceux attendus" << std::endl;
        }

        int pin1Id = editor.addPin(node1Id, "Output", false, NodeEditorCore::PinType::Blue);
        int pin2Id = editor.addPin(node2Id, "Input", true, NodeEditorCore::PinType::Blue);

        std::cout << "Pins créés: " << pin1Id << ", " << pin2Id << std::endl;

        auto* node1 = editor.getNode(node1Id);
        auto* node2 = editor.getNode(node2Id);

        if (node1) {
            std::cout << "Pins dans le nœud 1:" << std::endl;
            std::cout << "  Entrées: ";
            for (const auto& pin : node1->inputs) {
                std::cout << pin.id << " ";
            }
            std::cout << std::endl;
            std::cout << "  Sorties: ";
            for (const auto& pin : node1->outputs) {
                std::cout << pin.id << " ";
            }
            std::cout << std::endl;
        }

        if (node2) {
            std::cout << "Pins dans le nœud 2:" << std::endl;
            std::cout << "  Entrées: ";
            for (const auto& pin : node2->inputs) {
                std::cout << pin.id << " ";
            }
            std::cout << std::endl;
            std::cout << "  Sorties: ";
            for (const auto& pin : node2->outputs) {
                std::cout << pin.id << " ";
            }
            std::cout << std::endl;
        }

        if (pin1Id < 0 || pin2Id < 0) {
            std::cout << "ERREUR: Pins non créés correctement" << std::endl;
        }

        int connectionId = editor.addConnection(node1Id, pin1Id, node2Id, pin2Id);

        std::cout << "Connexion créée: " << connectionId << std::endl;

        if (connectionId < 0) {
            std::cout << "ERREUR: Connexion non créée correctement" << std::endl;
        }
    }
};

TEST_F(SubgraphTests, CreateSubgraph) {
    int subgraphId = editor.createSubgraph("TestSubgraph");
    EXPECT_GT(subgraphId, 0);
    
    Subgraph* subgraph = editor.getSubgraph(subgraphId);
    ASSERT_NE(subgraph, nullptr);
    EXPECT_EQ(subgraph->name, "TestSubgraph");
    EXPECT_EQ(subgraph->id, subgraphId);
}

TEST_F(SubgraphTests, CreateSubgraphWithUUID) {
    UUID subgraphUuid = editor.createSubgraphWithUUID("TestSubgraph");
    EXPECT_FALSE(subgraphUuid.empty());
    
    EXPECT_FALSE(subgraphUuid.empty());
}

TEST_F(SubgraphTests, AddNodeToSubgraph) {
    int subgraphId = editor.createSubgraph("TestSubgraph");

    editor.addNodeToSubgraph(1, subgraphId);
    editor.addNodeToSubgraph(2, subgraphId);

    std::vector<int> nodes = editor.getNodesInSubgraph(subgraphId);
    EXPECT_EQ(nodes.size(), 2);
    EXPECT_TRUE(std::find(nodes.begin(), nodes.end(), 1) != nodes.end());
    EXPECT_TRUE(std::find(nodes.begin(), nodes.end(), 2) != nodes.end());

    Node* node1 = editor.getNode(1);
    Node* node2 = editor.getNode(2);
    ASSERT_NE(node1, nullptr);
    ASSERT_NE(node2, nullptr);
    EXPECT_EQ(node1->getSubgraphId(), subgraphId);
    EXPECT_EQ(node2->getSubgraphId(), subgraphId);
}

TEST_F(SubgraphTests, AddConnectionToSubgraph) {
    int subgraphId = editor.createSubgraph("TestSubgraph");

    editor.addNodeToSubgraph(1, subgraphId);
    editor.addNodeToSubgraph(2, subgraphId);
    editor.addConnectionToSubgraph(1, subgraphId);

    std::vector<int> connections = editor.getConnectionsInSubgraph(subgraphId);
    EXPECT_TRUE(editor.isConnectionInSubgraph(1, subgraphId));

    EXPECT_FALSE(connections.empty());
    if (!connections.empty()) {
        EXPECT_EQ(connections[0], 1);
    }

    Connection* connection = editor.getConnection(1);
    ASSERT_NE(connection, nullptr);
    EXPECT_EQ(connection->getSubgraphId(), subgraphId);
}

TEST_F(SubgraphTests, RemoveNodeFromSubgraph) {
    int subgraphId = editor.createSubgraph("TestSubgraph");

    editor.addNodeToSubgraph(1, subgraphId);
    editor.addNodeToSubgraph(2, subgraphId);

    editor.removeNodeFromSubgraph(1, subgraphId);

    std::vector<int> nodes = editor.getNodesInSubgraph(subgraphId);
    EXPECT_EQ(nodes.size(), 1);
    EXPECT_EQ(nodes[0], 2);

    Node* node1 = editor.getNode(1);
    ASSERT_NE(node1, nullptr);
    EXPECT_EQ(node1->getSubgraphId(), -1);
}

TEST_F(SubgraphTests, RemoveConnectionFromSubgraph) {
    int subgraphId = editor.createSubgraph("TestSubgraph");

    editor.addConnectionToSubgraph(1, subgraphId);
    editor.removeConnectionFromSubgraph(1, subgraphId);

    std::vector<int> connections = editor.getConnectionsInSubgraph(subgraphId);
    EXPECT_TRUE(connections.empty());

    Connection* connection = editor.getConnection(1);
    ASSERT_NE(connection, nullptr);
    EXPECT_EQ(connection->getSubgraphId(), -1);
}

TEST_F(SubgraphTests, CurrentSubgraph) {
    int subgraphId = editor.createSubgraph("TestSubgraph");

    editor.setCurrentSubgraphId(subgraphId);
    EXPECT_EQ(editor.getCurrentSubgraphId(), subgraphId);

    editor.addNodeToSubgraph(1, subgraphId);
    EXPECT_TRUE(editor.isNodeInCurrentSubgraph(*editor.getNode(1)));
    EXPECT_FALSE(editor.isNodeInCurrentSubgraph(*editor.getNode(2)));
}

TEST_F(SubgraphTests, SubgraphProperties) {
    int subgraphId = editor.createSubgraph("TestSubgraph");
    Subgraph* subgraph = editor.getSubgraph(subgraphId);
    ASSERT_NE(subgraph, nullptr);

    EXPECT_EQ(subgraph->name, "TestSubgraph");
    EXPECT_TRUE(subgraph->nodeIds.empty());
    EXPECT_TRUE(subgraph->connectionIds.empty());
    EXPECT_EQ(subgraph->parentSubgraphId, -1);
    EXPECT_TRUE(subgraph->isExpanded);
}

TEST_F(SubgraphTests, BasicUUIDOperations) {
    UUID subgraphUuid = editor.createSubgraphWithUUID("TestSubgraph");
    EXPECT_FALSE(subgraphUuid.empty());

    UUID node1Uuid = editor.getNodeUUID(1);
    EXPECT_FALSE(node1Uuid.empty());

    Node* node = editor.getNodeByUUID(node1Uuid);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->id, 1);
}

TEST_F(SubgraphTests, EnterSubgraphByUUID) {
    UUID uuid = editor.createSubgraphWithUUID("TestSubgraph");
    ASSERT_FALSE(uuid.empty());

    int subgraphId = editor.getSubgraphId(uuid);
    ASSERT_NE(subgraphId, -1);

    bool result = editor.enterSubgraphByUUID(uuid);
    ASSERT_TRUE(result);
    EXPECT_EQ(editor.getCurrentSubgraphId(), subgraphId);

    result = editor.enterSubgraphByUUID("non-existent-uuid");
    ASSERT_FALSE(result);
}

TEST_F(SubgraphTests, ExitSubgraph) {
    UUID parentUuid = editor.createSubgraphWithUUID("ParentSubgraph");
    editor.enterSubgraphByUUID(parentUuid);

    UUID childUuid = editor.createSubgraphWithUUID("ChildSubgraph");
    editor.enterSubgraphByUUID(childUuid);

    EXPECT_EQ(editor.getCurrentSubgraphId(), editor.getSubgraphId(childUuid));

    bool result = editor.exitSubgraph();
    ASSERT_TRUE(result);
    EXPECT_EQ(editor.getCurrentSubgraphId(), editor.getSubgraphId(parentUuid));

    result = editor.exitSubgraph();
    ASSERT_TRUE(result);
    EXPECT_EQ(editor.getCurrentSubgraphId(), -1);

    result = editor.exitSubgraph();
    ASSERT_FALSE(result);
}

TEST_F(SubgraphTests, GetSubgraphId) {
    UUID uuid = editor.createSubgraphWithUUID("TestSubgraph");
    ASSERT_FALSE(uuid.empty());

    int subgraphId = editor.getSubgraphId(uuid);
    ASSERT_NE(subgraphId, -1);

    int nonExistentId = editor.getSubgraphId("non-existent-uuid");
    ASSERT_EQ(nonExistentId, -1);
}

TEST_F(SubgraphTests, CreateSubgraphNode) {
    UUID uuid = editor.createSubgraphWithUUID("TestSubgraph");
    int subgraphId = editor.getSubgraphId(uuid);

    editor.enterSubgraphByUUID(uuid);
    int inputNodeId = editor.addNode("Input", "Subgraph.Input", Vec2(100, 100));
    int inputPinId = editor.addPin(inputNodeId, "Value", false, PinType::Blue);
    std::cout << "Pin créé pour le nœud d'entrée: ID=" << inputPinId << std::endl;

    int outputNodeId = editor.addNode("Output", "Subgraph.Output", Vec2(500, 100));
    int outputPinId = editor.addPin(outputNodeId, "Value", true, PinType::Blue);
    std::cout << "Pin créé pour le nœud de sortie: ID=" << outputPinId << std::endl;

    Subgraph* subgraph = editor.getSubgraph(subgraphId);
    ASSERT_NE(subgraph, nullptr);
    subgraph->exposeOutput(inputNodeId, inputPinId);
    subgraph->exposeInput(outputNodeId, outputPinId);

    editor.exitSubgraph();

    Node* subgraphNode = editor.createSubgraphNode(subgraphId, "SubgraphNode", Vec2(300, 200));
    ASSERT_NE(subgraphNode, nullptr);

    EXPECT_TRUE(subgraphNode->isSubgraph);
    EXPECT_EQ(subgraphNode->subgraphId, subgraphId);
    EXPECT_EQ(subgraphNode->subgraphUuid, uuid);

    // Vérifier que les pins ont été créés
    EXPECT_EQ(subgraphNode->inputs.size(), 1);
    EXPECT_EQ(subgraphNode->outputs.size(), 1);

    Node* nonExistentNode = editor.createSubgraphNode(-1, "NonExistentNode", Vec2(400, 200));
    ASSERT_EQ(nonExistentNode, nullptr);
}

TEST_F(SubgraphTests, SubgraphNavigation) {
    UUID level1 = editor.createSubgraphWithUUID("Level1");
    editor.enterSubgraphByUUID(level1);

    UUID level2 = editor.createSubgraphWithUUID("Level2");
    editor.enterSubgraphByUUID(level2);

    UUID level3 = editor.createSubgraphWithUUID("Level3");
    editor.enterSubgraphByUUID(level3);

    EXPECT_EQ(editor.getCurrentSubgraphId(), editor.getSubgraphId(level3));

    editor.exitSubgraph();
    EXPECT_EQ(editor.getCurrentSubgraphId(), editor.getSubgraphId(level2));

    editor.exitSubgraph();
    EXPECT_EQ(editor.getCurrentSubgraphId(), editor.getSubgraphId(level1));

    editor.exitSubgraph();
    EXPECT_EQ(editor.getCurrentSubgraphId(), -1);

    // Test de navigation directe
    editor.enterSubgraphByUUID(level3);
    EXPECT_EQ(editor.getCurrentSubgraphId(), editor.getSubgraphId(level3));

    editor.enterSubgraphByUUID(level1);
    EXPECT_EQ(editor.getCurrentSubgraphId(), editor.getSubgraphId(level1));
}