#ifndef NODE_EDITOR_H
#define NODE_EDITOR_H

#include "Components/Node/NodeComponents.h"
#include "Core/Style/StyleDefinitions.h"
#include "Core/Types/UuidTypes.h"
#include "Core/Types/SerializedTypes.h"
#include <functional>
#include <stack>
#include <vector>
#include <map>
#include <memory>
#include <any>
#include <string>

#include "Evaluation/NodeEditorEvaluation.h"

namespace NodeEditorCore {
    enum class InteractionMode;

    class NodeEditor {
    public:
        using NodeCallback = std::function<void(int nodeId, const ANE::UUID& nodeUuid)>;
        using ConnectionCallback = std::function<void(int connectionId, const ANE::UUID& connectionUuid)>;
        using CanConnectCallback = std::function<bool(const ANE::Pin &startPin, const ANE::Pin &endPin)>;

        NodeEditor();
        ~NodeEditor();

        void beginFrame();
        void render();
        void endFrame();

        int addNode(const std::string &name, const std::string &type, const Vec2 &pos, const ANE::UUID& uuid = "");
        void removeNode(int nodeId);
        void removeNodeByUUID(const ANE::UUID& uuid);

        Node* getNode(int nodeId);
        Node* getNodeByUUID(const ANE::UUID& uuid);

        const Node* getNode(int nodeId) const;
        const Node* getNodeByUUID(const ANE::UUID& uuid) const;

        const std::vector<Node>& getNodes() const;

        ANE::UUID getNodeUUID(int nodeId) const;
        int getNodeId(const ANE::UUID& uuid) const;

        int addPin(int nodeId, const std::string &name, bool isInput, PinType type = PinType::Blue,
                  PinShape shape = PinShape::Circle, const ANE::UUID& uuid = "");
        int addPinByNodeUUID(const ANE::UUID& nodeUuid, const std::string &name, bool isInput,
                            PinType type = PinType::Blue, PinShape shape = PinShape::Circle, const ANE::UUID& uuid = "");

        void removePin(int nodeId, int pinId);
        void removePinByUUID(const ANE::UUID& nodeUuid, const ANE::UUID& pinUuid);

        ANE::Pin* getPin(int nodeId, int pinId);
        ANE::Pin* getPinByUUID(const ANE::UUID& nodeUuid, const ANE::UUID& pinUuid);

        const ANE::Pin* getPin(int nodeId, int pinId) const;
        const ANE::Pin* getPinByUUID(const ANE::UUID& nodeUuid, const ANE::UUID& pinUuid) const;

        ANE::UUID getPinUUID(int nodeId, int pinId) const;

        int addConnection(int startNodeId, int startPinId, int endNodeId, int endPinId, const ANE::UUID& uuid = "");
        int addConnectionByUUID(const ANE::UUID& startNodeUuid, const ANE::UUID& startPinUuid,
                              const ANE::UUID& endNodeUuid, const ANE::UUID& endPinUuid, const ANE::UUID& uuid = "");

        void removeConnection(int connectionId);
        void removeConnectionByUUID(const ANE::UUID& uuid);

        Connection* getConnection(int connectionId);
        Connection* getConnectionByUUID(const ANE::UUID& uuid);

        const Connection* getConnection(int connectionId) const;
        const Connection* getConnectionByUUID(const ANE::UUID& uuid) const;

        const std::vector<Connection>& getConnections() const;

        ANE::UUID getConnectionUUID(int connectionId) const;
        int getConnectionId(const ANE::UUID& uuid) const;

        bool isConnected(int nodeId, int pinId) const;
        bool isConnectedByUUID(const ANE::UUID& nodeUuid, const ANE::UUID& pinUuid) const;

        int addGroup(const std::string &name, const Vec2 &pos, const Vec2 &size, const ANE::UUID& uuid = "");
        void removeGroup(int groupId);
        void removeGroupByUUID(const ANE::UUID& uuid);

        Group* getGroup(int groupId);
        Group* getGroupByUUID(const ANE::UUID& uuid);

        const Group* getGroup(int groupId) const;
        const Group* getGroupByUUID(const ANE::UUID& uuid) const;

