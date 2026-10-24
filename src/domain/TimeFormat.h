#pragma once
#include <chrono>
#include <string>

namespace lms {

using TimePoint = std::chrono::system_clock::time_point;

// Times are stored in the database as text "YYYY-MM-DD HH:MM:SS" in local school
// time (ISO 8601), because SQLite has no date type. Text in this form sorts correctly.
std::string toText(TimePoint time);

// Throws ValidationError if the text is not in the format above.
TimePoint timeFromText(const std::string& text);

}  // namespace lms
