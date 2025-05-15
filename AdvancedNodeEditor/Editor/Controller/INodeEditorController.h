#ifndef I_NODE_EDITOR_CONTROLLER_H
#define I_NODE_EDITOR_CONTROLLER_H

#include "../Model/NodeEditorModel.h"
#include "../../Core/Types/CoreTypes.h"
#include <memory>
#include <functional>

namespace NodeEditorCore {

class INodeEditorController {
public:
    virtual ~INodeEditorController() = default;

    virtual int addNode(const std::string& name, const std::string& type, const Vec2& position) = 0;
    virtual void removeNode(int nodeId) = 0;
    virtual void updateNode(int nodeId, const std::function<void(Node&)>& updateFn) = 0;

    virtual int addPin(int nodeId, const std::string& name, bool isInput, PinType type = PinType::Blue, PinShape shape = PinShape::Circle) = 0;
    virtual void removePin(int nodeId, int pinId) = 0;
    virtual void updatePin(int nodeId, int pinId, const std::function<void(Pin&)>& updateFn) = 0;

    virtual int addConnection(int startNodeId, int startPinId, int endNodeId, int endPinId) = 0;
    virtual void removeConnection(int connectionId) = 0;
    virtual void updateConnection(int connectionId, const std::function<void(NodeEditorModel::Connection&)>& updateFn) = 0;

    virtual int addGroup(const std::string& name, const Vec2& position, const Vec2& size) = 0;
    virtual void removeGroup(int groupId) = 0;
    virtual void updateGroup(int groupId, const std::function<void(Group&)>& updateFn) = 0;
    virtual void addNodeToGroup(int nodeId, int groupId) = 0;
    virtual void removeNodeFromGroup(int nodeId, int groupId) = 0;

    virtual int createSubgraph(const std::string& name) = 0;
    virtual void removeSubgraph(int subgraphId) = 0;
    virtual int addNodeToSubgraph(int nodeId, int subgraphId) = 0;
    virtual int addSubgraphNode(int subgraphId, const std::string& name, const Vec2& position) = 0;
    virtual void enterSubgraph(int subgraphId) = 0;
    virtual void exitSubgraph() = 0;

    virtual void selectNode(int nodeId, bool append = false) = 0;
    virtual void deselectNode(int nodeId) = 0;
    virtual void selectAllNodes() = 0;
    virtual void deselectAllNodes() = 0;
    virtual std::vector<int> getSelectedNodes() const = 0;

    virtual void addEventListener(EventType type, EventCallback callback) = 0;
    virtual void removeEventListener(EventType type, EventCallback callback) = 0;
    virtual void dispatchEvent(const Event& event) = 0;

    virtual std::shared_ptr<NodeEditorModel> getModel() const = 0;
};

}

#endif