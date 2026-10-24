#include "ui/StudentMenu.h"

namespace lms {

StudentMenu::StudentMenu(Console& io, Services& services, const User& user)
    : Menu(io, services), user_(user) {}

std::string StudentMenu::title() const { return "Student menu: " + user_.fullName(); }

std::vector<std::string> StudentMenu::options() const {
    return {"View my assignments, deadlines and released marks", "Submit or resubmit work",
            "Change my password"};
}

void StudentMenu::handle(int choice) {
    switch (choice) {
        case 1: viewAssignments(); break;
        case 2: submitWork(); break;
        case 3: changePassword(user_); break;
    }
}

void StudentMenu::viewAssignments() {
    const auto rows = services_.submissions.listForStudent(user_.id());
    if (rows.empty()) {
        io_.print("You have no assignments yet.");
        return;
    }
    for (const auto& row : rows) {
        io_.print("[" + std::to_string(row.assignmentId) + "] " + row.courseCode + "  " +
                  row.title + "  due " + toText(row.effectiveDeadline) + "  " + row.status);
        if (!row.feedback.empty()) io_.print("      Feedback: " + row.feedback);
    }
}

void StudentMenu::submitWork() {
    const int assignmentId = io_.number("Assignment number (in [ ] in the list)");
    const std::string path = io_.path("Full path of your file");
    const Submission submission = services_.submissions.submit(user_.id(), assignmentId, path);
    if (submission.status == SubmissionStatus::Late) {
        io_.print("Received, but LATE by " + std::to_string(submission.daysLate) +
                  " day(s). A penalty will apply.");
    } else {
        io_.print("Received on time. You can resubmit until the deadline.");
    }
}

}  // namespace lms