        ANE::UUID getGroupUUID(int groupId) const;
        int getGroupId(const ANE::UUID& uuid) const;

        void addNodeToGroup(int nodeId, int groupId);
        void addNodeToGroupByUUID(const ANE::UUID& nodeUuid, const ANE::UUID& groupUuid);

        void removeNodeFromGroup(int nodeId, int groupId);
        void removeNodeFromGroupByUUID(const ANE::UUID& nodeUuid, const ANE::UUID& groupUuid);

        void selectNode(int nodeId, bool append = false);
        void selectNodeByUUID(const ANE::UUID& uuid, bool append = false);

        void deselectNode(int nodeId);
        void deselectNodeByUUID(const ANE::UUID& uuid);

        void selectAllNodes();
        void deselectAllNodes();

        std::vector<int> getSelectedNodes() const;
        std::vector<ANE::UUID> getSelectedNodeUUIDs() const;

        void setViewPosition(const Vec2 &position);
        Vec2 getViewPosition() const;

        void setViewScale(float scale);
        float getViewScale() const;

        void centerView();
        void centerOnNode(int nodeId);
        void centerOnNodeByUUID(const ANE::UUID& uuid);

        void setStyle(const NodeEditorStyle &style);
        const NodeEditorStyle &getStyle() const;

        void setNodeCreatedCallback(NodeCallback callback);
        void setNodeRemovedCallback(NodeCallback callback);

        void setConnectionCreatedCallback(ConnectionCallback callback);
        void setConnectionRemovedCallback(ConnectionCallback callback);

        void setCanConnectCallback(CanConnectCallback callback);

        Vec2 screenToCanvas(const Vec2 &pos) const;
        Vec2 canvasToScreen(const Vec2 &pos) const;

        int getHoveredNodeId() const;
        ANE::UUID getHoveredNodeUUID() const;

        int getHoveredPinId() const;
        ANE::UUID getHoveredPinUUID() const;

        int getHoveredConnectionId() const;
        ANE::UUID getHoveredConnectionUUID() const;

        int getHoveredGroupId() const;
        ANE::UUID getHoveredGroupUUID() const;

        void selectConnection(int connectionId, bool append = false);
        void selectConnectionByUUID(const ANE::UUID& uuid, bool append = false);

        void deselectConnection(int connectionId);
        void deselectConnectionByUUID(const ANE::UUID& uuid);

        void deselectAllConnections();

        void exposeNodeInput(int nodeId, int pinId);
        void exposeNodeInputByUUID(const ANE::UUID& nodeUuid, const ANE::UUID& pinUuid);

        void exposeNodeOutput(int nodeId, int pinId);
        void exposeNodeOutputByUUID(const ANE::UUID& nodeUuid, const ANE::UUID& pinUuid);

        void unexposeNodeInput(int nodeId, int pinId);
        void unexposeNodeInputByUUID(const ANE::UUID& nodeUuid, const ANE::UUID& pinUuid);

        void unexposeNodeOutput(int nodeId, int pinId);
        void unexposeNodeOutputByUUID(const ANE::UUID& nodeUuid, const ANE::UUID& pinUuid);

        void addNodeToSubgraph(int nodeId, int subgraphId);
        void addNodeToSubgraphByUUID(const ANE::UUID& nodeUuid, const ANE::UUID& subgraphUuid);

        void removeNodeFromSubgraph(int nodeId, int subgraphId);
        void removeNodeFromSubgraphByUUID(const ANE::UUID& nodeUuid, const ANE::UUID& subgraphUuid);

        void addConnectionToSubgraph(int connectionId, int subgraphId);
        void addConnectionToSubgraphByUUID(const ANE::UUID& connectionUuid, const ANE::UUID& subgraphUuid);

        void removeConnectionFromSubgraph(int connectionId, int subgraphId);
        void removeConnectionFromSubgraphByUUID(const ANE::UUID& connectionUuid, const ANE::UUID& subgraphUuid);

        std::vector<int> getNodesInSubgraph(int subgraphId) const;
        std::vector<ANE::UUID> getNodesInSubgraphByUUID(const ANE::UUID& subgraphUuid) const;

