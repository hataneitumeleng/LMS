#include "services/FileStore.h"

#include "domain/Errors.h"

#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

namespace lms {

FileStore::FileStore(std::string folder) : folder_(std::move(folder)) {
    fs::create_directories(folder_);
}

void FileStore::validate(const std::string& path) const {
    std::error_code error;
    if (!fs::is_regular_file(path, error)) {
        throw ValidationError("File not found: " + path);
    }
    if (fs::file_size(path, error) == 0 || error) {
        throw ValidationError("The file is empty: " + path);
    }
}

std::string FileStore::copyIn(const std::string& path, int submissionId, int versionNo) const {
    const fs::path target = fs::path(folder_) / ("sub" + std::to_string(submissionId) + "_v" +
                                                 std::to_string(versionNo) +
                                                 fs::path(path).extension().string());
    // Throws std::filesystem::filesystem_error if the copy fails (e.g. the disk is full).
    fs::copy_file(path, target, fs::copy_options::overwrite_existing);
    return target.generic_string();
}

void FileStore::remove(const std::string& storedPath) const noexcept {
    std::error_code ignored;
    fs::remove(storedPath, ignored);
}

int FileStore::removeOrphans(const std::vector<std::string>& knownPaths) const {
    int removed = 0;
    for (const auto& entry : fs::directory_iterator(folder_)) {
        const std::string path = entry.path().generic_string();
        if (std::find(knownPaths.begin(), knownPaths.end(), path) == knownPaths.end()) {
            remove(path);
            ++removed;
        }
    }
    return removed;
}

}  // namespace lms
