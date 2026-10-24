#include "ui/LoginMenu.h"

#include "ui/AdminMenu.h"
#include "ui/HodMenu.h"
#include "ui/LecturerMenu.h"
#include "ui/StudentMenu.h"

#include <stdexcept>

namespace lms {

LoginMenu::LoginMenu(Console& io, Services& services) : Menu(io, services) {}

void LoginMenu::handle(int /*choice*/) {
    const std::string username = io_.text("Username");
    const std::string password = io_.text("Password");
    // Throws AuthError on a wrong password or locked account; run() shows the message.
    std::unique_ptr<User> user = services_.auth.login(username, password);
    io_.print("Welcome, " + user->fullName() + " (" + toString(user->role()) + ").");
    menuFor(*user)->run();
    io_.print("Logged out.");
}

std::unique_ptr<Menu> LoginMenu::menuFor(const User& user) {
    // FR-02: each role only ever sees its own menu
    switch (user.role()) {
        case Role::Student: return std::make_unique<StudentMenu>(io_, services_, user);
        case Role::Lecturer: return std::make_unique<LecturerMenu>(io_, services_, user);
        case Role::HoD: return std::make_unique<HodMenu>(io_, services_, user);
        case Role::Administrator: return std::make_unique<AdminMenu>(io_, services_, user);
    }
    throw std::logic_error("Unknown role");
}

}  // namespace lms
