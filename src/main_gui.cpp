#include "app/System.h"
#include "gui/GuiApp.h"
#include "gui/GuiCommon.h"

#include <QApplication>

#include <exception>

// The desktop program, lms_gui. It uses the same lms.db, submissions folder and services
// as the console program lms, so both can be used on the same data.
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("Moderated LMS");
    QApplication::setQuitOnLastWindowClosed(false);  // GuiApp decides when to stop
    try {
        lms::System system("lms.db", "submissions");
        lms::GuiApp(system.services()).run();
    } catch (const std::exception& e) {
        // Only reached if the program cannot start at all, e.g. lms.db can't be opened.
        lms::showError(nullptr, "The program could not start: " + lms::qs(e.what()));
        return 1;
    }
    return 0;
}
