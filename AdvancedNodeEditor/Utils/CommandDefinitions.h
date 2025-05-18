#pragma once

namespace NodeEditorCommands {
    namespace Node {
        static constexpr const char* Add = "nodeeditor.node.add";
        static constexpr const char* Remove = "nodeeditor.node.remove";
        static constexpr const char* Select = "nodeeditor.node.select";
        static constexpr const char* Deselect = "nodeeditor.node.deselect";
        static constexpr const char* SelectAll = "nodeeditor.node.selectAll";
        static constexpr const char* DeselectAll = "nodeeditor.node.deselectAll";
        static constexpr const char* Duplicate = "nodeeditor.node.duplicate";
        static constexpr const char* SetExecuting = "nodeeditor.node.setExecuting";
        static constexpr const char* Move = "nodeeditor.node.move";
    }

    namespace Connection {
        static constexpr const char* Add = "nodeeditor.connection.add";
        static constexpr const char* Remove = "nodeeditor.connection.remove";
        static constexpr const char* Select = "nodeeditor.connection.select";
        static constexpr const char* Deselect = "nodeeditor.connection.deselect";
        static constexpr const char* ActivateFlow = "nodeeditor.connection.activateFlow";
        static constexpr const char* DeactivateFlow = "nodeeditor.connection.deactivateFlow";
        static constexpr const char* ActivateAllFlows = "nodeeditor.connection.activateAllFlows";
        static constexpr const char* DeactivateAllFlows = "nodeeditor.connection.deactivateAllFlows";
    }

    namespace Group {
        static constexpr const char* Add = "nodeeditor.group.add";
        static constexpr const char* Remove = "nodeeditor.group.remove";
        static constexpr const char* AddNode = "nodeeditor.group.addNode";
        static constexpr const char* RemoveNode = "nodeeditor.group.removeNode";
    }

    namespace Subgraph {
        static constexpr const char* Create = "nodeeditor.subgraph.create";
        static constexpr const char* Remove = "nodeeditor.subgraph.remove";
        static constexpr const char* Enter = "nodeeditor.subgraph.enter";
        static constexpr const char* Exit = "nodeeditor.subgraph.exit";
        static constexpr const char* AddNode = "nodeeditor.subgraph.addNode";
        static constexpr const char* RemoveNode = "nodeeditor.subgraph.removeNode";
    }

    namespace View {
        static constexpr const char* ZoomToFit = "nodeeditor.view.zoomToFit";
        static constexpr const char* ZoomToSelected = "nodeeditor.view.zoomToSelected";
        static constexpr const char* CenterView = "nodeeditor.view.centerView";
        static constexpr const char* CenterOnNode = "nodeeditor.view.centerOnNode";
        static constexpr const char* SetPosition = "nodeeditor.view.setPosition";
        static constexpr const char* SetScale = "nodeeditor.view.setScale";
    }

    namespace Style {
        static constexpr const char* SetGridColor = "nodeeditor.style.setGridColor";
        static constexpr const char* SetBackgroundColor = "nodeeditor.style.setBackgroundColor";
        static constexpr const char* SetConnectionStyle = "nodeeditor.style.setConnectionStyle";
        static constexpr const char* SetConnectionColor = "nodeeditor.style.setConnectionColor";
        static constexpr const char* SetGraphTitle = "nodeeditor.style.setGraphTitle";
    }

    namespace Evaluation {
        static constexpr const char* Evaluate = "nodeeditor.evaluation.evaluate";
        static constexpr const char* GetOrder = "nodeeditor.evaluation.getOrder";
    }

    namespace UI {
        static constexpr const char* ShowNodeCreated = "nodeeditor.ui.showNodeCreated";
        static constexpr const char* ShowNodeRemoved = "nodeeditor.ui.showNodeRemoved";
        static constexpr const char* ShowConnectionCreated = "nodeeditor.ui.showConnectionCreated";
        static constexpr const char* ShowConnectionRemoved = "nodeeditor.ui.showConnectionRemoved";
        static constexpr const char* ShowEvaluationResult = "nodeeditor.ui.showEvaluationResult";
        static constexpr const char* ShowError = "nodeeditor.ui.showError";
        static constexpr const char* UpdateNodeList = "nodeeditor.ui.updateNodeList";
        static constexpr const char* UpdateConnectionList = "nodeeditor.ui.updateConnectionList";
    }
}