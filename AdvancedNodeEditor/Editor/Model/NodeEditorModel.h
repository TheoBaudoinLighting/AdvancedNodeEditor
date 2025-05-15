#ifndef NODE_EDITOR_MODEL_H
#define NODE_EDITOR_MODEL_H

#include "../../Core/Types/CoreTypes.h"
#include <vector>
#include <memory>
#include <map>
#include <functional>

namespace NodeEditorCore {

class NodeEditorModel {
public:
    struct Node {
        int id;
        std::string name;
        std::string type;
        NodeEditorCore::Vec2 position;
        NodeEditorCore::Vec2 size;
        std::vector<NodeEditorCore::Pin> inputs;
        std::vector<NodeEditorCore::Pin> outputs;
        bool selected;
        bool disabled;
        int groupId;
        std::string iconSymbol;
        bool isTemplate;
        bool isCurrentFlag;
        NodeEditorCore::NodeLabelPosition labelPosition;
        bool isSubgraph;
        int subgraphId;
        NodeEditorCore::Metadata metadata;
    };
    
    struct Connection {
        int id;
        int startNodeId;
        int startPinId;
        int endNodeId;
        int endPinId;
        bool selected;
        NodeEditorCore::Metadata metadata;
    };
    
    NodeEditorModel();
    ~NodeEditorModel();
    
    int addNode(const std::string& name, const std::string& type, const NodeEditorCore::Vec2& position);
    void removeNode(int nodeId);
    Node* getNode(int nodeId);
    const Node* getNode(int nodeId) const;
    const std::vector<std::shared_ptr<Node>>& getNodes() const;
    
    int addPin(int nodeId, const std::string& name, bool isInput, NodeEditorCore::PinType type = NodeEditorCore::PinType::Blue, NodeEditorCore::PinShape shape = NodeEditorCore::PinShape::Circle);
    void removePin(int nodeId, int pinId);
    NodeEditorCore::Pin* getPin(int nodeId, int pinId);
    const NodeEditorCore::Pin* getPin(int nodeId, int pinId) const;
    
    int addConnection(int startNodeId, int startPinId, int endNodeId, int endPinId);
    void removeConnection(int connectionId);
    Connection* getConnection(int connectionId);
    const Connection* getConnection(int connectionId) const;
    const std::vector<std::shared_ptr<Connection>>& getConnections() const;
    bool isConnected(int nodeId, int pinId) const;
    
    int addGroup(const std::string& name, const NodeEditorCore::Vec2& position, const NodeEditorCore::Vec2& size);
    void removeGroup(int groupId);
    NodeEditorCore::Group* getGroup(int groupId);
    const NodeEditorCore::Group* getGroup(int groupId) const;
    const std::vector<std::shared_ptr<NodeEditorCore::Group>>& getGroups() const;
    void addNodeToGroup(int nodeId, int groupId);
    void removeNodeFromGroup(int nodeId, int groupId);
    
    int createSubgraph(const std::string& name);
    void removeSubgraph(int subgraphId);
    NodeEditorCore::Subgraph* getSubgraph(int subgraphId);
    const NodeEditorCore::Subgraph* getSubgraph(int subgraphId) const;
    const std::map<int, std::shared_ptr<NodeEditorCore::Subgraph>>& getSubgraphs() const;
    Node* createSubgraphNode(int subgraphId, const std::string& name, const NodeEditorCore::Vec2& position);
    
    void enterSubgraph(int subgraphId);
    void exitSubgraph();
    int getCurrentSubgraphId() const;
    
    void addNodeToSubgraph(int nodeId, int subgraphId);
    void addConnectionToSubgraph(int connectionId, int subgraphId);
    void addGroupToSubgraph(int groupId, int subgraphId);
    
    void selectNode(int nodeId, bool append = false);
    void deselectNode(int nodeId);
    void selectAllNodes();
    void deselectAllNodes();
    std::vector<int> getSelectedNodes() const;
    
    void setState(const std::string& key, const std::any& value);
    template<typename T>
    T getState(const std::string& key, const T& defaultValue = T()) const {
        auto it = m_state.find(key);
        if (it != m_state.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
            }
        }
        return defaultValue;
    }
    bool hasState(const std::string& key) const;
    
    void addEventListener(NodeEditorCore::EventType type, NodeEditorCore::EventCallback callback);
    void removeEventListener(NodeEditorCore::EventType type, NodeEditorCore::EventCallback callback);
    void dispatchEvent(const NodeEditorCore::Event& event);
    
private:
    std::vector<std::shared_ptr<Node>> m_nodes;
    std::vector<std::shared_ptr<Connection>> m_connections;
    std::vector<std::shared_ptr<NodeEditorCore::Group>> m_groups;
    std::map<int, std::shared_ptr<NodeEditorCore::Subgraph>> m_subgraphs;
    
    int m_nextNodeId;
    int m_nextPinId;
    int m_nextConnectionId;
    int m_nextGroupId;
    int m_nextSubgraphId;
    
    std::map<std::string, std::any> m_state;
    std::map<NodeEditorCore::EventType, std::vector<NodeEditorCore::EventCallback>> m_eventListeners;
};

}

#endif