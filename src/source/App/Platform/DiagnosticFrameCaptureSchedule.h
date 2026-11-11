#pragma once

#include <cstdint>

namespace mu
{

class DiagnosticFrameCaptureSchedule
{
public:
    explicit DiagnosticFrameCaptureSchedule(std::uint64_t targetFrame);

    bool BeforeFrame();
    bool AfterFrame() const;
    void Finish();

private:
    std::uint64_t targetFrame_ = 0;
    std::uint64_t currentFrame_ = 0;
    bool requestIssued_ = false;
    bool finished_ = false;
};

} // namespace mu
