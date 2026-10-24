#include "data/Database.h"
#include "data/SqliteAssignmentRepository.h"
#include "data/SqliteAuditRepository.h"
#include "data/SqliteCourseRepository.h"
#include "data/SqliteSubmissionRepository.h"
#include "data/SqliteUserRepository.h"
#include "domain/Errors.h"
#include "domain/UserFactory.h"
#include "services/AdminService.h"
#include "services/Argon2Hasher.h"
#include "services/AssignmentService.h"
#include "services/AuditService.h"
#include "services/MarkingService.h"
#include "services/SubmissionService.h"
#include "services/TenPercentPerDayPolicy.h"

#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>

namespace fs = std::filesystem;

namespace {

// A clock the demo can move, so the output is the same every run.
class DemoClock : public lms::IClock {
public:
    void set(const std::string& text) { now_ = lms::timeFromText(text); }
    lms::TimePoint now() const override { return now_; }

private:
    lms::TimePoint now_;
};

// Runs one action and prints the error message if a rule refuses it.
void attempt(const std::string& description, const std::function<void()>& action) {
    try {
        action();
        std::cout << "  OK:      " << description << "\n";
    } catch (const std::exception& e) {
        std::cout << "  Refused: " << description << " -> " << e.what() << "\n";
    }
}

std::string markText(const std::optional<double>& mark) {
    if (!mark) return "-";
    std::string text = std::to_string(*mark);
    return text.substr(0, text.find('.') + 2);  // one decimal place, e.g. 70.0
}

}  // namespace

