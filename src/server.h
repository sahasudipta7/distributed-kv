#pragma once

#include <string>
#include "kv_store.h"

class Server {
public:
    Server(int port, KVStore& store);
    void run();  // blocks forever, accepting and handling one client at a time

private:
    int port_;
    KVStore& store_;
};