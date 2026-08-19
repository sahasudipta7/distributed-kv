#pragma once

#include <string>

enum class CommandType {
    SET,
    GET,
    DELETE,
    UNKNOWN
};

struct Command {
    CommandType type;
    std::string key;
    std::string value;  // only used for SET
};

Command parseCommand(const std::string& line);