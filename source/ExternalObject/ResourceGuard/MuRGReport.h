
#ifndef _MURGREPORT_H_
#define _MURGREPORT_H_

#pragma once

#include "RGReportObj.h"
#include "./Utilities/Log/ErrorReport.h"

class CMuRGReport : public ResourceGuard::IRGReportObj
{
	bool m_bErrorWriting, m_bMsgWriting;
public:
	CMuRGReport() : m_bErrorWriting(true), m_bMsgWriting(true) {}
	~CMuRGReport() {}
	
	void EnableErrorWriting() { m_bErrorWriting = true; }
	void DisableErrorWriting() { m_bErrorWriting = false; }
	
	void EnableMsgWriting() { m_bMsgWriting = true; }
	void DisableMsgWriting() { m_bMsgWriting = false; }
	
	void WriteMessage(const std::string& msg, DWORD dwMsgType = ResourceGuard::MSG_TYPE_NORMAL, long lParam = 0)
	{
		if((m_bErrorWriting && dwMsgType == ResourceGuard::MSG_TYPE_ERROR) 
			|| (m_bMsgWriting && dwMsgType == ResourceGuard::MSG_TYPE_NORMAL))
		{
			std::string output;
			
			if((lParam & ResourceGuard::MSG_PARAM_HEAD) == ResourceGuard::MSG_PARAM_HEAD)
				output = "[ResourceGuard] ";
			
			switch(dwMsgType)
			{
			case ResourceGuard::MSG_TYPE_ERROR:
				output += "Error: ";
				break;
			}
			
			output += msg;
			
			if((lParam & ResourceGuard::MSG_PARAM_ENDL) == ResourceGuard::MSG_PARAM_ENDL)
				output += "\r\n";

			g_ErrorReport.Write(const_cast<char*>(output.c_str()));
		}
	}
};

#endif /* _MURGREPORT_H_ */