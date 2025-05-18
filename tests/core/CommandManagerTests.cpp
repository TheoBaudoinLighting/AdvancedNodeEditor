#include <gtest/gtest.h>
#include "../../AdvancedNodeEditor/Utils/CommandManager.h"
#include "../../AdvancedNodeEditor/Utils/CommandDefinitions.h"
#include <string>
#include <sstream>

class CommandManagerTest : public ::testing::Test {
protected:
    CommandManager manager;

    bool backendCalled = false;
    bool uiCalled = false;
    std::any backendData;
    std::any uiData;

    std::stringstream errorStream;
    std::streambuf* originalCerr;

    bool backendErrorHandlerCalled = false;
    bool uiErrorHandlerCalled = false;
    std::string lastBackendErrorCommand;
    std::string lastUIErrorCommand;

    void SetUp() override {
        backendCalled = false;
        uiCalled = false;
        backendData = std::any();
        uiData = std::any();
        backendErrorHandlerCalled = false;
        uiErrorHandlerCalled = false;
        lastBackendErrorCommand = "";
        lastUIErrorCommand = "";

        originalCerr = std::cerr.rdbuf();
        std::cerr.rdbuf(errorStream.rdbuf());

        manager.getBackendRouter().setErrorHandler([this](const std::string& cmd, const std::any&) {
            backendErrorHandlerCalled = true;
            lastBackendErrorCommand = cmd;
        });

        manager.getUIRouter().setErrorHandler([this](const std::string& cmd, const std::any&) {
            uiErrorHandlerCalled = true;
            lastUIErrorCommand = cmd;
        });
    }

    void TearDown() override {
        std::cerr.rdbuf(originalCerr);
    }
};

TEST_F(CommandManagerTest, BindAndDispatchToBackend) {
    manager.bindToBackend("backend.command", [this](const std::any& data) {
        backendCalled = true;
        backendData = data;
    });

    EXPECT_TRUE(manager.isBackendBound("backend.command"));
    EXPECT_FALSE(manager.isBackendBound("nonexistent.command"));

    manager.dispatchToBackend("backend.command", 42);
    EXPECT_TRUE(backendCalled);

    try {
        int value = std::any_cast<int>(backendData);
        EXPECT_EQ(value, 42);
    } catch (const std::bad_any_cast&) {
        FAIL() << "Failed to cast data to int";
    }
}

TEST_F(CommandManagerTest, BindAndDispatchToUI) {
    manager.bindToUI("ui.command", [this](const std::any& data) {
        uiCalled = true;
        uiData = data;
    });

    EXPECT_TRUE(manager.isUIBound("ui.command"));
    EXPECT_FALSE(manager.isUIBound("nonexistent.command"));

    manager.dispatchToUI("ui.command", std::string("test"));
    EXPECT_TRUE(uiCalled);

    try {
        std::string value = std::any_cast<std::string>(uiData);
        EXPECT_EQ(value, "test");
    } catch (const std::bad_any_cast&) {
        FAIL() << "Failed to cast data to string";
    }
}

TEST_F(CommandManagerTest, BackendToUIcommunication) {
    manager.bindToBackend("backend.process", [this](const std::any& data) {
        backendCalled = true;
        try {
            int input = std::any_cast<int>(data);
            manager.dispatchToUI("ui.result", std::string("Résultat: " + std::to_string(input * 2)));
        } catch (const std::bad_any_cast&) {
            manager.dispatchToUI("ui.error", std::string("Données d'entrée invalides"));
        }
    });

    manager.bindToUI("ui.result", [this](const std::any& data) {
        uiCalled = true;
        uiData = data;
    });

    manager.bindToUI("ui.error", [this](const std::any& data) {
        uiCalled = true;
        uiData = data;
    });

    manager.dispatchToBackend("backend.process", 21);

    EXPECT_TRUE(backendCalled);
    EXPECT_TRUE(uiCalled);

    try {
        std::string value = std::any_cast<std::string>(uiData);
        EXPECT_EQ(value, "Résultat: 42");
    } catch (const std::bad_any_cast&) {
        FAIL() << "Failed to cast UI data to string";
    }
}

TEST_F(CommandManagerTest, DispatchTypedToBackend) {
    manager.bindToBackend("backend.typed", [this](const std::any& data) {
        backendCalled = true;
        backendData = data;
    });

    manager.dispatchTypedToBackend("backend.typed", std::vector<double>{1.1, 2.2, 3.3});
    EXPECT_TRUE(backendCalled);

    try {
        auto value = std::any_cast<std::vector<double>>(backendData);
        EXPECT_EQ(value.size(), 3);
        EXPECT_DOUBLE_EQ(value[0], 1.1);
        EXPECT_DOUBLE_EQ(value[1], 2.2);
        EXPECT_DOUBLE_EQ(value[2], 3.3);
    } catch (const std::bad_any_cast&) {
        FAIL() << "Failed to cast data to vector<double>";
    }
}

