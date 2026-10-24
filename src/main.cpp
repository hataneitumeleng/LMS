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
#include "services/BackupService.h"
#include "services/MarkingService.h"
#include "services/ModerationService.h"
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
        // Step 5e demo. The database is in memory, and the files go into the build
        // folder (which git ignores), so nothing is left in your project.
        fs::remove_all("build/demo_submissions");
        fs::remove_all("build/demo_backup");
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
        lms::ModerationService moderation(db, submissionRepo, assignmentRepo, courses, users, audit,
                                          clock);
        lms::BackupService backup(db, "build/demo_submissions");

        // Set-up from steps 5a to 5d: two departments, each with its own HoD.
        int adminId = admin.createUser(lms::Role::Administrator, "admin", "System Admin", "Password1");
        int mpho = admin.createUser(lms::Role::Lecturer, "mpho", "Mpho HoD", "Password1");
        int cs101 = admin.createCourse("CS101", "Programming 1", admin.createDepartment("Science", mpho));
        int sello = admin.createUser(lms::Role::Lecturer, "sello", "Sello HoD", "Password1");
        admin.createDepartment("Maths", sello);
        int thabo = admin.createUser(lms::Role::Lecturer, "thabo", "Thabo Lecturer", "Password1");
        int itu = admin.createUser(lms::Role::Student, "itu", "Itumeleng Student", "Password1");
        int neo = admin.createUser(lms::Role::Student, "neo", "Neo Student", "Password1");
        int lindi = admin.createUser(lms::Role::Student, "lindi", "Lindi Student", "Password1");
        admin.assignLecturer(cs101, thabo);
        for (int student : {itu, neo, lindi}) admin.enrol(cs101, student);
        int project = assignments.createAssignment(thabo, cs101, "Project 1",
                                                   lms::timeFromText("2026-10-30 17:00:00"), 100);
        clock.set("2026-10-30 12:00:00");
        int ituSub = submissions.submit(itu, project, "build/demo_files/work.txt").id;
        int lindiSub = submissions.submit(lindi, project, "build/demo_files/work.txt").id;
        clock.set("2026-10-31 09:00:00");
        int neoSub = submissions.submit(neo, project, "build/demo_files/work.txt").id;
        clock.set("2026-11-05 10:00:00");
        marking.mark(thabo, ituSub, 70, "Good structure");
        marking.mark(thabo, neoSub, 60, "Test more cases");
        clock.set("2026-11-06 10:00:00");

        auto show = [&](const std::string& heading) {
            std::cout << "           " << heading << "\n";
            for (const auto& s : submissionRepo.findByAssignment(project)) {
                std::cout << "             " << users.findById(s.studentId)->username() << ": "
                          << lms::toString(s.status) << ", final " << markText(s.finalMark)
                          << "\n";
            }
        };

        std::cout << "1. Who may moderate Project 1 (CS101, Science) (FR-02):\n";
        attempt("mpho, HoD of Science, lists the marked work", [&] {
            std::cout << "           " << moderation.listForModeration(mpho, project).size()
                      << " marked submissions (lindi is not marked yet)\n";
        });
        attempt("sello, HoD of Maths, lists them", [&] { moderation.listForModeration(sello, project); });
        attempt("thabo, the lecturer, approves", [&] { moderation.approveAll(thabo, project); });

        std::cout << "2. Approve all (FR-14):\n";
        attempt("mpho approves all", [&] {
            std::cout << "           approved " << moderation.approveAll(mpho, project) << "\n";
        });
        show("Project 1 now:");

        std::cout << "3. Release is refused until everything is moderated (FR-15):\n";
        attempt("mpho releases Project 1", [&] { moderation.release(mpho, project); });

        std::cout << "4. Adjusting a mark (FR-14, BR-08):\n";
        marking.mark(thabo, lindiSub, 55, "Clear, but short");
        std::cout << "           thabo marks lindi 55\n";
        attempt("mpho adjusts lindi to 150", [&] {
            moderation.adjust(mpho, lindiSub, 150, "Too generous?");
        });
        attempt("mpho adjusts lindi with no reason", [&] { moderation.adjust(mpho, lindiSub, 60, ""); });
        attempt("mpho adjusts lindi to 60 with a reason", [&] {
            moderation.adjust(mpho, lindiSub, 60, "Marking scheme gives 60 for section B");
        });
        show("Project 1 now:");

        std::cout << "5. Release (FR-15, FR-16):\n";
        attempt("mpho releases Project 1", [&] {
            std::cout << "           released " << moderation.release(mpho, project) << "\n";
        });
        auto row = submissions.listForStudent(itu)[0];
        std::cout << "           itu sees: " << row.title << ", " << row.status << ", feedback \""
                  << row.feedback << "\"\n";

        std::cout << "6. A released mark can still be corrected by the HoD, with a reason (BR-08):\n";
        attempt("mpho changes neo from 50 to 55", [&] {
            moderation.adjust(mpho, neoSub, 55, "Penalty applied twice by mistake");
        });
        show("Project 1 now:");

        std::cout << "7. The audit trail (approve all added no entries, FR-17):\n";
        for (const auto& e : audit.entriesFor(*users.findById(adminId))) {
            std::cout << "           " << users.findById(e.userId)->username() << "  "
                      << markText(e.oldValue) << " -> " << markText(e.newValue) << "  \"" << e.reason
                      << "\"\n";
        }

        std::cout << "8. Backup (FR-20, NFR-11):\n";
        attempt("admin backs up to build/demo_backup", [&] { backup.backup("build/demo_backup"); });
        {
            // Restoring means opening the copy as the database, so check that it opens
            // and holds the same data.
            lms::Database copy("build/demo_backup/lms.db");
            lms::SqliteSubmissionRepository copiedSubmissions(copy);
            int files = 0;
            for ([[maybe_unused]] const auto& f : fs::directory_iterator("build/demo_backup/submissions")) {
                ++files;
            }
            std::cout << "           the copy has " << copiedSubmissions.findByAssignment(project).size()
                      << " submissions and " << files << " files\n";
        }
        attempt("admin backs up to the same folder again", [&] { backup.backup("build/demo_backup"); });
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
