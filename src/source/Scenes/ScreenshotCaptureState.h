#pragma once

#include <string>

class ScreenshotCaptureState
{
public:
    [[nodiscard]] bool Begin(std::wstring fileName, std::wstring message);
    [[nodiscard]] bool HasPending() const;
    [[nodiscard]] const std::wstring& FileName() const;
    [[nodiscard]] const std::wstring& Message() const;
    void Clear();

private:
    std::wstring fileName_;
    std::wstring message_;
    bool pending_ = false;
};
