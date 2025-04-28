#include <gtest/gtest.h>
#include <storage/bptStorage.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// Helper function to execute commands from input file
void executeCommands(BPTStorage<int, int>& storage, const std::string& inputFile, const std::string& outputFile) {
    std::ifstream input(inputFile);
    std::ifstream expectedOutput(outputFile);
    std::string command;
    int key, value;

    std::ostringstream actualOutput;

    while (input >> command) {
        if (command == "insert") {
            input >> key >> value;
            storage.insert(key, value);
        } else if (command == "find") {
            input >> key;
            auto result = storage.find(key);
            for (const auto& val : result) {
                actualOutput << val << " ";
            }
            actualOutput << "\n";
        } else if (command == "delete") {
            input >> key >> value;
            storage.remove(key, value);
        }
    }

    std::ostringstream expected;
    expected << expectedOutput.rdbuf();

    ASSERT_EQ(actualOutput.str(), expected.str());
}

// Test case for 1.in and 1.out
TEST(StorageTest, TestCase1) {
    BPTStorage<int, int> storage("test1.db");
    executeCommands(storage, "test/storage/1.in", "test/storage/1.out");
    std::remove("test1.db");
}

TEST(StorageTest, TestCase2) {
    BPTStorage<int, int> storage("test2.db");
    executeCommands(storage, "test/storage/2.in", "test/storage/2.out");
    std::remove("test2.db");
}

TEST(StorageTest, TestCase3) {
    BPTStorage<int, int> storage("test3.db");
    executeCommands(storage, "test/storage/3.in", "test/storage/3.out");
    std::remove("test3.db");
}

TEST(StorageTest, TestCase4) {
    BPTStorage<int, int> storage("test4.db");
    executeCommands(storage, "test/storage/4.in", "test/storage/4.out");
    std::remove("test4.db");
}

TEST(StorageTest, TestCase5) {
    BPTStorage<int, int> storage("test5.db");
    executeCommands(storage, "test/storage/5.in", "test/storage/5.out");
    std::remove("test5.db");
}

TEST(StorageTest, TestCase6) {
    BPTStorage<int, int> storage("test6.db");
    executeCommands(storage, "test/storage/6.in", "test/storage/6.out");
    std::remove("test6.db");
}
