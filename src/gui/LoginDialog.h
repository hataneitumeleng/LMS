#pragma once
#include "domain/User.h"

#include <QDialog>

#include <memory>

class QLabel;
class QLineEdit;

namespace lms {

class AdminService;
class AuthService;

// The login window (FR-01). The password is hidden while typing.
// Accepted only after a successful login; takeUser() then hands over the user.
class LoginDialog : public QDialog {
public:
    explicit LoginDialog(AuthService& auth, QWidget* parent = nullptr);

    std::unique_ptr<User> takeUser() { return std::move(user_); }

private:
    void tryLogin();

    AuthService& auth_;
    QLineEdit* username_;
    QLineEdit* password_;
    QLabel* error_;
    std::unique_ptr<User> user_;
};

// Shown on the very first start, when there are no users: creates the administrator.
class FirstAdminDialog : public QDialog {
public:
    explicit FirstAdminDialog(AdminService& admin, QWidget* parent = nullptr);

private:
    void create();

    AdminService& admin_;
    QLineEdit* username_;
    QLineEdit* fullName_;
    QLineEdit* password_;
    QLineEdit* repeat_;
    QLabel* error_;
};

}  // namespace lms
