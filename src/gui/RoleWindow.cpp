#include "gui/RoleWindow.h"

#include "gui/ChangePasswordDialog.h"
#include "gui/GuiCommon.h"

#include <QLabel>
#include <QMenuBar>
#include <QStatusBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QVBoxLayout>

namespace lms {

RoleWindow::RoleWindow(Services& services, std::unique_ptr<User> user, const QString& roleName)
    : services_(services), user_(std::move(user)), tabs_(new QTabWidget(this)) {
    setWindowTitle("Moderated LMS: " + roleName + ": " + qs(user_->fullName()));
    setCentralWidget(tabs_);
    resize(980, 640);

    QMenu* account = menuBar()->addMenu("&Account");
    account->addAction("Change my password", this, [this] {
        ChangePasswordDialog(services_.auth, user_->id(), this).exec();
    });
    account->addSeparator();
    account->addAction("Log out", this, [this] {
        loggedOut_ = true;
        close();
    });

    statusBar()->addWidget(new QLabel("Logged in as " + qs(user_->username()) + " (" +
                                      roleName + ")"));
    connect(tabs_, &QTabWidget::currentChanged, this, [this] { refreshCurrentPage(); });
}

void RoleWindow::addPage(QWidget* page, const QString& name, std::function<void()> refresh) {
    refreshers_.push_back(std::move(refresh));
    tabs_->addTab(page, name);  // the first tab is refreshed as soon as it is added
}

void RoleWindow::showPage(int index) { tabs_->setCurrentIndex(index); }

void RoleWindow::refreshCurrentPage() {
    const int index = tabs_->currentIndex();
    if (index < 0 || index >= static_cast<int>(refreshers_.size())) return;
    runAction(this, refreshers_[static_cast<std::size_t>(index)]);
}

void RoleWindow::closeEvent(QCloseEvent* event) {
    QMainWindow::closeEvent(event);
    if (onClosed_) onClosed_();
}

// ---- Audit trail tab ----

AuditPage::AuditPage(AuditService& audit, const User& viewer, QWidget* parent)
    : QWidget(parent), audit_(audit), viewer_(viewer) {
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Every mark change: who, when, old and new value, and why."));
    table_ = makeTable({"No.", "When", "Changed by (user no.)", "Submission", "Old", "New",
                        "Reason"},
                       this);
    table_->setObjectName("auditTable");
    layout->addWidget(table_);
}

void AuditPage::refresh() {
    std::vector<QStringList> rows;
    for (const AuditEntry& e : audit_.entriesFor(viewer_)) {
        rows.push_back({QString::number(e.id), qs(toText(e.changedAt)),
                        QString::number(e.userId), QString::number(e.submissionId),
                        markText(e.oldValue), markText(e.newValue), qs(e.reason)});
    }
    fillTable(table_, rows);
}

}  // namespace lms
