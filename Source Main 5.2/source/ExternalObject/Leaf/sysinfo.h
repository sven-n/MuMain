
#ifndef _SYSINFO_H_
#define _SYSINFO_H_

#include "regkey.h"

#pragma warning(disable : 4786)
#include <string>

namespace leaf {

	//. Interface Declaration

	bool GetOSInfoString(OUT std::string& osinfo);		//. Get OS infomation
	void GetCPUInfoString(OUT std::string& cpuinfo);	//. Get CPU infomation (Do NOT support duel CPU.)
}

#endif // _SYSINFO_H_