        std::vector<int> getConnectionsInSubgraph(int subgraphId) const;
        std::vector<ANE::UUID> getConnectionsInSubgraphByUUID(const ANE::UUID& subgraphUuid) const;

        void saveSubgraphViewState(int subgraphId);
        void saveSubgraphViewStateByUUID(const ANE::UUID& subgraphUuid);

        void restoreSubgraphViewState(int subgraphId);
        void restoreSubgraphViewStateByUUID(const ANE::UUID& subgraphUuid);

        void setCurrentSubgraphId(int subgraphId);
        void setCurrentSubgraphByUUID(const ANE::UUID& uuid);

        int getCurrentSubgraphId() const;
        ANE::UUID getCurrentSubgraphUUID() const;

        bool isNodeInCurrentSubgraph(const Node &node) const;
        bool isSubgraphContainer(const Node &node) const;
        bool isNodeInSubgraph(const Node &node, int subgraphId) const;
        bool isNodeInSubgraphByUUID(const Node &node, const ANE::UUID& subgraphUuid) const;
        bool isSubgraphContainer(int nodeId) const;
        bool isSubgraphContainerByUUID(const ANE::UUID& uuid) const;
        int getSubgraphFromNode(int nodeId) const;
        ANE::UUID getSubgraphUUIDFromNode(const ANE::UUID& nodeUuid) const;
        int getNodeSubgraph(int nodeId) const;

        void setSubgraphIdForNode(int nodeId, int subgraphId);

        void setSubgraphUUIDForNode(int nodeId, const ANE::UUID &uuid);

        ANE::UUID getSubgraphUUIDForNode(int nodeId) const;

        int getSubgraphIdForNode(int nodeId) const;

        ANE::UUID getNodeSubgraphUUID(const ANE::UUID& nodeUuid) const;

        void setDebugMode(bool enable) { m_debugMode = enable; }
        bool isDebugMode() const { return m_debugMode; }

        std::vector<int> getEvaluationOrder() const;
        std::vector<ANE::UUID> getEvaluationOrderUUIDs() const;

        std::vector<ANE::NodeEvaluator::ConnectionInfo> getInputConnections(int nodeId);
        std::vector<ANE::NodeEvaluator::ConnectionInfo> getInputConnectionsByUUID(const ANE::UUID& nodeUuid);

        std::vector<ANE::NodeEvaluator::ConnectionInfo> getOutputConnections(int nodeId);
        std::vector<ANE::NodeEvaluator::ConnectionInfo> getOutputConnectionsByUUID(const ANE::UUID& nodeUuid);

    private:
        struct State {
            std::vector<Node> nodes;
            ANE::UUIDMap<Node*> nodeUuidMap;
            std::vector<Connection> connections;
            ANE::UUIDMap<Connection*> connectionUuidMap;
            std::vector<Group> groups;
            ANE::UUIDMap<Group*> groupUuidMap;

            Vec2 viewPosition;
            float viewScale;

            int nextNodeId;
            int nextPinId;
            int nextConnectionId;
            int nextGroupId;

            int hoveredNodeId;
            ANE::UUID hoveredNodeUuid;
            int hoveredPinId;
            ANE::UUID hoveredPinUuid;
            int hoveredConnectionId;
            ANE::UUID hoveredConnectionUuid;
            int hoveredGroupId;
            ANE::UUID hoveredGroupUuid;

            int activeNodeId;
            ANE::UUID activeNodeUuid;
            int activeConnectionId;
            ANE::UUID activeConnectionUuid;
            int activeGroupId;
            ANE::UUID activeGroupUuid;

            bool connecting;
            int connectingNodeId;
            ANE::UUID connectingNodeUuid;
            int connectingPinId;
            ANE::UUID connectingPinUuid;

            bool dragging;
            Vec2 dragOffset;

            bool boxSelecting;
            Vec2 boxSelectStart;

            NodeEditorStyle style;

            NodeCallback nodeCreatedCallback;
            NodeCallback nodeRemovedCallback;
            ConnectionCallback connectionCreatedCallback;
            ConnectionCallback connectionRemovedCallback;
            CanConnectCallback canConnectCallback;

