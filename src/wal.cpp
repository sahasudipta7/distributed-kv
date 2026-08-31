#include "wal.h"
#include <stdexcept>
#include <sstream>
#include <fstream>

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

WriteAheadLog::WriteAheadLog(const std::string& filePath)
    : filePath_(filePath) {
    fd_ = _open(
        filePath_.c_str(),
        O_APPEND | O_CREAT | O_WRONLY | O_BINARY,
        S_IREAD | S_IWRITE
    );
    if (fd_ == -1) {
        throw std::runtime_error("Failed to open WAL file: " + filePath_);
    }
}

WriteAheadLog::~WriteAheadLog() {
    if (fd_ != -1) {
        _close(fd_);
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

    std::string toWrite = line + "\n";

    int written = _write(fd_, toWrite.c_str(), static_cast<unsigned int>(toWrite.size()));
    if (written != static_cast<int>(toWrite.size())) {
        throw std::runtime_error("WAL write failed or was incomplete: " + filePath_);
    }

    if (_commit(fd_) != 0) {
        throw std::runtime_error("WAL commit (fsync) failed: " + filePath_);
    }
}

void WriteAheadLog::replay(
    const std::function<void(const std::string&, const std::string&)>& onSet,
    const std::function<void(const std::string&)>& onDelete
) {
    std::ifstream in(filePath_);
    if (!in.is_open()) {
        return;
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
    }
}

