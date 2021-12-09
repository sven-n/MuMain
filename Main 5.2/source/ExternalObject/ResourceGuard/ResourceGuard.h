
#ifndef _RESOURCEGUARD_H_
#define _RESOURCEGUARD_H_

#pragma once 

#pragma warning(disable : 4786)
#include <string>
#include <list>

#include "define.h"
#include "./ExternalObject/Leaf/checkintegrity.h"	//. leaf header
#include "RGReportObj.h"

namespace ResourceGuard {
	
	class CResourceGuard
	{
#pragma pack(push, 1)
		typedef struct RG_HEADER {
			DWORD	dwSignature;		// RG 0x4752
			DWORD	dwNumberOfResource;
			DWORD	dwSizeOfHeaders;
			DWORD	dwCRC32;
		} * PRG_HEADER;
		
		typedef struct RG_RESOURCE_HEADER {
			DWORD	dwCRC32;
			WORD	wSizeOfName;

			/* name string */
		} * PRG_RESOURCE_HEADER;
#pragma pack(pop)
		
		struct RG_RESOURCE_INFO {
			std::string strName;
			DWORD		dwCRC32;
		};
		
		leaf::CCheckSumGenerator*		m_pCSGen;
		std::list<RG_RESOURCE_INFO*>	m_listResource;
		std::list<IRGReportObj*>		m_listReportObj;

		std::string		m_strWorkingDir;
		
		public:
			CResourceGuard();
			~CResourceGuard();
			
#ifdef ONLY_RESOURCE_GUARD_BUILDER
			void SetWorkingDirectory(const std::string& strDir);
			
			bool AddResource(const std::string& strResourceFile);
			bool RemoveResource(const std::string& strResourceFile);
			void RemoveAllResources();
			
			size_t GetResourceCount() const;
			
			bool Build(const std::string& strInPeFilePath, const std::string& strOutPeFilePath);
			
			void RemoveReportObj(IRGReportObj* pReportObj);
			void RemoveAllReportObjs();	
			bool CreateResourceIntegityChecksumFile(const std::string& strFilePath);
#endif // ONLY_RESOURCE_GUARD_BUILDER

			const std::string& GetWorkingDirectory() const;
			void AddReportObj(IRGReportObj* pReportObj);
			bool CheckIntegrityResourceChecksumFile(const std::string& strFilePath);
			bool CheckIntegrityPEFile(const std::string& strPeFilePath);
			void WriteMessage(const std::string& msg, DWORD dwMsgType = MSG_TYPE_NORMAL, 
								long lParam = MSG_PARAM_HEAD | MSG_PARAM_ENDL);
	};

}	// namespace leaf


#endif /* _REOURCEGUARD_H_ */