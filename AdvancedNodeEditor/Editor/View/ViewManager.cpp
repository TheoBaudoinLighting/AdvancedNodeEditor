#include "ViewManager.h"
#include <algorithm>
#include <cmath>
#include <imgui.h>
#include <limits>

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
    , m_boundingBoxProvider(nullptr)
{
}

ViewManager::~ViewManager() = default;

void ViewManager::setViewPosition(const Vec2& position) {
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

void ViewManager::centerView() {
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

    Vec2 viewCenter = Vec2(ImGui::GetWindowSize().x * 0.5f, ImGui::GetWindowSize().y * 0.5f);
    Vec2 newPosition = Vec2(
        viewCenter.x - center.x * m_currentState.scale,
        viewCenter.y - center.y * m_currentState.scale
    );

    setViewPosition(newPosition);
}

void ViewManager::centerOnNode(int nodeId) {
    // Implémentation requiert un fournisseur de position de nœud
}

void ViewManager::centerOnNodes(const std::vector<int>& nodeIds) {
    // Implémentation requiert un fournisseur de position de nœud
}

void ViewManager::zoomToFit(float padding) {
    if (!m_boundingBoxProvider) return;

    Vec2 min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2 max(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

    m_boundingBoxProvider(min, max);

    if (min.x == std::numeric_limits<float>::max()) {
        setViewScale(1.0f);
        setViewPosition(Vec2(0.0f, 0.0f));
        return;
    }

    ImVec2 windowSize = ImGui::GetWindowSize();
    float width = max.x - min.x + padding * 2.0f;
    float height = max.y - min.y + padding * 2.0f;

    float scaleX = windowSize.x / width;
    float scaleY = windowSize.y / height;
    float scale = std::min(scaleX, scaleY);

   // ViewManager.cpp (suite)
    scale = std::max(m_minZoom, std::min(scale, m_maxZoom));

    Vec2 center = Vec2(
        (min.x + max.x) * 0.5f,
        (min.y + max.y) * 0.5f
    );

    Vec2 viewCenter = Vec2(windowSize.x * 0.5f, windowSize.y * 0.5f);
    Vec2 newPosition = Vec2(
        viewCenter.x - center.x * scale,
        viewCenter.y - center.y * scale
    );

    ViewState targetState(newPosition, scale);
    startViewTransition(targetState);
}

void ViewManager::zoomToFitSelected(float padding) {
    // Similaire à zoomToFit mais avec les nœuds sélectionnés uniquement
    // Implémentation nécessite un fournisseur spécifique aux nœuds sélectionnés
}

void ViewManager::startViewTransition(const ViewState& targetState, float duration, ViewTransitionType type) {
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

void ViewManager::storeViewState(const std::string& key) {
    m_storedStates[key] = m_currentState;
}

void ViewManager::restoreViewState(const std::string& key) {
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

void ViewManager::setBoundingBoxProvider(std::function<void(Vec2&, Vec2&)> provider) {
    m_boundingBoxProvider = provider;
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

ViewManager::ViewState ViewManager::interpolateStates(const ViewState& start, const ViewState& end, float t) const {
    ViewState result;
    result.position.x = start.position.x + (end.position.x - start.position.x) * t;
    result.position.y = start.position.y + (end.position.y - start.position.y) * t;
    result.scale = start.scale + (end.scale - start.scale) * t;
    return result;
}

} // namespace NodeEditorCore