// ProtectAuto.cpp: implementation of the CProtectAuto class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef CSK_MOD_PROTECT_AUTO_V1

#include "ProtectAuto.h"
#include "NewUISystem.h"
#include <Tlhelp32.h>
#ifdef KJH_MOD_PROTECT_AUTO_FIND_USB
#include "wsclientinline.h"
#endif // KJH_MOD_PROTECT_AUTO_FIND_USB
#ifdef YDG_MOD_PROTECT_AUTO_FIND_USB
#include <setupapi.h>
#pragma comment( lib, "Setupapi" )
#endif	// YDG_MOD_PROTECT_AUTO_FIND_USB

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProtectAuto* CProtectAuto::ms_pProtectAuto = NULL;
CProtectAuto* CProtectAuto::Create()
{
	if(ms_pProtectAuto == NULL)
	{
		ms_pProtectAuto = new CProtectAuto;
	}
	return ms_pProtectAuto;
}
CProtectAuto* CProtectAuto::Get()
{
	return ms_pProtectAuto;
}
void CProtectAuto::Destroy()
{
	SAFE_DELETE(ms_pProtectAuto);
}

CProtectAuto::CProtectAuto()
{
#ifdef CSK_MOD_PROTECT_AUTO_V2
	m_bStartAuto = false;
	m_bAutoProcess = false;
#endif // CSK_MOD_PROTECT_AUTO_V2
#ifdef YDG_MOD_CHECK_PROTECT_AUTO_FLAG
	m_bNewVersion = false;
#endif	// YDG_MOD_CHECK_PROTECT_AUTO_FLAG

#ifdef YDG_MOD_PROTECT_AUTO_V4_R3
	m_pbtKeyAddressBlock4 = new BYTE[4];
	for (int i = 0; i < 4; ++i)
		m_pbtKeyAddressBlock4[i] = 0;
#endif	// YDG_MOD_PROTECT_AUTO_V4_R3
	
#ifdef KJH_MOD_PROTECT_AUTO_FIND_USB
	m_bFindRegisteredAutoUSB = false;
#endif // KJH_MOD_PROTECT_AUTO_FIND_USB
}

CProtectAuto::~CProtectAuto()
{
#ifdef YDG_MOD_PROTECT_AUTO_V4_R3
	delete [] m_pbtKeyAddressBlock4;
#endif	// YDG_MOD_PROTECT_AUTO_V4_R3
}

#ifdef YDG_MOD_CHECK_PROTECT_AUTO_FLAG
void CProtectAuto::SetNewVersion(bool bNew)
{
	m_bNewVersion = bNew;
}

bool CProtectAuto::IsNewVersion()
{
	return m_bNewVersion;
}

bool CProtectAuto::CheckFlag(const std::string& strCommand)
{
	if(strCommand == "$new")
	{
		m_bNewVersion = !m_bNewVersion;
#ifndef CSK_MOD_REMOVE_AUTO_V1_FLAG		// 정리할 때 지워야 하는 소스
		g_pNewUISystem->ToggleMoveCommandWindow();
#endif //! CSK_MOD_REMOVE_AUTO_V1_FLAG	// 정리할 때 지워야 하는 소스
#ifdef YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V4
		SEASON3B::CMoveCommandWindowEncrypt::Enable(m_bNewVersion);
#endif	// YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V4
		return true;
	}
	
	return false;
}
#endif	// YDG_MOD_CHECK_PROTECT_AUTO_FLAG

