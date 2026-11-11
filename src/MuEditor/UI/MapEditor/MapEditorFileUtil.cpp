#include "stdafx.h"

#ifdef _EDITOR

#include "MapEditorFileUtil.h"

#include "UI/Console/MuEditorConsoleUI.h"

#include <cstdio>
#include <filesystem>

#ifdef _WIN32
#include <commdlg.h> // GetOpenFileNameW
#endif

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

bool PickFileToOpen(const wchar_t* filter, const wchar_t* title, std::wstring& outPath)
{
#ifdef _WIN32
    wchar_t file[MAX_PATH] = { 0 };
    OPENFILENAMEW ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = file;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = title;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (!GetOpenFileNameW(&ofn))
        return false;
    outPath = file;
    return true;
#else
    (void)filter;
    (void)title;
    (void)outPath;
    g_MuEditorConsoleUI.LogEditor("[MapEditor] The file picker is only available on Windows.");
    return false;
#endif
}

} // namespace Editor::Files

#endif // _EDITOR
