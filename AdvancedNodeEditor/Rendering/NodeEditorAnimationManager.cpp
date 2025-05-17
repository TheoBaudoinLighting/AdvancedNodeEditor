#include "NodeEditorAnimationManager.h"
#include <algorithm>
#include <cmath>

namespace NodeEditorCore {
    void AnimationManager::update(float deltaTime) {
        for (auto& pair : m_nodeAnimations) {
            auto& state = pair.second;

            float scaleDiff = state.targetScaleFactor - state.hoverScaleFactor;
            if (std::abs(scaleDiff) > 0.001f) {
                state.hoverScaleFactor += scaleDiff * std::min(1.0f, deltaTime * SCALE_TRANSITION_SPEED);
            }

            if (state.isExecuting) {
                state.executionPulse += deltaTime * EXECUTION_PULSE_SPEED;
                if (state.executionPulse > 1.0f) {
                    state.executionPulse -= 1.0f;
                }
            } else if (state.executionPulse > 0.0f) {
                state.executionPulse = std::max(0.0f, state.executionPulse - deltaTime * EXECUTION_PULSE_SPEED);
            }
        }

        for (auto& pair : m_connectionAnimations) {
            auto& state = pair.second;
            state.flowAnimation += deltaTime * state.flowSpeed * CONNECTION_FLOW_SPEED;
            if (state.flowAnimation > 1.0f) {
                state.flowAnimation -= 1.0f;
            }
        }
    }

    NodeAnimationState& AnimationManager::getNodeAnimationState(int nodeId) {
        if (m_nodeAnimations.find(nodeId) == m_nodeAnimations.end()) {
            m_nodeAnimations[nodeId] = NodeAnimationState();
        }
        return m_nodeAnimations[nodeId];
    }

    ConnectionAnimationState& AnimationManager::getConnectionAnimationState(int connectionId) {
        if (m_connectionAnimations.find(connectionId) == m_connectionAnimations.end()) {
            m_connectionAnimations[connectionId] = ConnectionAnimationState();
        }
        return m_connectionAnimations[connectionId];
    }

    void AnimationManager::setNodeHovered(int nodeId, bool hovered) {
        auto& state = getNodeAnimationState(nodeId);
        state.targetScaleFactor = hovered ? HOVER_SCALE_FACTOR_TARGET : 1.0f;
    }

    void AnimationManager::setNodeExecuting(int nodeId, bool executing) {
        auto& state = getNodeAnimationState(nodeId);
        state.isExecuting = executing;
    }

    void AnimationManager::setNodeTargetPosition(int nodeId, const Vec2& position) {
        auto& state = getNodeAnimationState(nodeId);
        state.targetPosition = position;
    }

    void AnimationManager::updateNodePositions(std::vector<Node>& nodes, float deltaTime) {
        for (auto& node : nodes) {
            auto& state = getNodeAnimationState(node.id);

            if (!(state.targetPosition.x == 0.0f && state.targetPosition.y == 0.0f)) {
                Vec2 diff = state.targetPosition - node.position;
                float distSquared = diff.x * diff.x + diff.y * diff.y;
                
                if (distSquared > POSITION_THRESHOLD * POSITION_THRESHOLD) {
                    Vec2 springForce = diff * MOVEMENT_SPRING_STIFFNESS * deltaTime;
                    state.velocity = state.velocity + springForce;
                    
                    state.velocity = state.velocity * std::pow(MOVEMENT_DAMPING, deltaTime * 60.0f);
                    
                    node.position = node.position + state.velocity * deltaTime;
                } else {
                    // Snap to target when very close
                    node.position = state.targetPosition;
                    state.velocity = Vec2(0.0f, 0.0f);
                    state.targetPosition = Vec2(0.0f, 0.0f);
                }
            }
        }
    }

    void AnimationManager::updateConnectionFlows(std::vector<Connection>& connections, float deltaTime) {
        for (auto& connection : connections) {
            auto& state = getConnectionAnimationState(connection.id);
            state.flowAnimation += deltaTime * state.flowSpeed;
            if (state.flowAnimation > 1.0f) {
                state.flowAnimation -= 1.0f;
            }
        }
    }
}