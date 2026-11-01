#pragma once
#include "app/System.h"
#include "ui/Console.h"

#include <iosfwd>
#include <string>

namespace lms {

// The console front end: builds the System, then shows the login menu.
class App {
public:
    App(const std::string& databasePath, const std::string& submissionsFolder, std::istream& in,
        std::ostream& out);

    void run();

private:
    void createFirstAdministrator();

    Console io_;
    System system_;
    Services& services_;
};

}  // namespace lms
