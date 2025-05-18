#include <gtest/gtest.h>
#include "../../AdvancedNodeEditor/Utils/CommandRouter.h"
#include "../../AdvancedNodeEditor/Utils/CommandDefinitions.h"
#include <string>
#include <vector>
#include <sstream>

class CommandRouterTest : public ::testing::Test {
protected:
    CommandRouter router;

    bool handlerCalled = false;
    std::any lastData;

    std::stringstream errorStream;
    std::streambuf* originalCerr;

    bool errorHandlerCalled = false;
    std::string lastErrorCommand;
    std::any lastErrorData;

    void SetUp() override {
        handlerCalled = false;
        lastData = std::any();
        errorHandlerCalled = false;
        lastErrorCommand = "";
        lastErrorData = std::any();

        originalCerr = std::cerr.rdbuf();
        std::cerr.rdbuf(errorStream.rdbuf());
    }

    void TearDown() override {
        std::cerr.rdbuf(originalCerr);
    }
};

TEST_F(CommandRouterTest, BindAndDispatch) {
    router.bind("test.command", [this](const std::any& data) {
        handlerCalled = true;
        lastData = data;
    });

    EXPECT_TRUE(router.isBound("test.command"));
    EXPECT_FALSE(router.isBound("nonexistent.command"));

    router.dispatch("test.command");
    EXPECT_TRUE(handlerCalled);
}

TEST_F(CommandRouterTest, DispatchWithStringData) {
    router.bind("test.string", [this](const std::any& data) {
        handlerCalled = true;
        lastData = data;
    });

    router.dispatch("test.string", std::string("test value"));
    EXPECT_TRUE(handlerCalled);

    try {
        std::string value = std::any_cast<std::string>(lastData);
        EXPECT_EQ(value, "test value");
    } catch (const std::bad_any_cast&) {
        FAIL() << "Failed to cast data to string";
    }
}

TEST_F(CommandRouterTest, DispatchWithIntData) {
    router.bind("test.int", [this](const std::any& data) {
        handlerCalled = true;
        lastData = data;
    });

    router.dispatch("test.int", 42);
    EXPECT_TRUE(handlerCalled);

    try {
        int value = std::any_cast<int>(lastData);
        EXPECT_EQ(value, 42);
    } catch (const std::bad_any_cast&) {
        FAIL() << "Failed to cast data to int";
    }
}

TEST_F(CommandRouterTest, DispatchNonexistentCommand) {
    int callCount = 0;

    router.bind("test.command", [&callCount](const std::any&) {
        callCount++;
    });

    errorStream.str("");

    router.dispatch("nonexistent.command");
    EXPECT_EQ(callCount, 0);

    std::string errorOutput = errorStream.str();
    EXPECT_FALSE(errorOutput.empty());
    EXPECT_TRUE(errorOutput.find("Command non liée") != std::string::npos);
    EXPECT_TRUE(errorOutput.find("nonexistent.command") != std::string::npos);
}

TEST_F(CommandRouterTest, LoggingEnabledDisabled) {
    router.setLoggingEnabled(true);

    router.dispatch("command1", 42);
    router.dispatch("command2", std::string("value"));

    auto loggedCalls = router.getLoggedCalls();
    EXPECT_EQ(loggedCalls.size(), 2);
    EXPECT_EQ(loggedCalls[0].first, "command1");
    EXPECT_EQ(loggedCalls[1].first, "command2");

    router.setLoggingEnabled(false);

    loggedCalls = router.getLoggedCalls();
    EXPECT_EQ(loggedCalls.size(), 0);

    router.dispatch("command3", true);

    loggedCalls = router.getLoggedCalls();
    EXPECT_EQ(loggedCalls.size(), 0);
}

TEST_F(CommandRouterTest, GetBoundCommands) {
    router.bind("command1", [](const std::any&) {});
    router.bind("command2", [](const std::any&) {});
    router.bind("command3", [](const std::any&) {});

    auto commands = router.getBoundCommands();
    EXPECT_EQ(commands.size(), 3);

    std::sort(commands.begin(), commands.end());
    EXPECT_EQ(commands[0], "command1");
    EXPECT_EQ(commands[1], "command2");
    EXPECT_EQ(commands[2], "command3");
}

TEST_F(CommandRouterTest, DispatchTyped) {
    router.bind("test.typed", [this](const std::any& data) {
        handlerCalled = true;
        lastData = data;
    });

    dispatchTyped(router, "test.typed", std::vector<int>{1, 2, 3});
    EXPECT_TRUE(handlerCalled);

    try {
        auto value = std::any_cast<std::vector<int>>(lastData);
        EXPECT_EQ(value.size(), 3);
        EXPECT_EQ(value[0], 1);
        EXPECT_EQ(value[1], 2);
        EXPECT_EQ(value[2], 3);
    } catch (const std::bad_any_cast&) {
        FAIL() << "Failed to cast data to vector<int>";
    }
}

TEST_F(CommandRouterTest, RebindCommand) {
    int handler1Called = 0;
    int handler2Called = 0;

    router.bind("test.rebind", [&handler1Called](const std::any&) {
        handler1Called++;
    });

    router.dispatch("test.rebind");
    EXPECT_EQ(handler1Called, 1);
    EXPECT_EQ(handler2Called, 0);

    router.bind("test.rebind", [&handler2Called](const std::any&) {
        handler2Called++;
    });

    router.dispatch("test.rebind");
    EXPECT_EQ(handler1Called, 1);
    EXPECT_EQ(handler2Called, 1);
}

TEST_F(CommandRouterTest, ErrorHandler) {
    router.setErrorHandler([this](const std::string& cmd, const std::any& data) {
        errorHandlerCalled = true;
        lastErrorCommand = cmd;
        lastErrorData = data;
    });

    int testData = 42;
    router.dispatch("unknown.command", testData);

    EXPECT_TRUE(errorHandlerCalled);
    EXPECT_EQ(lastErrorCommand, "unknown.command");

    try {
        int value = std::any_cast<int>(lastErrorData);
        EXPECT_EQ(value, testData);
    } catch (const std::bad_any_cast&) {
        FAIL() << "Failed to cast error data to int";
    }
}

TEST_F(CommandRouterTest, UseCommandDefinitions) {
    bool graphEvaluateCalled = false;
    bool nodeAddCalled = false;

    router.bind(Commands::Backend::EvaluateGraph, [&graphEvaluateCalled](const std::any&) {
        graphEvaluateCalled = true;
    });

    router.bind(Commands::Backend::AddNode, [&nodeAddCalled](const std::any&) {
        nodeAddCalled = true;
    });

    router.dispatch(Commands::Backend::EvaluateGraph);
    EXPECT_TRUE(graphEvaluateCalled);

    router.dispatch(Commands::Backend::AddNode);
    EXPECT_TRUE(nodeAddCalled);

    EXPECT_TRUE(router.isBound(Commands::Backend::EvaluateGraph));
    EXPECT_TRUE(router.isBound(Commands::Backend::AddNode));
    EXPECT_FALSE(router.isBound(Commands::Backend::RemoveNode));
}