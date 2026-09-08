#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "storage/storage.hpp"

// basic SET then GET, should get back what was put in
TEST(StorageTest, SetThenGetReturnsValue) {
    KeyValueStore store;
    store.set("username", "alice");

    auto value = store.get("username");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, "alice");
}

// key was never SET, should come back empty and not crash
TEST(StorageTest, GetOnMissingKeyReturnsNullopt) {
    KeyValueStore store;
    EXPECT_FALSE(store.get("doesnt_exist").has_value());
}

// calling SET the same key twice should just overwrite it
TEST(StorageTest, SetOverwritesExistingKey) {
    KeyValueStore store;
    store.set("username", "alice");
    store.set("username", "bob");

    auto value = store.get("username");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, "bob");
}

// REMOVE should actually take the key out
TEST(StorageTest, RemoveDeletesKey) {
    KeyValueStore store;
    store.set("username", "alice");

    EXPECT_TRUE(store.remove("username"));
    EXPECT_FALSE(store.get("username").has_value());
}

// removing something that was never there should just return false
TEST(StorageTest, RemoveOnMissingKeyReturnsFalse) {
    KeyValueStore store;
    EXPECT_FALSE(store.remove("nonexistent"));
}

// EXISTS should track SET / REMOVE correctly
TEST(StorageTest, ExistsReflectsCurrentState) {
    KeyValueStore store;
    EXPECT_FALSE(store.exists("username"));

    store.set("username", "alice");
    EXPECT_TRUE(store.exists("username"));

    store.remove("username");
    EXPECT_FALSE(store.exists("username"));
}

// modify the store from a bunch of threads at once and make sure nothing breaks / gets corrupted
TEST(StorageTest, ConcurrentSetAndGetDontCorruptState) {
    KeyValueStore store;
    constexpr int num_threads = 8;
    constexpr int ops_per_thread = 200;

    std::vector<std::thread> threads;

    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&store, t] {
            std::string key = "key" + std::to_string(t);

            for (int i = 0; i < ops_per_thread; ++i) {
                store.set(key, std::to_string(i));
                store.get(key); // just checking this doesnt crash
            }
        });
    }

    for (auto& th : threads) th.join();

    // each key should end up with the last value its thread wrote
    for (int t = 0; t < num_threads; ++t) {
        auto value = store.get("key" + std::to_string(t));
        ASSERT_TRUE(value.has_value());
        EXPECT_EQ(*value, std::to_string(ops_per_thread - 1));
    }
}