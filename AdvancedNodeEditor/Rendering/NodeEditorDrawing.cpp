#include "../Core/NodeEditor.h"
#include <algorithm>

namespace {
    constexpr const char *kEditorPinDataTypeMetadataKey = "nodeeditor.dataType";

    const char *PinTypeTooltipLabel(const NodeEditorCore::Pin &pin) {
        const int rawDataType = pin.getMetadata<int>(kEditorPinDataTypeMetadataKey, -1);
        const auto dataType = static_cast<NodeEditorCore::DataType>(rawDataType);
        if (NodeEditorCore::PinStyleCatalog::HasDataType(dataType)) {
            return NodeEditorCore::PinStyleCatalog::ForDataType(dataType).label;
        }

        return nullptr;
    }

    void ShowPinTypeTooltip(const NodeEditorCore::Pin &pin) {
        const char *label = PinTypeTooltipLabel(pin);
        if (label) {
            ImGui::SetTooltip("%s", label);
        }
    }
}

namespace NodeEditorCore {
    void NodeEditor::drawDragConnection(ImDrawList *drawList, const ImVec2 &canvasPos) {
        if (!m_state.connecting)
            return;

        ImVec2 p1;
        ImVec2 p2;
        bool isEndInput = false;
        bool isSourceInput = false;
        auto setFreeEndToMouse = [&]() {
            p2 = ImGui::GetMousePos();
            isEndInput = !isSourceInput;
        };

        if (m_connectingFromReroute && m_connectingRerouteId != -1) {
            Reroute *reroute = getReroute(m_connectingRerouteId);
            if (!reroute) return;

            Vec2 rerouteScreenPos = canvasToScreen(reroute->position);
            p1 = rerouteScreenPos.toImVec2();

            const Connection *originalConnection = getConnection(reroute->connectionId);
            if (!originalConnection) return;

            const Node *startNode = getNode(originalConnection->startNodeId);
            if (!startNode) return;

            const Pin *startPin = startNode->findPin(originalConnection->startPinId);
            if (!startPin) return;

            isSourceInput = false;

            if (m_state.magnetPinNodeId != -1) {
                const Node *magnetNode = getNode(m_state.magnetPinNodeId);
                if (!magnetNode) {
                    setFreeEndToMouse();
                } else {
                    const Pin *magnetPinInternal = magnetNode->findPin(m_state.magnetPinId);
                    if (!magnetPinInternal) {
                        setFreeEndToMouse();
                    } else {
                        p2 = getPinPos(*magnetNode, *magnetPinInternal, canvasPos);
                        isEndInput = magnetPinInternal->isInput;

                        if (magnetPinInternal->isInput) {
                            m_state.canConnectToMagnetPin = canCreateConnection(*startPin, *magnetPinInternal);
                        } else {
                            const Node *endNode = getNode(originalConnection->endNodeId);
                            const Pin *endPin = endNode ? endNode->findPin(originalConnection->endPinId) : nullptr;

                            if (endPin) {
                                m_state.canConnectToMagnetPin = canCreateConnection(*magnetPinInternal, *endPin);
                            } else {
                                m_state.canConnectToMagnetPin = false;
                            }
                        }
                    }
                }
            } else {
                setFreeEndToMouse();
            }
        } else if (m_state.connectingNodeId != -1 && m_state.connectingPinId != -1) {
            const Node *sourceNode = getNode(m_state.connectingNodeId);
            if (!sourceNode) return;

            const Pin *sourcePinInternal = sourceNode->findPin(m_state.connectingPinId);
            if (!sourcePinInternal) return;

            p1 = getPinPos(*sourceNode, *sourcePinInternal, canvasPos);
            isSourceInput = sourcePinInternal->isInput;

            if (m_magnetRerouteId != -1) {
                const Reroute *magnetReroute = getReroute(m_magnetRerouteId);
                if (!magnetReroute) {
                    setFreeEndToMouse();
                } else {
                    Vec2 rerouteScreenPos = canvasToScreen(magnetReroute->position);
                    p2 = rerouteScreenPos.toImVec2();
                    isEndInput = !sourcePinInternal->isInput;
                }
            } else if (m_state.magnetPinNodeId != -1) {
                const Node *magnetNode = getNode(m_state.magnetPinNodeId);
                if (!magnetNode) {
                    setFreeEndToMouse();
                } else {
                    const Pin *magnetPinInternal = magnetNode->findPin(m_state.magnetPinId);
                    if (!magnetPinInternal) {
                        setFreeEndToMouse();
                    } else {
                        p2 = getPinPos(*magnetNode, *magnetPinInternal, canvasPos);
                        isEndInput = magnetPinInternal->isInput;

                        if (sourcePinInternal->isInput != magnetPinInternal->isInput) {
                            bool canConnect;
                            if (sourcePinInternal->isInput) {
                                canConnect = canCreateConnection(*magnetPinInternal, *sourcePinInternal);
                            } else {
                                canConnect = canCreateConnection(*sourcePinInternal, *magnetPinInternal);
                            }

                            m_state.canConnectToMagnetPin = canConnect;
                        } else {
                            m_state.canConnectToMagnetPin = false;
                        }
                    }
                }
            } else {
                setFreeEndToMouse();
            }
        } else {
            return;
        }

        std::string pinTypeName = "Blue";

        if (m_connectingFromReroute && m_connectingRerouteId != -1) {
            Reroute *reroute = getReroute(m_connectingRerouteId);
            if (reroute) {
                const Connection *originalConnection = getConnection(reroute->connectionId);
                if (originalConnection) {
                    const Node *startNode = getNode(originalConnection->startNodeId);
                    if (startNode) {
                        const Pin *startPin = startNode->findPin(originalConnection->startPinId);
                        if (startPin) {
                            pinTypeName = pinTypeToString(startPin->type);
                        }
                    }
                }
            }
        } else if (m_state.connectingNodeId != -1 && m_state.connectingPinId != -1) {
            const Node *sourceNode = getNode(m_state.connectingNodeId);
            if (sourceNode) {
                const Pin *sourcePinInternal = sourceNode->findPin(m_state.connectingPinId);
                if (sourcePinInternal) {
                    pinTypeName = pinTypeToString(sourcePinInternal->type);
                }
            }
        }

        const internal::PinColors &pinColors = m_state.style.pinColors.count(pinTypeName)
                                                   ? m_state.style.pinColors.at(pinTypeName)
                                                   : m_state.style.pinColors.at("Default");

        ImU32 dragColor;
        if ((m_state.magnetPinNodeId != -1 && !m_state.canConnectToMagnetPin) ||
            (m_magnetRerouteId != -1 && !m_canConnectToMagnetReroute)) {
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

        float distance = Math::distance(
            Vec2(p1.x, p1.y),
            Vec2(p2.x, p2.y));
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

        if ((m_state.magnetPinNodeId != -1 && !m_state.canConnectToMagnetPin) ||
            (m_magnetRerouteId != -1 && !m_canConnectToMagnetReroute)) {
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
        (void) nodePos;
        (void) nodeSize;

        struct PinDrawStyle {
            ImU32 fillColor;
            ImU32 outlineColor;
            float outlineThickness;
        };

        const float pinRadius = getPinVisualRadius();

        auto toImColor = [](const Color &color) {
            return IM_COL32(
                static_cast<int>(color.r * 255.0f),
                static_cast<int>(color.g * 255.0f),
                static_cast<int>(color.b * 255.0f),
                static_cast<int>(color.a * 255.0f)
            );
        };

        auto resolveDrawStyle = [&](const Pin &pin, bool hovered) {
            const std::string pinTypeName = pinTypeToString(pin.type);
            auto colorsIt = m_state.style.pinColors.find(pinTypeName);
            const internal::PinColors &pinColors = colorsIt != m_state.style.pinColors.end()
                                                       ? colorsIt->second
                                                       : m_state.style.pinColors.at("Default");

            PinDrawStyle style{
                toImColor(pinColors.base),
                IM_COL32(80, 80, 90, 180),
                1.0f
            };

            if (pin.connected) {
                style.fillColor = toImColor(pinColors.connected);
                style.outlineColor = IM_COL32(255, 255, 255, 100);
                style.outlineThickness = 1.5f;
            } else if (hovered) {
                style.fillColor = toImColor(pinColors.hover);
            }

            if (node.disabled) {
                ImVec4 fill = ImGui::ColorConvertU32ToFloat4(style.fillColor);
                fill.w *= 0.5f;
                style.fillColor = ImGui::ColorConvertFloat4ToU32(fill);

                ImVec4 outline = ImGui::ColorConvertU32ToFloat4(style.outlineColor);
                outline.w *= 0.5f;
                style.outlineColor = ImGui::ColorConvertFloat4ToU32(outline);
            }

            return style;
        };

        auto drawPin = [&](const Pin &pin) {
            const ImVec2 pinPos = getPinPos(node, pin, canvasPos);
            const bool hovered = isPinHovered(node, pin, canvasPos);
            const PinDrawStyle style = resolveDrawStyle(pin, hovered);

            drawPinShape(drawList, pinPos, pinRadius, pin.shape, style.fillColor, style.outlineColor,
                         style.outlineThickness, hovered);

            if (hovered) {
                ShowPinTypeTooltip(pin);
            }
        };

        for (const Pin &pin: node.inputs) {
            drawPin(pin);
        }

        for (const Pin &pin: node.outputs) {
            drawPin(pin);
        }
    }
}
