#ifndef NODE_COMPONENTS_H
#define NODE_COMPONENTS_H

#include "CoreTypes.h"
#include <vector>
#include <string>
#include <unordered_set>

namespace NodeEditorCore {

struct Pin {
    int id;
    std::string name;
    std::string label;
    bool isInput;
    PinType type;
    PinShape shape;
    Color color;
    bool connected;

    Pin(int id, const std::string& name, bool isInput, PinType type = PinType::Flow);
};

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

    Node(int id, const std::string& name, const std::string& type, const Vec2& pos);

    Pin* findPin(int pinId);
    const Pin* findPin(int pinId) const;
};

struct Connection {
    int id;
    int startNodeId;
    int startPinId;
    int endNodeId;
    int endPinId;
    bool selected;

    Connection(int id, int startNodeId, int startPinId, int endNodeId, int endPinId);
};

struct Group {
    int id;
    std::string name;
    Vec2 position;
    Vec2 size;
    std::unordered_set<int> nodes;
    bool collapsed;
    bool selected;
    Color color;
    GroupStyle style;

    Group(int id, const std::string& name, const Vec2& pos, const Vec2& size);
};

}

namespace ANE {

struct Node {
    int id;
    std::string name;
    std::string type;
    std::string iconSymbol;
    NodeLabelPosition labelPosition;
    bool disabled;
    bool isTemplate;
    bool isCurrentFlag;
    
    Node(int id, const std::string& name, const std::string& type);
    void setIconSymbol(const std::string& symbol);
    void setLabelPosition(NodeLabelPosition position);
    void setDisabled(bool value);
    void setAsTemplate(bool value);
    void setCurrentFlag(bool value);
};

struct Pin {
    int id;
    std::string name;
    bool isInput;
    PinType type;
    PinShape shape;
    
    Pin(int id, const std::string& name, bool isInput, PinType type, PinShape shape);
};

struct Group {
    int id;
    std::string name;
    Color color;
    GroupStyle style;
    bool collapsed;
    
    Group(int id, const std::string& name);
    void setColor(const Color& newColor);
    void setStyle(GroupStyle newStyle);
    void setCollapsed(bool value);
};

}

#endif