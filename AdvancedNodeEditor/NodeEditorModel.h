#ifndef NODE_EDITOR_MODEL_H
#define NODE_EDITOR_MODEL_H

#include "ANETypes.h"
#include "NodeComponents.h"
#include <vector>
#include <memory>
#include <map>
#include <functional>

namespace ANE {

class NodeEditorModel {
public:
    struct Node {
        int id;
        std::string name;
        std::string type;
        Vec2 position;
        Vec2 size;
        std::vector<Pin> inputs;
        std::vector<Pin> outputs;
        bool selected;
        bool disabled;
        int groupId;
        std::string iconSymbol;
        bool isTemplate;
        bool isCurrentFlag;
        NodeLabelPosition labelPosition;
        bool isSubgraph;
        int subgraphId;
        Metadata metadata;
    };
    
    struct Connection {
        int id;
        int startNodeId;
        int startPinId;
        int endNodeId;
        int endPinId;
        bool selected;
        Metadata metadata;
    };
    
    NodeEditorModel();
    ~NodeEditorModel();
    
    int addNode(const std::string& name, const std::string& type, const Vec2& position);
    void removeNode(int nodeId);
    Node* getNode(int nodeId);
    const Node* getNode(int nodeId) const;
    const std::vector<std::shared_ptr<Node>>& getNodes() const;
    
    int addPin(int nodeId, const std::string& name, bool isInput, PinType type = PinType::Blue, PinShape shape = PinShape::Circle);
    void removePin(int nodeId, int pinId);
    Pin* getPin(int nodeId, int pinId);
    const Pin* getPin(int nodeId, int pinId) const;
    
    int addConnection(int startNodeId, int startPinId, int endNodeId, int endPinId);
    void removeConnection(int connectionId);
    Connection* getConnection(int connectionId);
    const Connection* getConnection(int connectionId) const;
    const std::vector<std::shared_ptr<Connection>>& getConnections() const;
    bool isConnected(int nodeId, int pinId) const;
    
    int addGroup(const std::string& name, const Vec2& position, const Vec2& size);
    void removeGroup(int groupId);
    Group* getGroup(int groupId);
    const Group* getGroup(int groupId) const;
    const std::vector<std::shared_ptr<Group>>& getGroups() const;
    void addNodeToGroup(int nodeId, int groupId);
    void removeNodeFromGroup(int nodeId, int groupId);
    
    int createSubgraph(const std::string& name);
    void removeSubgraph(int subgraphId);
    Subgraph* getSubgraph(int subgraphId);
    const Subgraph* getSubgraph(int subgraphId) const;
    const std::map<int, std::shared_ptr<Subgraph>>& getSubgraphs() const;
    Node* createSubgraphNode(int subgraphId, const std::string& name, const Vec2& position);
    
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
    
    void addEventListener(EventType type, EventCallback callback);
    void removeEventListener(EventType type, EventCallback callback);
    void dispatchEvent(const Event& event);
    
private:
    std::vector<std::shared_ptr<Node>> m_nodes;
    std::vector<std::shared_ptr<Connection>> m_connections;
    std::vector<std::shared_ptr<Group>> m_groups;
    std::map<int, std::shared_ptr<Subgraph>> m_subgraphs;
    
    int m_nextNodeId;
    int m_nextPinId;
    int m_nextConnectionId;
    int m_nextGroupId;
    int m_nextSubgraphId;
    
    std::map<std::string, std::any> m_state;
    std::map<EventType, std::vector<EventCallback>> m_eventListeners;
};

}

#endif