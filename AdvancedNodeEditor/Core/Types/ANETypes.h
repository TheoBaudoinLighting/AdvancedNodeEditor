#ifndef ANE_TYPES_H
#define ANE_TYPES_H

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <any>

namespace ANE {

struct Vec2 {
    float x, y;
    
    Vec2() : x(0.0f), y(0.0f) {}
    Vec2(float x_, float y_) : x(x_), y(y_) {}
    
    Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
    Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
    Vec2 operator*(float scalar) const { return Vec2(x * scalar, y * scalar); }
    Vec2 operator/(float scalar) const { return Vec2(x / scalar, y / scalar); }
};

struct Color {
    float r, g, b, a;
    
    Color() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
    Color(float r_, float g_, float b_, float a_ = 1.0f) : r(r_), g(g_), b(b_), a(a_) {}
};

enum class PinType {
    Blue,
    Red,
    Green,
    Yellow,
    Purple,
    Cyan,
    Orange,
    White,
    Black,
    Gray,
    Custom
};

enum class PinShape {
    Circle,
    Square,
    Triangle,
    Diamond
};

enum class GroupStyle {
    Default,
    Subtle,
    Bold,
    Wire,
    Ghost
};

enum class NodeLabelPosition {
    None,
    Left,
    Right
};

struct Metadata {
    std::unordered_map<std::string, std::any> attributes;
    
    template<typename T>
    void setAttribute(const std::string& key, const T& value) {
        attributes[key] = value;
    }
    
    template<typename T>
    T getAttribute(const std::string& key, const T& defaultValue = T()) const {
        auto it = attributes.find(key);
        if (it != attributes.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
            }
        }
        return defaultValue;
    }
    
    bool hasAttribute(const std::string& key) const {
        return attributes.find(key) != attributes.end();
    }
};

enum class EventType {
    NodeCreated,
    NodeDeleted,
    NodeSelected,
    NodeDeselected,
    NodeMoved,
    NodeResized,
    NodeDragged,
    NodeDropped,
    PinConnected,
    PinDisconnected,
    ConnectionCreated,
    ConnectionDeleted,
    StateChanged,
    SubgraphCreated,
    SubgraphRemoved,
    SubgraphEntered,
    SubgraphExited,
    NodeAddedToSubgraph,
    NodeRemovedFromSubgraph,
    ConnectionAddedToSubgraph,
    ConnectionRemovedFromSubgraph,
    GroupAddedToSubgraph,
    GroupRemovedFromSubgraph,
    GroupCreated,
    GroupDeleted,
    Custom
};

struct Event {
    EventType type;
    
    int nodeId = -1;
    int pinId = -1;
    int connectionId = -1;
    int groupId = -1;
    int subgraphId = -1;
    std::unordered_map<std::string, std::any> data;
    
    Event(EventType type) : type(type) {}
    
    template<typename T>
    void setData(const std::string& key, const T& value) {
        data[key] = value;
    }
    
    template<typename T>
    T getData(const std::string& key, const T& defaultValue = T()) const {
        auto it = data.find(key);
        if (it != data.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
            }
        }
        return defaultValue;
    }
};

using EventCallback = std::function<void(const Event&)>;

}

#endif