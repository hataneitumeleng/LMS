#include "data/Database.h"
#include "data/DatabaseError.h"
#include "data/SqliteUserRepository.h"
#include "domain/Errors.h"
#include "domain/UserFactory.h"
#include "services/Argon2Hasher.h"
#include "services/AuthService.h"

#include <iostream>
#include <vector>

namespace {

// A clock we can move forward, to show the 5-minute lockout without waiting.
class DemoClock : public lms::IClock {
public:
    lms::TimePoint now() const override { return now_; }
    void advanceMinutes(int minutes) { now_ += std::chrono::minutes(minutes); }

private:
    lms::TimePoint now_ = std::chrono::system_clock::now();
};

void tryLogin(lms::AuthService& auth, const std::string& username, const std::string& password) {
    try {
        auto user = auth.login(username, password);
        std::cout << "  Login OK: " << user->fullName() << " (" << lms::toString(user->role())
                  << ")\n";
    } catch (const lms::AuthError& e) {
        std::cout << "  Login refused: " << e.what() << "\n";
    }
}

}  // namespace

int main() {
    try {
        // Step 4 demo uses an in-memory database, so nothing is saved to lms.db.
        lms::Database db(":memory:");
        db.initialiseSchema();
        db.prepare("INSERT INTO department (name) VALUES ('Science')").step();

        lms::UserFactory factory;
        lms::SqliteUserRepository users(db, factory);
        lms::Argon2Hasher hasher;
        DemoClock clock;
        lms::AuthService auth(users, hasher, clock);

        std::cout << "1. Users created by UserFactory and saved by SqliteUserRepository:\n";
        std::vector<std::unique_ptr<lms::User>> people;
        people.push_back(factory.create(lms::Role::Administrator, "admin", "Thabo Admin", {}));
        people.push_back(factory.create(lms::Role::Lecturer, "lect1", "Mpho Lecturer", {}));
        people.push_back(factory.create(lms::Role::HoD, "hod1", "Lerato HoD", 1));
        people.push_back(factory.create(lms::Role::Student, "stu1", "Itumeleng Student", {}));
        for (auto& person : people) {
            person->setPasswordHash(hasher.hash("Password1"));
            person->setId(users.save(*person));
            // Polymorphism: role() runs the version of the real subclass.
            std::cout << "  " << person->username() << " -> " << lms::toString(person->role())
                      << "\n";
        }

        std::cout << "2. Password stored in the database (Argon2id, not plain text):\n  "
                  << users.findByUsername("stu1")->passwordHash().substr(0, 40) << "...\n";

        std::cout << "3. Correct password:\n";
        tryLogin(auth, "stu1", "Password1");

        std::cout << "4. Five wrong passwords, then the correct one (NFR-04):\n";
        for (int i = 0; i < 5; ++i) tryLogin(auth, "stu1", "wrong");
        tryLogin(auth, "stu1", "Password1");

        std::cout << "5. Five minutes later:\n";
        clock.advanceMinutes(5);
        tryLogin(auth, "stu1", "Password1");

        std::cout << "6. Rules that are enforced:\n";
        try {
            auth.changePassword(people[3]->id(), "Password1", "short");
        } catch (const lms::ValidationError& e) {
            std::cout << "  " << e.what() << "\n";
        }
        try {
            factory.create(lms::Role::HoD, "hod2", "No Department", {});
        } catch (const lms::ValidationError& e) {
            std::cout << "  " << e.what() << "\n";
        }
        try {
            auto second = factory.create(lms::Role::HoD, "hod2", "Second HoD", 1);
            second->setPasswordHash(hasher.hash("Password1"));
            users.save(*second);
        } catch (const lms::DatabaseError& e) {
            std::cout << "  Second active HoD for Science refused: " << e.what() << "\n";
        }
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