#ifdef CSK_MOD_PROTECT_AUTO_V2
void CProtectAuto::CheckAuto()
{
	HANDLE hSnapShot;
	PROCESSENTRY32 pEntry;
	
	// 프로세스 리스트를 캡쳐한다.
	hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
	pEntry.dwSize = sizeof(pEntry);

	// 첫번째 프로세스를 얻어온다.
	Process32First(hSnapShot, &pEntry);

	while(1)
	{
		// 다음 프로세스를 얻어온다.
		BOOL bResult = Process32Next(hSnapShot, &pEntry);

		// 다음 프로세스가 없다면 리턴
		if(bResult == FALSE)
		{
			break;
		}

		// 오토 프로그램 찾았다.!
		int iFind = 0;
// 		if (pEntry.szExeFile[0] == 'A') ++iFind;
// 		if (pEntry.szExeFile[1] == 't') ++iFind;
// 		if (pEntry.szExeFile[2] == 'P') ++iFind;
// 		if (pEntry.szExeFile[3] == 'l') ++iFind;
// 		if (pEntry.szExeFile[4] == 'a') ++iFind;
// 		if (pEntry.szExeFile[5] == 'y') ++iFind;
// 		if (pEntry.szExeFile[6] == 'M') ++iFind;
// 		if (pEntry.szExeFile[7] == 'U') ++iFind;
// 		if (pEntry.szExeFile[8] == 'N') ++iFind;
// 		if (pEntry.szExeFile[9] == 'e') ++iFind;
// 		if (pEntry.szExeFile[10] == 'w') ++iFind;
// 		if (pEntry.szExeFile[11] == '.') ++iFind;
// 		if (pEntry.szExeFile[12] == 'e') ++iFind;
// 		if (pEntry.szExeFile[13] == 'x') ++iFind;
// 		if (pEntry.szExeFile[14] == 'e') ++iFind;

		if (strlen(pEntry.szExeFile) != 15) continue;

		if (pEntry.szExeFile[0] == 'A') ++iFind;
		if (pEntry.szExeFile[2] == 'P') ++iFind;
		if (pEntry.szExeFile[5] == 'y') ++iFind;
		if (pEntry.szExeFile[6] == 'M') ++iFind;
		if (pEntry.szExeFile[13] == 'x') ++iFind;
		if (pEntry.szExeFile[1] == 't') ++iFind;
		if (pEntry.szExeFile[7] == 'U') ++iFind;
		if (pEntry.szExeFile[4] == 'a') ++iFind;
		if (pEntry.szExeFile[11] == '.') ++iFind;
		if (pEntry.szExeFile[8] == 'N') ++iFind;
		if (pEntry.szExeFile[14] == 'e') ++iFind;
		if (pEntry.szExeFile[9] == 'e') ++iFind;
		if (pEntry.szExeFile[10] == 'w') ++iFind;
		if (pEntry.szExeFile[12] == 'e') ++iFind;
		if (pEntry.szExeFile[3] == 'l') ++iFind;

		if (iFind == 15)
// 		if(stricmp(pEntry.szExeFile, "AtPlayMUNew.exe") == 0)
		{
			m_bAutoProcess = true;
			break;
		}
	}
}

bool CProtectAuto::IsAutoProcess()
{
	return m_bAutoProcess;
}

void CProtectAuto::Check()
{
#ifdef YDG_MOD_PROTECT_AUTO_FIND_USB
#ifdef KJH_MOD_PROTECT_AUTO_FIND_USB
	if(m_bFindRegisteredAutoUSB == false )
	{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		FindDevice();
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		BOOL bFind = FindDevice();
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	}
#else // KJH_MOD_PROTECT_AUTO_FIND_USB
	BOOL bFind = FindDevice();
#endif // KJH_MOD_PROTECT_AUTO_FIND_USB
#endif	// YDG_MOD_PROTECT_AUTO_FIND_USB

	if(SEASON3B::IsPress(VK_INSERT)	== true)	// 아이로봇 듀얼
		m_bStartAuto = true;
	
#ifdef YDG_MOD_PROTECT_AUTO_V5
	if(SEASON3B::IsPress(VK_HOME)	== true)	// 즐젬
		m_bStartAuto = true;
#endif	// YDG_MOD_PROTECT_AUTO_V5
	
	if(SEASON3B::IsPress(VK_END) == true)
		m_bStartAuto = false;
}

void CProtectAuto::SetStartAuto(bool bStart)
{
	m_bStartAuto = bStart;
}

bool CProtectAuto::IsStartAuto()
{
	return m_bStartAuto;
}
#endif // CSK_MOD_PROTECT_AUTO_V2

