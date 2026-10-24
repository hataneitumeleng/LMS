#pragma once
#include "domain/AuditEntry.h"
#include "domain/User.h"
#include "ui/Console.h"
#include "ui/Services.h"

#include <string>
#include <vector>

namespace lms {

// Base class of every menu (Template Method pattern). run() is the same for all
// menus: show the options, read a choice, call handle(), and catch any error so it
// is shown as a message instead of stopping the program (NFR-07).
// Each subclass only says what its options are and what each one does.
class Menu {
public:
    static constexpr int kMaxOptions = 9;  // NFR-06

    Menu(Console& io, Services& services);
    virtual ~Menu() = default;

    void run();

protected:
    virtual std::string title() const = 0;
    virtual std::vector<std::string> options() const = 0;
    virtual void handle(int choice) = 0;
    virtual std::string exitLabel() const { return "Log out"; }

    // Shared screens used by more than one menu.
    void changePassword(const User& user);  // FR-19
    void showAudit(const std::vector<AuditEntry>& entries);

    Console& io_;
    Services& services_;
};

}  // namespace lms
