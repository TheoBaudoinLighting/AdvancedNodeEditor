#include <gtest/gtest.h>
#include "../../AdvancedNodeEditor/Core/NodeEditor.h"

using namespace NodeEditorCore;

class GroupTests : public ::testing::Test {
protected:
    NodeEditor editor;
    
    void SetUp() override {
        editor.addNode("Node1", "Default", Vec2(100, 100));
        editor.addNode("Node2", "Default", Vec2(300, 100));
    }
};

TEST_F(GroupTests, AddGroup) {
    int groupId = editor.addGroup("TestGroup", Vec2(50, 50), Vec2(400, 200));
    EXPECT_EQ(groupId, 1);
    
    const Group* group = editor.getGroup(groupId);
    ASSERT_NE(group, nullptr);
    EXPECT_EQ(group->name, "TestGroup");
    EXPECT_EQ(group->position.x, 50);
    EXPECT_EQ(group->position.y, 50);
    EXPECT_EQ(group->size.x, 400);
    EXPECT_EQ(group->size.y, 200);
    EXPECT_TRUE(group->nodes.empty());
    EXPECT_FALSE(group->collapsed);
    EXPECT_FALSE(group->selected);
}

TEST_F(GroupTests, GetGroup) {
    int groupId = editor.addGroup("TestGroup", Vec2(50, 50), Vec2(400, 200));
    
    const Group* group = editor.getGroup(groupId);
    ASSERT_NE(group, nullptr);
    EXPECT_EQ(group->id, groupId);
    
    const Group* nonExistentGroup = editor.getGroup(999);
    EXPECT_EQ(nonExistentGroup, nullptr);
}

TEST_F(GroupTests, RemoveGroup) {
    int groupId = editor.addGroup("TestGroup", Vec2(50, 50), Vec2(400, 200));
    
    editor.removeGroup(groupId);
    
    const Group* group = editor.getGroup(groupId);
    EXPECT_EQ(group, nullptr);
}

TEST_F(GroupTests, AddNodeToGroup) {
    int groupId = editor.addGroup("TestGroup", Vec2(50, 50), Vec2(400, 200));
    
    editor.addNodeToGroup(1, groupId);
    editor.addNodeToGroup(2, groupId);
    
    const Group* group = editor.getGroup(groupId);
    ASSERT_NE(group, nullptr);
    EXPECT_EQ(group->nodes.size(), 2);
    EXPECT_TRUE(group->nodes.find(1) != group->nodes.end());
    EXPECT_TRUE(group->nodes.find(2) != group->nodes.end());
    
    Node* node1 = editor.getNode(1);
    Node* node2 = editor.getNode(2);
    ASSERT_NE(node1, nullptr);
    ASSERT_NE(node2, nullptr);
    EXPECT_EQ(node1->groupId, groupId);
    EXPECT_EQ(node2->groupId, groupId);
}

TEST_F(GroupTests, RemoveNodeFromGroup) {
    int groupId = editor.addGroup("TestGroup", Vec2(50, 50), Vec2(400, 200));
    
    editor.addNodeToGroup(1, groupId);
    editor.addNodeToGroup(2, groupId);
    
    editor.removeNodeFromGroup(1, groupId);
    
    const Group* group = editor.getGroup(groupId);
    ASSERT_NE(group, nullptr);
    EXPECT_EQ(group->nodes.size(), 1);
    EXPECT_FALSE(group->nodes.find(1) != group->nodes.end());
    EXPECT_TRUE(group->nodes.find(2) != group->nodes.end());
    
    Node* node1 = editor.getNode(1);
    Node* node2 = editor.getNode(2);
    ASSERT_NE(node1, nullptr);
    ASSERT_NE(node2, nullptr);
    EXPECT_EQ(node1->groupId, -1);
    EXPECT_EQ(node2->groupId, groupId);
}

TEST_F(GroupTests, UUIDOperations) {
    UUID groupUuid = editor.addGroupWithUUID("TestGroup", Vec2(50, 50), Vec2(400, 200));
    EXPECT_FALSE(groupUuid.empty());
    
    int groupId = editor.getGroupId(groupUuid);
    EXPECT_GT(groupId, 0);
    
    UUID retrievedUuid = editor.getGroupUUID(groupId);
    EXPECT_EQ(retrievedUuid, groupUuid);
    
    Group* group = editor.getGroupByUUID(groupUuid);
    ASSERT_NE(group, nullptr);
    EXPECT_EQ(group->id, groupId);
    EXPECT_EQ(group->uuid, groupUuid);
    
    UUID node1Uuid = editor.getNodeUUID(1);
    editor.addNodeToGroupByUUID(node1Uuid, groupUuid);
    
    group = editor.getGroupByUUID(groupUuid);
    ASSERT_NE(group, nullptr);
    EXPECT_EQ(group->nodeUuids.size(), 1);
    EXPECT_TRUE(group->nodeUuids.find(node1Uuid) != group->nodeUuids.end());
    
    editor.removeGroupByUUID(groupUuid);
    group = editor.getGroupByUUID(groupUuid);
    EXPECT_EQ(group, nullptr);
}

TEST_F(GroupTests, GroupMetadata) {
    int groupId = editor.addGroup("TestGroup", Vec2(50, 50), Vec2(400, 200));
    Group* group = editor.getGroup(groupId);
    ASSERT_NE(group, nullptr);
    
    group->setMetadata("intValue", 42);
    group->setMetadata("floatValue", 3.14f);
    group->setMetadata("stringValue", std::string("test"));
    
    EXPECT_EQ(group->getMetadata<int>("intValue"), 42);
    EXPECT_EQ(group->getMetadata<float>("floatValue"), 3.14f);
    EXPECT_EQ(group->getMetadata<std::string>("stringValue"), "test");
    
    EXPECT_EQ(group->getMetadata<int>("nonExistent", 100), 100);
    EXPECT_EQ(group->getMetadata<std::string>("nonExistent", "default"), "default");
}