#include "wal.h"
#include <stdexcept>
#include <sstream>
#include <fstream>

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

void WriteAheadLog::replay(
    const std::function<void(const std::string&, const std::string&)>& onSet,
    const std::function<void(const std::string&)>& onDelete
) {
    std::ifstream in(filePath_);
    if (!in.is_open()) {
        return;  // no existing WAL file yet — nothing to replay
    }

    std::string line;
    while (std::getline(in, line)) {
        std::istringstream iss(line);
        std::string verb, key;
        iss >> verb >> key;

        if (verb == "SET") {
            std::string value;
            std::getline(iss, value);
            if (!value.empty() && value[0] == ' ') {
                value.erase(0, 1);
            }
            if (!key.empty()) {
                onSet(key, value);
            }
        } else if (verb == "DELETE") {
            if (!key.empty()) {
                onDelete(key);
            }
        }
        // Any malformed/partial line (e.g. from a crash mid-write) is silently skipped.
    }
}