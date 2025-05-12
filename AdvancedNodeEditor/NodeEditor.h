#ifndef NODE_EDITOR_H
#define NODE_EDITOR_H

#include "NodeComponents.h"
#include "StyleDefinitions.h"
#include <functional>
#include <stack>
#include <vector>
#include <map>
#include <memory>
#include <any>
#include <string>

namespace NodeEditorCore {

class NodeEditor {
public:
    using NodeCallback = std::function<void(int nodeId)>;
    using ConnectionCallback = std::function<void(int connectionId)>;
    using CanConnectCallback = std::function<bool(const ANE::Pin& startPin, const ANE::Pin& endPin)>;

    NodeEditor();
    ~NodeEditor();

    void beginFrame();
    void render();
    void endFrame();

    int addNode(const std::string& name, const std::string& type, const Vec2& pos);
    void removeNode(int nodeId);
    Node* getNode(int nodeId);
    const Node* getNode(int nodeId) const;
    const std::vector<Node>& getNodes() const;

    int addPin(int nodeId, const std::string& name, bool isInput, PinType type = PinType::Blue, PinShape shape = PinShape::Circle);
    void removePin(int nodeId, int pinId);
    ANE::Pin* getPin(int nodeId, int pinId);
    const ANE::Pin* getPin(int nodeId, int pinId) const;

    int addConnection(int startNodeId, int startPinId, int endNodeId, int endPinId);
    void removeConnection(int connectionId);
    Connection* getConnection(int connectionId);
    const Connection* getConnection(int connectionId) const;
    const std::vector<Connection>& getConnections() const;
    bool isConnected(int nodeId, int pinId) const;

    int addGroup(const std::string& name, const Vec2& pos, const Vec2& size);
    void removeGroup(int groupId);
    Group* getGroup(int groupId);
    const Group* getGroup(int groupId) const;
    void addNodeToGroup(int nodeId, int groupId);
    void removeNodeFromGroup(int nodeId, int groupId);

    void selectNode(int nodeId, bool append = false);
    void deselectNode(int nodeId);
    void selectAllNodes();
    void deselectAllNodes();
    std::vector<int> getSelectedNodes() const;

    void setViewPosition(const Vec2& position);
    Vec2 getViewPosition() const;
    void setViewScale(float scale);
    float getViewScale() const;
    void centerView();
    void centerOnNode(int nodeId);

    void setStyle(const NodeEditorStyle& style);
    const NodeEditorStyle& getStyle() const;

    void setNodeCreatedCallback(NodeCallback callback);
    void setNodeRemovedCallback(NodeCallback callback);
    void setConnectionCreatedCallback(ConnectionCallback callback);
    void setConnectionRemovedCallback(ConnectionCallback callback);
    void setCanConnectCallback(CanConnectCallback callback);

    Vec2 screenToCanvas(const Vec2& pos) const;
    Vec2 canvasToScreen(const Vec2& pos) const;

    int getHoveredNodeId() const;
    int getHoveredPinId() const;
    int getHoveredConnectionId() const;
    int getHoveredGroupId() const;

    void selectConnection(int connectionId, bool append = false);
    void deselectConnection(int connectionId);
    void deselectAllConnections();

    void exposeNodeInput(int nodeId, int pinId);
    void exposeNodeOutput(int nodeId, int pinId);
    void unexposeNodeInput(int nodeId, int pinId);
    void unexposeNodeOutput(int nodeId, int pinId);
    
    void addNodeToSubgraph(int nodeId, int subgraphId);
    void removeNodeFromSubgraph(int nodeId, int subgraphId);
    void addConnectionToSubgraph(int connectionId, int subgraphId);
    void removeConnectionFromSubgraph(int connectionId, int subgraphId);
    std::vector<int> getNodesInSubgraph(int subgraphId) const;
    std::vector<int> getConnectionsInSubgraph(int subgraphId) const;
    
    void saveSubgraphViewState(int subgraphId);
    void restoreSubgraphViewState(int subgraphId);

private:
    struct State {
        std::vector<Node> nodes;
        std::vector<Connection> connections;
        std::vector<Group> groups;

        Vec2 viewPosition;
        float viewScale;

        int nextNodeId;
        int nextPinId;
        int nextConnectionId;
        int nextGroupId;

        int hoveredNodeId;
        int hoveredPinId;
        int hoveredConnectionId;
        int hoveredGroupId;

        int activeNodeId;
        int activeConnectionId;
        int activeGroupId;

        bool connecting;
        int connectingNodeId;
        int connectingPinId;

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

        State();
    };

    State m_state;

    void processInteraction();
    void processBoxSelection(const ImVec2& canvasPos);
    void processNodeDragging();
    void processConnectionCreation();

    void drawGrid(ImDrawList* drawList, const ImVec2& canvasPos);
    void drawConnections(ImDrawList* drawList, const ImVec2& canvasPos);
    void drawNodes(ImDrawList* drawList, const ImVec2& canvasPos);
    void drawGroups(ImDrawList* drawList, const ImVec2& canvasPos);
    void drawBoxSelection(ImDrawList* drawList);

