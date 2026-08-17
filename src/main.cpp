#include <iostream>
#include "kv_store.h"

int main() {
    KVStore store;

    store.put("name", "Sudipta");
    store.put("language", "C++");

    auto name = store.get("name");
    std::cout << "name = " << (name ? *name : "NOT_FOUND") << std::endl;

    store.remove("language");
    auto lang = store.get("language");
    std::cout << "language = " << (lang ? *lang : "NOT_FOUND") << std::endl;

    return 0;
}