#include "stdafx.h"

#ifdef _EDITOR

#include "MapObjectImport.h"

#include "Data/DataHandler/LoadData.h"      // gLoadData (AccessModel / OpenTexture)
#include "Render/Models/ZzzBMD.h"           // BMD / Models[] / Texture_t
#include "Core/Globals/_enum.h"             // MODEL_WORLD_OBJECT / MAX_WORLD_OBJECTS
#include "Render/Sprites/GlobalBitmap.h"    // GL_REPEAT / GL_LINEAR
#include "UI/Console/MuEditorConsoleUI.h"

#include <algorithm>
#include <cwctype>
#include <filesystem>

namespace fs = std::filesystem;

namespace Editor::ObjectImport
{
namespace
{
    std::wstring ToLower(std::wstring s)
    {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](wchar_t c) { return (wchar_t)towlower(c); });
        return s;
    }

    std::wstring ObjectFolder(int world)
    {
        return L"Data\\Object" + std::to_wstring(world);
    }

    // The generic model filename for a slot: "Object01".."Object09" then
    // "Object10".."Object160" (matches CLoadData::AccessModel's padding).
    std::wstring ObjectBmdStem(int slot)
    {
        const int number = slot + 1;
        wchar_t buf[16];
        if (number < 10)
            swprintf_s(buf, L"Object0%d", number);
        else
            swprintf_s(buf, L"Object%d", number);
        return buf;
    }

    // First world-object slot that's empty on the current map (no loaded model and
    // no existing generic file to clobber), or -1 if none.
    int FindFreeSlot(int currentWorld)
    {
        const std::wstring folder = ObjectFolder(currentWorld);
        for (int i = 0; i < MAX_WORLD_OBJECTS; ++i)
        {
            if (Models[MODEL_WORLD_OBJECT + i].Meshs != nullptr)
                continue;
            std::error_code ec;
            if (fs::exists(folder + L"\\" + ObjectBmdStem(i) + L".bmd", ec))
                continue;
            return i;
        }
        return -1;
    }

    std::wstring NarrowToWide(const char* s)
    {
        std::wstring w;
        for (const char* p = s; *p; ++p)
            w.push_back((wchar_t)(unsigned char)*p);
        return w;
    }

    // The client stores textures as OZJ/OZT but a BMD references them by .jpg/.tga
    // names. Given a referenced name, return the real on-disk file name.
    std::wstring RealTextureFile(const std::wstring& referenced)
    {
        const fs::path p(referenced);
        const std::wstring ext = ToLower(p.extension().wstring());
        if (ext == L".jpg" || ext == L".jpeg")
            return p.stem().wstring() + L".OZJ";
        if (ext == L".tga")
            return p.stem().wstring() + L".OZT";
        return referenced;  // already OZJ/OZT/OZB/etc.
    }

    void CopyIfPresent(const std::wstring& from, const std::wstring& to, bool overwrite)
    {
        std::error_code ec;
        if (!fs::exists(from, ec))
            return;
        const auto opt = overwrite ? fs::copy_options::overwrite_existing
                                   : fs::copy_options::skip_existing;
        fs::copy_file(from, to, opt, ec);
    }
}

int LoadForPreview(int sourceWorld, const std::wstring& bmdFile)
{
    // Models[] is over-allocated past MAX_MODELS, so this index is always valid
    // memory and sits outside the world-object range (0..159), so it never shows
    // up as a placeable model on the current map.
    const int scratch = MAX_MODELS;
    const std::wstring srcDir = ObjectFolder(sourceWorld) + L"\\";
    const std::wstring srcSub = L"Object" + std::to_wstring(sourceWorld) + L"\\";
    const std::wstring stem   = fs::path(bmdFile).stem().wstring();

    gLoadData.AccessModel(scratch, srcDir.c_str(), stem.c_str(), -1);
    if (Models[scratch].Meshs == nullptr)
        return -1;
    gLoadData.OpenTexture(scratch, srcSub.c_str(), GL_REPEAT, GL_LINEAR, false);
    return scratch;
}

std::vector<std::wstring> ListModelFiles(int sourceWorld)
{
    std::vector<std::wstring> out;
    const fs::path folder = ObjectFolder(sourceWorld);
    std::error_code ec;
    if (!fs::is_directory(folder, ec))
        return out;
    for (const auto& entry : fs::directory_iterator(folder, ec))
    {
        if (entry.is_regular_file() && ToLower(entry.path().extension().wstring()) == L".bmd")
            out.push_back(entry.path().filename().wstring());
    }
    std::sort(out.begin(), out.end());
    return out;
}

int UseModelOnCurrentMap(int currentWorld, int sourceWorld, const std::wstring& bmdFile)
{
    const int slot = FindFreeSlot(currentWorld);
    if (slot < 0)
    {
        g_MuEditorConsoleUI.LogEditor("[MapEditor] Object import failed: no free model slot on this map");
        return -1;
    }

    // srcDir is the full folder; srcSub is the Data-relative form OpenTexture wants.
    const std::wstring srcDir = ObjectFolder(sourceWorld) + L"\\";
    const std::wstring srcSub = L"Object" + std::to_wstring(sourceWorld) + L"\\";
    const std::wstring stem   = fs::path(bmdFile).stem().wstring();

    // Load the model live from the source folder into the free slot, then its
    // textures (from the same folder). This makes it immediately placeable.
    gLoadData.AccessModel(slot, srcDir.c_str(), stem.c_str(), -1);
    if (Models[slot].Meshs == nullptr)
    {
        g_MuEditorConsoleUI.LogEditor("[MapEditor] Object import failed: could not load the .bmd");
        return -1;
    }
    gLoadData.OpenTexture(slot, srcSub.c_str(), GL_REPEAT, GL_LINEAR, false);

    // Copy the model + its textures into the current map's Object folder so the
    // placement survives a reload (the map re-loads Object{slot+1}.bmd from here).
    const std::wstring dstDir = ObjectFolder(currentWorld) + L"\\";
    std::error_code ec;
    fs::create_directories(dstDir, ec);

    CopyIfPresent(srcDir + bmdFile, dstDir + ObjectBmdStem(slot) + L".bmd", /*overwrite*/ true);

    for (int k = 0; k < Models[slot].NumMeshs; ++k)
    {
        const char* fn = Models[slot].Textures[k].FileName;
        if (fn == nullptr || fn[0] == '\0')
            continue;
        const std::wstring real = RealTextureFile(NarrowToWide(fn));
        // Don't clobber an existing current-map texture of the same name.
        CopyIfPresent(srcDir + real, dstDir + real, /*overwrite*/ false);
    }

    char msg[96];
    snprintf(msg, sizeof(msg), "[MapEditor] Imported object into model slot %d", slot);
    g_MuEditorConsoleUI.LogEditor(msg);
    return slot;
}

} // namespace Editor::ObjectImport

#endif // _EDITOR