            int currentSubgraphId;
            ANE::UUID currentSubgraphUuid;

            InteractionMode interactionMode;
            int contextMenuNodeId;
            ANE::UUID contextMenuNodeUuid;
            int contextMenuConnectionId;
            ANE::UUID contextMenuConnectionUuid;
            int contextMenuGroupId;
            ANE::UUID contextMenuGroupUuid;
            int contextMenuPinId;
            ANE::UUID contextMenuPinUuid;
            Vec2 dragStart;
            Vec2 groupStartSize;
            Vec2 contextMenuPos;

            State();
        };

        State m_state;
        bool m_debugMode;

        void processInteraction();
        void processBoxSelection(const ImVec2 &canvasPos);
        void processNodeDragging();
        void processConnectionCreation();
        void drawGrid(ImDrawList *drawList, const ImVec2 &canvasPos);
        void drawConnections(ImDrawList *drawList, const ImVec2 &canvasPos);
        void drawNodes(ImDrawList *drawList, const ImVec2 &canvasPos);
        void drawGroups(ImDrawList *drawList, const ImVec2 &canvasPos);
        void drawBoxSelection(ImDrawList *drawList);
        void drawNodePins(ImDrawList *drawList, const Node &node, const ImVec2 &nodePos, const ImVec2 &nodeSize,
                        const ImVec2 &canvasPos);
        void drawPinShape(ImDrawList *drawList, const ImVec2 &center, float radius, PinShape shape, ImU32 fillColor,
                        ImU32 borderColor, float borderThickness = 1.0f, bool isHovered = false);
        void drawDragConnection(ImDrawList *drawList, const ImVec2 &canvasPos);
        void drawDebugHitboxes(ImDrawList *drawList, const ImVec2 &canvasPos);
        void drawContextMenu(ImDrawList *drawList);
        std::string pinTypeToString(PinType type) const;
        ImVec2 getPinPos(const Node &node, const ANE::Pin &pin, const ImVec2 &canvasPos) const;
        bool isPinHovered(const Node &node, const ANE::Pin &pin, const ImVec2 &canvasPos);
        bool isConnectionHovered(const Connection &connection, const ImVec2 &canvasPos);
        bool doesConnectionExist(int startNodeId, int startPinId, int endNodeId, int endPinId) const;
        bool doesConnectionExistByUUID(const ANE::UUID& startNodeUuid, const ANE::UUID& startPinUuid,
                                     const ANE::UUID& endNodeUuid, const ANE::UUID& endPinUuid) const;
        bool canCreateConnection(const ANE::Pin &startPin, const ANE::Pin &endPin) const;
        void createConnection(int startNodeId, int startPinId, int endNodeId, int endPinId);
        void createConnectionByUUID(const ANE::UUID& startNodeUuid, const ANE::UUID& startPinUuid,
                                  const ANE::UUID& endNodeUuid, const ANE::UUID& endPinUuid);
        bool isPointInRect(const ImVec2 &point, const ImVec2 &rectMin, const ImVec2 &rectMax) const;
        bool isPointNearLine(const ImVec2 &point, const ImVec2 &lineStart, const ImVec2 &lineEnd,
                           float threshold) const;
        bool isPointNearCubicBezier(const ImVec2 &point, const ImVec2 &p1, const ImVec2 &p2, const ImVec2 &p3,
                                 const ImVec2 &p4, float threshold) const;
        ImU32 ImLerpColor(ImU32 col_a, ImU32 col_b, float t);
        ImVec2 ImBezierCubicCalc(const ImVec2 &p1, const ImVec2 &p2, const ImVec2 &p3, const ImVec2 &p4, float t);
        void startConnectionDrag(int nodeId, int pinId);
        void startConnectionDragByUUID(const ANE::UUID& nodeUuid, const ANE::UUID& pinUuid);
        void startNodeDrag(int nodeId, const ImVec2& mousePos);
        void startNodeDragByUUID(const ANE::UUID& nodeUuid, const ImVec2& mousePos);
        void startGroupInteraction(const ImVec2& mousePos);
        void startBoxSelect(const ImVec2& mousePos);
        void startPanCanvas();
        void endCurrentInteraction();
        void updateHoveredElements(const ImVec2& mousePos);
        void updateCurrentInteraction(const ImVec2& mousePos);
        void processGroupDragging();
        void processGroupResize();
        void processDeleteKeyPress();
        void processContextMenu();
        void processZoom(const ImVec2& mousePos);
        void duplicateNode(int nodeId);
        void duplicateNodeByUUID(const ANE::UUID& uuid);
        std::string getInteractionModeName() const;

