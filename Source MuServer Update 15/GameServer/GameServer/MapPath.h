// MapPath.h: interface for the CMapPath class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

struct PATH_INFO
{
	int PathNum;
	int PathX[15];
	int PathY[15];
	int CurrentPath;
};

class CMapPath
{
public:
	CMapPath();
	~CMapPath();
	bool FindPath(int startx,int starty,int endx,int endy,bool ErrorCheck);
	bool FindPath2(int startx,int starty,int endx,int endy,bool ErrorCheck);
	bool FindPath3(int startx,int starty,int endx,int endy,bool ErrorCheck);
	void SetMapDimensions(int width,int height,BYTE* MapAttr);
	int GetPath();
	BYTE* GetPathX();
	BYTE* GetPathY();
private:
	int GetDist(int x1,int y1,int x2,int y2);
	int VerifyThatOnPath(int x,int y);
	BOOL CanWeMoveForward(int x,int y);
	BOOL CanWeMoveForward2(int x,int y);
	BOOL CanWeMoveForward3(int x,int y);
	BOOL IsThisSpotOK(int x,int y);
	int FindNextDir(int sx,int sy,int dx,int dy,int dirstart,bool First,bool ErrorCheck);
	int FindNextDir2(int sx,int sy,int dx,int dy,int dirstart,bool First,bool ErrorCheck);
	int FindNextDir3(int sx,int sy,int dx,int dy,int dirstart,bool First,bool ErrorCheck);
private:
	int m_width;
	int m_height;
	int m_NumPath;
	BYTE m_PathX[500];
	BYTE m_PathY[500];
	int m_PathDir[16];
	int m_LastDir;
	int m_NumFails;
	BYTE* m_MapAttr;
	BYTE* m_HitAttr;
	int m_stx;
	int m_sty;
	int m_edx;
	int m_edy;
};
