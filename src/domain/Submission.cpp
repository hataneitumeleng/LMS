#include "domain/Submission.h"

#include "domain/Errors.h"

namespace lms {

bool Submission::isOpen() const {
    return status == SubmissionStatus::Submitted || status == SubmissionStatus::Late;
}

bool Submission::canResubmit(TimePoint now, TimePoint deadline) const {
    return isOpen() && now <= deadline;
}

void Submission::setLateness(int days) {
    if (!isOpen()) throw ValidationError("This submission has already been marked");
    daysLate = days;
    status = days > 0 ? SubmissionStatus::Late : SubmissionStatus::Submitted;
}

void Submission::recordMark(double raw, double final, const std::string& feedbackText,
                            int lecturerId) {
    if (!isOpen() && status != SubmissionStatus::Marked) {
        throw ValidationError("This mark has been moderated and can only be changed by the HoD");
    }
    rawMark = raw;
    finalMark = final;
    feedback = feedbackText;
    markedBy = lecturerId;
    status = SubmissionStatus::Marked;
}

}  // namespace lms
