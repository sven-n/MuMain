#include "stdafx.h"

#ifdef _EDITOR

#include "MapEditorFileUtil.h"

#include "UI/Console/MuEditorConsoleUI.h"

#include <cstdio>
#include <filesystem>

namespace fs = std::filesystem;

namespace Editor::Files
{

std::wstring MirrorNextToExe(const std::wstring& savedPath, int world)
{
    std::error_code ec;
    const fs::path src(savedPath);
    if (!fs::exists(src, ec))
        return {};

    // Mirror into a World{N}\ folder next to Main.exe, keeping the original file
    // name, so the whole folder can be dropped straight into src\bin\Data\.
    const fs::path dstDir = L"World" + std::to_wstring(world);
    fs::create_directories(dstDir, ec);

    const fs::path dst = dstDir / src.filename();
    fs::copy_file(src, dst, fs::copy_options::overwrite_existing, ec);
    if (ec)
    {
        g_MuEditorConsoleUI.LogEditor("[MapEditor] Could not copy the saved file next to Main.exe");
        return {};
    }

    char msg[192];
    snprintf(msg, sizeof(msg), "[MapEditor] Also saved a copy next to Main.exe: %ls", dst.wstring().c_str());
    g_MuEditorConsoleUI.LogEditor(msg);
    return dst.wstring();
}

} // namespace Editor::Files

#endif // _EDITOR
