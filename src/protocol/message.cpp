#include "message.hpp"

#include <charconv> 
#include <utility>

Message::Parser::Result Message::Parser::parse(const std::string& request) {
    std::vector<std::string_view> args;
    size_t cursor = 0;

    // result options
	auto incomplete = [] {
        return Result{
            .status = Status::Incomplete,
            .request = std::nullopt
        };
    };

    auto invalid = [] {
        return Result{
            .status = Status::Invalid,
            .request = std::nullopt
        };
    };

    auto complete = [](Command::Request request) {
        return Result{
            .status = Status::Complete,
            .request = std::move(request)
        };
    };

    // get next line after first ending in \r\n
    auto nextLine = [&]() -> std::optional<std::string_view> {
	    if (cursor >= request.size())
	        return std::nullopt;

	    size_t end = request.find("\r\n", cursor);

	    if (end == std::string_view::npos)
	        return std::nullopt;

	    std::string_view line =
	        std::string_view(request).substr(cursor, end - cursor);

	    cursor = end + 2;

	    return line;
	};

    // header tells us how many args there are to follow in the RESP message
    auto header = nextLine();

    if (!header) 
    	return incomplete();
    
    if (header->empty() || (*header)[0] != '*') 
    	return invalid();

    // convert numeric value in header from char to int
    int elementCount = 0;
    std::string_view strCount = header->substr(1);
    
    auto [ptr, ec] = std::from_chars(
	    strCount.data(),
	    strCount.data() + strCount.size(),
	    elementCount
	);

	if (ec != std::errc{} || ptr != strCount.data() + strCount.size()) {
	    return invalid();
	}

    if (elementCount <= 0)
    	return invalid();

    std::string_view cmd;

    // read each remaining length-data pair in the request
    for (int i = 0; i < elementCount; ++i) {
        // length line, i.e. $3 for argument of 3 char length
        auto length = nextLine();

        if (!length)
        	return incomplete();

        if (length->empty() || (*length)[0] != '$')  
            return invalid();

        // validate length is equal to data's actual length
        std::string_view lengthStr = length->substr(1);
        size_t expectedLength = 0;

        auto [ptr, ec] = std::from_chars(
            lengthStr.data(),
            lengthStr.data() + lengthStr.size(),
            expectedLength
        );

        if (ec != std::errc{} || ptr != lengthStr.data() + lengthStr.size()) {
            return invalid();
        }

        // data line, i.e. GET / SET for $3 in length line, $6 for EXISTS
        auto data = nextLine();

        if (!data) 
        	return incomplete();
        
        if (expectedLength != data->size()) {
		    return invalid();
		}

        // first line is command name, the rest are raw arguments
        if (i == 0) cmd = *data;
        else        args.push_back(*data);
    }

    if (cmd == "GET") {
        // missing key in request
        if (args.size() != 1) {
            reportError(Utils::Error::MissingKey);
            return invalid();
        }

        return complete(Command::Request{ .type=Utils::CommandType::GET, .args=args, .bytes_consumed=cursor });
    }

    else if (cmd == "SET") {
        // expecting key + value in request
        if (args.size() != 2) {
            reportError(Utils::Error::MissingKeyValue);
            return invalid();
        }

        return complete(Command::Request{ .type=Utils::CommandType::SET, .args=args, .bytes_consumed=cursor });
    }

    else if (cmd == "REMOVE") {
        if (args.size() != 1) {
            reportError(Utils::Error::MissingKey);
            return invalid();
        }

        return complete(Command::Request{ .type=Utils::CommandType::REMOVE, .args=args, .bytes_consumed=cursor });
    }

    else if (cmd == "EXISTS") {
        if (args.size() != 1) {
            reportError(Utils::Error::MissingKey);
            return invalid();
        }

        return complete(Command::Request{ .type=Utils::CommandType::EXISTS, .args=args, .bytes_consumed=cursor });
    }

    return invalid();
}

Command::Result Message::Executor::execute(const Command::Request& cmd) {
    switch (cmd.type) {
        case Utils::CommandType::GET: {
            auto value = m_store.get(cmd.args.front());
            if (!value) return { .status=Utils::Response::NotFound, .value=std::nullopt };

            return { .status=Utils::Response::OK, .value=std::string(*value) };
        }

        case Utils::CommandType::REMOVE: {
            auto value = m_store.remove(cmd.args.front());
            if (!value) return { .status=Utils::Response::NotFound, .value=std::nullopt };

            return { .status=Utils::Response::OK, .value="true" };
        }

        case Utils::CommandType::EXISTS: {
            auto value = m_store.exists(cmd.args.front());

            return { .status=Utils::Response::OK, .value=value ? "true" : "false" };
        }

        // set takes two elements, key + value
        case Utils::CommandType::SET: {
            if (cmd.args.size() != 2)
		        return {
		            .status = Utils::Response::BadRequest,
		            .value = std::nullopt
		        };

		    m_store.set(cmd.args[0], cmd.args[1]);

            return { .status=Utils::Response::OK, .value=std::string(cmd.args.back()) };
        }
    }

    return { .status=Utils::Response::BadRequest, .value="[executor] command type not found"};
}

void Message::Parser::reportError(Utils::Error err) {
    std::string header{ "[parser] error: "};

    switch(err) {
        case Utils::Error::MissingKey:
            std::cerr <<  header << "missing key\n";
            break;
        case Utils::Error::MissingKeyValue:
            std::cerr << header << "missing key/value pair\n";
            break;
    }
}