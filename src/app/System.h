#pragma once
#include "app/Services.h"
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

#include <string>

namespace lms {

// The composition root (Section 3): builds every object once and connects them through
// their constructors. It is the only class that knows the concrete classes, and both
// front ends (the console menus and the Qt windows) start from it.
// The members are created in the order they are listed.
class System {
public:
    System(const std::string& databasePath, const std::string& submissionsFolder);

    System(const System&) = delete;
    System& operator=(const System&) = delete;

    Services& services() { return services_; }

private:
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
