#include <gtest/gtest.h>

#include "protocol/message.hpp"
#include "storage/storage.hpp"

// set should store the value and say ok
TEST(ExecutorTest, SetStoresValueAndReturnsOk) {
    KeyValueStore store;
    Message::Executor executor{store};

    Command::Request cmd{
        .type = Utils::CommandType::SET,
        .args = {"username", "alice"},
        .bytes_consumed = 0
    };

    auto result = executor.execute(cmd);

    EXPECT_EQ(result.status, Utils::Response::OK);
    EXPECT_TRUE(store.exists("username"));
    EXPECT_EQ(store.get("username"), "alice");
}

// get should return whats already in the store
TEST(ExecutorTest, GetReturnsStoredValue) {
    KeyValueStore store;
    store.set("username", "alice");
    Message::Executor executor{store};

    Command::Request cmd{
        .type = Utils::CommandType::GET,
        .args = {"username"},
        .bytes_consumed = 0
    };

    auto result = executor.execute(cmd);

    EXPECT_EQ(result.status, Utils::Response::OK);
    ASSERT_TRUE(result.value.has_value());
    EXPECT_EQ(*result.value, "alice");
}

// get on a key that was never set should be not found
TEST(ExecutorTest, GetOnMissingKeyReturnsNotFound) {
    KeyValueStore store;
    Message::Executor executor{store};

    Command::Request cmd{
        .type = Utils::CommandType::GET,
        .args = {"nope"},
        .bytes_consumed = 0
    };

    auto result = executor.execute(cmd);
    EXPECT_EQ(result.status, Utils::Response::NotFound);
}

// remove should actually take the key out of the store
TEST(ExecutorTest, RemoveDeletesKeyFromStore) {
    KeyValueStore store;
    store.set("username", "alice");
    Message::Executor executor{store};

    Command::Request cmd{
        .type = Utils::CommandType::REMOVE,
        .args = {"username"},
        .bytes_consumed = 0
    };

    auto result = executor.execute(cmd);

    EXPECT_EQ(result.status, Utils::Response::OK);
    EXPECT_FALSE(store.exists("username"));
}

// removing a key that doesnt exist should just say not found, not crash
TEST(ExecutorTest, RemoveOnMissingKeyReturnsNotFound) {
    KeyValueStore store;
    Message::Executor executor{store};

    Command::Request cmd{
        .type = Utils::CommandType::REMOVE,
        .args = {"nope"},
        .bytes_consumed = 0
    };

    auto result = executor.execute(cmd);
    EXPECT_EQ(result.status, Utils::Response::NotFound);
}

// exists should flip from false to true once the key is set
TEST(ExecutorTest, ExistsReflectsStoreState) {
    KeyValueStore store;
    Message::Executor executor{store};

    Command::Request cmd{
        .type = Utils::CommandType::EXISTS,
        .args = {"username"},
        .bytes_consumed = 0
    };

    auto before = executor.execute(cmd);
    EXPECT_EQ(before.value, "false");

    store.set("username", "alice");
    auto after = executor.execute(cmd);
    EXPECT_EQ(after.value, "true");
}

// set with missing args should get rejected as a bad request
TEST(ExecutorTest, SetWithWrongArgCountReturnsBadRequest) {
    KeyValueStore store;
    Message::Executor executor{store};

    Command::Request cmd{
        .type = Utils::CommandType::SET,
        .args = {"onlyonearg"},
        .bytes_consumed = 0
    };

    auto result = executor.execute(cmd);
    EXPECT_EQ(result.status, Utils::Response::BadRequest);
}