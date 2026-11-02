#include "gui/LecturerWindow.h"

#include "gui/GuiCommon.h"

#include <QComboBox>
#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>

namespace lms {

namespace {

QDateTimeEdit* makeDateTimeEdit(const QString& name) {
    auto* edit = new QDateTimeEdit(QDateTime::currentDateTime().addDays(7));
    edit->setObjectName(name);
    edit->setDisplayFormat("yyyy-MM-dd HH:mm");
    edit->setCalendarPopup(true);
    return edit;
}

QPushButton* makeButton(const QString& text, const QString& name) {
    auto* button = new QPushButton(text);
    button->setObjectName(name);
    return button;
}

}  // namespace

LecturerWindow::LecturerWindow(Services& services, std::unique_ptr<User> user)
    : RoleWindow(services, std::move(user), "Lecturer") {
    addPage(buildAssignmentsPage(), "Courses and assignments", [this] { refreshCourses(); });
    addPage(buildExtensionsPage(), "Extensions",
            [this] { fillAssignmentCombo(extensionAssignment_); });
    addPage(buildMarkingPage(), "Marking", [this] {
        fillAssignmentCombo(markingAssignment_);
        refreshSubmissions();
    });
}

// ---- Courses and assignments (FR-07) ----

QWidget* LecturerWindow::buildAssignmentsPage() {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);

    auto* top = new QHBoxLayout;
    top->addWidget(new QLabel("Course:"));
    course_ = new QComboBox;
    course_->setObjectName("course");
    top->addWidget(course_, 1);
    layout->addLayout(top);

    assignments_ =
        makeTable({"No.", "Title", "Deadline", "Out of", "Brief", "Description"}, page);
    assignments_->setObjectName("assignments");
    layout->addWidget(assignments_);
    auto* briefRow = new QHBoxLayout;
    briefRow->addWidget(new QLabel("Brief file for the chosen assignment:"));
    briefRow->addStretch();
    QPushButton* openBrief = makeButton("Open brief", "openBrief");
    QPushButton* replaceBrief = makeButton("Attach or replace brief...", "replaceBrief");
    briefRow->addWidget(openBrief);
    briefRow->addWidget(replaceBrief);
    layout->addLayout(briefRow);

    auto* box = new QGroupBox("New assignment for this course");
    auto* form = new QFormLayout(box);
    title_ = new QLineEdit;
    title_->setObjectName("title");
    description_ = new QLineEdit;
    description_->setPlaceholderText("optional");
    deadline_ = makeDateTimeEdit("deadline");
    maxMark_ = new QSpinBox;
    maxMark_->setObjectName("maxMark");
    maxMark_->setRange(1, 100);
    maxMark_->setValue(100);
    auto* briefFile = new QHBoxLayout;
    brief_ = new QLineEdit;
    brief_->setObjectName("brief");
    brief_->setPlaceholderText("optional: instructions, a template... students can open it");
    QPushButton* browse = makeButton("Browse...", "browseBrief");
    briefFile->addWidget(brief_);
    briefFile->addWidget(browse);
    QPushButton* create = makeButton("Create assignment", "createAssignment");
    form->addRow("Title", title_);
    form->addRow("Description", description_);
    form->addRow("Deadline", deadline_);
    form->addRow("Maximum mark", maxMark_);
    form->addRow("Brief file", briefFile);
    form->addRow(create);
    layout->addWidget(box);

    connect(course_, qOverload<int>(&QComboBox::currentIndexChanged), this,
            [this] { runAction(this, [this] { refreshAssignments(); }); });
    connect(create, &QPushButton::clicked, this, [this] { createAssignment(); });
    connect(browse, &QPushButton::clicked, this, [this] {
        const QString path = chooseFile(this, "Choose the brief file");
        if (!path.isEmpty()) brief_->setText(path);
    });
    connect(openBrief, &QPushButton::clicked, this, [this] {
        const int r = selectedRow(assignments_);
        if (r < 0) return showError(this, "Choose an assignment in the list first.");
        const auto id = selectedId(assignments_);
        runAction(this, [&] {
            for (const Assignment& a :
                 services_.assignments.assignmentsFor(user().id(), *comboId(course_))) {
                if (a.id != *id) continue;
                if (a.briefPath.empty()) {
                    showError(this, "This assignment has no brief yet.");
                } else {
                    openStoredFile(this, a.briefPath);
                }
            }
        });
    });
    connect(replaceBrief, &QPushButton::clicked, this, [this] {
        const auto id = selectedId(assignments_);
        if (!id) return showError(this, "Choose an assignment in the list first.");
        const QString path = chooseFile(this, "Choose the brief file");
        if (path.isEmpty()) return;
        runAction(this, [&] {
            services_.assignments.attachBrief(user().id(), *id, str(path));
            showInfo(this, "Brief attached. Students can open it from their assignment list.");
            refreshAssignments();
        });
    });
    return page;
}

