#include "NodeBoundingBoxManager.h"
#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_set>

namespace NodeEditorCore {
    NodeBoundingBoxManager::NodeBoundingBoxManager() {
    }

    NodeBoundingBoxManager::~NodeBoundingBoxManager() = default;

    void NodeBoundingBoxManager::addBoundingBox(int nodeId, const Vec2 &position, const Vec2 &size) {
        m_boundingBoxes[nodeId] = BoundingBox(position, size, nodeId);
    }

    void NodeBoundingBoxManager::updateBoundingBox(int nodeId, const Vec2 &position, const Vec2 &size) {
        auto it = m_boundingBoxes.find(nodeId);
        if (it != m_boundingBoxes.end()) {
            it->second.position = position;
            it->second.size = size;
        } else {
            addBoundingBox(nodeId, position, size);
        }
    }

    void NodeBoundingBoxManager::removeBoundingBox(int nodeId) {
        m_boundingBoxes.erase(nodeId);
    }

    bool NodeBoundingBoxManager::isLineIntersectingAnyBox(const Vec2 &start, const Vec2 &end) const {
        for (const auto &pair: m_boundingBoxes) {
            if (std::find(m_excludedNodeIds.begin(), m_excludedNodeIds.end(), pair.first) != m_excludedNodeIds.end()) {
                continue;
            }
            if (pair.second.intersectsLine(start, end)) {
                return true;
            }
        }
        return false;
    }

    std::vector<Vec2> NodeBoundingBoxManager::findPathAroundNodes(const Vec2 &start, const Vec2 &end,
                                                                  float padding) const {
        std::vector<BoundingBox> paddedBoxes;
        for (const auto &pair: m_boundingBoxes) {
            if (std::find(m_excludedNodeIds.begin(), m_excludedNodeIds.end(), pair.first) != m_excludedNodeIds.end()) {
                continue;
            }
            BoundingBox paddedBox = pair.second;
            paddedBox.position.x -= padding;
            paddedBox.position.y -= padding;
            paddedBox.size.x += padding * 2.0f;
            paddedBox.size.y += padding * 2.0f;
            paddedBoxes.push_back(paddedBox);
        }
        bool directLineIntersects = false;
        for (const auto &box: paddedBoxes) {
            if (box.intersectsLine(start, end)) {
                directLineIntersects = true;
                break;
            }
        }
        if (!directLineIntersects) {
            return {start, end};
        }
        return findSimplePath(start, end, paddedBoxes, padding);
    }

    const NodeBoundingBoxManager::BoundingBox *NodeBoundingBoxManager::getBoundingBox(int nodeId) const {
        auto it = m_boundingBoxes.find(nodeId);
        if (it != m_boundingBoxes.end()) {
            return &it->second;
        }
        return nullptr;
    }

    void NodeBoundingBoxManager::setExcludedNodeIds(const std::vector<int> &excludedIds) {
        m_excludedNodeIds = excludedIds;
    }

    const std::vector<int> &NodeBoundingBoxManager::getExcludedNodeIds() const {
        return m_excludedNodeIds;
    }

    void NodeBoundingBoxManager::clear() {
        m_boundingBoxes.clear();
        m_excludedNodeIds.clear();
    }

    Vec2 NodeBoundingBoxManager::findNearestPointOnLine(const Vec2 &point, const Vec2 &lineStart,
                                                        const Vec2 &lineEnd) const {
        Vec2 line = Vec2(lineEnd.x - lineStart.x, lineEnd.y - lineStart.y);
        float lineLen = std::sqrt(line.x * line.x + line.y * line.y);
        if (lineLen <= 0.0001f) {
            return lineStart;
        }
        Vec2 lineDir = Vec2(line.x / lineLen, line.y / lineLen);
        Vec2 v = Vec2(point.x - lineStart.x, point.y - lineStart.y);
        float t = v.x * lineDir.x + v.y * lineDir.y;
        t = std::max(0.0f, std::min(t, lineLen));
        return Vec2(
            lineStart.x + t * lineDir.x,
            lineStart.y + t * lineDir.y
        );
    }

