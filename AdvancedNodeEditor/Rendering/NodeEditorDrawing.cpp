#include "../Core/NodeEditor.h"
#include <algorithm>

namespace NodeEditorCore {
    void NodeEditor::drawDragConnection(ImDrawList *drawList, const ImVec2 &canvasPos) {
    if (!m_state.connecting)
        return;

    ImVec2 p1;
    ImVec2 p2;
    bool isEndInput = false;
    bool isSourceInput = false;

    if (m_connectingFromReroute && m_connectingRerouteId != -1) {
        Reroute* reroute = getReroute(m_connectingRerouteId);
        if (!reroute) return;

        Vec2 rerouteScreenPos = canvasToScreen(reroute->position);
        p1 = rerouteScreenPos.toImVec2();

        const Connection* originalConnection = getConnection(reroute->connectionId);
        if (!originalConnection) return;

        const Node* startNode = getNode(originalConnection->startNodeId);
        if (!startNode) return;

        const Pin* startPin = startNode->findPin(originalConnection->startPinId);
        if (!startPin) return;

        isSourceInput = false; 

        if (m_state.magnetPinNodeId != -1) {
            const Node* magnetNode = getNode(m_state.magnetPinNodeId);
            if (!magnetNode) {
                p2 = ImGui::GetMousePos();
            } else {
                const Pin* magnetPinInternal = magnetNode->findPin(m_state.magnetPinId);
                if (!magnetPinInternal) {
                    p2 = ImGui::GetMousePos();
                } else {
                    Pin magnetPin;
                    magnetPin.id = magnetPinInternal->id;
                    magnetPin.name = magnetPinInternal->name;
                    magnetPin.isInput = magnetPinInternal->isInput;
                    magnetPin.type = static_cast<PinType>(magnetPinInternal->type);
                    magnetPin.shape = static_cast<PinShape>(magnetPinInternal->shape);

                    p2 = getPinPos(*magnetNode, magnetPin, canvasPos);
                    isEndInput = magnetPinInternal->isInput;

                    Pin sourceApiPin;
                    sourceApiPin.id = startPin->id;
                    sourceApiPin.name = startPin->name;
                    sourceApiPin.isInput = startPin->isInput;
                    sourceApiPin.type = static_cast<PinType>(startPin->type);

                    Pin magnetApiPin;
                    magnetApiPin.id = magnetPinInternal->id;
                    magnetApiPin.name = magnetPinInternal->name;
                    magnetApiPin.isInput = magnetPinInternal->isInput;
                    magnetApiPin.type = static_cast<PinType>(magnetPinInternal->type);

                    if (magnetPinInternal->isInput) {
                        m_state.canConnectToMagnetPin = canCreateConnection(sourceApiPin, magnetApiPin);
                    } else {
                        const Node* endNode = getNode(originalConnection->endNodeId);
                        const Pin* endPin = endNode ? endNode->findPin(originalConnection->endPinId) : nullptr;
                        
                        if (endPin) {
                            Pin endApiPin;
                            endApiPin.id = endPin->id;
                            endApiPin.name = endPin->name;
                            endApiPin.isInput = endPin->isInput;
                            endApiPin.type = static_cast<PinType>(endPin->type);
                            
                            m_state.canConnectToMagnetPin = canCreateConnection(magnetApiPin, endApiPin);
                        } else {
                            m_state.canConnectToMagnetPin = false;
                        }
                    }
                }
            }
        } else {
            p2 = ImGui::GetMousePos();
        }
    }
    else if (m_state.connectingNodeId != -1 && m_state.connectingPinId != -1) {
        const Node *sourceNode = getNode(m_state.connectingNodeId);
        if (!sourceNode) return;

        const Pin *sourcePinInternal = sourceNode->findPin(m_state.connectingPinId);
        if (!sourcePinInternal) return;

        Pin apiPin;
        apiPin.id = sourcePinInternal->id;
        apiPin.name = sourcePinInternal->name;
        apiPin.isInput = sourcePinInternal->isInput;
        apiPin.type = static_cast<PinType>(sourcePinInternal->type);
        apiPin.shape = static_cast<PinShape>(sourcePinInternal->shape);

        p1 = getPinPos(*sourceNode, apiPin, canvasPos);
        isSourceInput = sourcePinInternal->isInput;

        if (m_state.magnetPinNodeId != -1) {
            const Node *magnetNode = getNode(m_state.magnetPinNodeId);
            if (!magnetNode) {
                p2 = ImGui::GetMousePos();
            } else {
                const Pin *magnetPinInternal = magnetNode->findPin(m_state.magnetPinId);
                if (!magnetPinInternal) {
                    p2 = ImGui::GetMousePos();
                } else {
                    Pin magnetPin;
                    magnetPin.id = magnetPinInternal->id;
                    magnetPin.name = magnetPinInternal->name;
                    magnetPin.isInput = magnetPinInternal->isInput;
                    magnetPin.type = static_cast<PinType>(magnetPinInternal->type);
                    magnetPin.shape = static_cast<PinShape>(magnetPinInternal->shape);

                    p2 = getPinPos(*magnetNode, magnetPin, canvasPos);
                    isEndInput = magnetPinInternal->isInput;

                    Pin sourceApiPin;
                    sourceApiPin.id = sourcePinInternal->id;
                    sourceApiPin.name = sourcePinInternal->name;
                    sourceApiPin.isInput = sourcePinInternal->isInput;
                    sourceApiPin.type = static_cast<PinType>(sourcePinInternal->type);

                    Pin magnetApiPin;
                    magnetApiPin.id = magnetPinInternal->id;
                    magnetApiPin.name = magnetPinInternal->name;
                    magnetApiPin.isInput = magnetPinInternal->isInput;
                    magnetApiPin.type = static_cast<PinType>(magnetPinInternal->type);

                    if (sourcePinInternal->isInput != magnetPinInternal->isInput) {
                        bool canConnect;
                        if (sourcePinInternal->isInput) {
                            canConnect = canCreateConnection(magnetApiPin, sourceApiPin);
                        } else {
                            canConnect = canCreateConnection(sourceApiPin, magnetApiPin);
                        }

                        m_state.canConnectToMagnetPin = canConnect;
                    } else {
                        m_state.canConnectToMagnetPin = false;
                    }
                }
            }
        } else {
            p2 = ImGui::GetMousePos();
        }
    } else {
        return;
    }

    std::string pinTypeName = "Blue"; 
    
    if (m_connectingFromReroute && m_connectingRerouteId != -1) {
        Reroute* reroute = getReroute(m_connectingRerouteId);
        if (reroute) {
            const Connection* originalConnection = getConnection(reroute->connectionId);
            if (originalConnection) {
                const Node* startNode = getNode(originalConnection->startNodeId);
                if (startNode) {
                    const Pin* startPin = startNode->findPin(originalConnection->startPinId);
                    if (startPin) {
                        pinTypeName = pinTypeToString(startPin->type);
                    }
                }
            }
        }
    } 
    else if (m_state.connectingNodeId != -1 && m_state.connectingPinId != -1) {
        const Node* sourceNode = getNode(m_state.connectingNodeId);
        if (sourceNode) {
            const Pin* sourcePinInternal = sourceNode->findPin(m_state.connectingPinId);
            if (sourcePinInternal) {
                pinTypeName = pinTypeToString(sourcePinInternal->type);
            }
        }
    }

    const internal::PinColors &pinColors = m_state.style.pinColors.count(pinTypeName)
                                           ? m_state.style.pinColors.at(pinTypeName)
                                           : m_state.style.pinColors.at("Default");

    ImU32 dragColor;
    if (m_state.magnetPinNodeId != -1 && !m_state.canConnectToMagnetPin) {
        dragColor = IM_COL32(255, 50, 50, 200);
    } else {
        dragColor = IM_COL32(
            pinColors.base.r * 255,
            pinColors.base.g * 255,
            pinColors.base.b * 255,
            pinColors.base.a * 255 * 0.8f
        );
    }

    ImU32 outerColor = IM_COL32(40, 44, 52, 100);
    float thickness = m_state.style.connectionThickness * m_state.viewScale;

    float distance = std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
    float cpOffset = std::max(50.0f, distance * 0.5f);

    ImVec2 cp1, cp2;

    if (isSourceInput) {
        cp1 = ImVec2(p1.x, p1.y - cpOffset);
    } else {
        cp1 = ImVec2(p1.x, p1.y + cpOffset);
    }

    if (isEndInput) {
        cp2 = ImVec2(p2.x, p2.y - cpOffset);
    } else {
        cp2 = ImVec2(p2.x, p2.y + cpOffset);
    }

    drawList->AddBezierCubic(p1, cp1, cp2, p2, outerColor, thickness + 1.5f);
    drawList->AddBezierCubic(p1, cp1, cp2, p2, dragColor, thickness);

    ImVec4 brightColor = ImGui::ColorConvertU32ToFloat4(dragColor);
    brightColor.x = std::min(brightColor.x + 0.3f, 1.0f);
    brightColor.y = std::min(brightColor.y + 0.3f, 1.0f);
    brightColor.z = std::min(brightColor.z + 0.3f, 1.0f);
    brightColor.w = 0.7f;

    ImU32 brightDragColor = ImGui::ColorConvertFloat4ToU32(brightColor);
    drawList->AddBezierCubic(p1, cp1, cp2, p2, brightDragColor, thickness * 0.4f);

    const float glowRadius = 2.5f * m_state.viewScale;
    ImU32 glowColor = IM_COL32(
        std::min(static_cast<int>(pinColors.base.r * 255 + 50), 255),
        std::min(static_cast<int>(pinColors.base.g * 255 + 50), 255),
        std::min(static_cast<int>(pinColors.base.b * 255 + 50), 255),
        180
    );
    drawList->AddCircleFilled(p1, glowRadius, glowColor);

    if (m_state.magnetPinNodeId != -1 && !m_state.canConnectToMagnetPin) {
        ImVec2 midPoint = ImBezierCubicCalc(p1, cp1, cp2, p2, 0.5f);

        float crossSize = 8.0f * m_state.viewScale;
        float crossThickness = 2.0f * m_state.viewScale;
        ImU32 crossColor = IM_COL32(255, 50, 50, 230);

        drawList->AddLine(
            ImVec2(midPoint.x - crossSize, midPoint.y - crossSize),
            ImVec2(midPoint.x + crossSize, midPoint.y + crossSize),
            crossColor, crossThickness
        );

        drawList->AddLine(
            ImVec2(midPoint.x - crossSize, midPoint.y + crossSize),
            ImVec2(midPoint.x + crossSize, midPoint.y - crossSize),
            crossColor, crossThickness
        );
    }
}

