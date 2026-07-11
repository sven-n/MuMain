#include "ScreenshotCaptureState.h"

#include <utility>

bool ScreenshotCaptureState::Begin(std::wstring fileName, std::wstring message, bool includesMessage)
{
    if (pending_)
    {
        return false;
    }

    fileName_ = std::move(fileName);
    message_ = std::move(message);
    includesMessage_ = includesMessage;
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

bool ScreenshotCaptureState::IncludesMessage() const
{
    return includesMessage_;
}

void ScreenshotCaptureState::Clear()
{
    fileName_.clear();
    message_.clear();
    includesMessage_ = false;
    pending_ = false;
}
