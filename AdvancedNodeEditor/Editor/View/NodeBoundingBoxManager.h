// NodeBoundingBoxManager.h
#ifndef NODE_BOUNDING_BOX_MANAGER_H
#define NODE_BOUNDING_BOX_MANAGER_H

#include "../../Core/Types/CoreTypes.h"
#include <unordered_map>
#include <functional>
#include <vector>

namespace NodeEditorCore {

class NodeBoundingBoxManager {
public:
    struct BoundingBox {
        Vec2 position;
        Vec2 size;
        int nodeId;
        bool isActive;

        BoundingBox() : position(0.0f, 0.0f), size(0.0f, 0.0f), nodeId(-1), isActive(true) {}
        BoundingBox(const Vec2& pos, const Vec2& sz, int id)
            : position(pos), size(sz), nodeId(id), isActive(true) {}

        bool contains(const Vec2& point) const {
            return point.x >= position.x && point.x <= (position.x + size.x) &&
                   point.y >= position.y && point.y <= (position.y + size.y);
        }

        bool intersectsLine(const Vec2& start, const Vec2& end) const {
            // Test rapide d'intersection AABB - ligne
            if (!isActive) return false;

            // Tests de non-intersection
            if ((start.x < position.x && end.x < position.x) ||
                (start.x > position.x + size.x && end.x > position.x + size.x) ||
                (start.y < position.y && end.y < position.y) ||
                (start.y > position.y + size.y && end.y > position.y + size.y)) {
                return false;
            }

            // Test plus précis avec équation de ligne
            float dx = end.x - start.x;
            float dy = end.y - start.y;

            // Si la ligne est verticale
            if (std::abs(dx) < 0.00001f) {
                return start.x >= position.x && start.x <= position.x + size.x;
            }

            // Si la ligne est horizontale
            if (std::abs(dy) < 0.00001f) {
                return start.y >= position.y && start.y <= position.y + size.y;
            }

            float m = dy / dx;
            float b = start.y - m * start.x;

            // Calculer les intersections avec les bords de la boîte
            float t_min = 0.0f;
            float t_max = 1.0f;

            // Test avec les bords horizontaux (y = const)
            for (float y_test : {position.y, position.y + size.y}) {
                float t = (y_test - start.y) / dy;
                if (t >= 0.0f && t <= 1.0f) {
                    float x_at_t = start.x + t * dx;
                    if (x_at_t >= position.x && x_at_t <= position.x + size.x) {
                        return true;
                    }
                }
            }

            // Test avec les bords verticaux (x = const)
            for (float x_test : {position.x, position.x + size.x}) {
                float t = (x_test - start.x) / dx;
                if (t >= 0.0f && t <= 1.0f) {
                    float y_at_t = start.y + t * dy;
                    if (y_at_t >= position.y && y_at_t <= position.y + size.y) {
                        return true;
                    }
                }
            }

            return false;
        }
    };

    NodeBoundingBoxManager();
    ~NodeBoundingBoxManager();

    void addBoundingBox(int nodeId, const Vec2& position, const Vec2& size);
    void updateBoundingBox(int nodeId, const Vec2& position, const Vec2& size);
    void removeBoundingBox(int nodeId);

    bool isLineIntersectingAnyBox(const Vec2& start, const Vec2& end) const;
    std::vector<Vec2> findPathAroundNodes(const Vec2& start, const Vec2& end, float padding = 20.0f) const;

    const BoundingBox* getBoundingBox(int nodeId) const;

    void setExcludedNodeIds(const std::vector<int>& excludedIds);
    const std::vector<int>& getExcludedNodeIds() const;

    void clear();

private:
    std::unordered_map<int, BoundingBox> m_boundingBoxes;
    std::vector<int> m_excludedNodeIds;

    Vec2 findNearestPointOnLine(const Vec2& point, const Vec2& lineStart, const Vec2& lineEnd) const;
    bool isPointValid(const Vec2& point, const std::vector<BoundingBox>& boxes, float padding) const;

    // Algorithme de pathfinding simple
    std::vector<Vec2> findSimplePath(const Vec2& start, const Vec2& end, const std::vector<BoundingBox>& obstacles, float padding) const;
};

} // namespace NodeEditorCore

#endif // NODE_BOUNDING_BOX_MANAGER_H