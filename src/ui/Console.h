#pragma once
#include "domain/TimeFormat.h"

#include <exception>
#include <iosfwd>
#include <optional>
#include <string>

namespace lms {

// Thrown when the keyboard input ends (for example Ctrl+Z on Windows), so the
// program can close instead of asking the same question for ever.
class InputClosed : public std::exception {
public:
    const char* what() const noexcept override { return "Input closed"; }
};

// All reading and writing for the menus. Every read method keeps asking until
// the input is valid, so a typing mistake never stops the program (NFR-07).
class Console {
public:
    Console(std::istream& in, std::ostream& out);

    void print(const std::string& line);
    void heading(const std::string& title);
    void error(const std::string& message);

    std::string text(const std::string& prompt);  // may be empty
    // A file path. Removes the quotes Windows adds with "Copy as path".
    std::string path(const std::string& prompt);
    int number(const std::string& prompt);
    int choice(const std::string& prompt, int min, int max);
    double decimal(const std::string& prompt);
    TimePoint dateTime(const std::string& prompt);  // typed as YYYY-MM-DD HH:MM
    bool confirm(const std::string& question);      // y or n (NFR-06)

private:
    std::istream& in_;
    std::ostream& out_;
};

// A mark with one decimal place, or "-" when there is none yet.
std::string formatMark(const std::optional<double>& mark);

}  // namespace lms
