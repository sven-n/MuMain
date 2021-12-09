// LoadData.h: interface for the CLoadData class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CLoadData
{
public:
	CLoadData();
	virtual ~CLoadData();
	void AccessModel(int Type, char* Dir, char* FileName,int i=-1);
	void OpenTexture(int Model, char* SubFolder, int Wrap=GL_REPEAT, int Type=GL_NEAREST,bool Check=true);

	public:
};

extern CLoadData gLoadData;
