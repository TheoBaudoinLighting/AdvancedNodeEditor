#include "CommandManager.h"

void CommandManager::bindToBackend(const std::string& command, std::function<void(const std::any&)> handler) {
    toBackend.bind(command, handler);
}

void CommandManager::bindToUI(const std::string& command, std::function<void(const std::any&)> handler) {
    toUI.bind(command, handler);
}

void CommandManager::dispatchToBackend(const std::string& command, const std::any& data) {
    toBackend.dispatch(command, data);
}

void CommandManager::dispatchToUI(const std::string& command, const std::any& data) {
    toUI.dispatch(command, data);
}

bool CommandManager::isBackendBound(const std::string& command) const {
    return toBackend.isBound(command);
}

bool CommandManager::isUIBound(const std::string& command) const {
    return toUI.isBound(command);
}

CommandRouter& CommandManager::getBackendRouter() {
    return toBackend;
}

CommandRouter& CommandManager::getUIRouter() {
    return toUI;
}