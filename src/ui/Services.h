#pragma once
#include "services/AdminService.h"
#include "services/AssignmentService.h"
#include "services/AuditService.h"
#include "services/AuthService.h"
#include "services/BackupService.h"
#include "services/MarkingService.h"
#include "services/ModerationService.h"
#include "services/SubmissionService.h"

namespace lms {

// The services the menus may call. The menus never see a repository or the
// database, only these (strict layering, Section 2).
struct Services {
    AuthService& auth;
    AdminService& admin;
    AssignmentService& assignments;
    SubmissionService& submissions;
    MarkingService& marking;
    ModerationService& moderation;
    AuditService& audit;
    BackupService& backup;
};

}  // namespace lms
