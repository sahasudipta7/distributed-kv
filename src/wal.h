#pragma once

#include <string>
#include <fstream>
#include <mutex>

class WriteAheadLog {
public:
    explicit WriteAheadLog(const std::string& filePath);
    ~WriteAheadLog();

    void appendSet(const std::string& key, const std::string& value);
    void appendDelete(const std::string& key);

private:
    void appendLine(const std::string& line);

    std::string filePath_;
    std::ofstream file_;
    std::mutex mutex_;
};