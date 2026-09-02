#include "ScreenshotCaptureState.h"

#include <utility>

bool ScreenshotCaptureState::Begin(std::wstring fileName, std::wstring message)
{
    if (pending_)
    {
        return false;
    }

    fileName_ = std::move(fileName);
    message_ = std::move(message);
    pending_ = true;
    return true;
}

bool ScreenshotCaptureState::HasPending() const
{
    return pending_;
}

const std::wstring& ScreenshotCaptureState::FileName() const
{
    return fileName_;
}

const std::wstring& ScreenshotCaptureState::Message() const
{
    return message_;
}

void ScreenshotCaptureState::Clear()
{
    fileName_.clear();
    message_.clear();
    pending_ = false;
}
