// Unit tests for the User hierarchy and login lockout (NFR-04), and UserFactory (Factory
// pattern): the abstract User class with polymorphic role() (FR-02).
#include "../fakes/FakeClock.h"
#include "domain/Errors.h"
#include "domain/User.h"
#include "domain/UserFactory.h"

#include <gtest/gtest.h>

#include <chrono>

namespace lms::test {
namespace {

using std::chrono::minutes;
using std::chrono::seconds;

const TimePoint kNow = at("2026-10-24 09:00:00");

// ---- Lockout (NFR-04: 5 failed attempts in a row lock the account for 5 minutes) ----

TEST(UserLockoutTest, FourFailuresDoNotLock) {
    Student user("itu", "Itu");
    for (int i = 0; i < 4; ++i) user.recordFailedLogin(kNow);
    EXPECT_FALSE(user.isLocked(kNow));
    EXPECT_EQ(user.failedAttempts(), 4);
}

TEST(UserLockoutTest, FifthFailureLocks) {
    Student user("itu", "Itu");
    for (int i = 0; i < 5; ++i) user.recordFailedLogin(kNow);
    EXPECT_TRUE(user.isLocked(kNow));
    EXPECT_TRUE(user.isLocked(kNow + minutes(4) + seconds(59)));
}

TEST(UserLockoutTest, LockEndsAfterFiveMinutes) {
    Student user("itu", "Itu");
    for (int i = 0; i < 5; ++i) user.recordFailedLogin(kNow);
    EXPECT_FALSE(user.isLocked(kNow + minutes(5)));
}

TEST(UserLockoutTest, CountStartsAgainAfterTheLock) {
    Student user("itu", "Itu");
    for (int i = 0; i < 5; ++i) user.recordFailedLogin(kNow);
    EXPECT_EQ(user.failedAttempts(), 0);
}

TEST(UserLockoutTest, SuccessfulLoginResetsTheCount) {
    Student user("itu", "Itu");
    for (int i = 0; i < 4; ++i) user.recordFailedLogin(kNow);
    user.recordSuccessfulLogin();
    user.recordFailedLogin(kNow);
    EXPECT_EQ(user.failedAttempts(), 1);
    EXPECT_FALSE(user.isLocked(kNow));
}

// ---- UserFactory and polymorphism (FR-02, FR-03) ----

TEST(UserFactoryTest, CreatesTheRightSubclassForEachRole) {
    const UserFactory factory;
    EXPECT_EQ(factory.create(Role::Student, "s", "S", std::nullopt)->role(), Role::Student);
    EXPECT_EQ(factory.create(Role::Lecturer, "l", "L", std::nullopt)->role(), Role::Lecturer);
    EXPECT_EQ(factory.create(Role::Administrator, "a", "A", std::nullopt)->role(),
              Role::Administrator);
    EXPECT_EQ(factory.create(Role::HoD, "h", "H", 1)->role(), Role::HoD);
}

TEST(UserFactoryTest, OnlyAnHodHasADepartment) {
    const UserFactory factory;
    EXPECT_EQ(factory.create(Role::HoD, "h", "H", 7)->departmentId(), 7);
    EXPECT_FALSE(factory.create(Role::Lecturer, "l", "L", std::nullopt)->departmentId());
}

TEST(UserFactoryTest, HodWithoutDepartmentIsRefused) {
    const UserFactory factory;
    EXPECT_THROW(factory.create(Role::HoD, "h", "H", std::nullopt), ValidationError);
}

TEST(UserFactoryTest, DepartmentForANonHodIsRefused) {
    const UserFactory factory;
    EXPECT_THROW(factory.create(Role::Student, "s", "S", 1), ValidationError);
}

}  // namespace
}  // namespace lms::test
