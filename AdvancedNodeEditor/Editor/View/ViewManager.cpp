#include "ViewManager.h"
#include <algorithm>
#include <cmath>
#include <imgui.h>
#include <limits>

#include "imgui_internal.h"

namespace NodeEditorCore {
    ViewManager::ViewManager()
        : m_currentState(Vec2(0.0f, 0.0f), 1.0f)
        , m_targetState(Vec2(0.0f, 0.0f), 1.0f)
        , m_transitionDuration(0.0f)
        , m_transitionElapsed(0.0f)
        , m_transitionType(ViewTransitionType::Instant)
        , m_transitioning(false)
        , m_minZoom(0.1f)
        , m_maxZoom(10.0f)
        , m_windowSize(1280.0f, 720.0f)
        , m_boundingBoxProvider(nullptr)
        , m_nodeBoundingBoxProvider(nullptr)
        , m_selectedNodesBoundingBoxProvider(nullptr) {
    }

    ViewManager::~ViewManager() = default;

    void ViewManager::setViewPosition(const Vec2 &position) {
        m_currentState.position = position;
        m_transitioning = false;
    }

    Vec2 ViewManager::getViewPosition() const {
        return m_currentState.position;
    }

    void ViewManager::setViewScale(float scale) {
        scale = std::max(m_minZoom, std::min(scale, m_maxZoom));
        m_currentState.scale = scale;
        m_transitioning = false;
    }

    float ViewManager::getViewScale() const {
        return m_currentState.scale;
    }

    void ViewManager::setWindowSize(const Vec2& size) {
        if (size.x > 0 && size.y > 0) {
            m_windowSize = size;
        }
    }

    Vec2 ViewManager::getWindowSize() const {
        return m_windowSize;
    }

    Vec2 ViewManager::getEffectiveWindowSize(const Vec2& providedSize) const {
        if (providedSize.x > 0 && providedSize.y > 0) {
            return providedSize;
        }

        if (m_windowSize.x > 0 && m_windowSize.y > 0) {
            return m_windowSize;
        }

        return Vec2(1280, 720);
    }

