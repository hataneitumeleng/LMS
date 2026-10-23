#include "domain/SubmissionStatus.h"

#include <stdexcept>

namespace lms {

std::string toString(SubmissionStatus status) {
    switch (status) {
        case SubmissionStatus::Submitted: return "Submitted";
        case SubmissionStatus::Late:      return "Late";
        case SubmissionStatus::Marked:    return "Marked";
        case SubmissionStatus::Moderated: return "Moderated";
        case SubmissionStatus::Released:  return "Released";
    }
    throw std::invalid_argument("Unknown submission status");
}

SubmissionStatus submissionStatusFromString(const std::string& text) {
    if (text == "Submitted") return SubmissionStatus::Submitted;
    if (text == "Late")      return SubmissionStatus::Late;
    if (text == "Marked")    return SubmissionStatus::Marked;
    if (text == "Moderated") return SubmissionStatus::Moderated;
    if (text == "Released")  return SubmissionStatus::Released;
    throw std::invalid_argument("Unknown submission status: " + text);
}

}  // namespace lms