        void updateNodeUuidMap();
        void updateConnectionUuidMap();
        void updateGroupUuidMap();
    };
}

namespace ANE {
    struct NodeTypeInfo {
        std::string name;
        std::string category;
        std::string description;
        std::function<Node*(const Vec2 &)> builder;
    };

    using NodeEvaluateCallback = std::function<void(Node &, const std::unordered_map<std::string, std::any> &)>;
    using NodeEvaluateCallbackUUID = std::function<void(const ANE::UUID&, Node &, const std::unordered_map<std::string, std::any> &)>;
    using ActionCallback = std::function<void(const std::string &, const std::unordered_map<std::string, std::any> &)>;
    using NodeOverlayCallback = std::function<void(ImDrawList *, const Node &, const ImVec2 &)>;
    using NodeOverlayCallbackUUID = std::function<void(ImDrawList *, const ANE::UUID&, const Node &, const ImVec2 &)>;

    class NodeEvaluationContext {
    public:
        void setValue(const std::string &key, const std::any &value) {
            m_values[key] = value;
        }

        template<typename T>
        T getValue(const std::string &key, const T &defaultValue = T()) const {
            auto it = m_values.find(key);
            if (it != m_values.end()) {
                try {
                    return std::any_cast<T>(it->second);
                } catch (const std::bad_any_cast &) {
                }
            }
            return defaultValue;
        }

        std::unordered_map<std::string, std::any> getValues() const {
            return m_values;
        }

    private:
        std::unordered_map<std::string, std::any> m_values;
    };

    class NodeEditor {
    public:
        using CanConnectCallback = std::function<bool(const Pin &startPin, const Pin &endPin)>;

        NodeEditor();
        ~NodeEditor();

        void beginFrame();
        void render();
        void endFrame();

        int addNode(const std::string &name, const std::string &type, const Vec2 &position, const ANE::UUID& uuid = "");
        ANE::UUID addNodeWithUUID(const std::string &name, const std::string &type, const Vec2 &position);

        Node *getNode(int nodeId);
        Node *getNodeByUUID(const ANE::UUID& uuid);

        void removeNode(int nodeId);
        void removeNodeByUUID(const ANE::UUID& uuid);

        ANE::UUID getNodeUUID(int nodeId) const;
        int getNodeId(const ANE::UUID& uuid) const;

        int addPin(int nodeId, const std::string &name, bool isInput, PinType type = PinType::Blue,
                 PinShape shape = PinShape::Circle, const ANE::UUID& uuid = "");
        ANE::UUID addPinWithUUID(int nodeId, const std::string &name, bool isInput, PinType type = PinType::Blue,
                          PinShape shape = PinShape::Circle);
        ANE::UUID addPinWithUUIDByNodeUUID(const ANE::UUID& nodeUuid, const std::string &name, bool isInput,
                                    PinType type = PinType::Blue, PinShape shape = PinShape::Circle);

        Pin *getPin(int nodeId, int pinId);
        Pin *getPinByUUID(const ANE::UUID& nodeUuid, const ANE::UUID& pinUuid);

        int addConnection(int startNodeId, int startPinId, int endNodeId, int endPinId, const ANE::UUID& uuid = "");
        ANE::UUID addConnectionWithUUID(int startNodeId, int startPinId, int endNodeId, int endPinId);
        ANE::UUID addConnectionWithUUIDByUUID(const ANE::UUID& startNodeUuid, const ANE::UUID& startPinUuid,
                                      const ANE::UUID& endNodeUuid, const ANE::UUID& endPinUuid);

        void removeConnection(int connectionId);
        void removeConnectionByUUID(const ANE::UUID& uuid);

