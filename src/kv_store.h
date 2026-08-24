#pragma once

#include <string>
#include <unordered_map>
#include <optional>
#include <mutex>
#include "wal.h"

class KVStore {
public:
    explicit KVStore(const std::string& walPath);
    void put(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key);
    bool remove(const std::string& key);
private:
    std::unordered_map<std::string, std::string> store_;
    mutable std::mutex mutex_;
    WriteAheadLog wal_;
};