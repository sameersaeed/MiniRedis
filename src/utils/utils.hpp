#pragma once

#include <array>

namespace Utils {
    enum class CommandType {
        SET,
        GET,
        EXISTS,
        REMOVE
    };

    enum class Response {
        OK,
        BadRequest,
        NotFound
    };

    enum class Error {
        MissingKey,
        MissingKeyValue
    };
}