// Queue.cpp: implementation of the CQueue class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Queue.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CQueue::CQueue() // OK
{

}

CQueue::~CQueue() // OK
{
	this->ClearQueue();
}

void CQueue::ClearQueue() // OK
{
	this->m_critical.lock();

	this->m_QueueInfo.swap(std::queue<QUEUE_INFO>());

	this->m_critical.unlock();
}

DWORD CQueue::GetQueueSize() // OK
{
	DWORD size = 0;

	this->m_critical.lock();

	size = this->m_QueueInfo.size();

	this->m_critical.unlock();

	return size;
}

bool CQueue::AddToQueue(QUEUE_INFO* lpInfo) // OK
{
	bool result = 0;

	this->m_critical.lock();

	if(this->m_QueueInfo.size() < MAX_QUEUE_SIZE)
	{
		this->m_QueueInfo.push((*lpInfo));

		result = 1;
	}

	this->m_critical.unlock();

	return result;
}

bool CQueue::GetFromQueue(QUEUE_INFO* lpInfo) // OK
{
	bool result = 0;

	this->m_critical.lock();

	if(this->m_QueueInfo.empty() == 0)
	{
		(*lpInfo) = this->m_QueueInfo.front();

		this->m_QueueInfo.pop();

		result = 1;
	}

	this->m_critical.unlock();

	return 1;
}
