#include "CommandRouter.h"

void CommandRouter::bind(const std::string& command, std::function<void(const std::any&)> handler) {
    handlers[command] = handler;
}

void CommandRouter::dispatch(const std::string& command, const std::any& data) {
    if (loggingEnabled) {
        loggedCalls.emplace_back(command, data);
    }

    auto it = handlers.find(command);
    if (it != handlers.end()) {
        it->second(data);
    } else {
        std::cerr << "[CommandRouter] WARNING: Unbound command: '" << command << "'" << std::endl;
        if (errorHandler) {
            errorHandler(command, data);
        }
    }
}

bool CommandRouter::isBound(const std::string& command) const {
    return handlers.find(command) != handlers.end();
}

void CommandRouter::setLoggingEnabled(bool enabled) {
    loggingEnabled = enabled;
    if (!enabled) {
        loggedCalls.clear();
    }
}

std::vector<std::string> CommandRouter::getBoundCommands() const {
    std::vector<std::string> commands;
    commands.reserve(handlers.size());
    for (const auto& [command, _] : handlers) {
        commands.push_back(command);
    }
    return commands;
}

const std::vector<std::pair<std::string, std::any>>& CommandRouter::getLoggedCalls() const {
    return loggedCalls;
}