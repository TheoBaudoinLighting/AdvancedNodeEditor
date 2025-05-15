#include <gtest/gtest.h>
#include "../../AdvancedNodeEditor/Editor/View/NodeEditorView.h"
#include "../../AdvancedNodeEditor/Editor/Controller/NodeEditorController.h"

using namespace NodeEditorCore;

class ViewTests : public ::testing::Test {
protected:
    std::shared_ptr<NodeEditorController> controller;
    std::shared_ptr<NodeEditorView> view;
    
    void SetUp() override {
        controller = std::make_shared<NodeEditorController>();
        view = std::make_shared<NodeEditorView>(controller);
    }
};

TEST_F(ViewTests, SetViewPosition) {
    Vec2 position(100, 100);
    view->setViewPosition(position);
    
    Vec2 retrievedPosition = view->getViewPosition();
    EXPECT_EQ(retrievedPosition.x, position.x);
    EXPECT_EQ(retrievedPosition.y, position.y);
}

TEST_F(ViewTests, SetViewScale) {
    float scale = 2.0f;
    view->setViewScale(scale);
    
    float retrievedScale = view->getViewScale();
    EXPECT_EQ(retrievedScale, scale);
}

TEST_F(ViewTests, AddLayer) {
    bool layerCalled = false;
    
    int layerId = view->addLayer("TestLayer", 10, [&layerCalled](ImDrawList* drawList, const ImVec2& canvasPos) {
        layerCalled = true;
    });
    
    EXPECT_GT(layerId, 0);
    
    view->setLayerVisible(layerId, true);
    view->setLayerZOrder(layerId, 20);
    
}

TEST_F(ViewTests, SetStyle) {
    EditorStyle style;
    style.backgroundColor = Color(0.1f, 0.2f, 0.3f, 1.0f);
    style.gridColor = Color(0.4f, 0.5f, 0.6f, 1.0f);
    style.gridSpacing = 25.0f;
    
    view->setStyle(style);
    
}

TEST_F(ViewTests, CenterView) {
    controller->addNode("Node1", "Default", Vec2(100, 100));
    controller->addNode("Node2", "Default", Vec2(200, 200));
    
    Vec2 oldPosition = view->getViewPosition();
    
    view->centerViewWithSize(800.0f, 600.0f);

    Vec2 newPosition = view->getViewPosition();

    EXPECT_NE(newPosition.x, oldPosition.x);
    EXPECT_NE(newPosition.y, oldPosition.y);
}

TEST_F(ViewTests, CenterOnNode) {
    int nodeId = controller->addNode("TestNode", "Default", Vec2(500, 500));
    
    Vec2 oldPosition = view->getViewPosition();

    view->centerOnNodeWithSize(nodeId, 800.0f, 600.0f);

    Vec2 newPosition = view->getViewPosition();

    EXPECT_NE(newPosition.x, oldPosition.x);
    EXPECT_NE(newPosition.y, oldPosition.y);
}