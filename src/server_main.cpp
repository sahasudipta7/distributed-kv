#include "server.h"
#include "kv_store.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: kv_server.exe <port> <wal_file>" << std::endl;
        std::cerr << "Example: kv_server.exe 6380 node_a.wal" << std::endl;
        return 1;
    }

    int port = std::atoi(argv[1]);
    std::string walPath = argv[2];

    if (port <= 0 || port > 65535) {
        std::cerr << "Invalid port: " << argv[1] << std::endl;
        return 1;
    }

    std::cout << "Starting node on port " << port
              << " with WAL file " << walPath << std::endl;

    KVStore store(walPath);
    Server server(port, store);
    server.run();

    return 0;
}