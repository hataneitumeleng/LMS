#pragma once
#include "data/IUserRepository.h"
#include "domain/UserFactory.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace lms::test {

// IUserRepository kept in memory. Like the real repository it returns copies, so a service
// only changes what is stored by calling save(), which the tests can then check.
class FakeUserRepository : public IUserRepository {
public:
    std::unique_ptr<User> findByUsername(const std::string& username) override {
        for (const auto& [id, user] : users_) {
            if (user->username() == username) return copy(*user);
        }
        return nullptr;
    }
    std::unique_ptr<User> findById(int id) override {
        const auto it = users_.find(id);
        return it == users_.end() ? nullptr : copy(*it->second);
    }
    std::unique_ptr<User> findActiveHod(int departmentId) override {
        for (const auto& [id, user] : users_) {
            if (user->role() == Role::HoD && user->active() &&
                user->departmentId() == departmentId) {
                return copy(*user);
            }
        }
        return nullptr;
    }
    std::vector<std::unique_ptr<User>> findAll() override {
        std::vector<std::unique_ptr<User>> all;
        for (const auto& [id, user] : users_) all.push_back(copy(*user));
        return all;
    }
    int save(const User& user) override {
        auto stored = copy(user);
        if (stored->id() == 0) stored->setId(nextId_++);
        const int id = stored->id();
        users_[id] = std::move(stored);
        ++saves;
        return id;
    }

    int saves = 0;  // how many times save() was called

private:
    static std::unique_ptr<User> copy(const User& user) {
        auto result = UserFactory().create(user.role(), user.username(), user.fullName(),
                                           user.departmentId());
        result->setId(user.id());
        result->setPasswordHash(user.passwordHash());
        result->setActive(user.active());
        result->setLoginState(user.failedAttempts(), user.lockedUntil());
        return result;
    }

    std::map<int, std::unique_ptr<User>> users_;
    int nextId_ = 1;
};

}  // namespace lms::test