        int addGroup(const std::string &name, const Vec2 &position, const Vec2 &size, const ANE::UUID& uuid = "");
        ANE::UUID addGroupWithUUID(const std::string &name, const Vec2 &position, const Vec2 &size);

        Group *getGroup(int groupId);
        Group *getGroupByUUID(const ANE::UUID& uuid);

        void removeGroup(int groupId);
        void removeGroupByUUID(const ANE::UUID& uuid);

        void addNodeToGroup(int nodeId, int groupId);
        void addNodeToGroupByUUID(const ANE::UUID& nodeUuid, const ANE::UUID& groupUuid);

        void selectNode(int nodeId, bool append = false);
        void selectNodeByUUID(const ANE::UUID& uuid, bool append = false);

        std::vector<int> getSelectedNodes() const;
        std::vector<ANE::UUID> getSelectedNodeUUIDs() const;

        void centerView();

        void setViewScale(float scale);
        float getViewScale() const;

        void setViewPosition(const Vec2 &position);
        Vec2 getViewPosition() const;

        void setStyle(const EditorStyle &style);
        EditorStyle getStyle() const;

        void setCanConnectCallback(CanConnectCallback callback);

        void setNodeEvaluateCallback(NodeEvaluateCallback callback);
        void setNodeEvaluateCallbackUUID(NodeEvaluateCallbackUUID callback);

        void evaluateNode(int nodeId, const NodeEvaluationContext &context = NodeEvaluationContext());
        void evaluateNodeByUUID(const ANE::UUID& uuid, const NodeEvaluationContext &context = NodeEvaluationContext());

        void setActionCallback(ActionCallback callback);

        void dispatchAction(const std::string &action, const std::unordered_map<std::string, std::any> &data = {});

        void setNodeOverlayCallback(NodeOverlayCallback callback);
        void setNodeOverlayCallbackUUID(NodeOverlayCallbackUUID callback);

        void registerNodeType(const std::string &type, const std::string &category, const std::string &description,
                            std::function<Node*(const Vec2 &)> builder);

        std::vector<NodeTypeInfo> getRegisteredNodeTypes() const;

        Node *createNodeOfType(const std::string &type, const Vec2 &position);
        Node *createNodeOfTypeWithUUID(const std::string &type, const Vec2 &position, ANE::UUID& outUuid);

        SerializedState serialize() const;
        void deserialize(const SerializedState &state);

        int createSubgraph(const std::string &name, const ANE::UUID& uuid = "");
        ANE::UUID createSubgraphWithUUID(const std::string &name);

        Subgraph *getSubgraph(int subgraphId);
        Subgraph *getSubgraphByUUID(const ANE::UUID& uuid);

        void removeSubgraph(int subgraphId);
        void removeSubgraphByUUID(const ANE::UUID& uuid);

        ANE::UUID getSubgraphUUID(int subgraphId) const;
        int getSubgraphId(const ANE::UUID& uuid) const;

        Node *createSubgraphNode(int subgraphId, const std::string &name, const Vec2 &position, const ANE::UUID& uuid = "");
        Node *createSubgraphNodeByUUID(const ANE::UUID& subgraphUuid, const std::string &name, const Vec2 &position,
                                     const ANE::UUID& uuid = "");

        bool enterSubgraph(int subgraphId);
        bool enterSubgraphByUUID(const ANE::UUID& uuid);

        bool exitSubgraph();

        int getCurrentSubgraphId() const;
        ANE::UUID getCurrentSubgraphUUID() const;

        std::vector<int> getSubgraphStack() const;
        std::vector<ANE::UUID> getSubgraphStackUUIDs() const;

        void exposeNodeInput(int nodeId, int pinId);
        void exposeNodeInputByUUID(const ANE::UUID& nodeUuid, const ANE::UUID& pinUuid);

        void exposeNodeOutput(int nodeId, int pinId);
        void exposeNodeOutputByUUID(const ANE::UUID& nodeUuid, const ANE::UUID& pinUuid);

        void unexposeNodeInput(int nodeId, int pinId);
        void unexposeNodeInputByUUID(const ANE::UUID& nodeUuid, const ANE::UUID& pinUuid);

