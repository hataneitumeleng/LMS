#pragma once
#include <QDialog>

class QLineEdit;

namespace lms {

class AuthService;

// Change my own password (FR-19). The new password is typed twice.
class ChangePasswordDialog : public QDialog {
public:
    ChangePasswordDialog(AuthService& auth, int userId, QWidget* parent = nullptr);

private:
    void change();

    AuthService& auth_;
    int userId_;
    QLineEdit* current_;
    QLineEdit* newPassword_;
    QLineEdit* repeat_;
};

}  // namespace lms
