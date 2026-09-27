// LoadData.h: interface for the CLoadData class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <span>
#include <string>

class CLoadData
{
public:
    CLoadData();
    virtual ~CLoadData();
    void AccessModel(int Type, const wchar_t* Dir, const wchar_t* FileName, int i = -1);
    void OpenTexture(int Model, const wchar_t* SubFolder, int Wrap = GL_REPEAT, int Type = GL_NEAREST,
                     bool Check = true);
    // Loads each texture of the model from the first of the folders (below
    // Data\, each ending with '\') that has it.
    void OpenTexture(int Model, std::span<const std::wstring> SubFolders, int Wrap = GL_REPEAT, int Type = GL_NEAREST);

public:
};

extern CLoadData gLoadData;
