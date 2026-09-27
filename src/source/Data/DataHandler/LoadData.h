// LoadData.h: interface for the CLoadData class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <span>
#include <string>
#include <vector>

// A texture of a model that none of the texture folders has.
struct TextureProblem
{
    int mesh = 0;
    // The file name as the model has it, e.g. "Sword01.jpg".
    std::wstring fileName;
    // The path of an already loaded texture with that name, which is used
    // instead; empty when there is none and the mesh has no texture.
    std::wstring usedInstead;
    // Not a .jpg or .tga file, so it was not loaded.
    bool unsupportedType = false;
};

class CLoadData
{
public:
    CLoadData();
    virtual ~CLoadData();
    // Returns false when the model file could not be opened.
    bool AccessModel(int Type, const wchar_t* Dir, const wchar_t* FileName, int i = -1);
    // A texture the folder does not have is shown as an error.
    void OpenTexture(int Model, const wchar_t* SubFolder, int Wrap = GL_REPEAT, int Type = GL_NEAREST,
                     bool Check = true);
    // Loads each texture of the model from the first of the folders (below
    // Data\, each ending with '\') that has it. A texture none of them has is
    // shown as an error.
    void OpenTexture(int Model, std::span<const std::wstring> SubFolders, int Wrap = GL_REPEAT, int Type = GL_NEAREST);
    // Like above, but the textures none of the folders has are added to
    // Problems instead of being shown.
    void OpenTexture(int Model, std::span<const std::wstring> SubFolders, std::vector<TextureProblem>& Problems,
                     int Wrap = GL_REPEAT, int Type = GL_NEAREST);

private:
    void OpenModelTextures(int Model, std::span<const std::wstring> SubFolders, int Wrap, int Type,
                           std::vector<TextureProblem>* Problems);
    void RememberModelFile(int Model, const std::wstring& path);
    // The file AccessModel opened into the slot, for error messages.
    std::wstring GetModelFile(int Model) const;

    std::vector<std::wstring> m_modelFiles;
};

extern CLoadData gLoadData;
