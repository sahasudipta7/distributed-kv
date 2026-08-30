#pragma once

#include <string>
#include <mutex>
#include <functional>

class WriteAheadLog {
public:
    explicit WriteAheadLog(const std::string& filePath);
    ~WriteAheadLog();

    void appendSet(const std::string& key, const std::string& value);
    void appendDelete(const std::string& key);

    void replay(
        const std::function<void(const std::string& key, const std::string& value)>& onSet,
        const std::function<void(const std::string& key)>& onDelete
    );


private:
    void appendLine(const std::string& line);

    std::string filePath_;
    int fd_;   // raw file descriptor, not an ofstream
    std::mutex mutex_;
};