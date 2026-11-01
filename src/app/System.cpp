#include "app/System.h"

namespace lms {

System::System(const std::string& databasePath, const std::string& submissionsFolder)
    : db_(databasePath),
      store_(submissionsFolder),
      users_(db_, factory_),
      courses_(db_),
      assignmentRepo_(db_),
      submissionRepo_(db_),
      auditRepo_(db_),
      auth_(users_, hasher_, clock_),
      admin_(db_, users_, courses_, factory_, hasher_),
      assignments_(db_, assignmentRepo_, courses_, users_, submissionRepo_, policy_, clock_,
                   store_),
      submissions_(db_, submissionRepo_, assignmentRepo_, courses_, policy_, clock_, store_),
      audit_(auditRepo_, clock_),
      marking_(db_, submissionRepo_, assignmentRepo_, courses_, policy_, audit_),
      moderation_(db_, submissionRepo_, assignmentRepo_, courses_, users_, audit_, clock_),
      backup_(db_, submissionsFolder),
      services_{auth_,       admin_, assignments_, submissions_, marking_, moderation_,
                audit_,      backup_, clock_} {
    db_.initialiseSchema();
}

}  // namespace lms
