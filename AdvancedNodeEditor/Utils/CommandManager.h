#pragma once

#include "CommandRouter.h"

class CommandManager {
private:
    CommandRouter toBackend;
    CommandRouter toUI;

public:
    CommandManager() = default;
    ~CommandManager() = default;

    void bindToBackend(const std::string& command, std::function<void(const std::any&)> handler);
    void bindToUI(const std::string& command, std::function<void(const std::any&)> handler);
    
    void dispatchToBackend(const std::string& command, const std::any& data = std::any());
    void dispatchToUI(const std::string& command, const std::any& data = std::any());
    
    bool isBackendBound(const std::string& command) const;
    bool isUIBound(const std::string& command) const;
    
    CommandRouter& getBackendRouter();
    CommandRouter& getUIRouter();
    
    template<typename T>
    void dispatchTypedToBackend(const std::string& command, const T& data) {
        dispatchTyped(toBackend, command, data);
    }
    
    template<typename T>
    void dispatchTypedToUI(const std::string& command, const T& data) {
        dispatchTyped(toUI, command, data);
    }
};