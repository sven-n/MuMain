#ifndef _CALLSTACKDMP_H_
#define _CALLSTACKDMP_H_

#include <windows.h>

#pragma warning(disable : 4786)
#include <vector>

namespace leaf {

	class CCallStackDump {
	public:
		CCallStackDump();
		virtual ~CCallStackDump();

		void Dump(const CONTEXT* pContext);
		void Dump();
		void Clear();

		size_t GetStackDepth() const;
		void* GetFrameAddr(int index);
		void* GetReturnAddr(int index);
		void GetParameter(int index, DWORD* pBuf, size_t size);

	private:
		class CCallStackFrame {
		public:
			enum { MAX_PARAMETER_BUFSIZE = 16 };

			CCallStackFrame();
			virtual ~CCallStackFrame();
			
			bool Create(DWORD* pEbp);
			void Release();
			
			void* GetFrameAddr();
			void* GetReturnAddr();
			void GetParameter(DWORD* pBuf, size_t size);

		private:
			void*	m_pFrameAddr;
			void*	m_pReturnAddr;
			DWORD	m_pParameter[MAX_PARAMETER_BUFSIZE];
		};
		
		typedef std::vector<CCallStackFrame*> type_framevect;
		type_framevect m_listFrame;
	};
	
}

#endif // _CALLSTACKDMP_H_