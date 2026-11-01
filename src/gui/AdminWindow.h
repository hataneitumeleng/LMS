#pragma once
#include "gui/RoleWindow.h"

class QComboBox;
class QLineEdit;
class QTableWidget;

namespace lms {

// The administrator's window: users (FR-03), departments and courses (FR-04), teaching and
// enrolment (FR-05, FR-06), the full audit trail (FR-17) and backup (FR-20).
class AdminWindow : public RoleWindow {
public:
    AdminWindow(Services& services, std::unique_ptr<User> user);

private:
    QWidget* buildUsersPage();
    QWidget* buildDepartmentsPage();
    QWidget* buildTeachingPage();
    QWidget* buildBackupPage();

    void refreshUsers();
    void refreshDepartments();
    void refreshTeaching();
    // Fills a list with the active users of one role, shown by name.
    void fillUserCombo(QComboBox* combo, Role role);
    QString hodName(int departmentId);

    // Users tab
    QTableWidget* users_;
    QComboBox* newRole_;
    QLineEdit* username_;
    QLineEdit* fullName_;
    QLineEdit* password_;
    QLineEdit* rename_;
    // Departments and courses tab
    QTableWidget* departments_;
    QTableWidget* courses_;
    QLineEdit* departmentName_;
    QComboBox* hod_;
    QLineEdit* courseCode_;
    QLineEdit* courseTitle_;
    QComboBox* courseDepartment_;
    // Teaching and enrolment tab
    QComboBox* teachingCourse_;
    QComboBox* lecturer_;
    QComboBox* student_;
    // Backup tab
    QLineEdit* backupFolder_;
    AuditPage* audit_;
};

}  // namespace lms
