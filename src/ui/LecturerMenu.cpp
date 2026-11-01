#include "ui/LecturerMenu.h"

#include <filesystem>

namespace lms {

LecturerMenu::LecturerMenu(Console& io, Services& services, const User& user)
    : Menu(io, services), user_(user) {}

std::string LecturerMenu::title() const { return "Lecturer menu: " + user_.fullName(); }

std::vector<std::string> LecturerMenu::options() const {
    return {"View my courses and assignments", "Create an assignment", "Grant an extension",
            "View submissions for an assignment", "Mark a submission", "Amend my mark",
            "Show a submitted file", "Attach or replace an assignment brief",
            "Change my password"};
}

void LecturerMenu::handle(int choice) {
    switch (choice) {
        case 1: viewCourses(); break;
        case 2: createAssignment(); break;
        case 3: grantExtension(); break;
        case 4: viewSubmissions(); break;
        case 5: markSubmission(); break;
        case 6: amendMark(); break;
        case 7:
            showFile(services_.marking.latestFile(user_.id(), io_.number("Submission number")));
            break;
        case 8: attachBrief(); break;
        case 9: changePassword(user_); break;
    }
}

void LecturerMenu::viewCourses() {
    const auto courses = services_.assignments.coursesTaughtBy(user_.id());
    if (courses.empty()) {
        io_.print("You are not assigned to any course yet.");
        return;
    }
    for (const Course& course : courses) {
        io_.print("[" + std::to_string(course.id) + "] " + course.code + " " + course.title);
        for (const Assignment& a : services_.assignments.assignmentsFor(user_.id(), course.id)) {
            io_.print("      [" + std::to_string(a.id) + "] " + a.title + "  due " +
                      toText(a.deadline) + "  out of " + std::to_string(a.maxMark));
            if (!a.briefPath.empty()) {
                io_.print("          Brief: " + std::filesystem::absolute(a.briefPath).string());
            }
        }
    }
}

void LecturerMenu::createAssignment() {
    const int courseId = io_.number("Course number");
    const std::string title = io_.text("Title");
    const std::string description = io_.text("Description (optional)");
    const TimePoint deadline = io_.dateTime("Deadline");
    const int maxMark = io_.number("Maximum mark (1 to 100)");
    const std::string brief = io_.path("Brief file for students (optional, Enter for none)");
    const int id = services_.assignments.createAssignment(user_.id(), courseId, title, deadline,
                                                          maxMark, description);
    io_.print("Assignment created with number " + std::to_string(id) + ".");
    if (!brief.empty()) {
        services_.assignments.attachBrief(user_.id(), id, brief);
        io_.print("Brief attached.");
    }
}

void LecturerMenu::attachBrief() {
    const int assignmentId = io_.number("Assignment number");
    const std::string path = io_.path("Full path of the brief file");
    services_.assignments.attachBrief(user_.id(), assignmentId, path);
    io_.print("Brief attached. Students see it in their assignment list.");
}

void LecturerMenu::grantExtension() {
    const int assignmentId = io_.number("Assignment number");
    const int studentId = io_.number("Student number");
    const TimePoint newDeadline = io_.dateTime("New deadline");
    const std::string reason = io_.text("Reason");
    services_.assignments.grantExtension(user_.id(), assignmentId, studentId, newDeadline, reason);
    io_.print("Extension granted. Days late were worked out again if the work was submitted.");
}

void LecturerMenu::viewSubmissions() {
    const int assignmentId = io_.number("Assignment number");
    const auto submissions = services_.marking.listForMarking(user_.id(), assignmentId);
    if (submissions.empty()) {
        io_.print("No submissions yet.");
        return;
    }
    for (const Submission& s : submissions) {
        io_.print("[" + std::to_string(s.id) + "] student " + std::to_string(s.studentId) + "  " +
                  toString(s.status) + "  days late " + std::to_string(s.daysLate) + "  raw " +
                  formatMark(s.rawMark) + "  final " + formatMark(s.finalMark));
    }
}

void LecturerMenu::markSubmission() {
    const int submissionId = io_.number("Submission number");
    const double raw = io_.decimal("Raw mark");
    const std::string feedback = io_.text("Feedback");
    const double final = services_.marking.mark(user_.id(), submissionId, raw, feedback);
    io_.print("Mark saved. Final mark after any late penalty: " + formatMark(final));
}

void LecturerMenu::amendMark() {
    const int submissionId = io_.number("Submission number");
    const double raw = io_.decimal("New raw mark");
    const std::string feedback = io_.text("Feedback");
    const std::string reason = io_.text("Reason for the change");
    const double final =
        services_.marking.amend(user_.id(), submissionId, raw, feedback, reason);
    io_.print("Mark changed. New final mark: " + formatMark(final));
}

}  // namespace lms
