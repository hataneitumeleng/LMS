#include "gui/GuiApp.h"

#include "gui/AdminWindow.h"
#include "gui/GuiCommon.h"
#include "gui/HodWindow.h"
#include "gui/LecturerWindow.h"
#include "gui/LoginDialog.h"
#include "gui/StudentWindow.h"

#include <QEventLoop>

namespace lms {

GuiApp::GuiApp(Services& services) : services_(services) {}

void GuiApp::run() {
    // Week 3, Section 4.3: delete files left behind if the program stopped mid-save.
    services_.submissions.removeOrphanFiles();

    if (services_.admin.listUsers().empty()) {
        FirstAdminDialog first(services_.admin);
        if (first.exec() != QDialog::Accepted) return;
    }
    while (true) {
        LoginDialog login(services_.auth);
        if (login.exec() != QDialog::Accepted) return;

        std::unique_ptr<RoleWindow> window = windowFor(services_, login.takeUser());
        QEventLoop loop;  // runs until the window is closed
        window->setOnClosed([&loop] { loop.quit(); });
        window->show();
        loop.exec();
        if (!window->loggedOut()) return;
    }
}

std::unique_ptr<RoleWindow> GuiApp::windowFor(Services& services, std::unique_ptr<User> user) {
    switch (user->role()) {
        case Role::Student: return std::make_unique<StudentWindow>(services, std::move(user));
        case Role::Lecturer: return std::make_unique<LecturerWindow>(services, std::move(user));
        case Role::HoD: return std::make_unique<HodWindow>(services, std::move(user));
        case Role::Administrator:
            return std::make_unique<AdminWindow>(services, std::move(user));
    }
    return nullptr;
}

}  // namespace lms
