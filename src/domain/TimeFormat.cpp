#include "domain/TimeFormat.h"

#include "domain/Errors.h"

#include <ctime>
#include <iomanip>
#include <sstream>

namespace lms {

std::string toText(TimePoint time) {
    std::time_t t = std::chrono::system_clock::to_time_t(time);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &t);
#else
    localtime_r(&t, &local);
#endif
    std::ostringstream out;
    out << std::put_time(&local, "%Y-%m-%d %H:%M:%S");
    return out.str();
}

TimePoint timeFromText(const std::string& text) {
    std::tm local{};
    std::istringstream in(text);
    in >> std::get_time(&local, "%Y-%m-%d %H:%M:%S");
    if (in.fail() || !(in >> std::ws).eof()) {  // also refuses extra text after the time
        throw ValidationError("Date and time must look like 2026-11-30 23:59:00");
    }
    local.tm_isdst = -1;  // let the system decide about daylight saving
    std::time_t t = std::mktime(&local);
    if (t == -1) {
        throw ValidationError("Invalid date and time: " + text);
    }
    const TimePoint result = std::chrono::system_clock::from_time_t(t);
    // mktime quietly turns 31 February into 3 March, so convert back and compare
    // to refuse dates that don't exist.
    if (toText(result) != text) {
        throw ValidationError("This date does not exist: " + text);
    }
    return result;
}

}  // namespace lms
