#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "../utils/utils.hpp"
#include "../storage/storage.hpp"

namespace Command {

    struct Request {
        Utils::CommandType type;
        std::vector<std::string_view> args;
        size_t bytes_consumed;
    };

    struct Result {
        Utils::Response status;
        std::optional<std::string> value;
    };

} // namespace Command

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Message {

class Executor {
public:
    Executor(KeyValueStore& store) : m_store(store) {}

    Command::Result execute(const Command::Request& command);

private:
    KeyValueStore& m_store;
};

class Parser {
public:
    enum class Status {
	    Complete,
	    Incomplete,
	    Invalid
	};

    struct Result {
        Status status;
        std::optional<Command::Request> request;
	};

    Result parse(const std::string& request);

private:
    void reportError(Utils::Error err);
};

} // namespace Message