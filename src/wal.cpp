#include "wal.h"
#include <stdexcept>

WriteAheadLog::WriteAheadLog(const std::string& filePath)
    : filePath_(filePath) {
    // Open in append mode so existing log entries are preserved across restarts.
    file_.open(filePath_, std::ios::app);
    if (!file_.is_open()) {
        throw std::runtime_error("Failed to open WAL file: " + filePath_);
    }
}

WriteAheadLog::~WriteAheadLog() {
    if (file_.is_open()) {
        file_.close();
    }
}

void WriteAheadLog::appendSet(const std::string& key, const std::string& value) {
    appendLine("SET " + key + " " + value);
}

void WriteAheadLog::appendDelete(const std::string& key) {
    appendLine("DELETE " + key);
}

void WriteAheadLog::appendLine(const std::string& line) {
    std::lock_guard<std::mutex> lock(mutex_);
    file_ << line << "\n";
    file_.flush();  // ensure it's pushed to the OS
}