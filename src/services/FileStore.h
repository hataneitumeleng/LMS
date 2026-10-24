#pragma once
#include <string>
#include <vector>

namespace lms {

// Copies submitted files into the program's own folder, so later changes to the
// student's original file don't change what was submitted.
// The database only stores the path of each copy.
class FileStore {
public:
    explicit FileStore(std::string folder);

    // Throws ValidationError if the path is not an existing, non-empty file.
    void validate(const std::string& path) const;
    // Copies the file to "<folder>/sub<id>_v<version><extension>" and returns that path.
    std::string copyIn(const std::string& path, int submissionId, int versionNo) const;
    // Deletes a copy. Never throws, because it is called while handling another error.
    void remove(const std::string& storedPath) const noexcept;
    // Deletes files in the folder that the database doesn't know about, for example
    // after a crash between copying a file and committing (Week 3, Section 4.3).
    // Returns how many were deleted.
    int removeOrphans(const std::vector<std::string>& knownPaths) const;

private:
    std::string folder_;
};

}  // namespace lms
