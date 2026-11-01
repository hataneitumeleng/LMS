// Service tests for AuthService: login (FR-01), lockout (NFR-04), password rules and
// changing a password (FR-19). Uses the fake user repository, hasher and clock.
#include "../fakes/ServiceTest.h"
#include "domain/Errors.h"

#include <chrono>

namespace lms::test {
namespace {

using std::chrono::minutes;

class AuthServiceTest : public ServiceTest {
protected:
    void failLogins(int times) {
        for (int i = 0; i < times; ++i) {
            EXPECT_THROW(auth.login("itu", "WrongPass1"), AuthError);
        }
    }
};

TEST_F(AuthServiceTest, CorrectPasswordReturnsTheUserWithTheRightRole) {
    const auto user = auth.login("itu", "Password1");
    ASSERT_NE(user, nullptr);
    EXPECT_EQ(user->id(), itu);
    EXPECT_EQ(user->role(), Role::Student);  // FR-02: the role decides the menu
}

TEST_F(AuthServiceTest, WrongPasswordIsRefused) {
    EXPECT_THROW(auth.login("itu", "WrongPass1"), AuthError);
}

TEST_F(AuthServiceTest, UnknownUserGetsTheSameMessageAsAWrongPassword) {
    std::string unknown, wrong;
    try { auth.login("nobody", "Password1"); } catch (const AuthError& e) { unknown = e.what(); }
    try { auth.login("itu", "WrongPass1"); } catch (const AuthError& e) { wrong = e.what(); }
    EXPECT_EQ(unknown, "Invalid username or password");
    EXPECT_EQ(unknown, wrong);  // an attacker can't tell which usernames exist
}

TEST_F(AuthServiceTest, DeactivatedUserCannotLogIn) {
    auto user = users.findById(itu);
    user->setActive(false);
    users.save(*user);
    EXPECT_THROW(auth.login("itu", "Password1"), AuthError);
}

TEST_F(AuthServiceTest, FailedAttemptsAreSaved) {
    failLogins(2);
    EXPECT_EQ(users.findById(itu)->failedAttempts(), 2);
}

TEST_F(AuthServiceTest, FiveFailuresLockEvenTheCorrectPassword) {  // NFR-04
    failLogins(5);
    try {
        auth.login("itu", "Password1");
        FAIL() << "a locked account logged in";
    } catch (const AuthError& e) {
        EXPECT_NE(std::string(e.what()).find("locked until 2026-10-24 09:05:00"),
                  std::string::npos);
    }
}

TEST_F(AuthServiceTest, LockedAccountWorksAgainAfterFiveMinutes) {
    failLogins(5);
    clock.advance(minutes(5));
    EXPECT_NE(auth.login("itu", "Password1"), nullptr);
}

TEST_F(AuthServiceTest, SuccessfulLoginClearsFailedAttempts) {
    failLogins(3);
    auth.login("itu", "Password1");
    EXPECT_EQ(users.findById(itu)->failedAttempts(), 0);
}

TEST_F(AuthServiceTest, LockOfOneUserDoesNotAffectAnother) {
    failLogins(5);
    EXPECT_NE(auth.login("neo", "Password1"), nullptr);
}

// ---- Passwords (NFR-04, FR-19) ----

TEST_F(AuthServiceTest, PasswordOfSevenCharactersIsTooShort) {
    EXPECT_THROW(AuthService::checkPasswordRules("Pass123"), ValidationError);
}

TEST_F(AuthServiceTest, PasswordOfEightCharactersIsAccepted) {
    EXPECT_NO_THROW(AuthService::checkPasswordRules("Pass1234"));
}

TEST_F(AuthServiceTest, ChangePasswordNeedsTheCurrentPassword) {
    EXPECT_THROW(auth.changePassword(itu, "WrongPass1", "NewPassword9"), AuthError);
}

TEST_F(AuthServiceTest, ChangePasswordChecksTheNewPassword) {
    EXPECT_THROW(auth.changePassword(itu, "Password1", "short"), ValidationError);
}

TEST_F(AuthServiceTest, AfterChangingOnlyTheNewPasswordWorks) {
    auth.changePassword(itu, "Password1", "NewPassword9");
    EXPECT_NE(auth.login("itu", "NewPassword9"), nullptr);
    EXPECT_THROW(auth.login("itu", "Password1"), AuthError);
}

}  // namespace
}  // namespace lms::test
