#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include "callstackdmp.h"

#ifdef KJH_LOG_ERROR_DUMP
	#include "../DebugAngel.h"
#endif // KJH_LOG_ERROR_DUMP

using namespace leaf;


//. Implementation

CCallStackDump::CCallStackFrame::CCallStackFrame() : m_pFrameAddr(NULL), m_pReturnAddr(NULL) {
	ZeroMemory(m_pParameter, MAX_PARAMETER_BUFSIZE*sizeof(DWORD));
}
CCallStackDump::CCallStackFrame::~CCallStackFrame() {}

bool CCallStackDump::CCallStackFrame::Create(DWORD* pEbp) {
	if(IsBadReadPtr( pEbp, sizeof(DWORD)))
		return false;
	
	m_pFrameAddr = (void*) pEbp;
	m_pReturnAddr = (void*) *(pEbp+1);
#ifdef KJH_LOG_ERROR_DUMP
	DebugAngel_Write(LOG_ERROR_DUMP_FILENAME, " \t- CallStack Frame Address : 0x%00000008X\r\n", m_pFrameAddr);
	DebugAngel_Write(LOG_ERROR_DUMP_FILENAME, " \t- CallStack Return Address : 0x%00000008X\r\n", m_pReturnAddr);
#endif // KJH_LOG_ERROR_DUMP
	for(int i=0; i<MAX_PARAMETER_BUFSIZE; i++) {
		if(IsBadReadPtr(pEbp+2+i, sizeof(DWORD)))
			break;
		memcpy(m_pParameter+i, (void*)(pEbp+2+i), sizeof(DWORD));
	}
	return true;
}
void CCallStackDump::CCallStackFrame::Release() {
	m_pFrameAddr = m_pReturnAddr = NULL;
	ZeroMemory(m_pParameter, MAX_PARAMETER_BUFSIZE*sizeof(DWORD));
}

void* CCallStackDump::CCallStackFrame::GetFrameAddr() { return m_pFrameAddr; }
void* CCallStackDump::CCallStackFrame::GetReturnAddr() { return m_pReturnAddr; }
void CCallStackDump::CCallStackFrame::GetParameter(DWORD* pBuf, size_t size) {
	if(size > MAX_PARAMETER_BUFSIZE)
		size = MAX_PARAMETER_BUFSIZE;
	memcpy(pBuf, m_pParameter, size*sizeof(DWORD));
}


CCallStackDump::CCallStackDump() {}
CCallStackDump::~CCallStackDump() { Clear(); }

