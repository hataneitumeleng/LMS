#pragma once
#include "gui/RoleWindow.h"

class QComboBox;
class QDateTimeEdit;
class QDoubleSpinBox;
class QLineEdit;
class QSpinBox;
class QTableWidget;

namespace lms {

// The lecturer's window: assignments (FR-07), extensions (FR-08) and marking (FR-12, FR-18).
class LecturerWindow : public RoleWindow {
public:
    LecturerWindow(Services& services, std::unique_ptr<User> user);

private:
    QWidget* buildAssignmentsPage();
    QWidget* buildExtensionsPage();
    QWidget* buildMarkingPage();

    void refreshCourses();
    void refreshAssignments();
    void createAssignment();
    void grantExtension();
    void refreshSubmissions();
    void showChosenSubmission();
    void mark();
    void amend();
    void openFile();
    // Lists every assignment in the lecturer's courses, keeping the current choice.
    void fillAssignmentCombo(QComboBox* combo);

    // Assignments tab
    QComboBox* course_;
    QTableWidget* assignments_;
    QLineEdit* title_;
    QLineEdit* description_;
    QDateTimeEdit* deadline_;
    QSpinBox* maxMark_;
    QLineEdit* brief_;
    // Extensions tab
    QComboBox* extensionAssignment_;
    QSpinBox* student_;
    QDateTimeEdit* newDeadline_;
    QLineEdit* extensionReason_;
    // Marking tab
    QComboBox* markingAssignment_;
    QTableWidget* submissions_;
    QDoubleSpinBox* rawMark_;
    QLineEdit* feedback_;
    QLineEdit* amendReason_;
};

}  // namespace lms
