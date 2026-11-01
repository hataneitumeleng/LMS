#pragma once
#include "domain/TimeFormat.h"
#include "services/IClock.h"

#include <chrono>
#include <string>

namespace lms::test {

// A clock the test controls, so every test runs at the same "now" (Week 3: no code reads
// the system clock directly, which is what makes this possible).
class FakeClock : public IClock {
public:
    explicit FakeClock(const std::string& start = "2026-10-24 09:00:00")
        : now_(timeFromText(start)) {}

    TimePoint now() const override { return now_; }

    void set(const std::string& text) { now_ = timeFromText(text); }
    void advance(std::chrono::seconds by) { now_ += by; }

private:
    TimePoint now_;
};

// Short way to write a time in a test: at("2026-10-30 17:00:00").
inline TimePoint at(const std::string& text) { return timeFromText(text); }

}  // namespace lms::test
