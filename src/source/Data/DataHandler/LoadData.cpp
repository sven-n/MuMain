// LoadData.cpp: implementation of the CLoadData class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LoadData.h"

#include "Render/Sprites/GlobalBitmap.h"

#include "Render/Models/ZzzBMD.h"
#include "Render/Textures/ZzzTexture.h"
#include "Core/Text/Utf8.h"
#include "Data/GameData/ItemData/ItemTextureFiles.h"

#include <string>

CLoadData gLoadData;

CLoadData::CLoadData() // OK
{
}

CLoadData::~CLoadData() // OK
{
}

namespace
{
// "Sword" and 1 give "Sword01.bmd", without a number (-1) "Sword.bmd". Built
// as a string, because the names can come from the item model files and have
// any length.
std::wstring GetModelFileName(const wchar_t* FileName, int i)
{
    std::wstring name = FileName;
    if (i != -1)
    {
        if (i < 10)
        {
            name += L'0';
        }
        name += std::to_wstring(i);
    }
    return name + L".bmd";
}
} // namespace

bool CLoadData::AccessModel(int Type, const wchar_t* Dir, const wchar_t* FileName, int i)
{
    const std::wstring Name = GetModelFileName(FileName, i);

    bool Success = false;

    Models[Type].m_iBMDSeqID = Type;

    Success = Models[Type].Open2(Dir, Name.c_str());
    // Only a file that was opened is the model of the slot now.
    RememberModelFile(Type, Success ? std::wstring(Dir) + Name : std::wstring());

    if (Success == false)
    {
        g_ErrorReport.Write(L"AccessModel failed: %ls%ls (Type=%d)\r\n", Dir, Name.c_str(), Type);

        if (wcscmp(FileName, L"Monster") == 0 || wcscmp(FileName, L"Player") == 0 || wcscmp(FileName, L"PlayerTest") == 0 || wcscmp(FileName, L"Angel") == 0)
        {
            const std::wstring Text = Name + L" file does not exist.";
            MessageBox(g_hWnd, Text.c_str(), NULL, MB_OK);
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

// "Data\Item\x.jpg", and the other folders that were searched:
// "Data\Item\x.jpg (also searched Data\Player\)".
std::wstring DescribeSearchedPaths(std::span<const std::wstring> subFolders, const std::wstring& textureFileName)
{
    std::wstring text = GetTexturePath(subFolders.front(), textureFileName);
    for (size_t i = 1; i < subFolders.size(); ++i)
    {
        text += (i == 1 ? L" (also searched " : L", ") + (TextureRootFolder + subFolders[i]);
    }
    return subFolders.size() > 1 ? text + L")" : text;
}

void ShowMissingTexture(const std::wstring& modelFile, int model, std::span<const std::wstring> subFolders,
                        const std::wstring& textureFileName)
{
    const std::wstring message =
        L"OpenTexture Failed: " + DescribeSearchedPaths(subFolders, textureFileName) + L" of " + modelFile;
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

void CLoadData::OpenTexture(int Model, std::span<const std::wstring> SubFolders, int Wrap, int Type)
{
    OpenModelTextures(Model, SubFolders, Wrap, Type, nullptr);
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

        const bool hidden = Data::Items::IsHiddenTexture(fileName);
        // Other types keep the texture they have; item models report them.
        if (Problems != nullptr && !hidden && !Data::Items::GetStoredTextureFileName(fileName))
        {
            Problems->push_back({i, textureFileName, L"", true});
            continue;
        }

        if (hidden)
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
            ShowMissingTexture(GetModelFile(Model), Model, SubFolders, textureFileName);
        }
    }
}
