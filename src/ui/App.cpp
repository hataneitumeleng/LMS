#include "ui/App.h"

#include "domain/Errors.h"
#include "ui/LoginMenu.h"

namespace lms {

App::App(const std::string& databasePath, const std::string& submissionsFolder,
         std::istream& in, std::ostream& out)
    : io_(in, out),
      db_(databasePath),
      store_(submissionsFolder),
      users_(db_, factory_),
      courses_(db_),
      assignmentRepo_(db_),
      submissionRepo_(db_),
      auditRepo_(db_),
      auth_(users_, hasher_, clock_),
      admin_(db_, users_, courses_, factory_, hasher_),
      assignments_(db_, assignmentRepo_, courses_, users_, submissionRepo_, policy_, clock_),
      submissions_(db_, submissionRepo_, assignmentRepo_, courses_, policy_, clock_, store_),
      audit_(auditRepo_, clock_),
      marking_(db_, submissionRepo_, assignmentRepo_, courses_, policy_, audit_),
      moderation_(db_, submissionRepo_, assignmentRepo_, courses_, users_, audit_, clock_),
      backup_(db_, submissionsFolder),
      services_{auth_, admin_, assignments_, submissions_, marking_, moderation_, audit_, backup_} {
    db_.initialiseSchema();
}

void App::run() {
    // Week 3, Section 4.3: delete files left behind if the program stopped mid-save.
    const int removed = submissions_.removeOrphanFiles();
    if (removed > 0) io_.print("Removed " + std::to_string(removed) + " unfinished upload(s).");

    if (admin_.listUsers().empty()) createFirstAdministrator();
    LoginMenu(io_, services_).run();
    io_.print("Goodbye.");
}

void App::createFirstAdministrator() {
    io_.heading("First start: create the administrator account");
    while (true) {
        try {
            const std::string username = io_.text("Username");
            const std::string fullName = io_.text("Full name");
            const std::string password = io_.text("Password (at least 8 characters)");
            admin_.createUser(Role::Administrator, username, fullName, password);
            io_.print("Administrator created. Please log in.");
            return;
        } catch (const ValidationError& e) {
            io_.error(e.what());
        }
    }
}

}  // namespace lms
