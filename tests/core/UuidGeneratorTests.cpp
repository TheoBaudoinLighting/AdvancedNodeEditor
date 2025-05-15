#include <gtest/gtest.h>
#include "../../AdvancedNodeEditor/Utils/UuidGenerator.h"

using namespace NodeEditorCore;

TEST(UuidGeneratorTests, GenerateV4) {
    auto& generator = UuidGenerator::getInstance();
    
    Uuid uuid1 = generator.generateV4();
    Uuid uuid2 = generator.generateV4();
    
    EXPECT_EQ(uuid1.getVersion(), 4);
    EXPECT_EQ(uuid2.getVersion(), 4);
    EXPECT_NE(uuid1, uuid2);
    EXPECT_FALSE(uuid1.isNil());
    EXPECT_FALSE(uuid2.isNil());
}

TEST(UuidGeneratorTests, GenerateV1) {
    auto& generator = UuidGenerator::getInstance();
    
    Uuid uuid1 = generator.generateV1();
    Uuid uuid2 = generator.generateV1();
    
    EXPECT_EQ(uuid1.getVersion(), 1);
    EXPECT_EQ(uuid2.getVersion(), 1);
    EXPECT_NE(uuid1, uuid2);
    EXPECT_FALSE(uuid1.isNil());
    EXPECT_FALSE(uuid2.isNil());
}

TEST(UuidGeneratorTests, UuidToString) {
    auto& generator = UuidGenerator::getInstance();
    
    Uuid uuid = generator.generateV4();
    std::string uuidStr = uuid.toString();
    
    EXPECT_EQ(uuidStr.length(), 36);
    EXPECT_EQ(uuidStr[8], '-');
    EXPECT_EQ(uuidStr[13], '-');
    EXPECT_EQ(uuidStr[18], '-');
    EXPECT_EQ(uuidStr[23], '-');
    
    Uuid parsedUuid(uuidStr);
    EXPECT_EQ(uuid, parsedUuid);
}

TEST(UuidGeneratorTests, UuidComparison) {
    auto& generator = UuidGenerator::getInstance();
    
    Uuid uuid1 = generator.generateV4();
    Uuid uuid2 = generator.generateV4();
    Uuid uuid3(uuid1.toString());
    
    EXPECT_NE(uuid1, uuid2);
    EXPECT_EQ(uuid1, uuid3);
    EXPECT_TRUE(uuid1 == uuid3);
    EXPECT_TRUE(uuid1 != uuid2);
}

TEST(UuidGeneratorTests, NilUuid) {
    Uuid nil;
    EXPECT_TRUE(nil.isNil());
    EXPECT_FALSE(static_cast<bool>(nil));
    
    auto& generator = UuidGenerator::getInstance();
    Uuid uuid = generator.generateV4();
    EXPECT_FALSE(uuid.isNil());
    EXPECT_TRUE(static_cast<bool>(uuid));
}