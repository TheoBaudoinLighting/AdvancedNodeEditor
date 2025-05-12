#ifndef ANE_TYPES_H
#define ANE_TYPES_H

#include <string>
#include <vector>
#include <map>
#include <any>
#include <functional>

namespace ANE {

struct Vec2 {
    float x, y;
    Vec2() : x(0.0f), y(0.0f) {}
    Vec2(float x_, float y_) : x(x_), y(y_) {}
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

enum class NodeLabelPosition {
    None,
    Left,
    Right
};

enum class GroupStyle {
    Default,
    Subtle,
    Bold,
    Wire,
    Ghost
};

enum class EventType {
    NodeCreated,
    NodeDeleted,
    NodeSelected,
    NodeDeselected,
    ConnectionCreated,
    ConnectionDeleted,
    GroupCreated,
    GroupDeleted,
    StateChanged,
    ViewChanged,
    Custom
};

struct Metadata {
    std::map<std::string, std::any> attributes;

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

    void removeAttribute(const std::string& key) {
        attributes.erase(key);
    }
};

struct Event {
    EventType type;
    std::map<std::string, std::any> data;

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