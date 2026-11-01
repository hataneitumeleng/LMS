#pragma once
#include "gui/RoleWindow.h"

class QLabel;
class QLineEdit;
class QTableWidget;

namespace lms {

// The student's window: a Home page (welcome, courses, work still to do), then
// assignments, deadlines, released marks and feedback (FR-09, FR-16),
// and submitting or resubmitting a file (FR-10, FR-11).
class StudentWindow : public RoleWindow {
public:
    StudentWindow(Services& services, std::unique_ptr<User> user);

private:
    QWidget* buildHomePage();
    void refreshHome();
    QWidget* buildAssignmentsPage();
    void refreshAssignments();
    void browse();
    void openBrief();
    void submit();

    QLabel* welcome_;
    QLabel* courses_;
    QLabel* summary_;
    QTableWidget* toDo_;
    QTableWidget* assignments_;
    QLabel* chosen_;
    QLineEdit* file_;
};

}  // namespace lms