TEST_F(CommandManagerTest, DispatchTypedToUI) {
    manager.bindToUI("ui.typed", [this](const std::any& data) {
        uiCalled = true;
        uiData = data;
    });

    struct TestStruct {
        int id;
        std::string name;

        bool operator==(const TestStruct& other) const {
            return id == other.id && name == other.name;
        }
    };

    TestStruct test{42, "test"};
    manager.dispatchTypedToUI("ui.typed", test);
    EXPECT_TRUE(uiCalled);

    try {
        auto value = std::any_cast<TestStruct>(uiData);
        EXPECT_TRUE(value == test);
    } catch (const std::bad_any_cast&) {
        FAIL() << "Failed to cast data to TestStruct";
    }
}

TEST_F(CommandManagerTest, GetRouters) {
    auto& backendRouter = manager.getBackendRouter();
    auto& uiRouter = manager.getUIRouter();

    backendRouter.bind("test.backend", [](const std::any&) {});
    uiRouter.bind("test.ui", [](const std::any&) {});

    EXPECT_TRUE(manager.isBackendBound("test.backend"));
    EXPECT_TRUE(manager.isUIBound("test.ui"));

    backendRouter.setLoggingEnabled(true);
    manager.dispatchToBackend("test.backend");

    auto loggedCalls = backendRouter.getLoggedCalls();
    EXPECT_EQ(loggedCalls.size(), 1);
    EXPECT_EQ(loggedCalls[0].first, "test.backend");
}

TEST_F(CommandManagerTest, ComplexDataFlow) {
    int backendStep1Count = 0;
    int backendStep2Count = 0;
    int uiUpdateCount = 0;

    manager.bindToBackend("process.step1", [this, &backendStep1Count](const std::any& data) {
        backendStep1Count++;
        manager.dispatchToBackend("process.step2", 42);
    });

    manager.bindToBackend("process.step2", [this, &backendStep2Count](const std::any& data) {
        backendStep2Count++;
        try {
            int value = std::any_cast<int>(data);
            manager.dispatchToUI("ui.update", std::string("Étape 2 terminée avec valeur: " + std::to_string(value)));
        } catch (const std::bad_any_cast&) {
            manager.dispatchToUI("ui.update", std::string("Erreur dans l'étape 2"));
        }
    });

    manager.bindToUI("ui.update", [this, &uiUpdateCount](const std::any& data) {
        uiUpdateCount++;
        uiData = data;
    });

    manager.dispatchToBackend("process.step1");

    EXPECT_EQ(backendStep1Count, 1);
    EXPECT_EQ(backendStep2Count, 1);
    EXPECT_EQ(uiUpdateCount, 1);

    try {
        std::string value = std::any_cast<std::string>(uiData);
        EXPECT_EQ(value, "Étape 2 terminée avec valeur: 42");
    } catch (const std::bad_any_cast&) {
        FAIL() << "Failed to cast UI data to string";
    }
}

TEST_F(CommandManagerTest, ErrorHandlersTriggered) {
    errorStream.str("");

    manager.dispatchToBackend("unknown.backend.command");
    manager.dispatchToUI("unknown.ui.command");

    EXPECT_TRUE(backendErrorHandlerCalled);
    EXPECT_TRUE(uiErrorHandlerCalled);

    EXPECT_EQ(lastBackendErrorCommand, "unknown.backend.command");
    EXPECT_EQ(lastUIErrorCommand, "unknown.ui.command");

    std::string errorOutput = errorStream.str();
    EXPECT_FALSE(errorOutput.empty());
    EXPECT_TRUE(errorOutput.find("unknown.backend.command") != std::string::npos);
    EXPECT_TRUE(errorOutput.find("unknown.ui.command") != std::string::npos);
}

TEST_F(CommandManagerTest, UseCommandDefinitionsWithManager) {
    bool evalCalled = false;
    bool resultCalled = false;

    manager.bindToBackend(Commands::Backend::EvaluateGraph, [&evalCalled](const std::any&) {
        evalCalled = true;
    });

    manager.bindToUI(Commands::UI::ShowResult, [&resultCalled](const std::any&) {
        resultCalled = true;
    });

    manager.dispatchToBackend(Commands::Backend::EvaluateGraph);
    manager.dispatchToUI(Commands::UI::ShowResult);

    EXPECT_TRUE(evalCalled);
    EXPECT_TRUE(resultCalled);

    EXPECT_TRUE(manager.isBackendBound(Commands::Backend::EvaluateGraph));
    EXPECT_TRUE(manager.isUIBound(Commands::UI::ShowResult));

    EXPECT_FALSE(manager.isBackendBound(Commands::Backend::RemoveNode));
    EXPECT_FALSE(manager.isUIBound(Commands::UI::ShowError));
}