    void NodeEditor::drawNodePins(ImDrawList *drawList, const Node &node, const ImVec2 &nodePos, const ImVec2 &nodeSize,
                                  const ImVec2 &canvasPos) {
        float pinRadius = m_state.style.pinRadius * m_state.viewScale * 1.2f;

        for (size_t i = 0; i < node.inputs.size(); ++i) {
            const auto &pinInternal = node.inputs[i];

            Pin apiPin;
            apiPin.id = pinInternal.id;
            apiPin.name = pinInternal.name;
            apiPin.isInput = pinInternal.isInput;
            apiPin.type = static_cast<PinType>(pinInternal.type);
            apiPin.shape = static_cast<PinShape>(pinInternal.shape);

            ImVec2 pinPos = getPinPos(node, apiPin, canvasPos);

            std::string pinTypeName = pinTypeToString(pinInternal.type);
            const internal::PinColors &pinColors = m_state.style.pinColors.count(pinTypeName)
                                                       ? m_state.style.pinColors.at(pinTypeName)
                                                       : m_state.style.pinColors.at("Default");

            ImU32 pinColor = IM_COL32(
                pinColors.base.r * 255,
                pinColors.base.g * 255,
                pinColors.base.b * 255,
                pinColors.base.a * 255
            );

            ImU32 pinOutlineColor = IM_COL32(80, 80, 90, 180);
            float pinOutlineThickness = 1.0f;

            ImVec2 mousePos = ImGui::GetMousePos();
            float dx = mousePos.x - pinPos.x;
            float dy = mousePos.y - pinPos.y;
            bool pinHovered = (dx * dx + dy * dy) <= (pinRadius * pinRadius * 4.0f);

            if (pinInternal.connected) {
                pinColor = IM_COL32(
                    pinColors.connected.r * 255,
                    pinColors.connected.g * 255,
                    pinColors.connected.b * 255,
                    pinColors.connected.a * 255
                );
                pinOutlineColor = IM_COL32(255, 255, 255, 100);
                pinOutlineThickness = 1.5f;
            } else if (pinHovered) {
                pinColor = IM_COL32(
                    pinColors.hover.r * 255,
                    pinColors.hover.g * 255,
                    pinColors.hover.b * 255,
                    pinColors.hover.a * 255
                );
            }

            if (node.disabled) {
                ImVec4 colorVec = ImGui::ColorConvertU32ToFloat4(pinColor);
                colorVec.w *= 0.5f;
                pinColor = ImGui::ColorConvertFloat4ToU32(colorVec);

                ImVec4 outlineVec = ImGui::ColorConvertU32ToFloat4(pinOutlineColor);
                outlineVec.w *= 0.5f;
                pinOutlineColor = ImGui::ColorConvertFloat4ToU32(outlineVec);
            }

            drawPinShape(drawList, pinPos, pinRadius, pinInternal.shape, pinColor, pinOutlineColor, pinOutlineThickness,
                         pinHovered);
        }

        for (size_t i = 0; i < node.outputs.size(); ++i) {
            const auto &pinInternal = node.outputs[i];

            Pin apiPin;
            apiPin.id = pinInternal.id;
            apiPin.name = pinInternal.name;
            apiPin.isInput = pinInternal.isInput;
            apiPin.type = static_cast<PinType>(pinInternal.type);
            apiPin.shape = static_cast<PinShape>(pinInternal.shape);

            ImVec2 pinPos = getPinPos(node, apiPin, canvasPos);

            std::string pinTypeName = pinTypeToString(pinInternal.type);
            const internal::PinColors &pinColors = m_state.style.pinColors.count(pinTypeName)
                                                       ? m_state.style.pinColors.at(pinTypeName)
                                                       : m_state.style.pinColors.at("Default");

            ImU32 pinColor = IM_COL32(
                pinColors.base.r * 255,
                pinColors.base.g * 255,
                pinColors.base.b * 255,
                pinColors.base.a * 255
            );

            ImU32 pinOutlineColor = IM_COL32(80, 80, 90, 180);
            float pinOutlineThickness = 1.0f;

            ImVec2 mousePos = ImGui::GetMousePos();
            float dx = mousePos.x - pinPos.x;
            float dy = mousePos.y - pinPos.y;
            bool pinHovered = (dx * dx + dy * dy) <= (pinRadius * pinRadius * 4.0f);

            if (pinInternal.connected) {
                pinColor = IM_COL32(
                    pinColors.connected.r * 255,
                    pinColors.connected.g * 255,
                    pinColors.connected.b * 255,
                    pinColors.connected.a * 255
                );
                pinOutlineColor = IM_COL32(255, 255, 255, 100);
                pinOutlineThickness = 1.5f;
            } else if (pinHovered) {
                pinColor = IM_COL32(
                    pinColors.hover.r * 255,
                    pinColors.hover.g * 255,
                    pinColors.hover.b * 255,
                    pinColors.hover.a * 255
                );
            }

            if (node.disabled) {
                ImVec4 colorVec = ImGui::ColorConvertU32ToFloat4(pinColor);
                colorVec.w *= 0.5f;
                pinColor = ImGui::ColorConvertFloat4ToU32(colorVec);

                ImVec4 outlineVec = ImGui::ColorConvertU32ToFloat4(pinOutlineColor);
                outlineVec.w *= 0.5f;
                pinOutlineColor = ImGui::ColorConvertFloat4ToU32(outlineVec);
            }

            drawPinShape(drawList, pinPos, pinRadius, pinInternal.shape, pinColor, pinOutlineColor, pinOutlineThickness,
                         pinHovered);
        }
    }
}
