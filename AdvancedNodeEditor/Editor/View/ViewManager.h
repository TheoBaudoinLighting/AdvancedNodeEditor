#ifndef VIEW_MANAGER_H
#define VIEW_MANAGER_H

#include "../../Core/Types/CoreTypes.h"
#include <functional>
#include <unordered_map>
#include <string>
#include <vector>

namespace NodeEditorCore {
    class ViewManager {
    public:
        struct ViewState {
            Vec2 position;
            float scale;

            ViewState() : position(0.0f, 0.0f), scale(1.0f) {
            }

            ViewState(const Vec2 &pos, float scl) : position(pos), scale(scl) {
            }
        };

        enum class ViewTransitionType {
            Instant,
            Linear,
            EaseInOut
        };

        ViewManager();

        ~ViewManager();

        void setViewPosition(const Vec2 &position);

        Vec2 getViewPosition() const;

        void setViewScale(float scale);

        float getViewScale() const;

        void setWindowSize(const Vec2& size);
        Vec2 getWindowSize() const;

        void centerView(const Vec2& windowSize = Vec2(0.0f, 0.0f));
        void centerOnNode(int nodeId, const Vec2& windowSize = Vec2(0.0f, 0.0f));
        void centerOnNodes(const std::vector<int>& nodeIds, const Vec2& windowSize = Vec2(0.0f, 0.0f));
        void zoomToFit(float padding = 50.0f, const Vec2& windowSize = Vec2(0.0f, 0.0f));
        void zoomToFitSelected(float padding = 50.0f, const Vec2& windowSize = Vec2(0.0f, 0.0f));

        void startViewTransition(const ViewState &targetState, float duration = 0.3f,
                                 ViewTransitionType type = ViewTransitionType::EaseInOut);

        void updateViewTransition(float deltaTime);

        bool isViewTransitioning() const;

        void storeViewState(const std::string &key);

        void restoreViewState(const std::string &key);

        void setMinZoom(float minZoom);

        void setMaxZoom(float maxZoom);

        float getMinZoom() const;

        float getMaxZoom() const;

        void setBoundingBoxProvider(std::function<void(Vec2 &, Vec2 &)> provider);
        void setNodeBoundingBoxProvider(std::function<void(int, Vec2&, Vec2&)> provider);
        void setSelectedNodesBoundingBoxProvider(std::function<void(Vec2&, Vec2&)> provider);

    private:
        ViewState m_currentState;
        ViewState m_targetState;
        float m_transitionDuration;
        float m_transitionElapsed;
        ViewTransitionType m_transitionType;
        bool m_transitioning;

        float m_minZoom;
        float m_maxZoom;

        Vec2 m_windowSize;

        std::unordered_map<std::string, ViewState> m_storedStates;

        std::function<void(Vec2 &, Vec2 &)> m_boundingBoxProvider;
        std::function<void(int, Vec2&, Vec2&)> m_nodeBoundingBoxProvider;
        std::function<void(Vec2&, Vec2&)> m_selectedNodesBoundingBoxProvider;

        float applyEasing(float t) const;
        ViewState interpolateStates(const ViewState &start, const ViewState &end, float t) const;
        Vec2 getEffectiveWindowSize(const Vec2& providedSize) const;
    };
}

#endif