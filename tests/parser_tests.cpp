#include <gtest/gtest.h>

#include "protocol/message.hpp"

// basic SET command, request should succeed
TEST(ParserTest, ParsesSetCommand) {
    Message::Parser parser;
    std::string raw = "*3\r\n$3\r\nSET\r\n$4\r\nname\r\n$3\r\nbob\r\n";

    auto result = parser.parse(raw);

    ASSERT_EQ(result.status, Message::Parser::Status::Complete);
    ASSERT_TRUE(result.request.has_value());
    EXPECT_EQ(result.request->type, Utils::CommandType::SET);
    ASSERT_EQ(result.request->args.size(), 2);
    EXPECT_EQ(result.request->args[0], "name");
    EXPECT_EQ(result.request->args[1], "bob");
    EXPECT_EQ(result.request->bytes_consumed, raw.size());
}

// basic GET command, request should succeed
TEST(ParserTest, ParsesGetCommand) {
    Message::Parser parser;
    std::string raw = "*2\r\n$3\r\nGET\r\n$4\r\nname\r\n";

    auto result = parser.parse(raw);

    ASSERT_EQ(result.status, Message::Parser::Status::Complete);
    EXPECT_EQ(result.request->type, Utils::CommandType::GET);
    ASSERT_EQ(result.request->args.size(), 1);
    EXPECT_EQ(result.request->args[0], "name");
}

// header + command name only, value never arrives therefore incomplete
TEST(ParserTest, IncompleteWhenMissingTrailingCRLF) {
    Message::Parser parser;
    std::string raw = "*3\r\n$3\r\nSET\r\n$4\r\nname\r\n";

    auto result = parser.parse(raw);
    EXPECT_EQ(result.status, Message::Parser::Status::Incomplete);
}

// nothing sent at all, server should just send incomplete response and not crash
TEST(ParserTest, IncompleteWithNoData) {
    Message::Parser parser;
    auto result = parser.parse("");
    EXPECT_EQ(result.status, Message::Parser::Status::Incomplete);
}

// request header is missing the *, should be invalid as its not correctly formatted
TEST(ParserTest, InvalidWhenHeaderMissingAsterisk) {
    Message::Parser parser;
    std::string raw = "3\r\n$3\r\nSET\r\n$4\r\nname\r\n$3\r\nbob\r\n";

    auto result = parser.parse(raw);
    EXPECT_EQ(result.status, Message::Parser::Status::Invalid);
}

// GET request says 999 bytes but "test" is only 4 bytes - shouldn't work
TEST(ParserTest, InvalidWhenDeclaredLengthDoesntMatchData) {
    Message::Parser parser;
    std::string raw = "*2\r\n$3\r\nGET\r\n$999\r\ntest\r\n";

    auto result = parser.parse(raw);
    EXPECT_EQ(result.status, Message::Parser::Status::Invalid);
}

// SET with only a key, no value
TEST(ParserTest, InvalidWhenSetMissingValue) {
    Message::Parser parser;
    std::string raw = "*2\r\n$3\r\nSET\r\n$4\r\nname\r\n";

    auto result = parser.parse(raw);
    EXPECT_EQ(result.status, Message::Parser::Status::Invalid);
}

// GET only takes one arg, sending 2 should get rejected
TEST(ParserTest, InvalidWhenExtraArgsForGet) {
    Message::Parser parser;
    std::string raw = "*3\r\n$3\r\nGET\r\n$4\r\nname\r\n$5\r\nextra\r\n";

    auto result = parser.parse(raw);
    EXPECT_EQ(result.status, Message::Parser::Status::Invalid);
}

// simulates a command that shows up in two pieces instead of all at once
TEST(ParserTest, HandlesCommandSplitAcrossReads) {
    Message::Parser parser;

    // "test" is 4 bytes, split the value itself across two reads
    std::string buffer = "*2\r\n$3\r\nGET\r\n$4\r\nt";
 
    // received first half
    auto first = parser.parse(buffer);
    EXPECT_EQ(first.status, Message::Parser::Status::Incomplete);

    // received second half
    buffer += "est\r\n";
    auto second = parser.parse(buffer);

    // full command received, should parse as complete
    ASSERT_EQ(second.status, Message::Parser::Status::Complete);
    EXPECT_EQ(second.request->type, Utils::CommandType::GET);
    EXPECT_EQ(second.request->args[0], "test");
}