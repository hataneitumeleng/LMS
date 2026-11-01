#include "ui/Menu.h"

#include "data/DatabaseError.h"
#include "domain/Errors.h"

#include <filesystem>
#include <stdexcept>

namespace lms {

Menu::Menu(Console& io, Services& services) : io_(io), services_(services) {}

void Menu::run() {
    while (true) {
        const std::vector<std::string> list = options();
        if (list.size() > static_cast<std::size_t>(kMaxOptions)) {
            throw std::logic_error("A menu may have at most 9 options");
        }
        io_.heading(title());
        for (std::size_t i = 0; i < list.size(); ++i) {
            io_.print(std::to_string(i + 1) + ". " + list[i]);
        }
        io_.print("0. " + exitLabel());
        const int choice = io_.choice("Choose", 0, static_cast<int>(list.size()));
        if (choice == 0) return;

        try {
            handle(choice);
        } catch (const InputClosed&) {
            throw;  // the keyboard input has ended, so leave every menu
        } catch (const ValidationError& e) {
            io_.error(e.what());
        } catch (const AuthError& e) {
            io_.error(std::string("Not allowed: ") + e.what());
        } catch (const DatabaseError& e) {
            io_.error(std::string("The database refused this: ") + e.what());
        } catch (const std::exception& e) {
            io_.error(std::string("Something went wrong: ") + e.what());
        }
    }
}

void Menu::changePassword(const User& user) {
    const std::string oldPassword = io_.text("Current password");
    const std::string newPassword = io_.text("New password (at least 8 characters)");
    if (io_.text("Type the new password again") != newPassword) {
        io_.error("The two new passwords are different. Nothing was changed");
        return;
    }
    services_.auth.changePassword(user.id(), oldPassword, newPassword);
    io_.print("Password changed.");
}

void Menu::showAudit(const std::vector<AuditEntry>& entries) {
    if (entries.empty()) {
        io_.print("No mark changes yet.");
        return;
    }
    for (const AuditEntry& e : entries) {
        io_.print(toText(e.changedAt) + "  user " + std::to_string(e.userId) + "  submission " +
                  std::to_string(e.submissionId) + "  " + formatMark(e.oldValue) + " -> " +
                  formatMark(e.newValue) + "  \"" + e.reason + "\"");
    }
}

void Menu::showFile(const SubmissionVersion& version) {
    io_.print("Version " + std::to_string(version.versionNo) + ", submitted " +
              toText(version.submittedAt));
    io_.print("File: " + std::filesystem::absolute(version.filePath).string());
}

}  // namespace lms
