#pragma once
#include "data/Database.h"
#include "data/SqliteAssignmentRepository.h"
#include "data/SqliteAuditRepository.h"
#include "data/SqliteCourseRepository.h"
#include "data/SqliteSubmissionRepository.h"
#include "data/SqliteUserRepository.h"
#include "domain/UserFactory.h"
#include "services/Argon2Hasher.h"
#include "services/FileStore.h"
#include "services/TenPercentPerDayPolicy.h"
#include "ui/Console.h"
#include "ui/Services.h"

#include <iosfwd>
#include <string>

namespace lms {

// Builds every object once and connects them (constructor injection, Section 3).
// This is the only place that knows the concrete classes; everything else uses
// the interfaces. The members are created in the order they are listed.
class App {
public:
    App(const std::string& databasePath, const std::string& submissionsFolder, std::istream& in,
        std::ostream& out);

    void run();

private:
    void createFirstAdministrator();

    Console io_;
    SystemClock clock_;
    Database db_;
    UserFactory factory_;
    Argon2Hasher hasher_;
    TenPercentPerDayPolicy policy_;
    FileStore store_;

    SqliteUserRepository users_;
    SqliteCourseRepository courses_;
    SqliteAssignmentRepository assignmentRepo_;
    SqliteSubmissionRepository submissionRepo_;
    SqliteAuditRepository auditRepo_;

    AuthService auth_;
    AdminService admin_;
    AssignmentService assignments_;
    SubmissionService submissions_;
    AuditService audit_;
    MarkingService marking_;
    ModerationService moderation_;
    BackupService backup_;
    Services services_;
};

}  // namespace lms
