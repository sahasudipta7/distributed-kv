#include <iostream>
#include "kv_store.h"
#include "command.h"
#include "wal.h"

int main() {
    KVStore store("kv.wal");   // <-- real WAL file, constructor takes a path now
    std::string line;

    std::cout << "kv> ";
    while (std::getline(std::cin, line)) {
        Command cmd = parseCommand(line);

        switch (cmd.type) {
            case CommandType::SET:
                store.put(cmd.key, cmd.value);
                std::cout << "OK" << std::endl;
                break;
            case CommandType::GET: {
                auto val = store.get(cmd.key);
                std::cout << (val ? *val : "NOT_FOUND") << std::endl;
                break;
            }
            case CommandType::DELETE:
                store.remove(cmd.key);
                std::cout << "OK" << std::endl;
                break;
            case CommandType::UNKNOWN:
                std::cout << "ERROR: unknown command" << std::endl;
                break;
            case CommandType::INVALID:
                std::cout << "ERROR: missing arguments" << std::endl;
                break;
        }

        std::cout << "kv> ";
    }

    return 0;
}