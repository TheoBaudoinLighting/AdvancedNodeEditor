#ifndef UUID_TYPES_H
#define UUID_TYPES_H

#include "../../Utils/UuidGenerator.h"
#include <string>
#include <functional>
#include <unordered_map>

namespace ANE {

    using UUID = std::string;

    struct UUIDHash {
        std::size_t operator()(const UUID& uuid) const {
            return std::hash<std::string>{}(uuid);
        }
    };

    template<typename T>
    using UUIDMap = std::unordered_map<UUID, T, UUIDHash>;

    inline UUID generateUUID() {
        return NodeEditorInternal::UuidGenerator::getInstance().generateV4().toString();
    }

    inline int uuidToDisplayId(const UUID& uuid) {
        static std::unordered_map<UUID, int> displayIdCache;
        static int nextDisplayId = 1;

        auto it = displayIdCache.find(uuid);
        if (it != displayIdCache.end()) {
            return it->second;
        }

        int displayId = nextDisplayId++;
        displayIdCache[uuid] = displayId;
        return displayId;
    }

} // namespace ANE

#endif // UUID_TYPES_H