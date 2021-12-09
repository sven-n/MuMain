// Queue.h: interface for the CQueue class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "CriticalSection.h"

#define MAX_QUEUE_SIZE 2048

struct QUEUE_INFO
{
	WORD index;
	BYTE head;
	BYTE buff[2048];
	DWORD size;
};

class CQueue
{
public:
	CQueue();
	virtual ~CQueue();
	void ClearQueue();
	DWORD GetQueueSize();
	bool AddToQueue(QUEUE_INFO* lpInfo);
	bool GetFromQueue(QUEUE_INFO* lpInfo);
private:
	CCriticalSection m_critical;
	std::queue<QUEUE_INFO> m_QueueInfo;
};
