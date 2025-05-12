#ifndef NODE_EDITOR_CONTROLLER_H
#define NODE_EDITOR_CONTROLLER_H

#include "INodeEditorController.h"
#include "NodeEditorModel.h"
#include <memory>
#include <stack>

namespace ANE {

class NodeEditorController : public INodeEditorController {
public:
    NodeEditorController();
    ~NodeEditorController() override;
    
    int addNode(const std::string& name, const std::string& type, const Vec2& position) override;
    void removeNode(int nodeId) override;
    void updateNode(int nodeId, const std::function<void(Node&)>& updateFn) override;
    
    int addPin(int nodeId, const std::string& name, bool isInput, PinType type = PinType::Blue, PinShape shape = PinShape::Circle) override;
    void removePin(int nodeId, int pinId) override;
    void updatePin(int nodeId, int pinId, const std::function<void(Pin&)>& updateFn) override;
    
    int addConnection(int startNodeId, int startPinId, int endNodeId, int endPinId) override;
    void removeConnection(int connectionId) override;
    void updateConnection(int connectionId, const std::function<void(NodeEditorModel::Connection&)>& updateFn) override;
    
    int addGroup(const std::string& name, const Vec2& position, const Vec2& size) override;
    void removeGroup(int groupId) override;
    void updateGroup(int groupId, const std::function<void(Group&)>& updateFn) override;
    void addNodeToGroup(int nodeId, int groupId) override;
    void removeNodeFromGroup(int nodeId, int groupId) override;
    
    int createSubgraph(const std::string& name) override;
    void removeSubgraph(int subgraphId) override;
    int addNodeToSubgraph(int nodeId, int subgraphId) override;
    int addSubgraphNode(int subgraphId, const std::string& name, const Vec2& position) override;
    void enterSubgraph(int subgraphId) override;
    void exitSubgraph() override;
    
    void selectNode(int nodeId, bool append = false) override;
    void deselectNode(int nodeId) override;
    void selectAllNodes() override;
    void deselectAllNodes() override;
    std::vector<int> getSelectedNodes() const override;
    
    void addEventListener(EventType type, EventCallback callback) override;
    void removeEventListener(EventType type, EventCallback callback) override;
    void dispatchEvent(const Event& event) override;
    
    std::shared_ptr<NodeEditorModel> getModel() const override;
    
private:
    std::shared_ptr<NodeEditorModel> m_model;
    std::stack<int> m_subgraphNavigationStack;
    int m_currentSubgraphId;
};

}

#endif