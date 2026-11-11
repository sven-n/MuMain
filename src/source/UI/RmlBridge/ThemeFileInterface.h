#pragma once

#include <RmlUi/Core/FileInterface.h>

namespace UI::RmlBridge
{
// Serves inherited files and expands RCSS tokens from the selected theme's chain.
// RmlUi links and caches retain the original virtual path for child overrides.
class ThemeFileInterface final : public Rml::FileInterface
{
public:
    Rml::FileHandle Open(const Rml::String& path) override;
    void Close(Rml::FileHandle file) override;
    size_t Read(void* buffer, size_t size, Rml::FileHandle file) override;
    bool Seek(Rml::FileHandle file, long offset, int origin) override;
    size_t Tell(Rml::FileHandle file) override;
};
} // namespace UI::RmlBridge