void CCallStackDump::Dump(const CONTEXT* pContext) {
	Clear();
#ifdef KJH_LOG_ERROR_DUMP
	DebugAngel_Write(LOG_ERROR_DUMP_FILENAME, " \t* Context Pointer\r\n");
	DebugAngel_Write(LOG_ERROR_DUMP_FILENAME, " \t\t- pContext->SegCs : 0x%00000008X\r\n", pContext->SegCs);
	DebugAngel_Write(LOG_ERROR_DUMP_FILENAME, " \t\t- pContext->SegDs : 0x%00000008X\r\n", pContext->SegDs);
	DebugAngel_Write(LOG_ERROR_DUMP_FILENAME, " \t\t- pContext->SegEs : 0x%00000008X\r\n", pContext->SegEs);
	DebugAngel_Write(LOG_ERROR_DUMP_FILENAME, " \t\t- pContext->SegSs : 0x%00000008X\r\n", pContext->SegSs);
	DebugAngel_Write(LOG_ERROR_DUMP_FILENAME, " \t\t- pContext->SegFs : 0x%00000008X\r\n", pContext->SegFs);
	DebugAngel_Write(LOG_ERROR_DUMP_FILENAME, " \t\t- pContext->SegGs : 0x%00000008X\r\n", pContext->SegGs);
	DebugAngel_Write(LOG_ERROR_DUMP_FILENAME, " \t\t- pContext->Eax : 0x%00000008X\r\n", pContext->Eax);
	DebugAngel_Write(LOG_ERROR_DUMP_FILENAME, " \t\t- pContext->Ebx : 0x%00000008X\r\n", pContext->Ebx);
	DebugAngel_Write(LOG_ERROR_DUMP_FILENAME, " \t\t- pContext->Ecx : 0x%00000008X\r\n", pContext->Ecx);
	DebugAngel_Write(LOG_ERROR_DUMP_FILENAME, " \t\t- pContext->Edx : 0x%00000008X\r\n", pContext->Edx);
	DebugAngel_Write(LOG_ERROR_DUMP_FILENAME, " \t\t- pContext->Esi : 0x%00000008X\r\n", pContext->Esi);
	DebugAngel_Write(LOG_ERROR_DUMP_FILENAME, " \t\t- pContext->Edi : 0x%00000008X\r\n", pContext->Edi);
	DebugAngel_Write(LOG_ERROR_DUMP_FILENAME, " \t\t- pContext->Esp : 0x%00000008X\r\n", pContext->Esp);
	DebugAngel_Write(LOG_ERROR_DUMP_FILENAME, " \t\t- pContext->Ebp : 0x%00000008X\r\n", pContext->Ebp);
	DebugAngel_Write(LOG_ERROR_DUMP_FILENAME, " \t\t- pContext->Eip : 0x%00000008X\r\n", pContext->Eip);        // 명령 포인터
	DebugAngel_Write(LOG_ERROR_DUMP_FILENAME, " \t\t- pContext->EFlags : 0x%00000008X\r\n", pContext->EFlags);	// 플레그 레지스터
#endif // KJH_LOG_ERROR_DUMP
	
	DWORD* pEbp = (DWORD*)pContext->Ebp;
	for(int i=0; i<64; i++) {
		CCallStackFrame* pCallStackFrame = new CCallStackFrame;
		if(!pCallStackFrame->Create(pEbp)) {
			delete pCallStackFrame; break;
		}
		m_listFrame.push_back(pCallStackFrame);

#ifdef KJH_LOG_ERROR_DUMP
		DebugAngel_Write(LOG_ERROR_DUMP_FILENAME, " \t* CallStack Frame Pointer : 0x%00000008X, Number : %d\r\n", pCallStackFrame, i);
#endif // KJH_LOG_ERROR_DUMP
		
		if(IsBadReadPtr((DWORD*)*pEbp, sizeof(DWORD)))
			break;

		//. 이전 함수로
		pEbp = (DWORD*)*pEbp;
	}
}
void CCallStackDump::Dump() {
	CONTEXT ct;
	ZeroMemory(&ct, sizeof(CONTEXT));
	
	ct.ContextFlags = CONTEXT_FULL;
	::GetThreadContext( GetCurrentThread(), &ct);
	
	Dump(&ct);
}
void CCallStackDump::Clear() {
	type_framevect::iterator fvi = m_listFrame.begin();
	for(; fvi != m_listFrame.end(); fvi++)
		delete (*fvi);
	m_listFrame.clear();
}

size_t CCallStackDump::GetStackDepth() const { return m_listFrame.size(); }
void* CCallStackDump::GetFrameAddr(int index) {
#ifdef _VS2008PORTING
	if(index >= (int)GetStackDepth()) return NULL;
#else // _VS2008PORTING
	if(index >= GetStackDepth()) return NULL;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          
#endif // _VS2008PORTING
	return m_listFrame[index]->GetFrameAddr();
}
void* CCallStackDump::GetReturnAddr(int index) {
#ifdef _VS2008PORTING                                                                      
	if(index >= (int)GetStackDepth()) return NULL;
#else // _VS2008PORTING
	if(index >= GetStackDepth()) return NULL;
#endif // _VS2008PORTING
	return m_listFrame[index]->GetReturnAddr();
}
void CCallStackDump::GetParameter(int index, DWORD* pBuf, size_t size) {
#ifdef _VS2008PORTING
	if(index >=0 && index < (int)GetStackDepth()) {
#else // _VS2008PORTING
	if(index >=0 && index < GetStackDepth()) {
#endif // _VS2008PORTING
		m_listFrame[index]->GetParameter(pBuf, size);
	}
}