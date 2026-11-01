#include "gui/StudentWindow.h"

#include "gui/GuiCommon.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <chrono>

namespace lms {

StudentWindow::StudentWindow(Services& services, std::unique_ptr<User> user)
    : RoleWindow(services, std::move(user), "Student") {
    addPage(buildHomePage(), "Home", [this] { refreshHome(); });
    addPage(buildAssignmentsPage(), "My assignments", [this] { refreshAssignments(); });
}

// ---- Home: a summary of what the student needs to know first ----

namespace {

// "2 days 3 h left", "5 h left" or "overdue by 1 day".
QString timeLeft(TimePoint now, TimePoint deadline) {
    using std::chrono::duration_cast;
    using std::chrono::hours;
    const long long h = duration_cast<hours>(deadline > now ? deadline - now : now - deadline)
                            .count();
    const QString amount =
        h >= 24 ? QString::number(h / 24) + (h / 24 == 1 ? " day" : " days") +
                      (h % 24 ? " " + QString::number(h % 24) + " h" : QString())
                : QString::number(h) + " h";
    if (deadline > now) return amount + " left";
    return "overdue by " + amount + " (you can still submit; a late penalty applies)";
}

}  // namespace

QWidget* StudentWindow::buildHomePage() {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    welcome_ = new QLabel;
    welcome_->setObjectName("welcome");
    courses_ = new QLabel;
    courses_->setObjectName("courses");
    courses_->setWordWrap(true);
    summary_ = new QLabel;
    summary_->setObjectName("summary");
    layout->addWidget(welcome_);
    layout->addWidget(courses_);
    layout->addWidget(summary_);

    layout->addWidget(new QLabel("<b>Still to submit</b> (closest deadline first)"));
    toDo_ = makeTable({"No.", "Course", "Title", "Deadline", "Time left"}, page);
    toDo_->setObjectName("toDo");
    layout->addWidget(toDo_);

    auto* open = new QPushButton("Go to My assignments to submit");
    open->setObjectName("openAssignments");
    auto* row = new QHBoxLayout;
    row->addStretch();
    row->addWidget(open);
    layout->addLayout(row);

    connect(open, &QPushButton::clicked, this, [this] { showPage(1); });
    // Double-clicking a row opens that assignment on the My assignments tab.
    connect(toDo_, &QTableWidget::cellDoubleClicked, this, [this](int r) {
        const QString id = toDo_->item(r, 0)->text();
        showPage(1);
        for (int i = 0; i < assignments_->rowCount(); ++i) {
            if (assignments_->item(i, 0)->text() == id) assignments_->selectRow(i);
        }
    });
    return page;
}

void StudentWindow::refreshHome() {
    const TimePoint now = services_.clock.now();
    welcome_->setText("<h2>Welcome, " + qs(user().fullName()).toHtmlEscaped() + "</h2>Today is " +
                      toQt(now).toString("dddd d MMMM yyyy, HH:mm") + ".");

    const auto rows = services_.submissions.listForStudent(user().id());
    QStringList codes;
    int toDo = 0, waiting = 0, released = 0;
    std::vector<const StudentAssignmentRow*> open;
    for (const auto& row : rows) {
        if (!codes.contains(qs(row.courseCode))) codes << qs(row.courseCode);
        if (row.status == "Not submitted") {
            ++toDo;
            open.push_back(&row);
        } else if (row.status.rfind("Released", 0) == 0) {
            ++released;
        } else {
            ++waiting;
        }
    }
    courses_->setText(codes.isEmpty() ? "You are not enrolled in any course yet."
                                      : "Your courses: " + codes.join(", "));
    summary_->setText(QString::number(toDo) + " to submit, " + QString::number(waiting) +
                      " handed in and waiting for a mark, " + QString::number(released) +
                      " with a released mark.");

    std::sort(open.begin(), open.end(), [](const auto* a, const auto* b) {
        return a->effectiveDeadline < b->effectiveDeadline;
    });
    std::vector<QStringList> table;
    for (const auto* row : open) {
        table.push_back({QString::number(row->assignmentId), qs(row->courseCode), qs(row->title),
                         qs(toText(row->effectiveDeadline)),
                         timeLeft(now, row->effectiveDeadline)});
    }
    fillTable(toDo_, table);
}

QWidget* StudentWindow::buildAssignmentsPage() {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    layout->addWidget(new QLabel(
        "Your assignments with your own deadline (including any extension). A mark appears "
        "only after the Head of Department releases it."));

    assignments_ = makeTable(
        {"No.", "Course", "Title", "Deadline", "Status", "Brief", "Feedback"}, page);
    assignments_->setObjectName("assignments");
    layout->addWidget(assignments_);
    auto* briefRow = new QHBoxLayout;
    briefRow->addWidget(new QLabel("If your lecturer attached a file (Brief: yes), open it here "
                                   "or double-click the row."));
    briefRow->addStretch();
    auto* briefButton = new QPushButton("Open assignment brief");
    briefButton->setObjectName("openBrief");
    briefRow->addWidget(briefButton);
    layout->addLayout(briefRow);

    auto* box = new QGroupBox("Submit or resubmit work");
    auto* boxLayout = new QVBoxLayout(box);
    chosen_ = new QLabel("Choose an assignment in the list above.");
    boxLayout->addWidget(chosen_);

    auto* row = new QHBoxLayout;
    file_ = new QLineEdit;
    file_->setObjectName("file");
    file_->setPlaceholderText("Your file, e.g. C:\\Users\\you\\Desktop\\essay.docx");
    auto* browseButton = new QPushButton("Browse...");
    auto* submitButton = new QPushButton("Submit");
    submitButton->setObjectName("submit");
    row->addWidget(file_);
    row->addWidget(browseButton);
    row->addWidget(submitButton);
    boxLayout->addLayout(row);
    layout->addWidget(box);

    connect(assignments_, &QTableWidget::itemSelectionChanged, this, [this] {
        const int r = selectedRow(assignments_);
        if (r < 0) {
            chosen_->setText("Choose an assignment in the list above.");
            return;
        }
        chosen_->setText("Assignment: " + assignments_->item(r, 1)->text() + " " +
                         assignments_->item(r, 2)->text());
    });
    connect(briefButton, &QPushButton::clicked, this, [this] { openBrief(); });
    connect(assignments_, &QTableWidget::cellDoubleClicked, this, [this] { openBrief(); });
    connect(browseButton, &QPushButton::clicked, this, [this] { browse(); });
    connect(submitButton, &QPushButton::clicked, this, [this] { submit(); });
    return page;
}

void StudentWindow::refreshAssignments() {
    std::vector<QStringList> rows;
    for (const auto& row : services_.submissions.listForStudent(user().id())) {
        rows.push_back({QString::number(row.assignmentId), qs(row.courseCode), qs(row.title),
                        qs(toText(row.effectiveDeadline)), qs(row.status),
                        row.hasBrief ? "yes" : "", qs(row.feedback)});
    }
    fillTable(assignments_, rows);
}

void StudentWindow::browse() {
    const QString path = chooseFile(this, "Choose the file to submit");
    if (!path.isEmpty()) file_->setText(path);
}

void StudentWindow::openBrief() {
    const auto assignmentId = selectedId(assignments_);
    if (!assignmentId) {
        showError(this, "Choose an assignment in the list first.");
        return;
    }
    runAction(this, [&] {
        openStoredFile(this, services_.submissions.briefFor(user().id(), *assignmentId));
    });
}

void StudentWindow::submit() {
    const auto assignmentId = selectedId(assignments_);
    if (!assignmentId) {
        showError(this, "Choose an assignment in the list first.");
        return;
    }
    QString path = file_->text().trimmed();
    if (path.size() >= 2 && path.startsWith('"') && path.endsWith('"')) {
        path = path.mid(1, path.size() - 2);  // "Copy as path" adds quotes
    }
    runAction(this, [&] {
        const Submission s = services_.submissions.submit(user().id(), *assignmentId, str(path));
        if (s.status == SubmissionStatus::Late) {
            showInfo(this, "Received, but LATE by " + QString::number(s.daysLate) +
                               " day(s). A penalty will apply.");
        } else {
            showInfo(this, "Received on time. You can resubmit until the deadline.");
        }
        file_->clear();
        refreshAssignments();
    });
}

}  // namespace lms
