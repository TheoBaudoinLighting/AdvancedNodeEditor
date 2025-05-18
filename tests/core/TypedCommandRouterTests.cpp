#include <gtest/gtest.h>
#include "../../AdvancedNodeEditor/Utils/TypedCommandRouter.h"
#include "../../AdvancedNodeEditor/Utils/CommandDefinitions.h"
#include <string>
#include <vector>

class TypedCommandRouterTest : public ::testing::Test {
protected:
    StrictCommandRouter router;

    struct TestStruct {
        int id;
        std::string name;

        bool operator==(const TestStruct& other) const {
            return id == other.id && name == other.name;
        }
    };
};

TEST_F(TypedCommandRouterTest, BindAndDispatchCorrectType) {
    bool handlerCalled = false;
    int receivedValue = 0;

    router.bind<int>("test.int", [&handlerCalled, &receivedValue](const int& value) {
        handlerCalled = true;
        receivedValue = value;
    });

    router.dispatchTyped("test.int", 42);

    EXPECT_TRUE(handlerCalled);
    EXPECT_EQ(receivedValue, 42);
}

TEST_F(TypedCommandRouterTest, BindAndDispatchDifferentTypes) {
    bool intHandlerCalled = false;
    bool stringHandlerCalled = false;
    bool structHandlerCalled = false;

    router.bind<int>("test.int", [&intHandlerCalled](const int& value) {
        intHandlerCalled = true;
    });

    router.bind<std::string>("test.string", [&stringHandlerCalled](const std::string& value) {
        stringHandlerCalled = true;
    });

    router.bind<TestStruct>("test.struct", [&structHandlerCalled](const TestStruct& value) {
        structHandlerCalled = true;
    });

    router.dispatchTyped("test.int", 42);
    router.dispatchTyped("test.string", std::string("test"));
    router.dispatchTyped("test.struct", TestStruct{1, "test"});

    EXPECT_TRUE(intHandlerCalled);
    EXPECT_TRUE(stringHandlerCalled);
    EXPECT_TRUE(structHandlerCalled);
}

TEST_F(TypedCommandRouterTest, VerifyTypeChecking) {
    router.bind<int>("test.int", [](const int& value) {});
    router.bind<std::string>("test.string", [](const std::string& value) {});

    // Ces appels doivent réussir
    router.dispatchTyped("test.int", 42);
    router.dispatchTyped("test.string", std::string("test"));

    // Ces appels doivent échouer avec des exceptions
    EXPECT_THROW({
        router.dispatch("test.int", std::any(std::string("wrong type")));
    }, std::runtime_error);

    EXPECT_THROW({
        router.dispatch("test.string", std::any(42));
    }, std::runtime_error);

    EXPECT_THROW({
        router.dispatchTyped("test.int", std::string("wrong type"));
    }, std::runtime_error);
}

TEST_F(TypedCommandRouterTest, GetExpectedType) {
    router.bind<int>("test.int", [](const int& value) {});
    router.bind<std::string>("test.string", [](const std::string& value) {});
    router.bind<TestStruct>("test.struct", [](const TestStruct& value) {});

    EXPECT_EQ(router.getExpectedType("test.int"), std::type_index(typeid(int)));
    EXPECT_EQ(router.getExpectedType("test.string"), std::type_index(typeid(std::string)));
    EXPECT_EQ(router.getExpectedType("test.struct"), std::type_index(typeid(TestStruct)));

    EXPECT_THROW({
        router.getExpectedType("nonexistent.command");
    }, std::runtime_error);
}

TEST_F(TypedCommandRouterTest, CommandNotFound) {
    EXPECT_THROW({
        router.dispatch("nonexistent.command", std::any(42));
    }, std::runtime_error);

    EXPECT_THROW({
        router.dispatchTyped("nonexistent.command", 42);
    }, std::runtime_error);
}

TEST_F(TypedCommandRouterTest, RebindCommand) {
    int handler1Called = 0;
    int handler2Called = 0;

    router.bind<int>("test.rebind", [&handler1Called](const int& value) {
        handler1Called++;
    });

    router.dispatchTyped("test.rebind", 42);
    EXPECT_EQ(handler1Called, 1);
    EXPECT_EQ(handler2Called, 0);

    router.bind<int>("test.rebind", [&handler2Called](const int& value) {
        handler2Called++;
    });

    router.dispatchTyped("test.rebind", 42);
    EXPECT_EQ(handler1Called, 1);
    EXPECT_EQ(handler2Called, 1);
}

TEST_F(TypedCommandRouterTest, UseCommandDefinitions) {
    bool evalCalled = false;
    bool addNodeCalled = false;

    router.bind<std::nullptr_t>(Commands::Backend::EvaluateGraph, [&evalCalled](const std::nullptr_t&) {
        evalCalled = true;
    });

    router.bind<int>(Commands::Backend::AddNode, [&addNodeCalled](const int& nodeId) {
        addNodeCalled = true;
    });

    router.dispatchTyped(Commands::Backend::EvaluateGraph, nullptr);
    router.dispatchTyped(Commands::Backend::AddNode, 1);

    EXPECT_TRUE(evalCalled);
    EXPECT_TRUE(addNodeCalled);

    EXPECT_EQ(router.getExpectedType(Commands::Backend::EvaluateGraph), std::type_index(typeid(std::nullptr_t)));
    EXPECT_EQ(router.getExpectedType(Commands::Backend::AddNode), std::type_index(typeid(int)));
}