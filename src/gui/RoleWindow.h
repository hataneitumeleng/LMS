#pragma once
#include "app/Services.h"
#include "domain/User.h"

#include <QMainWindow>

#include <functional>
#include <memory>
#include <vector>

class QCloseEvent;
class QTabWidget;
class QTableWidget;

namespace lms {

// Base class of the four role windows (the GUI version of Menu). It owns the logged-in user,
// shows one tab per task and has an Account menu (change password, log out).
// Each subclass only builds its own tabs, so a student never sees a lecturer's tab (FR-02).
class RoleWindow : public QMainWindow {
public:
    RoleWindow(Services& services, std::unique_ptr<User> user, const QString& roleName);

    bool loggedOut() const { return loggedOut_; }
    // Called when the window closes (by Log out or the window's X button).
    void setOnClosed(std::function<void()> onClosed) { onClosed_ = std::move(onClosed); }

protected:
    // Adds a tab. refresh() reloads its lists; it runs whenever the tab is opened.
    void addPage(QWidget* page, const QString& name, std::function<void()> refresh);
    void refreshCurrentPage();
    void showPage(int index);
    void closeEvent(QCloseEvent* event) override;

    const User& user() const { return *user_; }

    Services& services_;

private:
    std::unique_ptr<User> user_;
    QTabWidget* tabs_;
    std::vector<std::function<void()>> refreshers_;
    bool loggedOut_ = false;
    std::function<void()> onClosed_;
};

// The audit trail tab (FR-17), shared by the HoD and administrator windows.
// AuditService decides which entries the viewer may see.
class AuditPage : public QWidget {
public:
    AuditPage(AuditService& audit, const User& viewer, QWidget* parent = nullptr);

    void refresh();

private:
    AuditService& audit_;
    const User& viewer_;
    QTableWidget* table_;
};

}  // namespace lms
