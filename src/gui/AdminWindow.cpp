#include "gui/AdminWindow.h"

#include "gui/GuiCommon.h"

#include <QComboBox>
#include <QDate>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include <map>

namespace lms {

namespace {

QPushButton* makeButton(const QString& text, const QString& name) {
    auto* button = new QPushButton(text);
    button->setObjectName(name);
    return button;
}

QLineEdit* makeEdit(const QString& name) {
    auto* edit = new QLineEdit;
    edit->setObjectName(name);
    return edit;
}

QComboBox* makeCombo(const QString& name) {
    auto* combo = new QComboBox;
    combo->setObjectName(name);
    return combo;
}

}  // namespace

AdminWindow::AdminWindow(Services& services, std::unique_ptr<User> user)
    : RoleWindow(services, std::move(user), "Administrator") {
    addPage(buildUsersPage(), "Users", [this] { refreshUsers(); });
    addPage(buildDepartmentsPage(), "Departments and courses", [this] { refreshDepartments(); });
    addPage(buildTeachingPage(), "Teaching and enrolment", [this] { refreshTeaching(); });
    audit_ = new AuditPage(services_.audit, this->user());
    addPage(audit_, "Audit trail", [this] { audit_->refresh(); });
    addPage(buildBackupPage(), "Backup", [] {});
}

// ---- Users (FR-03) ----

QWidget* AdminWindow::buildUsersPage() {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    users_ = makeTable({"No.", "Username", "Full name", "Role", "Department", "Active"}, page);
    users_->setObjectName("users");
    layout->addWidget(users_);

    auto* boxes = new QHBoxLayout;
    auto* create = new QGroupBox("New user");
    auto* createForm = new QFormLayout(create);
    newRole_ = makeCombo("role");
    newRole_->addItem("Student", static_cast<int>(Role::Student));
    newRole_->addItem("Lecturer", static_cast<int>(Role::Lecturer));
    newRole_->addItem("Administrator", static_cast<int>(Role::Administrator));
    username_ = makeEdit("username");
    fullName_ = makeEdit("fullName");
    password_ = makeEdit("password");
    password_->setEchoMode(QLineEdit::Password);
    QPushButton* createButton = makeButton("Create user", "createUser");
    createForm->addRow("Role", newRole_);
    createForm->addRow("Username", username_);
    createForm->addRow("Full name", fullName_);
    createForm->addRow("First password", password_);
    createForm->addRow(new QLabel("An HoD is made by choosing a lecturer for a department."));
    createForm->addRow(createButton);
    boxes->addWidget(create);

    auto* change = new QGroupBox("Chosen user");
    auto* changeForm = new QFormLayout(change);
    rename_ = makeEdit("rename");
    QPushButton* renameButton = makeButton("Change name", "renameUser");
    QPushButton* deactivateButton = makeButton("Deactivate", "deactivateUser");
    changeForm->addRow("New full name", rename_);
    changeForm->addRow(renameButton);
    changeForm->addRow(deactivateButton);
    boxes->addWidget(change);
    layout->addLayout(boxes);

    connect(createButton, &QPushButton::clicked, this, [this] {
        runAction(this, [this] {
            const auto role = static_cast<Role>(newRole_->currentData().toInt());
            const int id = services_.admin.createUser(role, str(username_->text()),
                                                      str(fullName_->text()),
                                                      password_->text().toStdString());
            showInfo(this, "User created with number " + QString::number(id) + ".");
            username_->clear();
            fullName_->clear();
            password_->clear();
            refreshUsers();
        });
    });
    connect(renameButton, &QPushButton::clicked, this, [this] {
        const auto id = selectedId(users_);
        if (!id) return showError(this, "Choose a user in the list first.");
        runAction(this, [&] {
            services_.admin.updateUserName(*id, str(rename_->text()));
            rename_->clear();
            refreshUsers();
        });
    });
    connect(deactivateButton, &QPushButton::clicked, this, [this] {
        const auto id = selectedId(users_);
        if (!id) return showError(this, "Choose a user in the list first.");
        // NFR-06: confirm before an action that locks someone out
        if (!confirm(this, "Deactivate this user? They will not be able to log in.")) return;
        runAction(this, [&] {
            services_.admin.deactivateUser(*id);
            refreshUsers();
        });
    });
    return page;
}

void AdminWindow::refreshUsers() {
    std::vector<QStringList> rows;
    for (const auto& u : services_.admin.listUsers()) {
        const QString department =
            u->departmentId() ? QString::number(*u->departmentId()) : QString();
        rows.push_back({QString::number(u->id()), qs(u->username()), qs(u->fullName()),
                        qs(toString(u->role())), department, u->active() ? "yes" : "no"});
    }
    fillTable(users_, rows);
}

// ---- Departments and courses (FR-04) ----

QWidget* AdminWindow::buildDepartmentsPage() {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    auto* tables = new QHBoxLayout;
    departments_ = makeTable({"No.", "Department", "HoD"}, page);
    departments_->setObjectName("departments");
    courses_ = makeTable({"No.", "Code", "Title", "Department"}, page);
    courses_->setObjectName("courses");
    tables->addWidget(departments_);
    tables->addWidget(courses_);
    layout->addLayout(tables);

    auto* boxes = new QHBoxLayout;
    auto* departmentBox = new QGroupBox("Department");
    auto* departmentForm = new QFormLayout(departmentBox);
    departmentName_ = makeEdit("departmentName");
    hod_ = makeCombo("hod");
    QPushButton* createDepartment = makeButton("Create department", "createDepartment");
    QPushButton* renameDepartment = makeButton("Rename chosen department", "renameDepartment");
    QPushButton* replaceHod = makeButton("Make this lecturer the chosen department's HoD",
                                         "replaceHod");
    departmentForm->addRow("Name", departmentName_);
    departmentForm->addRow("HoD (a lecturer)", hod_);
    departmentForm->addRow(createDepartment);
    departmentForm->addRow(renameDepartment);
    departmentForm->addRow(replaceHod);
    boxes->addWidget(departmentBox);

    auto* courseBox = new QGroupBox("Course");
    auto* courseForm = new QFormLayout(courseBox);
    courseCode_ = makeEdit("courseCode");
    courseTitle_ = makeEdit("courseTitle");
    courseDepartment_ = makeCombo("courseDepartment");
    QPushButton* createCourse = makeButton("Create course", "createCourse");
    QPushButton* updateCourse = makeButton("Change chosen course's code and title",
                                           "updateCourse");
    courseForm->addRow("Code", courseCode_);
    courseForm->addRow("Title", courseTitle_);
    courseForm->addRow("Department", courseDepartment_);
    courseForm->addRow(createCourse);
    courseForm->addRow(updateCourse);
    boxes->addWidget(courseBox);
    layout->addLayout(boxes);

    connect(createDepartment, &QPushButton::clicked, this, [this] {
        const auto lecturerId = comboId(hod_);
        if (!lecturerId) return showError(this, "Create a lecturer first; they become the HoD.");
        runAction(this, [&] {
            const int id = services_.admin.createDepartment(str(departmentName_->text()),
                                                            *lecturerId);
            showInfo(this, "Department created with number " + QString::number(id) + ".");
            departmentName_->clear();
            refreshDepartments();
        });
    });
    connect(renameDepartment, &QPushButton::clicked, this, [this] {
        const auto id = selectedId(departments_);
        if (!id) return showError(this, "Choose a department in the list first.");
        runAction(this, [&] {
            services_.admin.renameDepartment(*id, str(departmentName_->text()));
            departmentName_->clear();
            refreshDepartments();
        });
    });
    connect(replaceHod, &QPushButton::clicked, this, [this] {
        const auto id = selectedId(departments_);
        const auto lecturerId = comboId(hod_);
        if (!id || !lecturerId) {
            return showError(this, "Choose a department in the list and a lecturer first.");
        }
        if (!confirm(this, "The current HoD will be deactivated. Continue?")) return;
        runAction(this, [&] {
            services_.admin.replaceHod(*id, *lecturerId);
            refreshDepartments();
        });
    });
    connect(createCourse, &QPushButton::clicked, this, [this] {
        const auto departmentId = comboId(courseDepartment_);
        if (!departmentId) return showError(this, "Create a department first.");
        runAction(this, [&] {
            const int id = services_.admin.createCourse(str(courseCode_->text()),
                                                        str(courseTitle_->text()), *departmentId);
            showInfo(this, "Course created with number " + QString::number(id) + ".");
            courseCode_->clear();
            courseTitle_->clear();
            refreshDepartments();
        });
    });
    connect(updateCourse, &QPushButton::clicked, this, [this] {
        const auto id = selectedId(courses_);
        if (!id) return showError(this, "Choose a course in the list first.");
        runAction(this, [&] {
            services_.admin.updateCourse(*id, str(courseCode_->text()),
                                         str(courseTitle_->text()));
            courseCode_->clear();
            courseTitle_->clear();
            refreshDepartments();
        });
    });
    return page;
}

void AdminWindow::refreshDepartments() {
    std::vector<QStringList> rows;
    std::map<int, QString> departmentNames;
    courseDepartment_->clear();
    for (const Department& d : services_.admin.listDepartments()) {
        rows.push_back({QString::number(d.id), qs(d.name), hodName(d.id)});
        courseDepartment_->addItem(qs(d.name), d.id);
        departmentNames[d.id] = qs(d.name);
    }
    fillTable(departments_, rows);

    rows.clear();
    for (const Course& c : services_.admin.listCourses()) {
        rows.push_back({QString::number(c.id), qs(c.code), qs(c.title),
                        departmentNames[c.departmentId]});
    }
    fillTable(courses_, rows);
    fillUserCombo(hod_, Role::Lecturer);
}

QString AdminWindow::hodName(int departmentId) {
    for (const auto& u : services_.admin.listUsers()) {
        if (u->role() == Role::HoD && u->active() && u->departmentId() == departmentId) {
            return qs(u->fullName());
        }
    }
    return "none";
}

// ---- Teaching and enrolment (FR-05, FR-06) ----

QWidget* AdminWindow::buildTeachingPage() {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    auto* form = new QFormLayout;
    teachingCourse_ = makeCombo("teachingCourse");
    lecturer_ = makeCombo("lecturer");
    student_ = makeCombo("student");
    QPushButton* assign = makeButton("Assign lecturer to course", "assignLecturer");
    QPushButton* enrol = makeButton("Enrol student", "enrol");
    QPushButton* unenrol = makeButton("Remove student from course", "unenrol");
    form->addRow("Course", teachingCourse_);
    form->addRow("Lecturer", lecturer_);
    form->addRow(assign);
    form->addRow("Student", student_);
    auto* studentButtons = new QHBoxLayout;
    studentButtons->addWidget(enrol);
    studentButtons->addWidget(unenrol);
    form->addRow(studentButtons);
    layout->addLayout(form);
    layout->addStretch();

    connect(assign, &QPushButton::clicked, this, [this] {
        const auto courseId = comboId(teachingCourse_);
        const auto lecturerId = comboId(lecturer_);
        if (!courseId || !lecturerId) return showError(this, "Choose a course and a lecturer.");
        runAction(this, [&] {
            services_.admin.assignLecturer(*courseId, *lecturerId);
            showInfo(this, lecturer_->currentText() + " now teaches " +
                               teachingCourse_->currentText() + ".");
        });
    });
    connect(enrol, &QPushButton::clicked, this, [this] {
        const auto courseId = comboId(teachingCourse_);
        const auto studentId = comboId(student_);
        if (!courseId || !studentId) return showError(this, "Choose a course and a student.");
        runAction(this, [&] {
            services_.admin.enrol(*courseId, *studentId);
            showInfo(this, student_->currentText() + " is enrolled in " +
                               teachingCourse_->currentText() + ".");
        });
    });
    connect(unenrol, &QPushButton::clicked, this, [this] {
        const auto courseId = comboId(teachingCourse_);
        const auto studentId = comboId(student_);
        if (!courseId || !studentId) return showError(this, "Choose a course and a student.");
        if (!confirm(this, "Remove this student from the course?")) return;
        runAction(this, [&] {
            services_.admin.unenrol(*courseId, *studentId);
            showInfo(this, "Student removed from the course.");
        });
    });
    return page;
}

void AdminWindow::refreshTeaching() {
    teachingCourse_->clear();
    for (const Course& c : services_.admin.listCourses()) {
        teachingCourse_->addItem(qs(c.code + "  " + c.title), c.id);
    }
    fillUserCombo(lecturer_, Role::Lecturer);
    fillUserCombo(student_, Role::Student);
}

void AdminWindow::fillUserCombo(QComboBox* combo, Role role) {
    combo->clear();
    for (const auto& u : services_.admin.listUsers()) {
        if (u->role() != role || !u->active()) continue;
        combo->addItem(qs(u->fullName() + " (" + u->username() + ")"), u->id());
    }
}

// ---- Backup (FR-20) ----

QWidget* AdminWindow::buildBackupPage() {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    layout->addWidget(new QLabel("Copies lms.db and every submitted file into a new folder."));
    auto* row = new QHBoxLayout;
    backupFolder_ = makeEdit("backupFolder");
    backupFolder_->setText("backups/" + QDate::currentDate().toString("yyyy-MM-dd"));
    QPushButton* browse = makeButton("Browse...", "browseBackup");
    QPushButton* backUp = makeButton("Back up now", "backUp");
    row->addWidget(new QLabel("New folder:"));
    row->addWidget(backupFolder_, 1);
    row->addWidget(browse);
    row->addWidget(backUp);
    layout->addLayout(row);
    layout->addStretch();

    connect(browse, &QPushButton::clicked, this, [this] {
        const QString folder = QFileDialog::getExistingDirectory(this, "Where to put the backup");
        if (!folder.isEmpty()) {
            backupFolder_->setText(folder + "/lms-backup-" +
                                   QDate::currentDate().toString("yyyy-MM-dd"));
        }
    });
    connect(backUp, &QPushButton::clicked, this, [this] {
        runAction(this, [this] {
            services_.backup.backup(str(backupFolder_->text()));
            showInfo(this, "Backup saved in " + backupFolder_->text() +
                               " (lms.db and the submitted files).");
        });
    });
    return page;
}

}  // namespace lms
