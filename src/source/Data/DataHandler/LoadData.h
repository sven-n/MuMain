// LoadData.h: interface for the CLoadData class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CLoadData
{
public:
    CLoadData();
    virtual ~CLoadData();
    void AccessModel(int Type, const wchar_t* Dir, const wchar_t* FileName, int i = -1);
    void OpenTexture(int Model, const wchar_t* SubFolder, int Wrap = GL_REPEAT, int Type = GL_NEAREST,
                     bool Check = true);

public:
};

extern CLoadData gLoadData;
