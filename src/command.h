#pragma once

#include <string>

enum class CommandType {
    SET,
    GET,
    DELETE,
    UNKNOWN,
    INVALID
};

struct Command {
    CommandType type;
    std::string key;
    std::string value;  // only used for SET
};

Command parseCommand(const std::string& line);