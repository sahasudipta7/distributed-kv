#include <gtest/gtest.h>
#include "../src/kv_store.h"
#include "../src/command.h"
#include <thread>
#include <vector>

// --- KVStore tests ---

TEST(KVStoreTest, PutAndGet) {
    KVStore store;
    store.put("name", "Sudipta");
    ASSERT_EQ(store.get("name").value(), "Sudipta");
}

TEST(KVStoreTest, UpdateExistingKey) {
    KVStore store;
    store.put("name", "Sudipta");
    store.put("name", "Rahul");
    ASSERT_EQ(store.get("name").value(), "Rahul");
}

TEST(KVStoreTest, GetMissingKeyReturnsNullopt) {
    KVStore store;
    ASSERT_FALSE(store.get("missing").has_value());
}

TEST(KVStoreTest, DeleteExistingKey) {
    KVStore store;
    store.put("name", "Sudipta");
    ASSERT_TRUE(store.remove("name"));
    ASSERT_FALSE(store.get("name").has_value());
}

TEST(KVStoreTest, DeleteMissingKeyReturnsFalse) {
    KVStore store;
    ASSERT_FALSE(store.remove("missing"));
}

TEST(KVStoreTest, EmptyValueIsStored) {
    KVStore store;
    store.put("key", "");
    ASSERT_TRUE(store.get("key").has_value());
    ASSERT_EQ(store.get("key").value(), "");
}

// --- Command parser tests ---

TEST(CommandParserTest, ParsesSet) {
    Command cmd = parseCommand("SET name Sudipta");
    ASSERT_EQ(cmd.type, CommandType::SET);
    ASSERT_EQ(cmd.key, "name");
    ASSERT_EQ(cmd.value, "Sudipta");
}

TEST(CommandParserTest, ParsesGet) {
    Command cmd = parseCommand("GET name");
    ASSERT_EQ(cmd.type, CommandType::GET);
    ASSERT_EQ(cmd.key, "name");
}

TEST(CommandParserTest, ParsesDelete) {
    Command cmd = parseCommand("DELETE name");
    ASSERT_EQ(cmd.type, CommandType::DELETE);
    ASSERT_EQ(cmd.key, "name");
}

TEST(CommandParserTest, UnknownVerbIsUnknown) {
    Command cmd = parseCommand("BLAH foo bar");
    ASSERT_EQ(cmd.type, CommandType::UNKNOWN);
}

TEST(CommandParserTest, SetWithNoKeyIsInvalid) {
    Command cmd = parseCommand("SET");
    ASSERT_EQ(cmd.type, CommandType::INVALID);
}

TEST(CommandParserTest, GetWithNoKeyIsInvalid) {
    Command cmd = parseCommand("GET");
    ASSERT_EQ(cmd.type, CommandType::INVALID);
}

TEST(KVStoreTest, ConcurrentWritesAndReadsDontCrash) {
    KVStore store;
    const int numThreads = 8;
    const int opsPerThread = 1000;

    std::vector<std::thread> threads;

    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&store, t, opsPerThread]() {
            for (int i = 0; i < opsPerThread; ++i) {
                std::string key = "key" + std::to_string(t);
                store.put(key, std::to_string(i));
                auto val = store.get(key);
                store.remove(key);
            }
        });
    }

    for (auto& th : threads) {
        th.join();
    }

    // If we get here without crashing/hanging, synchronization is working.
    SUCCEED();
}