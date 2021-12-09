//************************************************************************
//
// Decompiled by @myheart, @synth3r
// <https://forum.ragezone.com/members/2000236254.html>
//
//
// FILE: Path.cpp
//
//

#include "stdafx.h"  
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "Path.h"

#include <fstream>
#include <crtdbg.h>
#include <strsafe.h>

TCHAR* Path::GetCurrentFullPath(TCHAR* szPath)
{
	if(!szPath) return 0;

	GetModuleFileName(0,szPath,MAX_PATH);

	return szPath;
}

TCHAR* Path::GetCurrentDirectory(TCHAR* szPath)
{
	if(!szPath) return 0;

	GetModuleFileName(0,szPath,MAX_PATH);

	char* chr = _tcsrchr(szPath,'\\');

	if(!chr) return 0;

	(*chr) = 0;

	return szPath;
}

TCHAR* Path::GetCurrentFileName(TCHAR* szPath)
{
	if(!szPath) return 0;

	GetModuleFileName(0,szPath,MAX_PATH);

	return Path::GetFileName(szPath);
}

TCHAR* Path::SetDirString(TCHAR* szPath)
{
	if(!szPath) return 0;

	if((*szPath))
	{
		std::size_t len = 0;
		StringCchLengthA(szPath,MAX_PATH,&len);

		if(szPath[len-1]!='\\')
		{
			szPath[len] = '\\';
			szPath[len+1] = 0;
		}
	}
	else
	{
		szPath[0] = '\\';
		szPath[1] = 0;
	}

	return szPath;
}

TCHAR* Path::ClearDirString(TCHAR* szPath)
{
	if(!szPath|| !(*szPath)) return szPath;

	std::size_t len = 0;
	StringCchLengthA(szPath,MAX_PATH,&len);

	if(len>0&&(szPath[len-1]=='\\'||szPath[len-1]=='/'||szPath[len-1]=='"'))
	{
		szPath[len-1] = 0;
	}
	
	if(len>=2&&(szPath[len-2]=='\\'||szPath[len-2]=='/'||szPath[len-2]=='"'))
	{
		szPath[len-2] = 0;
	}

	if(szPath[0]=='"')
	{
		StringCchCopy(szPath,MAX_PATH,szPath+1);
	}

	return szPath;
}

TCHAR* Path::GetDirectory(TCHAR* szPath)
{
	if(!szPath||!(*szPath)) return szPath;

	char* chr = _tcsrchr(szPath,'\\');

	if(!chr)
		chr = _tcsrchr(szPath,'/');

	if(chr)
		(*chr) = 0;
	else
		StringCchCopy(szPath,MAX_PATH,".");

	return szPath;
}

TCHAR* Path::GetFileName(TCHAR* szPath)
{
	if(!szPath||!(*szPath)) return szPath;

	char* chr = _tcsrchr(szPath,'\\');

	if(!chr)
		chr = _tcsrchr(szPath,'/');

	if(chr)
		StringCchCopy(szPath,MAX_PATH,szPath+1);

	return szPath;
}

TCHAR* Path::ChangeSlashToBackSlash(TCHAR* szPath)
{
	if(!szPath||!(*szPath)) return szPath;

	std::size_t len = 0;
	StringCchLengthA(szPath,MAX_PATH,&len);

	for(std::size_t n = 0;n<len;n++)
	{
		if(szPath[n]=='\\')
			szPath[n] = '/';
	}

	return szPath;
}

TCHAR* Path::ChangeBackSlashToSlash(TCHAR* szPath)
{
	if(!szPath||!(*szPath)) return szPath;

	std::size_t len = 0;
	StringCchLengthA(szPath,MAX_PATH,&len);

	for(std::size_t n = 0;n<len;n++)
	{
		if(szPath[n]=='/')
			szPath[n] = '\\';
	}

	return szPath;
}

BOOL			Path::ReadFileLastLine(TCHAR* szFile,TCHAR* szLastLine)
{
	std::ifstream ifs(szFile,std::ifstream::in|std::ifstream::binary);

	char buff[1024] = {0};

	if(szFile&&szLastLine&&*szFile&&ifs.is_open())
	{
		std::size_t len = 0;

		while(!ifs.eof())
		{
			ifs.getline(buff,sizeof(buff));

			len = 0;
			StringCchLength(buff,sizeof(buff),&len);

			if(len>1)
				StringCchCopy(szLastLine,sizeof(buff),buff);
		}

		ifs.close();

		len = 0;
		StringCchLength(szLastLine,1024,&len);

		if(len>1)
		{
			return 1;
		}
	}

	return 0;
}

BOOL			Path::WriteNewFile(TCHAR* szFile,TCHAR* szText,INT nTextSize)
{
	if(!szFile||!szText) return 0;

	std::ofstream ofs(szFile,std::ofstream::out|std::ofstream::trunc|std::ofstream::binary);

	if(ofs.is_open())
	{
		ofs.seekp(0,std::ofstream::end);
		ofs.write(szText,nTextSize);
		ofs.close();

		return 1;
	}

	return 0;
}

BOOL			Path::CreateDirectorys(TCHAR* szFilePath,BOOL bIsFile)

{
	if(!szFilePath||!(*szFilePath)) return 0;

	char PathName[MAX_PATH] = {0};
	char buff2[MAX_PATH] = {0};

	StringCchCopy(PathName,sizeof(PathName),szFilePath);

	if(bIsFile)
		Path::GetDirectory(PathName);

	if(CreateDirectory(PathName,0)==0)
	{
		StringCchCopy(buff2,sizeof(buff2),PathName);

		char* chr1 = _tcsrchr(buff2,'\\');
		char* chr2 = _tcschr(buff2,'\\');

		if(!chr1||!chr2||chr1==chr2)
			return 0;

		(*chr1) = 0;

		if(!Path::CreateDirectorys(buff2,0))
			return 0;

		return CreateDirectory(PathName,0);
	}

	return 0;
}
#endif