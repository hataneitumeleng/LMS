#include "gui/ChangePasswordDialog.h"

#include "gui/GuiCommon.h"
#include "services/AuthService.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>

namespace lms {

ChangePasswordDialog::ChangePasswordDialog(AuthService& auth, int userId, QWidget* parent)
    : QDialog(parent), auth_(auth), userId_(userId) {
    setWindowTitle("Change my password");
    auto* form = new QFormLayout(this);
    current_ = new QLineEdit;
    newPassword_ = new QLineEdit;
    repeat_ = new QLineEdit;
    for (QLineEdit* edit : {current_, newPassword_, repeat_}) {
        edit->setEchoMode(QLineEdit::Password);
    }
    form->addRow("Current password", current_);
    form->addRow("New password (8 or more)", newPassword_);
    form->addRow("New password again", repeat_);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, [this] { change(); });
    connect(buttons, &QDialogButtonBox::rejected, this, [this] { reject(); });
}

void ChangePasswordDialog::change() {
    if (newPassword_->text() != repeat_->text()) {
        showError(this, "The two new passwords are different. Nothing was changed.");
        return;
    }
    const bool ok = runAction(this, [this] {
        auth_.changePassword(userId_, current_->text().toStdString(),
                             newPassword_->text().toStdString());
    });
    if (ok) {
        showInfo(this, "Password changed.");
        accept();
    }
}

}  // namespace lms
