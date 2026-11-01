#include "gui/HodWindow.h"

#include "gui/GuiCommon.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

namespace lms {

HodWindow::HodWindow(Services& services, std::unique_ptr<User> user)
    : RoleWindow(services, std::move(user), "Head of Department") {
    addPage(buildModerationPage(), "Moderation", [this] { refreshAssignments(); });
    audit_ = new AuditPage(services_.audit, this->user());
    addPage(audit_, "Audit trail", [this] { audit_->refresh(); });
}

QWidget* HodWindow::buildModerationPage() {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    layout->addWidget(new QLabel("Assignments in your department. Choose one to see its "
                                 "marked work."));
    assignments_ = makeTable(
        {"No.", "Course", "Title", "Deadline", "Submissions", "Not moderated"}, page);
    assignments_->setObjectName("hodAssignments");
    layout->addWidget(assignments_, 2);

    chosen_ = new QLabel("No assignment chosen.");
    layout->addWidget(chosen_);
    markedWork_ = makeTable(
        {"No.", "Student no.", "Status", "Raw", "Days late", "Final", "Feedback"}, page);
    markedWork_->setObjectName("markedWork");
    layout->addWidget(markedWork_, 3);

    auto* actions = new QHBoxLayout;
    auto* approveButton = new QPushButton("Approve all marks");
    approveButton->setObjectName("approveAll");
    auto* releaseButton = new QPushButton("Release marks to students");
    releaseButton->setObjectName("release");
    actions->addWidget(approveButton);
    actions->addWidget(releaseButton);
    auto* openButton = new QPushButton("Open submitted file");
    openButton->setObjectName("openFile");
    actions->addWidget(openButton);
    actions->addStretch();
    layout->addLayout(actions);

    auto* box = new QGroupBox("Adjust the chosen submission's final mark");
    auto* form = new QFormLayout(box);
    finalMark_ = new QDoubleSpinBox;
    finalMark_->setObjectName("finalMark");
    finalMark_->setRange(0.0, 100.0);
    finalMark_->setDecimals(1);
    reason_ = new QLineEdit;
    reason_->setObjectName("adjustReason");
    reason_->setPlaceholderText("goes into the audit trail");
    auto* adjustButton = new QPushButton("Adjust mark");
    adjustButton->setObjectName("adjust");
    form->addRow("New final mark", finalMark_);
    form->addRow("Reason", reason_);
    form->addRow(adjustButton);
    layout->addWidget(box);

    connect(assignments_, &QTableWidget::itemSelectionChanged, this,
            [this] { runAction(this, [this] { refreshMarkedWork(); }); });
    connect(markedWork_, &QTableWidget::itemSelectionChanged, this, [this] {
        const int r = selectedRow(markedWork_);
        if (r < 0) return;
        const QString final = markedWork_->item(r, 5)->text();
        finalMark_->setValue(final == "-" ? 0.0 : final.toDouble());
    });
    connect(openButton, &QPushButton::clicked, this, [this] { openFile(); });
    connect(markedWork_, &QTableWidget::cellDoubleClicked, this, [this] { openFile(); });
    connect(approveButton, &QPushButton::clicked, this, [this] { approveAll(); });
    connect(releaseButton, &QPushButton::clicked, this, [this] { release(); });
    connect(adjustButton, &QPushButton::clicked, this, [this] { adjust(); });
    return page;
}

void HodWindow::refreshAssignments() {
    const auto chosen = selectedId(assignments_);
    std::vector<QStringList> rows;
    for (const auto& row : services_.moderation.assignmentsInDepartment(user().id())) {
        rows.push_back({QString::number(row.assignment.id), qs(row.courseCode),
                        qs(row.assignment.title), qs(toText(row.assignment.deadline)),
                        QString::number(row.submissions), QString::number(row.notModerated)});
    }
    fillTable(assignments_, rows);
    if (chosen) {
        for (int r = 0; r < assignments_->rowCount(); ++r) {
            if (assignments_->item(r, 0)->text().toInt() == *chosen) assignments_->selectRow(r);
        }
    }
    refreshMarkedWork();
}

void HodWindow::refreshMarkedWork() {
    const auto assignmentId = selectedId(assignments_);
    std::vector<QStringList> rows;
    if (assignmentId) {
        const int r = selectedRow(assignments_);
        chosen_->setText("Marked work for " + assignments_->item(r, 1)->text() + " " +
                         assignments_->item(r, 2)->text() + ":");
        for (const Submission& s :
             services_.moderation.listForModeration(user().id(), *assignmentId)) {
            rows.push_back({QString::number(s.id), QString::number(s.studentId),
                            qs(toString(s.status)), markText(s.rawMark),
                            QString::number(s.daysLate), markText(s.finalMark), qs(s.feedback)});
        }
    } else {
        chosen_->setText("No assignment chosen.");
    }
    fillTable(markedWork_, rows);
}

void HodWindow::approveAll() {
    const auto assignmentId = selectedId(assignments_);
    if (!assignmentId) {
        showError(this, "Choose an assignment first.");
        return;
    }
    if (!confirm(this, "Approve every marked submission without changes?")) return;
    runAction(this, [&] {
        const int count = services_.moderation.approveAll(user().id(), *assignmentId);
        showInfo(this, QString::number(count) + " submission(s) approved.");
        refreshAssignments();
    });
}

void HodWindow::adjust() {
    const auto submissionId = selectedId(markedWork_);
    if (!submissionId) {
        showError(this, "Choose a submission in the marked work first.");
        return;
    }
    runAction(this, [&] {
        services_.moderation.adjust(user().id(), *submissionId, finalMark_->value(),
                                    str(reason_->text()));
        showInfo(this, "Mark adjusted and recorded in the audit trail.");
        reason_->clear();
        refreshAssignments();
    });
}

void HodWindow::release() {
    const auto assignmentId = selectedId(assignments_);
    if (!assignmentId) {
        showError(this, "Choose an assignment first.");
        return;
    }
    // NFR-06: release can't be undone, so ask first
    if (!confirm(this, "Release the marks? Students will see them, and this cannot be "
                       "undone.")) {
        return;
    }
    runAction(this, [&] {
        const int count = services_.moderation.release(user().id(), *assignmentId);
        showInfo(this, QString::number(count) + " mark(s) released to students.");
        refreshAssignments();
    });
}

void HodWindow::openFile() {
    const auto submissionId = selectedId(markedWork_);
    if (!submissionId) {
        showError(this, "Choose a submission in the marked work first.");
        return;
    }
    runAction(this, [&] {
        openSubmittedFile(this, services_.moderation.latestFile(user().id(), *submissionId));
    });
}

}  // namespace lms