void LecturerWindow::refreshCourses() {
    const auto chosen = comboId(course_);
    course_->blockSignals(true);
    course_->clear();
    for (const Course& c : services_.assignments.coursesTaughtBy(user().id())) {
        course_->addItem(qs(c.code + "  " + c.title), c.id);
        if (chosen && *chosen == c.id) course_->setCurrentIndex(course_->count() - 1);
    }
    course_->blockSignals(false);
    refreshAssignments();
}

void LecturerWindow::refreshAssignments() {
    std::vector<QStringList> rows;
    if (const auto courseId = comboId(course_)) {
        for (const Assignment& a : services_.assignments.assignmentsFor(user().id(), *courseId)) {
            rows.push_back({QString::number(a.id), qs(a.title), qs(toText(a.deadline)),
                            QString::number(a.maxMark), a.briefPath.empty() ? "" : "yes",
                            qs(a.description)});
        }
    }
    fillTable(assignments_, rows);
}

void LecturerWindow::createAssignment() {
    const auto courseId = comboId(course_);
    if (!courseId) {
        showError(this, "You are not assigned to any course yet.");
        return;
    }
    QString path = brief_->text().trimmed();
    if (path.size() >= 2 && path.startsWith('"') && path.endsWith('"')) {
        path = path.mid(1, path.size() - 2);  // "Copy as path" adds quotes
    }
    int id = 0;
    const bool created = runAction(this, [&] {
        id = services_.assignments.createAssignment(user().id(), *courseId, str(title_->text()),
                                                    fromQt(deadline_->dateTime()),
                                                    maxMark_->value(), str(description_->text()));
        title_->clear();
        description_->clear();
        refreshAssignments();
    });
    if (!created) return;
    QString message = "Assignment created with number " + QString::number(id) + ".";
    if (!path.isEmpty()) {
        // The assignment exists even if the file can't be attached; it can be attached later.
        const bool attached = runAction(this, [&] {
            services_.assignments.attachBrief(user().id(), id, str(path));
            brief_->clear();
            refreshAssignments();
        });
        message += attached ? " The brief is attached."
                            : " The brief was not attached; use \"Attach or replace brief\".";
    }
    showInfo(this, message);
}

// ---- Extensions (FR-08, BR-03, BR-04) ----

QWidget* LecturerWindow::buildExtensionsPage() {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    layout->addWidget(new QLabel(
        "An extension is at most 14 days after the normal deadline and needs a reason. If the "
        "student already submitted, their days late are worked out again."));

    auto* form = new QFormLayout;
    extensionAssignment_ = new QComboBox;
    extensionAssignment_->setObjectName("extensionAssignment");
    student_ = new QSpinBox;
    student_->setObjectName("student");
    student_->setRange(1, 1000000);
    newDeadline_ = makeDateTimeEdit("newDeadline");
    extensionReason_ = new QLineEdit;
    extensionReason_->setObjectName("extensionReason");
    QPushButton* grant = makeButton("Grant extension", "grant");
    QPushButton* grantClass = makeButton("Extend deadline for whole class", "grantClass");
    form->addRow("Assignment", extensionAssignment_);
    form->addRow("Student number", student_);
    form->addRow("New deadline", newDeadline_);
    form->addRow("Reason", extensionReason_);
    form->addRow(grant);
    form->addRow(grantClass);
    connect(grant, &QPushButton::clicked, this, [this] { grantExtension(); });
    connect(grantClass, &QPushButton::clicked, this, [this] {
    const auto assignmentId = comboId(extensionAssignment_);

    if (!assignmentId) {
        showError(this, "You have no assignments yet.");
        return;
    }

    runAction(this, [&] {
        services_.assignments.extendClassDeadline(
            user().id(),
            *assignmentId,
            fromQt(newDeadline_->dateTime()),
            str(extensionReason_->text())
        );

        showInfo(this, "Deadline extended for the whole class.");
        extensionReason_->clear();
        refreshAssignments();
    });
});
    layout->addLayout(form);
    layout->addStretch();

    connect(grant, &QPushButton::clicked, this, [this] { grantExtension(); });
    return page;
}

void LecturerWindow::grantExtension() {
    const auto assignmentId = comboId(extensionAssignment_);
    if (!assignmentId) {
        showError(this, "You have no assignments yet.");
        return;
    }
    runAction(this, [&] {
        services_.assignments.grantExtension(user().id(), *assignmentId, student_->value(),
                                             fromQt(newDeadline_->dateTime()),
                                             str(extensionReason_->text()));
        showInfo(this, "Extension granted. Days late were worked out again if the work "
                       "was submitted.");
        extensionReason_->clear();
    });
}

// ---- Marking (FR-12, FR-18, BR-01) ----

