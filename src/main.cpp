#include "ui/App.h"

#include <iostream>

int main() {
    try {
        // lms.db and the submissions folder are created next to where the program runs.
        lms::App app("lms.db", "submissions", std::cin, std::cout);
        app.run();
    } catch (const lms::InputClosed&) {
        return 0;  // the keyboard input ended, so just close
    } catch (const std::exception& e) {
        // Only reached if the program cannot start at all, e.g. lms.db can't be opened.
        std::cerr << "The program could not start: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