#ifdef YDG_MOD_PROTECT_AUTO_V5
BOOL CProtectAuto::FindAutoTool(char * pszProcessName)
{
	HANDLE hSnapShot;
	PROCESSENTRY32 pEntry;
	
	// 프로세스 리스트를 캡쳐한다.
	hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
	pEntry.dwSize = sizeof(pEntry);

	// 첫번째 프로세스를 얻어온다.
	Process32First(hSnapShot, &pEntry);

	BOOL bFind = FALSE;

	while(1)
	{
		// 다음 프로세스를 얻어온다.
		BOOL bResult = Process32Next(hSnapShot, &pEntry);

		// 다음 프로세스가 없다면 리턴
		if(bResult == FALSE)
		{
			break;
		}

		// 오토 프로그램 찾았다.!
		if (strlen(pEntry.szExeFile) != strlen(pszProcessName)) continue;
		if(stricmp(pEntry.szExeFile, pszProcessName) == 0)
		{
			bFind = TRUE;
			break;
		}
	}
#ifdef YDG_MOD_PROTECT_AUTO_V5_KEYCHECK_OFF
	return bFind;
#else	// YDG_MOD_PROTECT_AUTO_V5_KEYCHECK_OFF
	return (bFind && m_bStartAuto);
#endif	// YDG_MOD_PROTECT_AUTO_V5_KEYCHECK_OFF
}
#endif	// YDG_MOD_PROTECT_AUTO_V5

#ifdef YDG_MOD_PROTECT_AUTO_FIND_USB
BOOL CProtectAuto::FindDevice()
{
	GUID guidUSB = { 0xa5dcbf10, 0x6530, 0x11d2, { 0x90, 0x1f, 0x00, 0xc0, 0x4f, 0xb9, 0x51, 0xed } };

	HDEVINFO hDeviceInfoSet = SetupDiCreateDeviceInfoList(NULL, NULL);
	if(hDeviceInfoSet == INVALID_HANDLE_VALUE) 
	{
		return FALSE;
	}

 	HDEVINFO hNewDeviceInfoSet = SetupDiGetClassDevs(&guidUSB, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
 	if(hNewDeviceInfoSet == INVALID_HANDLE_VALUE) 
 	{
 		return FALSE;
 	}

	DWORD dwRequiredSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) + 512;	// 넉넉하게 잡음
	PSP_DEVICE_INTERFACE_DETAIL_DATA pDeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)new char[dwRequiredSize];
	ZeroMemory(pDeviceInterfaceDetailData, dwRequiredSize);
	pDeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

	SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
	ZeroMemory(&DeviceInterfaceData, sizeof(SP_DEVICE_INTERFACE_DATA));
	DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	int iDeviceIndex = 0;
 	BOOL bFind = FALSE;

	while(SetupDiEnumDeviceInterfaces(hNewDeviceInfoSet, NULL, &guidUSB, iDeviceIndex, &DeviceInterfaceData) == TRUE)
	{
		if (DeviceInterfaceData.Flags != SPINT_ACTIVE) continue;

		if (FAILED(SetupDiGetDeviceInterfaceDetail(hNewDeviceInfoSet, &DeviceInterfaceData, pDeviceInterfaceDetailData, dwRequiredSize, NULL, NULL)))
		{
			DWORD dwError = GetLastError();
			if (dwError == ERROR_INSUFFICIENT_BUFFER)
			{
				// 버퍼 부족: dwRequiredSize를 늘려 잡아야한다
				// SetupDiGetDeviceInterfaceDetail(hNewDeviceInfoSet, &DeviceInterfaceData, NULL, NULL, &dwRequiredSize, NULL);
				// 호출을 통해 정확한 메모리 길이를 얻을수 있다
			}
			continue;
		}

#ifdef KJH_MOD_PROTECT_AUTO_FIND_USB
		//********************************************************
		// ** AutoUSB Index
		// 20000. 기타오토
		// 10001. 즐젬(vid_090c&pid_1000#mwr1020s10000012#{a5dcbf10-6530-11d2-901f-00c04fb951ed})
		// 10002. 아이로봇4듀얼(vid_03eb&pid_7038#1.0.0#{a5dcbf10-6530-11d2-901f-00c04fb951ed})
		// 10003. 아이칸(??)
		// 이후부터 계속 추가!!! (소문자로 써야함)
		//********************************************************
		
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
		int iKindOfAutoUSB = 0;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
		if(strstr(pDeviceInterfaceDetailData->DevicePath, "vid_090c&pid_1000#mwr1020s10000012#{a5dcbf10-6530-11d2-901f-00c04fb951ed}") != NULL )		// 즐젬
		{
			SendRequestCheckAutoToolResult(10001, 1);
			m_bFindRegisteredAutoUSB = true;
		}
		else if(strstr(pDeviceInterfaceDetailData->DevicePath, "vid_03eb&pid_7038#1.0.0#{a5dcbf10-6530-11d2-901f-00c04fb951ed}") != NULL )		// 아이로봇4 듀얼(블루용)
		{
			SendRequestCheckAutoToolResult(10002, 1);
			m_bFindRegisteredAutoUSB = true;
		}
#else // KJH_MOD_PROTECT_AUTO_FIND_USB
		if (strstr(pDeviceInterfaceDetailData->DevicePath, "vid_03eb") != NULL && 
			strstr(pDeviceInterfaceDetailData->DevicePath, "pid_2373") != NULL)		// 아이로봇4 듀얼
		{
			bFind = TRUE;
		}
#endif // KJH_MOD_PROTECT_AUTO_FIND_USB

		++iDeviceIndex;
	}

	SetupDiDestroyDeviceInfoList(hNewDeviceInfoSet);
	SetupDiDestroyDeviceInfoList(hDeviceInfoSet);

	delete [] (char *)pDeviceInterfaceDetailData;

	return bFind;
}
#endif	// YDG_MOD_PROTECT_AUTO_FIND_USB