    bool NodeBoundingBoxManager::isPointValid(const Vec2 &point, const std::vector<BoundingBox> &boxes,
                                              float padding) const {
        for (const auto &box: boxes) {
            if (box.contains(point)) {
                return false;
            }
        }
        return true;
    }

    std::vector<Vec2> NodeBoundingBoxManager::findSimplePath(const Vec2 &start, const Vec2 &end,
                                                             const std::vector<BoundingBox> &obstacles,
                                                             float padding) const {
        const Vec2 directions[] = {
            Vec2(1.0f, -1.0f), Vec2(1.0f, 1.0f), Vec2(-1.0f, -1.0f), Vec2(-1.0f, 1.0f),
            Vec2(1.0f, 0.0f), Vec2(-1.0f, 0.0f), Vec2(0.0f, -1.0f), Vec2(0.0f, 1.0f)
        };
        float diffX = end.x - start.x;
        float diffY = end.y - start.y;
        bool preferHorizontal = std::abs(diffX) > std::abs(diffY);
        Vec2 middle;
        if (preferHorizontal) {
            middle = Vec2(start.x + diffX * 0.5f, start.y);
            if (!isLineIntersectingAnyBox(start, middle) && !isLineIntersectingAnyBox(middle, end)) {
                return {start, middle, end};
            }
            middle = Vec2(start.x, start.y + diffY * 0.5f);
            if (!isLineIntersectingAnyBox(start, middle) && !isLineIntersectingAnyBox(middle, end)) {
                return {start, middle, end};
            }
        } else {
            middle = Vec2(start.x, start.y + diffY * 0.5f);
            if (!isLineIntersectingAnyBox(start, middle) && !isLineIntersectingAnyBox(middle, end)) {
                return {start, middle, end};
            }
            middle = Vec2(start.x + diffX * 0.5f, start.y);
            if (!isLineIntersectingAnyBox(start, middle) && !isLineIntersectingAnyBox(middle, end)) {
                return {start, middle, end};
            }
        }
        Vec2 middle1, middle2;
        if (preferHorizontal) {
            middle1 = Vec2(start.x + diffX * 0.5f, start.y);
            middle2 = Vec2(start.x + diffX * 0.5f, end.y);
            if (!isLineIntersectingAnyBox(start, middle1) &&
                !isLineIntersectingAnyBox(middle1, middle2) &&
                !isLineIntersectingAnyBox(middle2, end)) {
                return {start, middle1, middle2, end};
            }
            middle1 = Vec2(start.x, start.y + diffY * 0.5f);
            middle2 = Vec2(end.x, start.y + diffY * 0.5f);
            if (!isLineIntersectingAnyBox(start, middle1) &&
                !isLineIntersectingAnyBox(middle1, middle2) &&
                !isLineIntersectingAnyBox(middle2, end)) {
                return {start, middle1, middle2, end};
            }
        } else {
            middle1 = Vec2(start.x, start.y + diffY * 0.5f);
            middle2 = Vec2(end.x, start.y + diffY * 0.5f);
            if (!isLineIntersectingAnyBox(start, middle1) &&
                !isLineIntersectingAnyBox(middle1, middle2) &&
                !isLineIntersectingAnyBox(middle2, end)) {
                return {start, middle1, middle2, end};
            }
            middle1 = Vec2(start.x + diffX * 0.5f, start.y);
            middle2 = Vec2(start.x + diffX * 0.5f, end.y);
            if (!isLineIntersectingAnyBox(start, middle1) &&
                !isLineIntersectingAnyBox(middle1, middle2) &&
                !isLineIntersectingAnyBox(middle2, end)) {
                return {start, middle1, middle2, end};
            }
        }
        for (float factorX = 0.25f; factorX <= 0.75f; factorX += 0.25f) {
            for (float factorY = 0.25f; factorY <= 0.75f; factorY += 0.25f) {
                Vec2 mid = Vec2(
                    start.x + diffX * factorX,
                    start.y + diffY * factorY
                );
                if (!isLineIntersectingAnyBox(start, mid) && !isLineIntersectingAnyBox(mid, end)) {
                    return {start, mid, end};
                }
            }
        }
        return {start, end};
    }
} // namespace NodeEditorCore
