#pragma once

#include <RmlUi/Core/FileInterface.h>

namespace UI::RmlBridge
{
// Serves themed RCSS with token(name) expanded while leaving RmlUi's links and
// stylesheet cache keyed by the original file path.
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
