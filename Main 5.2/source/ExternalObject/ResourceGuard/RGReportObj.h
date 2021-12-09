
#ifndef _RGREPORTOBJ_H_
#define _RGREPORTOBJ_H_

#pragma once

#pragma warning(disable : 4786)
#include <string>

namespace ResourceGuard {

	enum {
		MSG_TYPE_NONE = 0,
		MSG_TYPE_NORMAL,
		MSG_TYPE_ERROR,

		MSG_PARAM_HEAD = 0x80000000,
		MSG_PARAM_ENDL = 0x0000000a
	};

	class IRGReportObj
	{
	public:
		virtual void WriteMessage(const std::string& msg, DWORD dwMsgType = MSG_TYPE_NORMAL, long lParam = 0) = 0;
	};
}

#endif /* _RGREPORTOBJ_H_ */