        void unexposeNodeOutput(int nodeId, int pinId);
        void unexposeNodeOutputByUUID(const ANE::UUID& nodeUuid, const ANE::UUID& pinUuid);

        void saveSubgraphViewState(int subgraphId);
        void saveSubgraphViewStateByUUID(const ANE::UUID& uuid);

        void restoreSubgraphViewState(int subgraphId);
        void restoreSubgraphViewStateByUUID(const ANE::UUID& uuid);


        void setDebugMode(bool enable);
        bool isDebugMode() const;

        std::vector<ANE::UUID> getAllNodeUUIDs() const;
        std::vector<ANE::UUID> getAllConnectionUUIDs() const;
        std::vector<ANE::UUID> getAllGroupUUIDs() const;
        std::vector<ANE::UUID> getAllSubgraphUUIDs() const;
        
        std::vector<NodeEvaluator::ConnectionInfo> getInputConnections(int nodeId);
        std::vector<NodeEvaluator::ConnectionInfo> getInputConnectionsByUUID(const ANE::UUID& nodeUuid);
        std::vector<NodeEvaluator::ConnectionInfo> getOutputConnections(int nodeId);
        std::vector<NodeEvaluator::ConnectionInfo> getOutputConnectionsByUUID(const ANE::UUID& nodeUuid);
        std::vector<int> getEvaluationOrder();
        std::vector<ANE::UUID> getEvaluationOrderUUIDs();

    private:
        NodeEditorCore::NodeEditor m_editor;
        std::stack<int> m_subgraphStack;
        std::stack<ANE::UUID> m_subgraphUuidStack;
        int m_currentSubgraphId;
        ANE::UUID m_currentSubgraphUuid;
        std::map<int, std::shared_ptr<Subgraph>> m_subgraphs;
        UUIDMap<std::shared_ptr<Subgraph>> m_subgraphsByUuid;
        NodeEvaluateCallback m_nodeEvaluateCallback;
        NodeEvaluateCallbackUUID m_nodeEvaluateCallbackUUID;
        ActionCallback m_actionCallback;
        NodeOverlayCallback m_nodeOverlayCallback;
        NodeOverlayCallbackUUID m_nodeOverlayCallbackUUID;
        std::unordered_map<std::string, NodeTypeInfo> m_registeredNodeTypes;

        NodeEditorCore::NodeEditorStyle convertToInternalStyle(const EditorStyle &style) const;
        EditorStyle convertToAPIStyle(const NodeEditorCore::NodeEditorStyle &style) const;

        void addNodeToSubgraph(int nodeId, int subgraphId);
        void addNodeToSubgraphByUUID(const ANE::UUID& nodeUuid, const ANE::UUID& subgraphUuid);

        void removeNodeFromSubgraph(int nodeId, int subgraphId);
        void removeNodeFromSubgraphByUUID(const ANE::UUID& nodeUuid, const ANE::UUID& subgraphUuid);

        void addConnectionToSubgraph(int connectionId, int subgraphId);
        void addConnectionToSubgraphByUUID(const ANE::UUID& connectionUuid, const ANE::UUID& subgraphUuid);

        void removeConnectionFromSubgraph(int connectionId, int subgraphId);
        void removeConnectionFromSubgraphByUUID(const ANE::UUID& connectionUuid, const ANE::UUID& subgraphUuid);

        std::vector<int> getNodesInSubgraph(int subgraphId) const;
        std::vector<ANE::UUID> getNodesInSubgraphByUUID(const ANE::UUID& subgraphUuid) const;

        std::vector<int> getConnectionsInSubgraph(int subgraphId) const;
        std::vector<ANE::UUID> getConnectionsInSubgraphByUUID(const ANE::UUID& subgraphUuid) const;

        struct ConnectionInfo {
            int id;
            ANE::UUID uuid;
            int startNodeId;
            ANE::UUID startNodeUuid;
            int endNodeId;
            ANE::UUID endNodeUuid;
        };

        ConnectionInfo getConnectionInfo(int connectionId) const;
        ConnectionInfo getConnectionInfoByUUID(const ANE::UUID& uuid) const;

        void updateSubgraphUuidMap();
    };
}

#endif