#pragma once
#include "gui/RoleWindow.h"

class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QTableWidget;

namespace lms {

// The Head of Department's window: moderation (FR-13, FR-14), release (FR-15) and the
// department's audit trail (FR-17).
class HodWindow : public RoleWindow {
public:
    HodWindow(Services& services, std::unique_ptr<User> user);

private:
    QWidget* buildModerationPage();
    void refreshAssignments();
    void refreshMarkedWork();
    void approveAll();
    void adjust();
    void release();
    void openFile();

    QTableWidget* assignments_;
    QLabel* chosen_;
    QTableWidget* markedWork_;
    QDoubleSpinBox* finalMark_;
    QLineEdit* reason_;
    AuditPage* audit_;
};

}  // namespace lms
