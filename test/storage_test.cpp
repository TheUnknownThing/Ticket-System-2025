#include <gtest/gtest.h>
#include <storage/bptStorage.hpp>
#include <string>
#include <vector>

class StorageTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup test environment
        storage = new BPTStorage<std::string, int>("test.db");
    }

    void TearDown() override {
        delete storage;
        // Clean up test files
        std::remove("test.db");
    }

    BPTStorage<std::string, int>* storage;
};

// Test basic insertion
TEST_F(StorageTest, InsertTest) {
    ASSERT_TRUE(storage->insert("key1", 100));
    ASSERT_TRUE(storage->insert("key1", 200));
    ASSERT_TRUE(storage->insert("key2", 300));
    
    // Test duplicate value for same key
    ASSERT_FALSE(storage->insert("key1", 100));
}

// Test basic deletion
TEST_F(StorageTest, DeleteTest) {
    ASSERT_TRUE(storage->insert("key1", 100));
    ASSERT_TRUE(storage->insert("key1", 200));
    
    ASSERT_TRUE(storage->remove("key1", 100));
    ASSERT_FALSE(storage->remove("key1", 100)); // Already deleted
    ASSERT_FALSE(storage->remove("key2", 300)); // Non-existent key
}

// Test finding values
TEST_F(StorageTest, FindTest) {
    // Insert values in random order
    ASSERT_TRUE(storage->insert("key1", 300));
    ASSERT_TRUE(storage->insert("key1", 100));
    ASSERT_TRUE(storage->insert("key1", 200));
    ASSERT_TRUE(storage->insert("key2", 400));

    // Test sorted retrieval
    int* result1 = storage->find("key1");
    ASSERT_NE(result1, nullptr);
    EXPECT_EQ(result1[0], 100);
    EXPECT_EQ(result1[1], 200);
    EXPECT_EQ(result1[2], 300);

    // Test non-existent key
    int* result2 = storage->find("key3");
    ASSERT_EQ(result2, nullptr);
}

// Test complex operations
TEST_F(StorageTest, ComplexOperationsTest) {
    // Insert multiple values
    ASSERT_TRUE(storage->insert("test", 500));
    ASSERT_TRUE(storage->insert("test", 300));
    ASSERT_TRUE(storage->insert("test", 400));
    
    // Delete middle value
    ASSERT_TRUE(storage->remove("test", 400));
    
    // Check remaining values are still sorted
    int* result = storage->find("test");
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result[0], 300);
    EXPECT_EQ(result[1], 500);
}

// Test string key length limits
TEST_F(StorageTest, KeyLengthTest) {
    std::string longKey(65, 'a'); // 65 characters
    ASSERT_FALSE(storage->insert(longKey, 100));
    
    std::string validKey(64, 'a'); // 64 characters
    ASSERT_TRUE(storage->insert(validKey, 100));
}

// Test whitespace in keys
TEST_F(StorageTest, KeyWhitespaceTest) {
    ASSERT_FALSE(storage->insert("invalid key", 100));
    ASSERT_FALSE(storage->insert("invalid\tkey", 100));
    ASSERT_FALSE(storage->insert("invalid\nkey", 100));
}
