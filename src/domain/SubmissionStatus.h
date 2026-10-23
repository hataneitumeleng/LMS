#pragma once
#include <string>

namespace lms {

// The five submission statuses from Week 2 (Figure 4 state machine).
enum class SubmissionStatus { Submitted, Late, Marked, Moderated, Released };

// The text stored in submission.status (must match the CHECK in schema.sql).
std::string toString(SubmissionStatus status);

// Converts database text back to the enum. Throws std::invalid_argument for unknown text.
SubmissionStatus submissionStatusFromString(const std::string& text);

}  // namespace lms
