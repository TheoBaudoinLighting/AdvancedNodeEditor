#pragma once

#include <unordered_map>
#include "../Core/Types/CoreTypes.h"

namespace NodeEditorCore {
    struct NodeAnimationState {
        float hoverScaleFactor = 1.0f;
        float targetScaleFactor = 1.0f;
        float executionPulse = 2.0f;
        float connectionGlow = 3.0f;
        bool isExecuting = false;
        bool justConnected = false;
        float connectionGlowAngle = 0.0f;
        int lastConnectedPinType = 0;
        Vec2 targetPosition;
        Vec2 velocity;
        float transitionSpeed = 10.0f;
    };

    struct ConnectionAnimationState {
        float flowAnimation = 1.0f;
        float flowSpeed = 0.0f;
        bool isTemporary = false;
        float duration = 3.0f;
        float elapsedTime = 0.0f;
    };

    class AnimationManager {
    public:
        AnimationManager() = default;
        ~AnimationManager() = default;

        void update(float deltaTime);

        NodeAnimationState& getNodeAnimationState(int nodeId);
        ConnectionAnimationState& getConnectionAnimationState(int connectionId);

        void setNodeHovered(int nodeId, bool hovered);
        void setNodeExecuting(int nodeId, bool executing);
        void setNodeTargetPosition(int nodeId, const Vec2& position);
        void setNodeJustConnected(int nodeId, int pinType);

        void activateConnectionFlow(int connectionId, bool infinite = true, float duration = 3.0f);
        void deactivateConnectionFlow(int connectionId);

        void updateNodePositions(std::vector<Node>& nodes, float deltaTime);
        void updateConnectionFlows(std::vector<Connection>& connections, float deltaTime);

    private:
        std::unordered_map<int, NodeAnimationState> m_nodeAnimations;
        std::unordered_map<int, ConnectionAnimationState> m_connectionAnimations;

        const float HOVER_SCALE_FACTOR_TARGET = 1.07f;
        const float SCALE_TRANSITION_SPEED = 8.0f;
        const float EXECUTION_PULSE_SPEED = 4.0f;
        const float MOVEMENT_DAMPING = 0.75f;
        const float MOVEMENT_SPRING_STIFFNESS = 8.0f;
        const float POSITION_THRESHOLD = 0.1f;
        const float CONNECTION_FLOW_SPEED = 0.5f;
    };
}