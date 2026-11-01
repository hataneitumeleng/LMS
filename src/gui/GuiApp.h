#pragma once
#include "app/Services.h"
#include "gui/RoleWindow.h"

#include <memory>

namespace lms {

// The Qt front end: first-start check, then login, the role's window, and back to login
// after "Log out". Closing a window or the login box ends the program.
class GuiApp {
public:
    explicit GuiApp(Services& services);

    void run();

    // FR-02: picks the window for the user's role (like LoginMenu::menuFor in the console).
    static std::unique_ptr<RoleWindow> windowFor(Services& services, std::unique_ptr<User> user);

private:
    Services& services_;
};

}  // namespace lms
