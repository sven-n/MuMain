// LoadData.cpp: implementation of the CLoadData class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LoadData.h"

#include "Render/Sprites/GlobalBitmap.h"

#include "Render/Models/ZzzBMD.h"
#include "Render/Textures/ZzzTexture.h"
#include "Core/Text/Utf8.h"

#include <string>

CLoadData gLoadData;

CLoadData::CLoadData() // OK
{
}

CLoadData::~CLoadData() // OK
{
}

bool CLoadData::AccessModel(int Type, const wchar_t* Dir, const wchar_t* FileName, int i)
{
    wchar_t Name[64];
    if (i == -1)
        mu_swprintf(Name, L"%ls.bmd", FileName);
    else if (i < 10)
        mu_swprintf(Name, L"%ls0%d.bmd", FileName, i);
    else
        mu_swprintf(Name, L"%ls%d.bmd", FileName, i);

    RememberModelFile(Type, std::wstring(Dir) + Name);

    bool Success = false;

    Models[Type].m_iBMDSeqID = Type;

    Success = Models[Type].Open2(Dir, Name);

    if (Success == false)
    {
        g_ErrorReport.Write(L"AccessModel failed: %ls%ls (Type=%d)\r\n", Dir, Name, Type);

        if (wcscmp(FileName, L"Monster") == 0 || wcscmp(FileName, L"Player") == 0 || wcscmp(FileName, L"PlayerTest") == 0 || wcscmp(FileName, L"Angel") == 0)
        {
            wchar_t Text[256];
            mu_swprintf(Text, L"%ls file does not exist.", Name);
            MessageBox(g_hWnd, Text, NULL, MB_OK);
            SendMessage(g_hWnd, WM_DESTROY, 0, 0);
        }
    }
    return Success;
}

void CLoadData::RememberModelFile(int Model, const std::wstring& path)
{
    if (Model < 0)
    {
        return;
    }
    if (static_cast<size_t>(Model) >= m_modelFiles.size())
    {
        m_modelFiles.resize(static_cast<size_t>(Model) + 1);
    }
    m_modelFiles[Model] = path;
}

std::wstring CLoadData::GetModelFile(int Model) const
{
    if (Model >= 0 && static_cast<size_t>(Model) < m_modelFiles.size() && !m_modelFiles[Model].empty())
    {
        return m_modelFiles[Model];
    }
    return Core::Text::FromUtf8(Models[Model].Name);
}

namespace
{
constexpr const wchar_t* TextureRootFolder = L"Data\\";

std::wstring GetTexturePath(const std::wstring& subFolder, const std::wstring& textureFileName)
{
    return TextureRootFolder + subFolder + textureFileName;
}

bool IsHiddenTexture(const char* fileName)
{
    return fileName[0] == 'h' && fileName[1] == 'i' && fileName[2] == 'd';
}

// Loads the texture from the first folder that has it. Only .tga and .jpg
// files are loaded; for other files `current` is kept.
GLuint LoadTextureFromFolders(const std::wstring& textureFileName, std::span<const std::wstring> subFolders, int wrap,
                              int filter, GLuint current)
{
    wchar_t extension[_MAX_EXT] = {0};
    _wsplitpath(textureFileName.c_str(), NULL, NULL, NULL, extension);
    const wchar_t type = static_cast<wchar_t>(towlower(extension[1]));
    if (type != L't' && type != L'j')
    {
        return current;
    }

    for (const std::wstring& subFolder : subFolders)
    {
        // TGA textures are always sharp; the filter applies to JPG textures.
        const GLuint index =
            Bitmaps.LoadImage(GetTexturePath(subFolder, textureFileName), type == L't' ? GL_NEAREST : filter, wrap);
        if (index != BITMAP_UNKNOWN)
        {
            return index;
        }
    }
    return BITMAP_UNKNOWN;
}

void MarkSkinAndHair(const char* fileName, const std::wstring& textureFileName, GLuint textureIndex)
{
    const bool isSkin = (fileName[0] == 's' && fileName[1] == 'k' && fileName[2] == 'i') ||
                        !wcsnicmp(textureFileName.c_str(), L"level", 5);
    const bool isHair = fileName[0] == 'h' && fileName[1] == 'a' && fileName[2] == 'i' && fileName[3] == 'r';
    if (!isSkin && !isHair)
    {
        return;
    }

    BITMAP_t* pBitmap =
        textureIndex != BITMAP_UNKNOWN ? Bitmaps.FindTexture(textureIndex) : Bitmaps.FindTextureByName(textureFileName);
    if (pBitmap)
    {
        pBitmap->IsSkin = isSkin;
        pBitmap->IsHair = isHair;
    }
}

// A texture that no folder has may already be loaded from another folder,
// e.g. by another model; that one is used.
BITMAP_t* UseLoadedTexture(const std::wstring& textureFileName)
{
    BITMAP_t* pBitmap = Bitmaps.FindTextureByName(textureFileName);
    if (pBitmap)
    {
        Bitmaps.LoadImage(pBitmap->BitmapIndex, pBitmap->FileName);
    }
    return pBitmap;
}

void ShowMissingTexture(const std::wstring& modelFile, int model, const std::wstring& path)
{
    const std::wstring message = L"OpenTexture Failed: " + path + L" of " + modelFile;
    g_ErrorReport.Write(L"%ls (Model=%d)\r\n", message.c_str(), model);
#ifdef FOR_WORK
    PopUpErrorCheckMsgBox(message.c_str());
#else  // FOR_WORK
    PopUpErrorCheckMsgBox(message.c_str(), true);
#endif // FOR_WORK
}
} // namespace

void CLoadData::OpenTexture(int Model, const wchar_t* SubFolder, int Wrap, int Type, bool Check)
{
    const std::wstring subFolder = SubFolder;
    OpenModelTextures(Model, std::span<const std::wstring>(&subFolder, 1), Wrap, Type, nullptr);
}

void CLoadData::OpenTexture(int Model, std::span<const std::wstring> SubFolders, std::vector<TextureProblem>& Problems,
                            int Wrap, int Type)
{
    OpenModelTextures(Model, SubFolders, Wrap, Type, &Problems);
}

void CLoadData::OpenModelTextures(int Model, std::span<const std::wstring> SubFolders, int Wrap, int Type,
                                  std::vector<TextureProblem>* Problems)
{
    if (SubFolders.empty())
    {
        return;
    }

    BMD* pModel = &Models[Model];
    for (int i = 0; i < pModel->NumMeshs; i++)
    {
        const char* fileName = pModel->Textures[i].FileName;
        const std::wstring textureFileName = Core::Text::FromUtf8(fileName);
        GLuint& textureIndex = pModel->IndexTexture[i];

        if (IsHiddenTexture(fileName))
        {
            textureIndex = BITMAP_HIDE;
        }
        else
        {
            textureIndex = LoadTextureFromFolders(textureFileName, SubFolders, Wrap, Type, textureIndex);
        }

        MarkSkinAndHair(fileName, textureFileName, textureIndex);

        if (textureIndex != BITMAP_UNKNOWN)
        {
            continue;
        }

        const BITMAP_t* loaded = UseLoadedTexture(textureFileName);
        textureIndex = loaded != nullptr ? loaded->BitmapIndex : BITMAP_UNKNOWN;
        if (Problems != nullptr)
        {
            Problems->push_back({i, textureFileName, loaded != nullptr ? loaded->FileName : L""});
        }
        else if (loaded == nullptr)
        {
            ShowMissingTexture(GetModelFile(Model), Model, GetTexturePath(SubFolders.front(), textureFileName));
        }
    }
}