#ifdef YDG_MOD_PROTECT_AUTO_TEST
// 
// #define RC_INVOKED
// #include <C:\WINDDK\3790.1830\inc\crt\specstrings.h>
// #include <C:\WINDDK\3790.1830\inc\ddk\wxp\ntdef.h>
// #include <C:\WINDDK\3790.1830\inc\ddk\wdm\wxp\wdm.h>
// 
// #pragma pack(1)
// typedef struct _SERVICE_DESCRIPTOR_ENTRY
// {
// 	unsigned int *ServiceTableBase;
// 	unsigned int *ServiceCounterTableBase;
// 	unsigned int NumberOfServices;
// 	unsigned char *ParamTableBase;
// } SERVICE_DESCRIPTOR_ENTRY, *PSERVICE_DESCRIPTOR_ENTRY;
// #pragma pack()
// 
// _declspec(dllimport) SERVICE_DESCRIPTOR_ENTRY KeServiceDescriptorTable;
// 
// #define CR0_WP_MASK 0x0FFFEFFFF
// 
// VOID ClearWriteProtection(VOID)
// {
// 	__asm
// 	{
// 		push	eax;
// 		mov		eax, cr0;
// 		and		eax, CR0_WP_MASK;
// 		mov		cr0, eax;
// 		pop		eax;
// 	}
// }
// 
// VOID SetWriteProtection(VOID)
// {
// 	__asm
// 	{
// 		push	eax;
// 		mov		eax, cr0;
// 		or		eax, not CR0_WP_MASK;
// 		mov		cr0, eax;
// 		pop		eax;
// 	}
// }
// 
// extern
// NTSYSAPI
// NTSTATUS
// NTAPI
// ZwOpenProcess(
// 	OUT PHANDLE ProcessHandle,
// 	IN ACCESS_MASK DesiredAccess,
// 	IN POBJECT_ATTRIBUTES ObjectAttributes,
// 	IN PCLIENT_ID ClientId OPTIONAL
// 	);
// 
// 

//#include <native.h>
// 
// #include <ntddk.h>
// 
// extern
// NTSYSAPI
// NTSTATUS
// NTAPI
// ZwOpenProcess(
// 	OUT PHANDLE ProcessHandle,
// 	IN ACCESS_MASK DesiredAccess,
// 	IN POBJECT_ATTRIBUTES ObjectAttributes,
// 	IN PCLIENT_ID ClientId OPTIONAL
// 	);
// 
BOOL CProtectAuto::BlockOpenProcess()
{
	return TRUE;
}
#endif	// YDG_MOD_PROTECT_AUTO_TEST

#endif // CSK_MOD_PROTECT_AUTO_V1