    void ViewManager::centerView(const Vec2& windowSize) {
        if (!m_boundingBoxProvider) return;

        Vec2 min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        Vec2 max(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

        m_boundingBoxProvider(min, max);

        if (min.x == std::numeric_limits<float>::max()) {
            setViewPosition(Vec2(0.0f, 0.0f));
            return;
        }

        Vec2 center = Vec2(
            (min.x + max.x) * 0.5f,
            (min.y + max.y) * 0.5f
        );

        Vec2 effectiveSize = getEffectiveWindowSize(windowSize);
        Vec2 viewCenter = Vec2(effectiveSize.x * 0.5f, effectiveSize.y * 0.5f);
        Vec2 newPosition = Vec2(
            viewCenter.x - center.x * m_currentState.scale,
            viewCenter.y - center.y * m_currentState.scale
        );

        setViewPosition(newPosition);
    }

    void ViewManager::centerOnNode(int nodeId, const Vec2& windowSize) {
        if (!m_nodeBoundingBoxProvider) return;

        Vec2 min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        Vec2 max(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

        m_nodeBoundingBoxProvider(nodeId, min, max);

        if (min.x == std::numeric_limits<float>::max()) {
            return;
        }

        Vec2 center = Vec2(
            (min.x + max.x) * 0.5f,
            (min.y + max.y) * 0.5f
        );

        Vec2 effectiveSize = getEffectiveWindowSize(windowSize);
        Vec2 viewCenter = Vec2(effectiveSize.x * 0.5f, effectiveSize.y * 0.5f);
        Vec2 newPosition = Vec2(
            viewCenter.x - center.x * m_currentState.scale,
            viewCenter.y - center.y * m_currentState.scale
        );

        setViewPosition(newPosition);
    }

    void ViewManager::centerOnNodes(const std::vector<int> &nodeIds, const Vec2& windowSize) {
        if (!m_nodeBoundingBoxProvider || nodeIds.empty()) return;

        Vec2 min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        Vec2 max(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

        for (int nodeId : nodeIds) {
            Vec2 nodeMin, nodeMax;
            m_nodeBoundingBoxProvider(nodeId, nodeMin, nodeMax);

            if (nodeMin.x != std::numeric_limits<float>::max()) {
                min.x = std::min(min.x, nodeMin.x);
                min.y = std::min(min.y, nodeMin.y);
                max.x = std::max(max.x, nodeMax.x);
                max.y = std::max(max.y, nodeMax.y);
            }
        }

        if (min.x == std::numeric_limits<float>::max()) {
            return;
        }

        Vec2 center = Vec2(
            (min.x + max.x) * 0.5f,
            (min.y + max.y) * 0.5f
        );

        Vec2 effectiveSize = getEffectiveWindowSize(windowSize);
        Vec2 viewCenter = Vec2(effectiveSize.x * 0.5f, effectiveSize.y * 0.5f);
        Vec2 newPosition = Vec2(
            viewCenter.x - center.x * m_currentState.scale,
            viewCenter.y - center.y * m_currentState.scale
        );

        setViewPosition(newPosition);
    }

    void ViewManager::zoomToFit(float padding, const Vec2& windowSize) {
        if (!m_boundingBoxProvider) return;

        Vec2 min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        Vec2 max(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

        m_boundingBoxProvider(min, max);

        if (min.x == std::numeric_limits<float>::max()) {
            setViewScale(1.0f);
            setViewPosition(Vec2(0.0f, 0.0f));
            return;
        }

        Vec2 effectiveSize = getEffectiveWindowSize(windowSize);
        float width = max.x - min.x + padding * 2.0f;
        float height = max.y - min.y + padding * 2.0f;

        float scaleX = effectiveSize.x / width;
        float scaleY = effectiveSize.y / height;
        float scale = std::min(scaleX, scaleY);

        scale = std::max(m_minZoom, std::min(scale, m_maxZoom));

        Vec2 center = Vec2(
            (min.x + max.x) * 0.5f,
            (min.y + max.y) * 0.5f
        );

        Vec2 viewCenter = Vec2(effectiveSize.x * 0.5f, effectiveSize.y * 0.5f);
        Vec2 newPosition = Vec2(
            viewCenter.x - center.x * scale,
            viewCenter.y - center.y * scale
        );

        ViewState targetState(newPosition, scale);
        startViewTransition(targetState);
    }

    void ViewManager::zoomToFitSelected(float padding, const Vec2& windowSize) {
        if (!m_selectedNodesBoundingBoxProvider) return;

        Vec2 min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        Vec2 max(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

        m_selectedNodesBoundingBoxProvider(min, max);

        if (min.x == std::numeric_limits<float>::max()) {
            zoomToFit(padding, windowSize);
            return;
        }

        Vec2 effectiveSize = getEffectiveWindowSize(windowSize);
        float width = max.x - min.x + padding * 2.0f;
        float height = max.y - min.y + padding * 2.0f;

        float scaleX = effectiveSize.x / width;
        float scaleY = effectiveSize.y / height;
        float scale = std::min(scaleX, scaleY);

        scale = std::max(m_minZoom, std::min(scale, m_maxZoom));

        Vec2 center = Vec2(
            (min.x + max.x) * 0.5f,
            (min.y + max.y) * 0.5f
        );

        Vec2 viewCenter = Vec2(effectiveSize.x * 0.5f, effectiveSize.y * 0.5f);
        Vec2 newPosition = Vec2(
            viewCenter.x - center.x * scale,
            viewCenter.y - center.y * scale
        );

        ViewState targetState(newPosition, scale);
        startViewTransition(targetState);
    }

    void ViewManager::startViewTransition(const ViewState &targetState, float duration, ViewTransitionType type) {
        m_targetState = targetState;
        m_transitionDuration = duration;
        m_transitionElapsed = 0.0f;
        m_transitionType = type;
        m_transitioning = true;
    }

    void ViewManager::updateViewTransition(float deltaTime) {
        if (!m_transitioning) return;

        m_transitionElapsed += deltaTime;
        float t = m_transitionElapsed / m_transitionDuration;

        if (t >= 1.0f) {
            m_currentState = m_targetState;
            m_transitioning = false;
            return;
        }

        t = applyEasing(t);
        m_currentState = interpolateStates(m_currentState, m_targetState, t);
    }

    bool ViewManager::isViewTransitioning() const {
        return m_transitioning;
    }

    void ViewManager::storeViewState(const std::string &key) {
        m_storedStates[key] = m_currentState;
    }

    void ViewManager::restoreViewState(const std::string &key) {
        auto it = m_storedStates.find(key);
        if (it == m_storedStates.end()) return;

        ViewState targetState = it->second;
        startViewTransition(targetState);
    }

    void ViewManager::setMinZoom(float minZoom) {
        m_minZoom = minZoom;
        if (m_currentState.scale < minZoom) {
            setViewScale(minZoom);
        }
    }

    void ViewManager::setMaxZoom(float maxZoom) {
        m_maxZoom = maxZoom;
        if (m_currentState.scale > maxZoom) {
            setViewScale(maxZoom);
        }
    }

    float ViewManager::getMinZoom() const {
        return m_minZoom;
    }

    float ViewManager::getMaxZoom() const {
        return m_maxZoom;
    }

    void ViewManager::setBoundingBoxProvider(std::function<void(Vec2 &, Vec2 &)> provider) {
        m_boundingBoxProvider = provider;
    }

    void ViewManager::setNodeBoundingBoxProvider(std::function<void(int, Vec2&, Vec2&)> provider) {
        m_nodeBoundingBoxProvider = provider;
    }

    void ViewManager::setSelectedNodesBoundingBoxProvider(std::function<void(Vec2&, Vec2&)> provider) {
        m_selectedNodesBoundingBoxProvider = provider;
    }

    float ViewManager::applyEasing(float t) const {
        switch (m_transitionType) {
            case ViewTransitionType::Instant:
                return 1.0f;

            case ViewTransitionType::Linear:
                return t;

            case ViewTransitionType::EaseInOut:
                return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;

            default:
                return t;
        }
    }

    ViewManager::ViewState ViewManager::interpolateStates(const ViewState &start, const ViewState &end, float t) const {
        ViewState result;
        result.position.x = start.position.x + (end.position.x - start.position.x) * t;
        result.position.y = start.position.y + (end.position.y - start.position.y) * t;
        result.scale = start.scale + (end.scale - start.scale) * t;
        return result;
    }
}