#include "gui/LoginDialog.h"

#include "domain/Errors.h"
#include "gui/GuiCommon.h"
#include "services/AdminService.h"
#include "services/AuthService.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include <exception>

namespace lms {

namespace {

QLineEdit* addField(QFormLayout* form, const QString& label, const QString& name,
                    bool hidden = false) {
    auto* edit = new QLineEdit;
    edit->setObjectName(name);
    if (hidden) edit->setEchoMode(QLineEdit::Password);
    form->addRow(label, edit);
    return edit;
}

QLabel* makeErrorLabel() {
    auto* label = new QLabel;
    label->setObjectName("error");
    label->setStyleSheet("color: #b00020;");
    label->setWordWrap(true);
    return label;
}

}  // namespace

// ---- Login ----

LoginDialog::LoginDialog(AuthService& auth, QWidget* parent) : QDialog(parent), auth_(auth) {
    setWindowTitle("Moderated LMS: log in");
    auto* layout = new QVBoxLayout(this);
    auto* heading = new QLabel("<h2>Moderated LMS</h2>Log in with your username and password.");
    layout->addWidget(heading);

    auto* form = new QFormLayout;
    username_ = addField(form, "Username", "username");
    password_ = addField(form, "Password", "password", true);
    layout->addLayout(form);

    error_ = makeErrorLabel();
    layout->addWidget(error_);

    auto* buttons = new QDialogButtonBox;
    QPushButton* login = buttons->addButton("Log in", QDialogButtonBox::AcceptRole);
    login->setObjectName("login");
    login->setDefault(true);
    QPushButton* exit = buttons->addButton("Exit", QDialogButtonBox::RejectRole);
    layout->addWidget(buttons);

    connect(login, &QPushButton::clicked, this, [this] { tryLogin(); });
    connect(exit, &QPushButton::clicked, this, [this] { reject(); });
    setMinimumWidth(360);
}

void LoginDialog::tryLogin() {
    try {
        user_ = auth_.login(str(username_->text()), password_->text().toStdString());
        password_->clear();
        accept();
    } catch (const AuthError& e) {
        error_->setText(qs(e.what()));
        password_->clear();
        password_->setFocus();
    } catch (const std::exception& e) {
        error_->setText("Something went wrong: " + qs(e.what()));
    }
}

// ---- First administrator ----

FirstAdminDialog::FirstAdminDialog(AdminService& admin, QWidget* parent)
    : QDialog(parent), admin_(admin) {
    setWindowTitle("Moderated LMS: first start");
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("<h3>First start</h3>Create the administrator account."));

    auto* form = new QFormLayout;
    username_ = addField(form, "Username", "username");
    fullName_ = addField(form, "Full name", "fullName");
    password_ = addField(form, "Password (8 or more)", "password", true);
    repeat_ = addField(form, "Password again", "repeat", true);
    layout->addLayout(form);

    error_ = makeErrorLabel();
    layout->addWidget(error_);

    auto* buttons = new QDialogButtonBox;
    QPushButton* create = buttons->addButton("Create", QDialogButtonBox::AcceptRole);
    create->setObjectName("create");
    create->setDefault(true);
    QPushButton* exit = buttons->addButton("Exit", QDialogButtonBox::RejectRole);
    layout->addWidget(buttons);

    connect(create, &QPushButton::clicked, this, [this] { this->create(); });
    connect(exit, &QPushButton::clicked, this, [this] { reject(); });
    setMinimumWidth(380);
}

void FirstAdminDialog::create() {
    if (password_->text() != repeat_->text()) {
        error_->setText("The two passwords are different.");
        return;
    }
    try {
        admin_.createUser(Role::Administrator, str(username_->text()), str(fullName_->text()),
                          password_->text().toStdString());
        showInfo(this, "Administrator created. Please log in.");
        accept();
    } catch (const std::exception& e) {
        error_->setText(qs(e.what()));
    }
}

}  // namespace lms
