#include "data/Database.h"
#include "data/DatabaseError.h"
#include "data/SqliteAssignmentRepository.h"
#include "data/SqliteCourseRepository.h"
#include "data/SqliteSubmissionRepository.h"
#include "data/SqliteUserRepository.h"
#include "domain/Errors.h"
#include "domain/UserFactory.h"
#include "services/AdminService.h"
#include "services/Argon2Hasher.h"
#include "services/AssignmentService.h"
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

void writeFile(const std::string& path, const std::string& text) {
    std::ofstream(path) << text;
}

int fileCount(const std::string& folder) {
    int count = 0;
    for ([[maybe_unused]] const auto& entry : fs::directory_iterator(folder)) ++count;
    return count;
}

}  // namespace

int main() {
    try {
        // Step 5c demo. The database is in memory, and the files go into the build
        // folder (which git ignores), so nothing is left in your project.
        fs::remove_all("build/demo_submissions");
        fs::create_directories("build/demo_files");
        writeFile("build/demo_files/itu_v1.txt", "Itu's first draft");
        writeFile("build/demo_files/itu_v2.txt", "Itu's final version");
        writeFile("build/demo_files/neo.txt", "Neo's work");
        writeFile("build/demo_files/empty.txt", "");

        lms::Database db(":memory:");
        db.initialiseSchema();
        lms::UserFactory factory;
        lms::SqliteUserRepository users(db, factory);
        lms::SqliteCourseRepository courses(db);
        lms::SqliteAssignmentRepository assignmentRepo(db);
        lms::SqliteSubmissionRepository submissionRepo(db);
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

        // Set-up from steps 5a and 5b.
        int hod = admin.createUser(lms::Role::Lecturer, "mpho", "Mpho Lecturer", "Password1");
        int cs101 = admin.createCourse("CS101", "Programming 1", admin.createDepartment("Science", hod));
        int thabo = admin.createUser(lms::Role::Lecturer, "thabo", "Thabo Lecturer", "Password1");
        int itu = admin.createUser(lms::Role::Student, "itu", "Itumeleng Student", "Password1");
        int neo = admin.createUser(lms::Role::Student, "neo", "Neo Student", "Password1");
        int lindi = admin.createUser(lms::Role::Student, "lindi", "Lindi Student", "Password1");
        int sipho = admin.createUser(lms::Role::Student, "sipho", "Sipho Student", "Password1");
        admin.assignLecturer(cs101, thabo);
        for (int student : {itu, neo, lindi}) admin.enrol(cs101, student);
        int project = assignments.createAssignment(thabo, cs101, "Project 1",
                                                   lms::timeFromText("2026-10-30 17:00:00"), 100);
        std::cout << "Project 1 is due 2026-10-30 17:00:00\n";

        auto show = [&](int student, const std::string& name) {
            lms::Submission s = *submissionRepo.findCurrent(student, project);
            std::cout << "           " << name << ": " << lms::toString(s.status) << ", days late "
                      << s.daysLate << ", versions " << submissionRepo.versions(s.id).size() << "\n";
        };

        std::cout << "1. Submit and resubmit before the deadline (FR-10, FR-11, BR-05):\n";
        clock.set("2026-10-28 10:00:00");
        attempt("28 Oct 10:00, itu submits itu_v1.txt",
                [&] { submissions.submit(itu, project, "build/demo_files/itu_v1.txt"); });
        clock.set("2026-10-30 16:00:00");
        attempt("30 Oct 16:00, itu resubmits itu_v2.txt",
                [&] { submissions.submit(itu, project, "build/demo_files/itu_v2.txt"); });
        show(itu, "itu");
        for (const auto& v : submissionRepo.versions(submissionRepo.findCurrent(itu, project)->id)) {
            std::cout << "           version " << v.versionNo << ": " << v.filePath << " at "
                      << lms::toText(v.submittedAt) << "\n";
        }

        std::cout << "2. Refused submissions (BR-06, NFR-07):\n";
        clock.set("2026-10-31 09:00:00");
        attempt("31 Oct 09:00, itu resubmits after the deadline",
                [&] { submissions.submit(itu, project, "build/demo_files/itu_v1.txt"); });
        attempt("neo submits a file that doesn't exist",
                [&] { submissions.submit(neo, project, "build/demo_files/missing.txt"); });
        attempt("neo submits an empty file",
                [&] { submissions.submit(neo, project, "build/demo_files/empty.txt"); });
        attempt("sipho, who is not enrolled, submits",
                [&] { submissions.submit(sipho, project, "build/demo_files/neo.txt"); });

        std::cout << "3. Late first submissions are allowed (FR-10, BR-02, BR-06):\n";
        attempt("31 Oct 09:00, neo submits for the first time",
                [&] { submissions.submit(neo, project, "build/demo_files/neo.txt"); });
        show(neo, "neo");
        clock.set("2026-11-02 10:00:00");
        attempt("2 Nov 10:00, lindi submits for the first time",
                [&] { submissions.submit(lindi, project, "build/demo_files/neo.txt"); });
        show(lindi, "lindi");

        std::cout << "4. Extensions after a late submission (BR-04):\n";
        attempt("thabo gives neo and lindi until 1 Nov 17:00", [&] {
            assignments.grantExtension(thabo, project, neo, lms::timeFromText("2026-11-01 17:00:00"),
                                       "Clinic visit");
            assignments.grantExtension(thabo, project, lindi,
                                       lms::timeFromText("2026-11-01 17:00:00"), "Funeral");
        });
        show(neo, "neo");
        show(lindi, "lindi");

        std::cout << "5. Once work is marked (marking itself comes in step 5d):\n";
        {
            lms::Submission s = *submissionRepo.findCurrent(itu, project);
            s.status = lms::SubmissionStatus::Marked;
            s.rawMark = 70;
            s.finalMark = 70;
            s.feedback = "Good work";
            s.markedBy = thabo;
            submissionRepo.save(s);
        }
        clock.set("2026-10-29 09:00:00");
        attempt("itu resubmits", [&] {
            submissions.submit(itu, project, "build/demo_files/itu_v1.txt");
        });
        attempt("thabo gives itu an extension", [&] {
            assignments.grantExtension(thabo, project, itu, lms::timeFromText("2026-11-01 17:00:00"),
                                       "Sick");
        });

        std::cout << "6. neo's assignment list (FR-09, BR-07):\n";
        for (const auto& row : submissions.listForStudent(neo)) {
            std::cout << "           " << row.courseCode << "  " << row.title << "  due "
                      << lms::toText(row.effectiveDeadline) << "  " << row.status << "\n";
        }
        std::cout << "           itu sees: " << submissions.listForStudent(itu)[0].status << "\n";

        std::cout << "7. A database error after the file was copied (Figure 6, NFR-07):\n";
        int lerato = admin.createUser(lms::Role::Student, "lerato", "Lerato Student", "Password1");
        admin.enrol(cs101, lerato);
        // Make the next insert into submission_version fail, to act like a disk error.
        db.prepare("CREATE TEMP TRIGGER fail BEFORE INSERT ON submission_version "
                   "BEGIN SELECT RAISE(ABORT, 'simulated disk error'); END").step();
        std::cout << "           files before: " << fileCount("build/demo_submissions") << "\n";
        attempt("lerato submits", [&] {
            submissions.submit(lerato, project, "build/demo_files/neo.txt");
        });
        std::cout << "           files after: " << fileCount("build/demo_submissions")
                  << ", lerato's submission saved: "
                  << (submissionRepo.findCurrent(lerato, project) ? "yes" : "no") << "\n";

        std::cout << "8. Start-up clean-up of stray files (Week 3, Section 4.3):\n";
        writeFile("build/demo_submissions/sub99_v1.txt", "left behind by a crash");
        std::cout << "           stray files removed: "
                  << store.removeOrphans(submissionRepo.allFilePaths()) << "\n";
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
