#ifndef NODE_EDITOR_API_ROBUST_H
#define NODE_EDITOR_API_ROBUST_H

#include "Core/NodeEditor.h"
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <any>

namespace NodeEditorCore {

class NodeEditorAPI {
public:
    struct NodeDefinition {
        std::string type;
        std::string name;
        std::string category;
        std::string description;
        std::string iconSymbol;
        std::vector<std::pair<std::string, PinType>> inputs;
        std::vector<std::pair<std::string, PinType>> outputs;
    };

    struct EvaluationResult {
        std::any value;
        std::vector<UUID> evaluationOrder;
    };

    NodeEditorAPI();
    ~NodeEditorAPI();

    void initialize();
    void setupWindow(void* window, void* renderer);
    void registerNodeType(const NodeDefinition& definition);

    void beginFrame();
    void render();
    void endFrame();

    void setWindowSize(float width, float height);
    void setWindowSize(const Vec2& size);
    Vec2 getWindowSize() const;

    UUID createGraph(const std::string& name);
    void switchToGraph(const UUID& graphId);
    void enterSubgraph(const UUID& subgraphId);
    void exitSubgraph();

    std::vector<UUID> traceConnectionPath(const UUID &startNodeId, const std::string &outputPinName,
                                          const UUID &endNodeId,
                                          const std::string &inputPinName);

    void centerView();
    void centerOnNode(const UUID& nodeId);
    void zoomToFit(float padding = 50.0f);
    void zoomToFitSelected(float padding = 50.0f);
    void smoothCenterView(float duration = 0.3f);
    void smoothCenterOnNode(const UUID& nodeId, float duration = 0.3f);

    void setViewPosition(const Vec2& position);
    Vec2 getViewPosition() const;
    void setViewScale(float scale);
    float getViewScale() const;

    UUID createNode(const std::string& type, const std::string& name, const Vec2& position);
    void removeNode(const UUID& nodeId);
    void selectNode(const UUID& nodeId, bool append = false);
    void deselectNode(const UUID& nodeId);
    void deselectAllNodes();
    std::vector<UUID> getSelectedNodes() const;

    UUID addPinToNode(const UUID& nodeId, const std::string& name, bool isInput, PinType type = PinType::Blue);

    UUID connectNodes(const UUID& startNodeId, const std::string& outputPinName,
                     const UUID& endNodeId, const std::string& inputPinName);
    bool disconnectNodes(const UUID& connectionId);

    UUID addRerouteToConnection(const UUID& connectionId, const Vec2& position);

    UUID createGroup(const std::string& name, const Vec2& position, const Vec2& size);
    void addNodeToGroup(const UUID& nodeId, const UUID& groupId);
    void removeNodeFromGroup(const UUID& nodeId, const UUID& groupId);

    void registerEvaluator(const std::string& nodeType,
                          std::function<std::any(const std::vector<std::any>&)> evaluator);
    EvaluationResult evaluateGraph(const UUID& outputNodeId = "");

    void setConstantValue(const UUID& nodeId, const std::any& value);
    std::any getConstantValue(const UUID& nodeId) const;

    void setNodeCreatedCallback(std::function<void(const UUID&)> callback);
    void setNodeRemovedCallback(std::function<void(const UUID&)> callback);
    void setConnectionCreatedCallback(std::function<void(const UUID&)> callback);
    void setConnectionRemovedCallback(std::function<void(const UUID&)> callback);

    void executeCommand(const std::string& command, const std::any& data = std::any());

    NodeEditor* getUnderlyingEditor();

private:
    std::unique_ptr<NodeEditor> m_editor;
    std::unordered_map<std::string, std::function<std::any(const std::vector<std::any>&)>> m_evaluators;
    std::unordered_map<UUID, std::any> m_constantValues;
    std::unordered_map<std::string, NodeDefinition> m_nodeDefinitions;

    UUID createNodeWithPins(const std::string& type, const std::string& name, const Vec2& position);
    int findPinIdByName(const UUID& nodeId, const std::string& pinName, bool isInput);
};

}

#endif