#pragma once

namespace Commands {
    namespace Backend {
        static constexpr const char* EvaluateGraph = "graph.evaluate";
        static constexpr const char* CompileGraph = "graph.compile";
        static constexpr const char* AddNode = "node.add";
        static constexpr const char* RemoveNode = "node.remove";
        static constexpr const char* ConnectNodes = "nodes.connect";
        static constexpr const char* DisconnectNodes = "nodes.disconnect";
    }

    namespace UI {
        static constexpr const char* ShowResult = "ui.show_result";
        static constexpr const char* ShowError = "ui.show_error";
        static constexpr const char* UpdateNodeView = "ui.update_node_view";
        static constexpr const char* UpdatePropertyPanel = "ui.update_property_panel";
    }

    namespace System {
        static constexpr const char* SaveProject = "system.save_project";
        static constexpr const char* LoadProject = "system.load_project";
        static constexpr const char* ExportGraph = "system.export_graph";
        static constexpr const char* ImportGraph = "system.import_graph";
    }
}