#ifndef _EXCEPTION_HANDLER_H_
#define _EXCEPTION_HANDLER_H_

#include "sysinfo.h"
#include "callstackdmp.h"

#pragma warning(disable : 4786)
#include <string>

#pragma comment(lib, "Version.lib")

#define _CALLSTACK_DUMP leaf::CCallStackDump

/*
    //. Callback function prototype
    bool MyCallback(_EXCEPTION_POINTERS* pExceptionInfo);
*/

namespace leaf {
    typedef bool (*EXCEPTION_CALLBACK)(_EXCEPTION_POINTERS* pExceptionInfo);

    //. Interface Declaration

    bool AttachExceptionHandler(const std::wstring& dmpfile, bool bSecondChance = false);
    bool AttachExceptionHandler(EXCEPTION_CALLBACK pfCallback, bool bSecondChance = false);
    bool DetachExceptionHandler();

    //. if it returns true, exception should be passed to the application's debugger
    bool IsContinueExceptionToSecondChance();

    bool SaveExceptionDumpFile(const std::wstring& filename, CONTEXT* pContext, _EXCEPTION_POINTERS* pExceptionInfo = NULL);

#pragma pack(push, 1)

    //. Modules infomation
    typedef struct _DMPPROCESSINFOHEADER {
        DWORD	ProcessId;
        WCHAR	szImageName[256];
        DWORD	CheckSum32;
        DWORD	ThreadCount;
        BYTE	IsExistFixedFileInfo;
        DWORD	NumOfModules;
    } DMPPROCESSINFOHEADER, * LPDMPPROCESSINFOHEADER;

    typedef struct _DMPMODULEINFO {
        WCHAR	szModuleBaseName[128];
        WCHAR	szModulePath[256];
        LPVOID	lpBaseAddr;		// The load address of the module
        DWORD	SizeOfModule;	// Size, in bytes, of the module.
    } DMPMODULEINFO, * LPDMPMODULEINFO;

    //. Exception infomation
    typedef struct _DMPEXCEPTIONINFOHEADER {
        DWORD	ExceptionCode;
        DWORD	ExceptionAddress;
        WCHAR	ExceptionCodeString[128];
    } DMPEXCEPTIONINFOHEADER, * LPDMPEXCEPTIONINFOHEADER;

    typedef struct _DMPCALLSTACKFRAME {
        DWORD	FrameAddr;
        DWORD	ReturnAddr;
        DWORD	Parameter[16];
    } DMPCALLSTACKFRAME, * LPDMPCALLSTACKFRAME;

    typedef struct _DMPFILEHEADER {
        BYTE	Sign[4];	// 'W' 'D' 'M' 'P'
        BYTE	Version;
        DWORD	HeaderSize;

        //. Creation infomation
        DWORD	CreationTimeStamp;
        //. System infomation
        WCHAR	OSInfoString[128];
        WCHAR	CPUInfoString[128];
        MEMORYSTATUS	MemStatus;
        //. Process infomation
        DMPPROCESSINFOHEADER	ProcessInfo;
        //. Exception infomation
        DMPEXCEPTIONINFOHEADER	ExceptionInfo;
        //. Context infomation
        CONTEXT	ContextRecord;
        //. Callstack infomation
        DWORD	CallStackDepth;
    } DMPFILEHEADER, * LPDMPFILEHEADER;

#pragma pack(pop)

    //. class CExceptionHandler

    class CExceptionHandler {
        LPTOP_LEVEL_EXCEPTION_FILTER	m_pfPrevExceptionFilter;
        EXCEPTION_CALLBACK				m_pfExceptionCallback;

        std::wstring	m_filename;
        bool		m_bSecondChance;

    public:
        ~CExceptionHandler();

        bool AttachExceptionHandler(const std::wstring& filename, bool bSecondChance);
        bool AttachExceptionHandler(EXCEPTION_CALLBACK pfCallback, bool bSecondChance);
        bool DetachExceptionHandler();
        bool IsEnableSecondChance() const;

        const std::wstring& GetDmpFileName() const;
        bool SaveDmpFile(const std::wstring& filename, CONTEXT* pContext, _EXCEPTION_POINTERS* pExceptionInfo);

        bool IsEnableCallback() const;
        bool CallExceptionCallback(_EXCEPTION_POINTERS* pExceptionInfo);

        static CExceptionHandler* GetObjPtr();		//. Get singleton object pointer
        static LONG __stdcall OnException(_EXCEPTION_POINTERS* pExceptionInfo);

    private:
        CExceptionHandler();	//. Ban create instance

        void SetProcessInfoHeader(DMPPROCESSINFOHEADER* pProcessInfoHeader, DMPMODULEINFO** ppModuleInfo, int MaxModules, VS_FIXEDFILEINFO& ImageFileInfo);
        void SetExceptionInfoHeader(DMPEXCEPTIONINFOHEADER* pExceptionInfoHeader, _EXCEPTION_POINTERS* pExceptionInfo);
        void GetExceptionCodeString(IN PEXCEPTION_RECORD pExceptionRecord, OUT std::wstring& type);
    };

    //. class CDmpFileLoader

    class CDmpFileLoader {
        typedef std::vector<DMPCALLSTACKFRAME*> t_framevect;
        typedef std::vector<DMPMODULEINFO*>	t_modulevect;

        DMPFILEHEADER		m_DmpFileHeader;
        VS_FIXEDFILEINFO	m_FixedImageFileInfo;

        t_framevect		m_listStackFrame;
        t_modulevect	m_listModule;

        void Init();
    public:
        CDmpFileLoader();
        virtual ~CDmpFileLoader();

        bool Create(const std::wstring& dmpfile);
        void Release();

        DWORD GetCreationTimeStamp() const;

        const wchar_t* GetOSInfoString() const;
        const wchar_t* GetCPUInfoString() const;
        const MEMORYSTATUS& GetMemoryStatus() const;

        //. Process infomation
        DWORD GetProcessId() const;
        DWORD GetCheckSum32() const;
        DWORD GetThreadCount() const;
        const wchar_t* GetImageName() const;
        const VS_FIXEDFILEINFO* GetFixedImageFileInfo() const;

        //. Modules infomation
        size_t GetNumOfModules() const;
        const DMPMODULEINFO* GetModuleInfo(int index) const;

        //. Exception infomation
        const CONTEXT& GetContextRecord() const;
        DWORD GetExceptionCode() const;
        DWORD GetExceptionAddress() const;
        const wchar_t* GetExceptionCodeString() const;

        //. Callstack infomation
        size_t GetStackDepth() const;
        const DMPCALLSTACKFRAME* GetCallStackFrame(int index) const;
    };
}

#endif // _EXCEPTION_HANDLER_H_
