#include "ui/Console.h"

#include "domain/Errors.h"

#include <iomanip>
#include <iostream>
#include <sstream>

namespace lms {

namespace {
std::string trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r");
    if (first == std::string::npos) return "";
    const auto last = value.find_last_not_of(" \t\r");
    return value.substr(first, last - first + 1);
}
}  // namespace

Console::Console(std::istream& in, std::ostream& out) : in_(in), out_(out) {}

void Console::print(const std::string& line) { out_ << line << "\n"; }

void Console::heading(const std::string& title) {
    out_ << "\n=== " << title << " ===\n";
}

void Console::error(const std::string& message) { out_ << "  ! " << message << "\n"; }

std::string Console::text(const std::string& prompt) {
    out_ << prompt << ": " << std::flush;
    std::string line;
    if (!std::getline(in_, line)) throw InputClosed();
    return trim(line);
}

std::string Console::path(const std::string& prompt) {
    std::string value = text(prompt);
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
        value = value.substr(1, value.size() - 2);
    }
    return value;
}

int Console::number(const std::string& prompt) {
    while (true) {
        const std::string input = text(prompt);
        try {
            std::size_t used = 0;
            const int value = std::stoi(input, &used);
            if (used == input.size()) return value;
        } catch (const std::exception&) {
            // not a number: ask again below
        }
        error("Please type a whole number");
    }
}

int Console::choice(const std::string& prompt, int min, int max) {
    while (true) {
        const int value = number(prompt);
        if (value >= min && value <= max) return value;
        error("Please choose " + std::to_string(min) + " to " + std::to_string(max));
    }
}

double Console::decimal(const std::string& prompt) {
    while (true) {
        const std::string input = text(prompt);
        try {
            std::size_t used = 0;
            const double value = std::stod(input, &used);
            if (used == input.size()) return value;
        } catch (const std::exception&) {
            // not a number: ask again below
        }
        error("Please type a number, for example 65 or 65.5");
    }
}

TimePoint Console::dateTime(const std::string& prompt) {
    while (true) {
        const std::string input = text(prompt + " (YYYY-MM-DD HH:MM)");
        try {
            return timeFromText(input.size() == 16 ? input + ":00" : input);
        } catch (const ValidationError&) {
            error("Please type the date and time like 2026-10-30 17:00");
        }
    }
}

bool Console::confirm(const std::string& question) {
    while (true) {
        const std::string answer = text(question + " (y/n)");
        if (answer == "y" || answer == "Y") return true;
        if (answer == "n" || answer == "N") return false;
        error("Please type y or n");
    }
}

std::string formatMark(const std::optional<double>& mark) {
    if (!mark) return "-";
    std::ostringstream out;
    out << std::fixed << std::setprecision(1) << *mark;
    return out.str();
}

}  // namespace lms
