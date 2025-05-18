#pragma once

#include <string>
#include <functional>
#include <any>
#include <typeindex>
#include <stdexcept>

template<typename T>
class TypedCommandRouter {
private:
    struct HandlerInfo {
        std::function<void(const T&)> handler;
        std::type_index expectedType;

        HandlerInfo(std::function<void(const T&)> h, std::type_index t)
            : handler(std::move(h)), expectedType(t) {}
    };

    std::unordered_map<std::string, HandlerInfo> handlers;

public:
    template<typename PayloadType>
    void bind(const std::string& command, std::function<void(const PayloadType&)> handler) {
        auto wrappedHandler = [handler, command](const T& data) {
            try {
                const PayloadType& typedData = std::any_cast<const PayloadType&>(data);
                handler(typedData);
            } catch (const std::bad_any_cast& e) {
                throw std::runtime_error("Type incompatible pour la commande '" + command +
                                        "'. Attendu: " + typeid(PayloadType).name() +
                                        ", Reçu: " + data.type().name());
            }
        };

        handlers.insert_or_assign(command, HandlerInfo{
            wrappedHandler,
            std::type_index(typeid(PayloadType))
        });
    }

    void dispatch(const std::string& command, const T& data) {
        auto it = handlers.find(command);
        if (it != handlers.end()) {
            const HandlerInfo& info = it->second;
            
            if (data.type() != info.expectedType) {
                throw std::runtime_error("Type incompatible pour la commande '" + command + 
                                         "'. Attendu: " + info.expectedType.name() + 
                                         ", Reçu: " + data.type().name());
            }
            
            info.handler(data);
        } else {
            throw std::runtime_error("Commande non liée: '" + command + "'");
        }
    }

    template<typename PayloadType>
    void dispatchTyped(const std::string& command, const PayloadType& payload) {
        dispatch(command, T(payload));
    }

    bool isBound(const std::string& command) const {
        return handlers.find(command) != handlers.end();
    }

    std::type_index getExpectedType(const std::string& command) const {
        auto it = handlers.find(command);
        if (it != handlers.end()) {
            return it->second.expectedType;
        }
        throw std::runtime_error("Commande non liée: '" + command + "'");
    }
};

using StrictCommandRouter = TypedCommandRouter<std::any>;