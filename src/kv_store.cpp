#include "kv_store.h"

KVStore::KVStore(const std::string& walPath)
    : wal_(walPath) {
    // Rebuild in-memory state from the WAL before accepting new commands.
    wal_.replay(
        [this](const std::string& key, const std::string& value) {
            store_[key] = value;
        },
        [this](const std::string& key) {
            store_.erase(key);
        }
    );
}

void KVStore::put(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    wal_.appendSet(key, value);
    store_[key] = value;
}

std::optional<std::string> KVStore::get(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = store_.find(key);
    if (it == store_.end()) {
        return std::nullopt;
    }
    return it->second;
}

bool KVStore::remove(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    bool existed = store_.erase(key) > 0;
    if (existed) {
        wal_.appendDelete(key);
    }
    return existed;
}