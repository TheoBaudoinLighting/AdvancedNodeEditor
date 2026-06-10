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
            enum class ParameterType {
                Float,
                Int,
                Bool,
                String,
                Vec3,
                File,
                Color,
                Enum
            };

            struct EnumOption {
                std::string value;
                std::string label;
            };

            struct ParameterDescriptor {
                std::string name;
                std::string label;
                ParameterType type = ParameterType::String;
                std::any defaultValue;
                std::string drivenPin;
                std::string category;
                std::string description;
                std::vector<EnumOption> enumOptions;
                bool persistentOutput = false;
            };

            std::string type;
            std::string name;
            std::string category;
            std::string description;
            std::string iconSymbol;
            std::vector<std::pair<std::string, PinType> > inputs;
            std::vector<std::pair<std::string, PinType> > outputs;
            std::vector<ParameterDescriptor> parameters;
        };

        struct EvaluationResult {
            std::any value;
            std::vector<UUID> evaluationOrder;
        };

        struct SubgraphBreadcrumbItem {
            int subgraphId = -1;
            std::string label;
        };

        NodeEditorAPI();

        ~NodeEditorAPI();

        void initialize();

        void setupWindow(void *window, void *renderer);

        void registerNodeType(const NodeDefinition &definition);

        const NodeDefinition *getNodeDefinition(const std::string &type) const;

        std::vector<NodeDefinition::ParameterDescriptor> getParameterDescriptors(const std::string &type) const;

        void beginFrame();

        void render();

        void endFrame();

        void setWindowSize(float width, float height);

        void setWindowSize(const Vec2 &size);

        Vec2 getWindowSize() const;

        UUID createGraph(const std::string &name);

        void switchToGraph(const UUID &graphId);

        void enterSubgraph(const UUID &subgraphId);

        void exitSubgraph();

        std::vector<SubgraphBreadcrumbItem> getCurrentSubgraphBreadcrumb() const;

        bool navigateToSubgraphBreadcrumb(int subgraphId);

        std::vector<UUID> traceConnectionPath(const UUID &startNodeId, const std::string &outputPinName,
                                              const UUID &endNodeId,
                                              const std::string &inputPinName);

        void centerView();

        void centerOnNode(const UUID &nodeId);

        void zoomToFit(float padding = 50.0f);

        void zoomToFitSelected(float padding = 50.0f);

        void smoothCenterView(float duration = 0.3f);

        void smoothCenterOnNode(const UUID &nodeId, float duration = 0.3f);

        void setViewPosition(const Vec2 &position);

        Vec2 getViewPosition() const;

        void setViewScale(float scale);

        float getViewScale() const;

        UUID createNode(const std::string &type, const std::string &name, const Vec2 &position);

        void removeNode(const UUID &nodeId);

        void selectNode(const UUID &nodeId, bool append = false);

        void deselectNode(const UUID &nodeId);

        void selectAllNodes();

        void deselectAllNodes();

        void alignSelectedNodesLeft();

        void alignSelectedNodesRight();

        void autoLayoutSelectedNodesTopToBottom();

        void toggleNodeSnapEnabled();

        std::vector<UUID> getSelectedNodes() const;

        UUID addPinToNode(const UUID &nodeId, const std::string &name, bool isInput, PinType type = PinType::Blue);

        UUID connectNodes(const UUID &startNodeId, const std::string &outputPinName,
                          const UUID &endNodeId, const std::string &inputPinName);

        bool disconnectNodes(const UUID &connectionId);

        UUID addRerouteToConnection(const UUID &connectionId, const Vec2 &position);

        UUID createGroup(const std::string &name, const Vec2 &position, const Vec2 &size);

        void addNodeToGroup(const UUID &nodeId, const UUID &groupId);

        void removeNodeFromGroup(const UUID &nodeId, const UUID &groupId);

        void registerEvaluator(const std::string &nodeType,
                               std::function<std::any(const std::vector<std::any> &)> evaluator);

        EvaluationResult evaluateGraph(const UUID &outputNodeId = "");

        void setConstantValue(const UUID &nodeId, const std::any &value);

        void setPersistentOutputValue(const UUID &nodeId, const std::string &outputName, const std::any &value);

        std::any getConstantValue(const UUID &nodeId) const;

        std::any getPersistentOutputValue(const UUID &nodeId, const std::string &outputName) const;

        void setNodeName(const UUID &nodeId, const std::string &name);

        std::string getNodeName(const UUID &nodeId) const;

        void setNodePosition(const UUID &nodeId, const Vec2 &position);

        Vec2 getNodePosition(const UUID &nodeId) const;

        void setNodeEnabled(const UUID &nodeId, bool enabled);

        bool getNodeEnabled(const UUID &nodeId) const;

        void setNodeInputValue(const UUID &nodeId, const std::string &inputName, const std::any &value);
        void setNodeInputValueSilent(const UUID &nodeId, const std::string &inputName, const std::any &value);

        std::any getNodeInputValue(const UUID &nodeId, const std::string &inputName) const;

        ListenerHandle addNodeCreatedListener(std::function<void(const UUID &)> callback);
        ListenerHandle addNodeRemovedListener(std::function<void(const UUID &)> callback);
        ListenerHandle addConnectionCreatedListener(std::function<void(const UUID &)> callback);
        ListenerHandle addConnectionRemovedListener(std::function<void(const UUID &)> callback);

        void removeNodeCreatedListener(ListenerHandle handle);
        void removeNodeRemovedListener(ListenerHandle handle);
        void removeConnectionCreatedListener(ListenerHandle handle);
        void removeConnectionRemovedListener(ListenerHandle handle);

        void executeCommand(const std::string &command, const std::any &data = std::any());

        NodeEditor *getUnderlyingEditor();

        void executeConnectedNodes();
        void executeNodeChain(const UUID &startNodeId);
    private:
        std::unique_ptr<NodeEditor> m_editor;
        std::unordered_map<std::string, std::function<std::any(const std::vector<std::any> &)> > m_evaluators;
        std::unordered_map<UUID, std::any> m_constantValues;
        std::unordered_map<std::string, NodeDefinition> m_nodeDefinitions;

        UUID createNodeWithPins(const std::string &type, const std::string &name, const Vec2 &position);

        int findPinIdByName(const UUID &nodeId, const std::string &pinName, bool isInput);
    };
}

#endif
