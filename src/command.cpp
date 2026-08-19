#include "command.h"
#include <sstream>

Command parseCommand(const std::string& line) {
    std::istringstream iss(line);
    std::string verb;
    iss >> verb;

    Command cmd;

    if (verb == "SET") {
        cmd.type = CommandType::SET;
        iss >> cmd.key;
        std::getline(iss, cmd.value);
        // trim leading space left by getline after the key
        if (!cmd.value.empty() && cmd.value[0] == ' ') {
            cmd.value.erase(0, 1);
        }
    } else if (verb == "GET") {
        cmd.type = CommandType::GET;
        iss >> cmd.key;
    } else if (verb == "DELETE") {
        cmd.type = CommandType::DELETE;
        iss >> cmd.key;
    } else {
        cmd.type = CommandType::UNKNOWN;
    }

    return cmd;
}