// LoadData.h: interface for the CLoadData class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CLoadData
{
public:
    CLoadData();
    virtual ~CLoadData();
    void AccessModel(int Type, wchar_t* Dir, wchar_t* FileName, int i = -1);
    void OpenTexture(int Model, wchar_t* SubFolder, int Wrap = GL_REPEAT, int Type = GL_NEAREST, bool Check = true);

public:
};

extern CLoadData gLoadData;
