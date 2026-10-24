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

void Submission::moderate(double final, int hodId, TimePoint now) {
    if (isOpen()) throw ValidationError("This work has not been marked yet");
    finalMark = final;
    moderatedBy = hodId;
    moderatedAt = now;
    if (status == SubmissionStatus::Marked) status = SubmissionStatus::Moderated;
}

void Submission::release() {
    if (status != SubmissionStatus::Moderated) {
        throw ValidationError("Only moderated work can be released");
    }
    status = SubmissionStatus::Released;
}

}  // namespace lms
