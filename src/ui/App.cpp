#include "ui/App.h"

#include "domain/Errors.h"
#include "ui/LoginMenu.h"

namespace lms {

App::App(const std::string& databasePath, const std::string& submissionsFolder,
         std::istream& in, std::ostream& out)
    : io_(in, out), system_(databasePath, submissionsFolder), services_(system_.services()) {}

void App::run() {
    // Week 3, Section 4.3: delete files left behind if the program stopped mid-save.
    const int removed = services_.submissions.removeOrphanFiles();
    if (removed > 0) io_.print("Removed " + std::to_string(removed) + " unfinished upload(s).");

    if (services_.admin.listUsers().empty()) createFirstAdministrator();
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
            services_.admin.createUser(Role::Administrator, username, fullName, password);
            io_.print("Administrator created. Please log in.");
            return;
        } catch (const ValidationError& e) {
            io_.error(e.what());
        }
    }
}

}  // namespace lms
