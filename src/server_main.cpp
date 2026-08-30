#include "server.h"
#include "kv_store.h"

int main() {
    KVStore store("server_test.wal");
    Server server(6380, store);
    server.run();
    return 0;
}