#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include <any>
#include <vector>
#include <iostream>
#include <typeindex>

class CommandRouter {
private:
    std::unordered_map<std::string, std::function<void(const std::any&)>> handlers;
    bool loggingEnabled = false;
    std::vector<std::pair<std::string, std::any>> loggedCalls;
    std::function<void(const std::string&, const std::any&)> errorHandler;

public:
    CommandRouter() = default;
    ~CommandRouter() = default;

    void bind(const std::string& command, std::function<void(const std::any&)> handler);
    void dispatch(const std::string& command, const std::any& data = std::any());
    bool isBound(const std::string& command) const;

    void setLoggingEnabled(bool enabled);
    std::vector<std::string> getBoundCommands() const;
    const std::vector<std::pair<std::string, std::any>>& getLoggedCalls() const;

    void setErrorHandler(std::function<void(const std::string&, const std::any&)> handler) {
        errorHandler = handler;
    }
};

template<typename T>
void dispatchTyped(CommandRouter& router, const std::string& command, const T& data) {
    router.dispatch(command, std::any(data));
}