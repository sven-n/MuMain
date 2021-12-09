

#ifndef _LAUNCHERHELPER_H_
#define _LAUNCHERHELPER_H_

#pragma once

#pragma warning(disable : 4786)
#include <string>

#pragma pack(push, 1)
typedef struct __LAUNCHINFO {
	std::string ip;			//. ip address to connect
	unsigned long port;		//. port number to connect
} WZLAUNCHINFO, *LPWZLAUNCHINFO;
#pragma pack(pop)

bool wzRegisterConnectionKey();		//. Register connection key.
void wzUnregisterConnectionKey();	//. Unregister connection key
unsigned long wzGetConnectionKey();
/* Connection Key가 없다면 실패한다.(return 0xFFFFFFFF) */

bool wzPushLaunchInfo(const WZLAUNCHINFO& LaunchInfo);
/* 접속키가 없거나 접속키가 등록된후 5초 이내에 함수가 호출되지 못했을경우 실패한다. */
/* 함수가 호출된 뒤에는 접속키가 등록해제된다 */

bool wzPopLaunchInfo(WZLAUNCHINFO& LaunchInfo);				
/* LaunchInfo가 없다면 실패한다. */
/* 함수가 호출된 뒤에는 접속정보가 삭제된다 */

/*
	// example

	//. Mu online launcher side
	WZLAUNCHINFO LaunchInfo;
	LaunchInfo.ip = "10.1.210.3";
	LaunchInfo.port = 44405;
	if(wzPushLaunchInfo(LaunchInfo)) {		//. encryption management
		//. success
		//. launching Mu update application
	}

	//. Mu online application side
	WZLAUNCHINFO LaunchInfo;
	if(wzPopLaunchInfo(LaunchInfo)) {
		//. success
		//. connect to LaunchInfo.ip:LaunchInfo.port
	}
	else {
		//. failed.
		//. exit process
	}

*/

#endif // _LAUNCHERHELPER_H_