    void drawNodePins(ImDrawList* drawList, const Node& node, const ImVec2& nodePos, const ImVec2& nodeSize, const ImVec2& canvasPos);
    void drawPinShape(ImDrawList* drawList, const ImVec2& center, float radius, PinShape shape, ImU32 fillColor, ImU32 borderColor, float borderThickness = 1.0f, bool isHovered = false);
    void drawDragConnection(ImDrawList* drawList, const ImVec2& canvasPos);
    std::string pinTypeToString(PinType type) const;

    ImVec2 getPinPos(const Node& node, const ANE::Pin& pin, const ImVec2& canvasPos) const;
    bool isPinHovered(const Node& node, const ANE::Pin& pin, const ImVec2& canvasPos);
    bool isConnectionHovered(const Connection& connection, const ImVec2& canvasPos);
    bool doesConnectionExist(int startNodeId, int startPinId, int endNodeId, int endPinId) const;
    bool canCreateConnection(const ANE::Pin& startPin, const ANE::Pin& endPin) const;
    void createConnection(int startNodeId, int startPinId, int endNodeId, int endPinId);

    bool isPointInRect(const ImVec2& point, const ImVec2& rectMin, const ImVec2& rectMax) const;
    bool isPointNearLine(const ImVec2& point, const ImVec2& lineStart, const ImVec2& lineEnd, float threshold) const;
    bool isPointNearCubicBezier(const ImVec2& point, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, float threshold) const;

    ImU32 ImLerpColor(ImU32 col_a, ImU32 col_b, float t);
    ImVec2 ImBezierCubicCalc(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, float t);
    
    void setCurrentSubgraphId(int subgraphId);
    int getCurrentSubgraphId() const;
};

}

namespace ANE {

class NodeEditor {
public:
    using CanConnectCallback = std::function<bool(const Pin& startPin, const Pin& endPin)>;

    NodeEditor();
    ~NodeEditor();

    void beginFrame();
    void render();
    void endFrame();

    int addNode(const std::string& name, const std::string& type, const Vec2& position);
    Node* getNode(int nodeId);
    void removeNode(int nodeId);

    int addPin(int nodeId, const std::string& name, bool isInput, PinType type = PinType::Blue, PinShape shape = PinShape::Circle);
    Pin* getPin(int nodeId, int pinId);

    int addConnection(int startNodeId, int startPinId, int endNodeId, int endPinId);
    void removeConnection(int connectionId);

    int addGroup(const std::string& name, const Vec2& position, const Vec2& size);
    Group* getGroup(int groupId);
    void addNodeToGroup(int nodeId, int groupId);

    void selectNode(int nodeId, bool append = false);
    std::vector<int> getSelectedNodes() const;

    void centerView();
    void setViewScale(float scale);
    float getViewScale() const;

    void setStyle(const EditorStyle& style);
    EditorStyle getStyle() const;

    void setCanConnectCallback(CanConnectCallback callback);

    int createSubgraph(const std::string& name);
    Subgraph* getSubgraph(int subgraphId);
    void removeSubgraph(int subgraphId);
    Node* createSubgraphNode(int subgraphId, const std::string& name, const Vec2& position);
    
    bool enterSubgraph(int subgraphId);
    bool exitSubgraph();
    int getCurrentSubgraphId() const;
    std::vector<int> getSubgraphStack() const;

    void exposeNodeInput(int nodeId, int pinId);

    void exposeNodeOutput(int nodeId, int pinId);

    void unexposeNodeInput(int nodeId, int pinId);

    void unexposeNodeOutput(int nodeId, int pinId);

    void saveSubgraphViewState(int subgraphId);
    void restoreSubgraphViewState(int subgraphId);

private:
    NodeEditorCore::NodeEditor m_editor;
    std::stack<int> m_subgraphStack;
    int m_currentSubgraphId;
    std::map<int, std::shared_ptr<Subgraph>> m_subgraphs;

    NodeEditorCore::NodeEditorStyle convertToInternalStyle(const EditorStyle& style) const;
    EditorStyle convertToAPIStyle(const NodeEditorCore::NodeEditorStyle& style) const;

    void addNodeToSubgraph(int nodeId, int subgraphId);

    void removeNodeFromSubgraph(int nodeId, int subgraphId);

    void addConnectionToSubgraph(int connectionId, int subgraphId);

    void removeConnectionFromSubgraph(int connectionId, int subgraphId);

    std::vector<int> getNodesInSubgraph(int subgraphId) const;

    std::vector<int> getConnectionsInSubgraph(int subgraphId) const;

    struct ConnectionInfo {
        int id;
        int startNodeId;
        int endNodeId;
    };
    ConnectionInfo getConnectionInfo(int connectionId) const;
};

}

#endif