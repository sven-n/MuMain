#include "DiagnosticFrameCaptureSchedule.h"

namespace mu
{

DiagnosticFrameCaptureSchedule::DiagnosticFrameCaptureSchedule(const std::uint64_t targetFrame)
    : targetFrame_(targetFrame)
{
}

bool DiagnosticFrameCaptureSchedule::BeforeFrame()
{
    if (targetFrame_ == 0 || finished_)
    {
        return false;
    }

    ++currentFrame_;
    if (requestIssued_ || currentFrame_ != targetFrame_)
    {
        return false;
    }

    requestIssued_ = true;
    return true;
}

bool DiagnosticFrameCaptureSchedule::AfterFrame() const
{
    return requestIssued_ && !finished_;
}

void DiagnosticFrameCaptureSchedule::Finish()
{
    finished_ = true;
}

} // namespace mu