QWidget* LecturerWindow::buildMarkingPage() {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);

    auto* top = new QHBoxLayout;
    top->addWidget(new QLabel("Assignment:"));
    markingAssignment_ = new QComboBox;
    markingAssignment_->setObjectName("markingAssignment");
    top->addWidget(markingAssignment_, 1);
    layout->addLayout(top);

    submissions_ = makeTable(
        {"No.", "Student no.", "Status", "Days late", "Raw", "Final", "Feedback"}, page);
    submissions_->setObjectName("submissions");
    layout->addWidget(submissions_);
    auto* fileRow = new QHBoxLayout;
    fileRow->addWidget(new QLabel("Choose a submission, then open the student's latest file "
                                  "(or double-click the row)."));
    fileRow->addStretch();
    QPushButton* openButton = makeButton("Open submitted file", "openFile");
    fileRow->addWidget(openButton);
    layout->addLayout(fileRow);

    auto* box = new QGroupBox("Mark the chosen submission");
    auto* form = new QFormLayout(box);
    rawMark_ = new QDoubleSpinBox;
    rawMark_->setObjectName("rawMark");
    rawMark_->setRange(0.0, 100.0);
    rawMark_->setDecimals(1);
    feedback_ = new QLineEdit;
    feedback_->setObjectName("feedback");
    amendReason_ = new QLineEdit;
    amendReason_->setObjectName("amendReason");
    amendReason_->setPlaceholderText("only needed to change a mark");
    auto* buttons = new QHBoxLayout;
    QPushButton* markButton = makeButton("Save mark", "mark");
    QPushButton* amendButton = makeButton("Change my mark", "amend");
    buttons->addWidget(markButton);
    buttons->addWidget(amendButton);
    buttons->addStretch();
    form->addRow("Raw mark", rawMark_);
    form->addRow("Feedback", feedback_);
    form->addRow("Reason for a change", amendReason_);
    form->addRow(buttons);
    layout->addWidget(box);

    connect(markingAssignment_, qOverload<int>(&QComboBox::currentIndexChanged), this,
            [this] { runAction(this, [this] { refreshSubmissions(); }); });
    connect(submissions_, &QTableWidget::itemSelectionChanged, this,
            [this] { showChosenSubmission(); });
    connect(openButton, &QPushButton::clicked, this, [this] { openFile(); });
    connect(submissions_, &QTableWidget::cellDoubleClicked, this, [this] { openFile(); });
    connect(markButton, &QPushButton::clicked, this, [this] { mark(); });
    connect(amendButton, &QPushButton::clicked, this, [this] { amend(); });
    return page;
}

void LecturerWindow::refreshSubmissions() {
    std::vector<QStringList> rows;
    if (const auto assignmentId = comboId(markingAssignment_)) {
        for (const Submission& s : services_.marking.listForMarking(user().id(), *assignmentId)) {
            rows.push_back({QString::number(s.id), QString::number(s.studentId),
                            qs(toString(s.status)), QString::number(s.daysLate),
                            markText(s.rawMark), markText(s.finalMark), qs(s.feedback)});
        }
    }
    fillTable(submissions_, rows);
}

void LecturerWindow::showChosenSubmission() {
    const int r = selectedRow(submissions_);
    if (r < 0) return;
    const QString raw = submissions_->item(r, 4)->text();
    rawMark_->setValue(raw == "-" ? 0.0 : raw.toDouble());
    feedback_->setText(submissions_->item(r, 6)->text());
}

void LecturerWindow::openFile() {
    const auto submissionId = selectedId(submissions_);
    if (!submissionId) {
        showError(this, "Choose a submission in the list first.");
        return;
    }
    runAction(this, [&] {
        openSubmittedFile(this, services_.marking.latestFile(user().id(), *submissionId));
    });
}

void LecturerWindow::mark() {
    const auto submissionId = selectedId(submissions_);
    if (!submissionId) {
        showError(this, "Choose a submission in the list first.");
        return;
    }
    runAction(this, [&] {
        const double final = services_.marking.mark(user().id(), *submissionId,
                                                    rawMark_->value(), str(feedback_->text()));
        showInfo(this, "Mark saved. Final mark after any late penalty: " + markText(final));
        refreshSubmissions();
    });
}

void LecturerWindow::amend() {
    const auto submissionId = selectedId(submissions_);
    if (!submissionId) {
        showError(this, "Choose a submission in the list first.");
        return;
    }
    runAction(this, [&] {
        const double final =
            services_.marking.amend(user().id(), *submissionId, rawMark_->value(),
                                    str(feedback_->text()), str(amendReason_->text()));
        showInfo(this, "Mark changed. New final mark: " + markText(final));
        amendReason_->clear();
        refreshSubmissions();
    });
}

void LecturerWindow::fillAssignmentCombo(QComboBox* combo) {
    const auto chosen = comboId(combo);
    combo->blockSignals(true);
    combo->clear();
    for (const Course& c : services_.assignments.coursesTaughtBy(user().id())) {
        for (const Assignment& a : services_.assignments.assignmentsFor(user().id(), c.id)) {
            combo->addItem(qs(c.code + "  " + a.title + "  (no. " + std::to_string(a.id) + ")"),
                           a.id);
            if (chosen && *chosen == a.id) combo->setCurrentIndex(combo->count() - 1);
        }
    }
    combo->blockSignals(false);
}

}  // namespace lms
