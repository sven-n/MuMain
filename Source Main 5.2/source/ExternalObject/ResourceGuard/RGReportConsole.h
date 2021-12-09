
#ifndef _RGREPORTCONSOLE_H_
#define _RGREPORTCONSOLE_H_

#pragma once

#include "RGReportObj.h"

#include <iostream>

namespace ResourceGuard {
	
	class CRGReportConsole : public IRGReportObj
	{
		bool m_bErrorWriting, m_bMsgWriting;
	public:
		CRGReportConsole() : m_bErrorWriting(true), m_bMsgWriting(true) {}
		~CRGReportConsole() {}

		void EnableErrorWriting() { m_bErrorWriting = true; }
		void DisableErrorWriting() { m_bErrorWriting = false; }

		void EnableMsgWriting() { m_bMsgWriting = true; }
		void DisableMsgWriting() { m_bMsgWriting = false; }

		void WriteMessage(const std::string& msg, DWORD dwMsgType = MSG_TYPE_NORMAL, long lParam = 0)
		{
			if((m_bErrorWriting && dwMsgType == MSG_TYPE_ERROR) 
				|| (m_bMsgWriting && dwMsgType == MSG_TYPE_NORMAL))
			{
				std::string output;
				
				if((lParam & MSG_PARAM_HEAD) == MSG_PARAM_HEAD)
					output = "[ResourceGuard] ";
				
				switch(dwMsgType)
				{
//				case MSG_TYPE_NORMAL:
//					output += "Msg: ";
//					break;
				case MSG_TYPE_ERROR:
					output += "Error: ";
					break;
				}
				
				output += msg;
				std::cout << output.c_str();
				
				if((lParam & MSG_PARAM_ENDL) == MSG_PARAM_ENDL)
					std::cout << std::endl;
			}
		}
	};
}

#endif /* _RGREPORTCONSOLE_H_ */