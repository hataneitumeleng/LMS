#include "services/BackupService.h"

#include "domain/Errors.h"

#include <filesystem>

namespace fs = std::filesystem;

namespace lms {

BackupService::BackupService(Database& db, std::string submissionsFolder)
    : db_(db), submissionsFolder_(std::move(submissionsFolder)) {}

void BackupService::backup(const std::string& folder) {
    if (fs::exists(folder)) {
        throw ValidationError("'" + folder + "' already exists. Choose a new folder name");
    }
    fs::create_directories(folder);
    try {
        // VACUUM INTO writes a complete, consistent copy of the open database to a
        // new file. The path is bound with ? like any other value.
        Statement stmt = db_.prepare("VACUUM INTO ?");
        stmt.bind(1, (fs::path(folder) / "lms.db").string());
        stmt.step();
        // The database only stores file paths, so the files must be copied too.
        if (fs::exists(submissionsFolder_)) {
            fs::copy(submissionsFolder_, fs::path(folder) / "submissions",
                     fs::copy_options::recursive);
        }
    } catch (...) {
        std::error_code ignored;
        fs::remove_all(folder, ignored);  // never leave half a backup behind
        throw;
    }
}

}  // namespace lms
