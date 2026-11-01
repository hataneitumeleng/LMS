#include "ui/HodMenu.h"

namespace lms {

HodMenu::HodMenu(Console& io, Services& services, const User& user)
    : Menu(io, services), user_(user) {}

std::string HodMenu::title() const { return "Head of Department menu: " + user_.fullName(); }

std::vector<std::string> HodMenu::options() const {
    return {"View assignments in my department", "View marked work for an assignment",
            "Approve all marks for an assignment", "Adjust a mark", "Release an assignment's marks",
            "View the audit trail", "Show a submitted file", "Change my password"};
}

void HodMenu::handle(int choice) {
    switch (choice) {
        case 1: viewAssignments(); break;
        case 2: viewMarkedWork(); break;
        case 3: approveAll(); break;
        case 4: adjustMark(); break;
        case 5: releaseMarks(); break;
        case 6: showAudit(services_.audit.entriesFor(user_)); break;
        case 7:
            showFile(services_.moderation.latestFile(user_.id(), io_.number("Submission number")));
            break;
        case 8: changePassword(user_); break;
    }
}

void HodMenu::viewAssignments() {
    const auto rows = services_.moderation.assignmentsInDepartment(user_.id());
    if (rows.empty()) {
        io_.print("No assignments in your department yet.");
        return;
    }
    for (const auto& row : rows) {
        io_.print("[" + std::to_string(row.assignment.id) + "] " + row.courseCode + "  " +
                  row.assignment.title + "  " + std::to_string(row.submissions) +
                  " submission(s), " + std::to_string(row.notModerated) + " not moderated");
    }
}

void HodMenu::viewMarkedWork() {
    const int assignmentId = io_.number("Assignment number");
    const auto submissions = services_.moderation.listForModeration(user_.id(), assignmentId);
    if (submissions.empty()) {
        io_.print("Nothing has been marked yet.");
        return;
    }
    for (const Submission& s : submissions) {
        io_.print("[" + std::to_string(s.id) + "] student " + std::to_string(s.studentId) + "  " +
                  toString(s.status) + "  raw " + formatMark(s.rawMark) + "  days late " +
                  std::to_string(s.daysLate) + "  final " + formatMark(s.finalMark));
        io_.print("      Feedback: " + s.feedback);
    }
}

void HodMenu::approveAll() {
    const int assignmentId = io_.number("Assignment number");
    if (!io_.confirm("Approve every marked submission without changes?")) return;
    const int count = services_.moderation.approveAll(user_.id(), assignmentId);
    io_.print(std::to_string(count) + " submission(s) approved.");
}

void HodMenu::adjustMark() {
    const int submissionId = io_.number("Submission number");
    const double final = io_.decimal("New final mark");
    const std::string reason = io_.text("Reason (it goes into the audit trail)");
    services_.moderation.adjust(user_.id(), submissionId, final, reason);
    io_.print("Mark adjusted and recorded in the audit trail.");
}

void HodMenu::releaseMarks() {
    const int assignmentId = io_.number("Assignment number");
    // NFR-06: release can't be undone, so ask first
    if (!io_.confirm("Release the marks? Students will see them, and this cannot be undone")) {
        return;
    }
    const int count = services_.moderation.release(user_.id(), assignmentId);
    io_.print(std::to_string(count) + " mark(s) released to students.");
}

}  // namespace lms