int main() {
    try {
        // Step 5d demo. The database is in memory, and the files go into the build
        // folder (which git ignores), so nothing is left in your project.
        fs::remove_all("build/demo_submissions");
        fs::create_directories("build/demo_files");
        std::ofstream("build/demo_files/work.txt") << "Some work";

        lms::Database db(":memory:");
        db.initialiseSchema();
        lms::UserFactory factory;
        lms::SqliteUserRepository users(db, factory);
        lms::SqliteCourseRepository courses(db);
        lms::SqliteAssignmentRepository assignmentRepo(db);
        lms::SqliteSubmissionRepository submissionRepo(db);
        lms::SqliteAuditRepository auditRepo(db);
        lms::Argon2Hasher hasher;
        lms::TenPercentPerDayPolicy policy;
        lms::FileStore store("build/demo_submissions");
        DemoClock clock;
        clock.set("2026-10-24 09:00:00");
        lms::AdminService admin(db, users, courses, factory, hasher);
        lms::AssignmentService assignments(db, assignmentRepo, courses, users, submissionRepo,
                                           policy, clock);
        lms::SubmissionService submissions(db, submissionRepo, assignmentRepo, courses, policy,
                                           clock, store);
        lms::AuditService audit(auditRepo, clock);
        lms::MarkingService marking(db, submissionRepo, assignmentRepo, courses, policy, audit);

        // Set-up from steps 5a to 5c.
        int adminId = admin.createUser(lms::Role::Administrator, "admin", "System Admin", "Password1");
        int mpho = admin.createUser(lms::Role::Lecturer, "mpho", "Mpho Lecturer", "Password1");
        int cs101 = admin.createCourse("CS101", "Programming 1", admin.createDepartment("Science", mpho));
        int thabo = admin.createUser(lms::Role::Lecturer, "thabo", "Thabo Lecturer", "Password1");
        int lerato = admin.createUser(lms::Role::Lecturer, "lerato", "Lerato Lecturer", "Password1");
        int sello = admin.createUser(lms::Role::Lecturer, "sello", "Sello Lecturer", "Password1");
        int itu = admin.createUser(lms::Role::Student, "itu", "Itumeleng Student", "Password1");
        int neo = admin.createUser(lms::Role::Student, "neo", "Neo Student", "Password1");
        int lindi = admin.createUser(lms::Role::Student, "lindi", "Lindi Student", "Password1");
        admin.assignLecturer(cs101, thabo);
        admin.assignLecturer(cs101, lerato);
        for (int student : {itu, neo, lindi}) admin.enrol(cs101, student);
        int project = assignments.createAssignment(thabo, cs101, "Project 1",
                                                   lms::timeFromText("2026-10-30 17:00:00"), 100);
        clock.set("2026-10-30 12:00:00");
        int ituSub = submissions.submit(itu, project, "build/demo_files/work.txt").id;
        clock.set("2026-10-31 09:00:00");
        int neoSub = submissions.submit(neo, project, "build/demo_files/work.txt").id;
        clock.set("2026-11-04 09:00:00");
        int lindiSub = submissions.submit(lindi, project, "build/demo_files/work.txt").id;
        clock.set("2026-11-05 10:00:00");

        auto show = [&](int submissionId, const std::string& name) {
            lms::Submission s = *submissionRepo.findById(submissionId);
            std::cout << "           " << name << ": " << lms::toString(s.status) << ", days late "
                      << s.daysLate << ", raw " << markText(s.rawMark) << ", final "
                      << markText(s.finalMark) << ", feedback \"" << s.feedback << "\"\n";
        };

        std::cout << "1. Submissions to mark for Project 1 (out of 100):\n";
        for (const auto& s : marking.listForMarking(thabo, project)) {
            std::cout << "           submission " << s.id << ": " << lms::toString(s.status)
                      << ", days late " << s.daysLate << "\n";
        }

        std::cout << "2. thabo marks them (FR-12, FR-13, BR-01):\n";
        attempt("itu 120 out of 100", [&] { marking.mark(thabo, ituSub, 120, "Great"); });
        attempt("itu 70 with no feedback", [&] { marking.mark(thabo, ituSub, 70, ""); });
        attempt("itu 70, on time", [&] { marking.mark(thabo, ituSub, 70, "Good structure"); });
        attempt("neo 60, 1 day late", [&] { marking.mark(thabo, neoSub, 60, "Test more cases"); });
        attempt("lindi 80, 5 days late", [&] { marking.mark(thabo, lindiSub, 80, "Strong, but late"); });
        show(ituSub, "itu");
        show(neoSub, "neo");
        show(lindiSub, "lindi");

        std::cout << "3. Other refusals (FR-02):\n";
        attempt("itu again with mark()", [&] { marking.mark(thabo, ituSub, 75, "Again"); });
        attempt("sello (not CS101's lecturer) views the list",
                [&] { marking.listForMarking(sello, project); });

        std::cout << "4. Amending a mark (FR-18):\n";
        attempt("thabo changes itu to 75 with a reason", [&] {
            marking.amend(thabo, ituSub, 75, "Good structure", "Missed the appendix at first");
        });
        attempt("thabo changes neo with no reason",
                [&] { marking.amend(thabo, neoSub, 65, "Test more cases", " "); });
        attempt("thabo changes neo with empty feedback",
                [&] { marking.amend(thabo, neoSub, 65, "", "Re-checked"); });
        attempt("lerato (also CS101) changes thabo's mark",
                [&] { marking.amend(lerato, neoSub, 65, "Fine", "I disagree"); });
        show(ituSub, "itu");
        {
            // Pretend the HoD moderated neo's mark (ModerationService comes in step 5e).
            lms::Submission s = *submissionRepo.findById(neoSub);
            s.status = lms::SubmissionStatus::Moderated;
            s.moderatedBy = mpho;
            s.moderatedAt = clock.now();
            submissionRepo.save(s);
        }
        attempt("thabo changes neo after moderation",
                [&] { marking.amend(thabo, neoSub, 65, "Test more cases", "Re-checked"); });

        std::cout << "5. The audit trail (FR-17, NFR-08):\n";
        for (const auto& e : audit.entriesFor(*users.findById(adminId))) {
            std::cout << "           submission " << e.submissionId << "  "
                      << users.findById(e.userId)->username() << "  " << lms::toText(e.changedAt)
                      << "  " << markText(e.oldValue) << " -> " << markText(e.newValue) << "  \""
                      << e.reason << "\"\n";
        }
        std::cout << "           HoD mpho sees " << audit.entriesFor(*users.findById(mpho)).size()
                  << " entries for Science\n";
        attempt("student itu views the audit trail",
                [&] { audit.entriesFor(*users.findById(itu)); });

        std::cout << "6. If the audit entry can't be saved, the mark isn't saved either (NFR-08):\n";
        db.prepare("CREATE TEMP TRIGGER fail BEFORE INSERT ON audit_entry "
                   "BEGIN SELECT RAISE(ABORT, 'simulated disk error'); END").step();
        attempt("thabo changes itu to 90",
                [&] { marking.amend(thabo, ituSub, 90, "Good structure", "Re-checked"); });
        show(ituSub, "itu");

        std::cout << "7. What itu sees before release (BR-07, FR-16):\n";
        std::cout << "           " << submissions.listForStudent(itu)[0].title << ": "
                  << submissions.listForStudent(itu)[0].status << "\n";
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
