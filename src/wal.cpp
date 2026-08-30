#include "wal.h"
#include <stdexcept>
#include <sstream>
#include <fstream>

#include <io.h>      // _open, _write, _close, _commit
#include <fcntl.h>   // O_APPEND, O_CREAT, O_WRONLY
#include <sys/stat.h> // S_IREAD, S_IWRITE


WriteAheadLog::WriteAheadLog(const std::string& filePath)
    : filePath_(filePath) {
    // O_APPEND: always write at end of file
    // O_CREAT:  create the file if it doesn't exist
    // O_WRONLY: write-only
    // O_BINARY: don't let Windows silently translate \n <-> \r\n on us
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

#include "wal.h"
#include <stdexcept>
#include <sstream>
#include <fstream>

#include <io.h>      // _open, _write, _close, _commit
#include <fcntl.h>   // O_APPEND, O_CREAT, O_WRONLY
#include <sys/stat.h> // S_IREAD, S_IWRITE

WriteAheadLog::WriteAheadLog(const std::string& filePath)
    : filePath_(filePath) {
    // O_APPEND: always write at end of file
    // O_CREAT:  create the file if it doesn't exist
    // O_WRONLY: write-only
    // O_BINARY: don't let Windows silently translate \n <-> \r\n on us
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

    // Force the OS to flush this out of the page cache and onto physical disk.
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