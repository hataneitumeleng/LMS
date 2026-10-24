#pragma once
#include "data/Database.h"

#include <string>

namespace lms {

// Administrator backup (FR-20, NFR-11).
class BackupService {
public:
    BackupService(Database& db, std::string submissionsFolder);

    // Creates <folder>/lms.db (a full copy of the database) and copies the submitted
    // files into <folder>/submissions. The folder must not exist yet.
    void backup(const std::string& folder);

private:
    Database& db_;
    std::string submissionsFolder_;
};

}  // namespace lms
