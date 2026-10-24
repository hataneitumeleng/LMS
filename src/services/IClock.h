#pragma once
#include "domain/TimeFormat.h"

namespace lms {

// Gives the current time. Services never read the clock directly, so tests can
// use a fake clock to check deadlines and lockouts (risk R10, Section 3.2).
class IClock {
public:
    virtual ~IClock() = default;
    virtual TimePoint now() const = 0;
};

// The real clock used by the program.
class SystemClock : public IClock {
public:
    TimePoint now() const override { return std::chrono::system_clock::now(); }
};

}  // namespace lms
