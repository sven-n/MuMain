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
        } *PRG_HEADER;

        typedef struct RG_RESOURCE_HEADER {
            DWORD	dwCRC32;
            WORD	wSizeOfName;

            /* name string */
        } *PRG_RESOURCE_HEADER;
#pragma pack(pop)

        struct RG_RESOURCE_INFO {
            std::wstring strName;
            DWORD		dwCRC32;
        };

        leaf::CCheckSumGenerator* m_pCSGen;
        std::list<RG_RESOURCE_INFO*>	m_listResource;
        std::list<IRGReportObj*>		m_listReportObj;

        std::wstring		m_strWorkingDir;

    public:
        CResourceGuard();
        ~CResourceGuard();

#ifdef ONLY_RESOURCE_GUARD_BUILDER
        void SetWorkingDirectory(const std::wstring& strDir);

        bool AddResource(const std::wstring& strResourceFile);
        bool RemoveResource(const std::wstring& strResourceFile);
        void RemoveAllResources();

        size_t GetResourceCount() const;

        bool Build(const std::wstring& strInPeFilePath, const std::wstring& strOutPeFilePath);

        void RemoveReportObj(IRGReportObj* pReportObj);
        void RemoveAllReportObjs();
        bool CreateResourceIntegityChecksumFile(const std::wstring& strFilePath);
#endif // ONLY_RESOURCE_GUARD_BUILDER

        const std::wstring& GetWorkingDirectory() const;
        void AddReportObj(IRGReportObj* pReportObj);
        bool CheckIntegrityResourceChecksumFile(const std::wstring& strFilePath);
        bool CheckIntegrityPEFile(const std::wstring& strPeFilePath);
        void WriteMessage(const std::wstring& msg, DWORD dwMsgType = MSG_TYPE_NORMAL,
            long lParam = MSG_PARAM_HEAD | MSG_PARAM_ENDL);
    };
}	// namespace leaf

#endif /* _REOURCEGUARD_H_ */