
#pragma once

#include "include.h"

class CFTPFileDownLoader
{
public:
	CFTPFileDownLoader();
	virtual ~CFTPFileDownLoader();

public:
	WZResult DownLoadFiles(DownloaderType type, 
						   std::string strServerIP, 
						   unsigned short PortNum, 
						   std::string strUserName, 
						   std::string strPWD, 
						   std::string strRemotepath, 
						   std::string strlocalpath, 
						   bool bPassiveMode, 
						   CListVersionInfo Version, 
						   std::vector<std::string>	vScriptFiles);

	void	Break();

	FileDownloader* GetFileDownloader(){return m_pFileDownloader;}

private:
	BOOL CreateFolder(std::string strFilePath);
	BOOL m_Break;
	FileDownloader*	m_pFileDownloader;
};
