//*****************************************************************************
// File: PList.cpp
//
// Desc: Implementation of the CPList class.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#include "stdafx.h"
#include "PList.h"

//*****************************************************************************
// Name: 생성자, 소멸자 정의
//*****************************************************************************
CPList::CPList()
{
	m_nCount = 0;
	m_pNodeHead = m_pNodeTail = NULL;
}

CPList::~CPList()
{
	RemoveAll();
}

// Name: Head/Tail Access 함수 정의
/**
//*****************************************************************************
// 함수 이름 : GetHead()\n
// 함수 설명 : head의 값을 얻음.
//*****************************************************************************/
void* CPList::GetHead() const
{ 
	if (NULL == m_pNodeHead)	// node가 없으면 NULL 리턴.
		return NULL;

	return m_pNodeHead->data;		
}
/**
//*****************************************************************************
// 함수 이름 : GetTail()\n
// 함수 설명 : Tail의 값을 얻음.
//*****************************************************************************/
void* CPList::GetTail() const
{
	if (NULL == m_pNodeTail)	// node가 없으면 NULL 리턴.
		return NULL;

	return m_pNodeTail->data;
}

// Name: Operations 함수 정의
/**
//*****************************************************************************
// 함수 이름 : RemoveHead()\n
// 함수 설명 : Head를 메모리에서 삭제 후 그 값을 리턴.
//*****************************************************************************/
void* CPList::RemoveHead()
{
	if (NULL == m_pNodeHead)			// node가 없으면 NULL 리턴.
		return NULL;

	if (::IsBadReadPtr(m_pNodeHead, sizeof(NODE)))	// m_pNodeHead가 메모리에
		return NULL;								//없으면 NULL 리턴.

	NODE* pOldNode = m_pNodeHead;		// pOldNode와 m_pNodeHead가 같은 주소.
	void* returnValue = pOldNode->data;	// head의 값을 returnValue에.
	m_pNodeHead = pOldNode->pNext;		// 두번째 node의 주소를 m_pNodeHead에 기억.
	if (m_pNodeHead != NULL)			// node가 둘 이상이라면.
		m_pNodeHead->pPrev = NULL;		// head가 지워질 것이므로.
	else								// node가 하나였다면.
		m_pNodeTail = NULL;				// node가 하나도 없을 것이므로.
	FreeNode(pOldNode);					// 원래 head를 지운다.

	return returnValue;					// 원래 head의 값을 return.
}
/**
//*****************************************************************************
// 함수 이름 : RemoveTail()\n
// 함수 설명 : Tail를 메모리에서 삭제 후 그 값을 리턴.
//*****************************************************************************/
void* CPList::RemoveTail()
{
	if (NULL == m_pNodeTail)
		return NULL;

	if (::IsBadReadPtr(m_pNodeTail, sizeof(NODE)))
		return NULL;

	NODE* pOldNode = m_pNodeTail;
	void* returnValue = pOldNode->data;
	m_pNodeTail = pOldNode->pPrev;
	if (m_pNodeTail != NULL)
		m_pNodeTail->pNext = NULL;
	else
		m_pNodeHead = NULL;
	FreeNode(pOldNode);

	return returnValue;
}
/**
//*****************************************************************************
// 함수 이름 : AddHead()\n
// 함수 설명 : Head에 새로운 값을 삽입하고 그 position 리턴.\n
// 매개 변수 : newElement	: 새로운 값.
//*****************************************************************************/
NODE* CPList::AddHead(void* newElement)
{
	// pPrev가 NULL이고 pNext가 m_pNodeHead인 새 노드 생성.
	NODE* pNewNode = NewNode(NULL, m_pNodeHead);
	pNewNode->data = newElement;		// 새 노드에 새로운 값을 넣는다.
	if (m_pNodeHead != NULL)			// 비어 있는 list가 아니라면.
		m_pNodeHead->pPrev = pNewNode;	//m_pNodeHead->pPrev에 새 노드 연결.
	else								// 비어 있는 list라면.
		m_pNodeTail = pNewNode;			// 새 노드를 tail에.
	m_pNodeHead = pNewNode;				// 새 노드를 head에.

	return pNewNode;					// 새 노드 리턴.
}
/**
//*****************************************************************************
// 함수 이름 : AddTail()\n
// 함수 설명 : Tail에 새로운 값을 삽입하고 그 position 리턴.\n
// 매개 변수 : newElement	: 새로운 값.
//*****************************************************************************/
NODE* CPList::AddTail(void* newElement)
{
	NODE* pNewNode = NewNode(m_pNodeTail, NULL);
	pNewNode->data = newElement;
	if (m_pNodeTail != NULL)
		m_pNodeTail->pNext = pNewNode;
	else
		m_pNodeHead = pNewNode;
	m_pNodeTail = pNewNode;

	return pNewNode;
}
/**
//*****************************************************************************
// 함수 이름 : AddHead()\n
// 함수 설명 : Head에 새로운 list를 삽입.\n
// 매개 변수 : pNewList	: 삽입할 CPList 객체의 포인터.
//*****************************************************************************/
BOOL CPList::AddHead(CPList* pNewList)
{
	if (NULL == pNewList)	// 붙일 list의 주소가 NULL이면.
		return FALSE;		// 실패.

	// 붙일 list의 tail의 주소 pos에 저장.
	NODE* pos = pNewList->GetTailPosition();
	while (pos != NULL)					// 붙일 list의 head까지 반복.
		AddHead(pNewList->GetPrev(pos));// 붙일 list를 현 list에 붙인다.

	return TRUE;					
}
/**
//*****************************************************************************
// 함수 이름 : AddTail()\n
// 함수 설명 : Tail에 새로운 list를 삽입.\n
// 매개 변수 : pNewList	: 삽입할 CPList 객체의 포인터.
//*****************************************************************************/
BOOL CPList::AddTail(CPList* pNewList)
{
	if (NULL == pNewList)
		return FALSE;

	NODE* pos = pNewList->GetHeadPosition();
	while (pos != NULL)
		AddTail(pNewList->GetNext(pos));

	return TRUE;
}
/**
//*****************************************************************************
// 함수 이름 : RemoveAll()\n
// 함수 설명 : list의 모든 메모리를 삭제.
//*****************************************************************************/
void CPList::RemoveAll()
{
	NODE* pNode;
	while (NULL != m_pNodeHead)	// m_pNodeHead가 NULL이 아닐 때까지 반복.
	{
		pNode = m_pNodeHead;
		// 그 다음 노드의 주소를 m_pNodeHead에 넣는다.
		m_pNodeHead = m_pNodeHead->pNext;
		delete pNode;			// 현재 head를 지운다.
	}
	m_pNodeTail = NULL;			// 데이터 멤버 초기화.
	m_nCount = 0;
}

// Name: Iteration 함수 정의
/**
//*****************************************************************************
// 함수 이름 : GetNext()\n
// 함수 설명 : rPosition이 다음 position으로 바뀌고 원래 rPosition의 값 리턴.\n
// 매개 변수 : rPosition	: 기준 position.
//							  함수 실행 후에 다음 position으로 바뀜.
//*****************************************************************************/
void* CPList::GetNext(NODE*& rPosition) const
{
	NODE* pNode = rPosition;
	if (::IsBadReadPtr(pNode, sizeof(NODE)))	// pNode가 메모리에 없으면.
		return NULL;							// NULL 리턴.

	rPosition = pNode->pNext;					// 다음 노드의 주소 저장.

	return pNode->data;							// 현 노드의 값 리턴.
}
/**
//*****************************************************************************
// 함수 이름 : GetPrev()\n
// 함수 설명 : rPosition이 이전 position으로 바뀌고 원래의 rPosition의 값 리턴.\n
// 매개 변수 : rPosition	: 기준 position.
//							  함수 실행 후에 이전 position으로 바뀜.
//*****************************************************************************/
void* CPList::GetPrev(NODE*& rPosition) const
{
	NODE* pNode = rPosition;
	if (::IsBadReadPtr(pNode, sizeof(NODE)))	// pNode가 메모리에 없으면.
		return NULL;

	rPosition = pNode->pPrev;

	return pNode->data;
}

// Name: Retrieval/Modification 함수 정의
/**
//*****************************************************************************
// 함수 이름 : GetAt()\n
// 함수 설명 : position의 값을 얻음.\n
// 매개 변수 : position	: 얻고 싶은 값의 position.
//*****************************************************************************/
void* CPList::GetAt(NODE* position) const
{ 
	NODE* pNode = position;
	if (::IsBadReadPtr(pNode, sizeof(NODE)))	// pNode가 메모리에 없으면.
		return NULL;							// NULL 리턴.

	return pNode->data;							// 현 node 값 리턴.
}
/**
//*****************************************************************************
// 함수 이름 : SetAt()\n
// 함수 설명 : position의 값을 세팅.\n
// 매개 변수 : position	: 세팅하고 싶은 값의 position.
//*****************************************************************************/
BOOL CPList::SetAt(NODE* pos, void* newElement)
{ 
	NODE* pNode = pos;
	if (::IsBadReadPtr(pNode, sizeof(NODE)))	// pNode가 메모리에 없으면.
		return FALSE;							// 실패.

	pNode->data = newElement;					// 새 값을 현 node에 저장.

	return TRUE;
}
/**
//*****************************************************************************
// 함수 이름 : RemoveAt()\n
// 함수 설명 : position의 값을 삭제\n
// 매개 변수 : position	: 삭제하고 싶은 값의 position.
//*****************************************************************************/
BOOL CPList::RemoveAt(NODE* position)
{
	NODE* pOldNode = position;
	if (::IsBadReadPtr(pOldNode, sizeof(NODE)))	// pOldNode가 메모리에 없으면.
		return FALSE;							// 실패.

	if (pOldNode == m_pNodeHead)				// 현 지점이 head라면.
		// m_pNodeHead가 현 지점에서 다음 node를 가리키게 함.
		m_pNodeHead = pOldNode->pNext;
	else										// 현 지점이 head가 아니라면.
	{
		// pOldNode->pPrev가 메모리에 없으면 실패.
		if (::IsBadReadPtr(pOldNode->pPrev, sizeof(NODE)))
			return FALSE;
		pOldNode->pPrev->pNext = pOldNode->pNext;// 전 node를 다음 node로 연결.
	}

	if (pOldNode == m_pNodeTail)				// 현 지점이 tail이라면.
		// m_pNodeTail이 현 지점에서 전 node를 가리키게 함.
		m_pNodeTail = pOldNode->pPrev;
	else										// 현 지점이 tail이 아니라면.
	{
		// pOldNode->pNext가 메모리에 없으면 실패.
		if (::IsBadReadPtr(pOldNode->pNext, sizeof(NODE)))
			return FALSE;
		pOldNode->pNext->pPrev = pOldNode->pPrev;// 다음 node를 전 노드로 연결.
	}
	FreeNode(pOldNode);							// 현 노드 삭제.

	return TRUE;
}

// Name: Insertion 함수 정의
/**
//*****************************************************************************
// 함수 이름 : InsertBefore()\n
// 함수 설명 : position 전에 새로운 값 삽입 후 새로운 값의 position 리턴.\n
// 매개 변수 : position		: 기준 position.\n
//			   newElement	: 삽입할 값.
//*****************************************************************************/
NODE* CPList::InsertBefore(NODE* position, void* newElement)
{
	if (position == NULL)		// 첫번째 인자가 NULL이면.
		//  두번째 인자의 값을 Head 앞에 붙이고, 붙인 노드의 주소 리턴.
		return AddHead(newElement);

	NODE* pOldNode = position;
	// pPrev가 현 지점에서 전주소이고, pNext가 현 지점의 주소인 새 노드 생성.
	NODE* pNewNode = NewNode(pOldNode->pPrev, pOldNode);
	pNewNode->data = newElement;				// 새 노드에 새로운 값 저장.
	if (pOldNode->pPrev != NULL)				// 현 노드가 head가 아니면.
		pOldNode->pPrev->pNext = pNewNode;		// 이전 노드를 새 노드에 연결.
	else										// 현 노드가 head이면.
		m_pNodeHead = pNewNode;					// 새 노드가 haed가 됨.
	pOldNode->pPrev = pNewNode;					// 현 노드와 새노드 연결.

	return pNewNode;							// 새노드의 주소 리턴.
}
/**
//*****************************************************************************
// 함수 이름 : InsertAfter()\n
// 함수 설명 : position 뒤에 새로운 값 삽입 후 새로운 값의 position 리턴.\n
// 매개 변수 : position		: 기준 position.\n
//			   newElement	: 삽입할 값.
//*****************************************************************************/
NODE* CPList::InsertAfter(NODE* position, void* newElement)
{
	if (position == NULL)		// 첫번째 인자가 NULL이면.
		// 두번째 인자의 값을 Tail 뒤에 붙이고, 붙인 노드의 주소 리턴.
		return AddTail(newElement);

	NODE* pOldNode = position;
	NODE* pNewNode = NewNode(pOldNode, pOldNode->pNext);
	pNewNode->data = newElement;
	if (pOldNode->pNext != NULL)
		pOldNode->pNext->pPrev = pNewNode;
	else
		m_pNodeTail = pNewNode;
	pOldNode->pNext = pNewNode;

	return pNewNode;
}
/**
//*****************************************************************************
// 함수 이름 : Swap()\n
// 함수 설명 : 두 position의 값을 맞바꿈.\n
// 매개 변수 : pNode1	: 맞바꿀 값의 position.\n
//			   pNode2	: 맞바꿀 값의 position.
//*****************************************************************************/
void CPList::Swap(NODE* pNode1, NODE* pNode2)
{
	if (pNode1 == NULL || pNode2 == NULL)
		return;

	void* temp;
	temp = pNode1->data;
	pNode1->data = pNode2->data;
	pNode2->data = temp;
}

// Name: Searching 함수 정의
/**
//*****************************************************************************
// 함수 이름 : Find()\n
// 함수 설명 : searchValue를 startAfter의 position 부터 찾음.
//			   찾았다면 그 position 리턴. 못 찾았다면 NULL을 리턴.\n
// 매개 변수 : searchValue	: 찾을 값.\n
//			   startAfter	: 기준 position. NULL이면 Head 부터 찾음.
//							  (기본값은 NULL.)
//*****************************************************************************/
NODE* CPList::Find(void* searchValue, NODE* startAfter) const
{
	NODE* pNode = startAfter;
	if (pNode == NULL)				// 두번째 인자가 NULL이면 Head 부터 찾는다.
		pNode = m_pNodeHead;
	else
	{
		if (::IsBadReadPtr(pNode, sizeof(NODE)))	// pNode가 메모리에 없으면.
			return NULL;							// NULL 리턴.

		// 두번째 인자가 NULL이 아니면 그 다음 노드부터 찾는다.
		pNode = pNode->pNext;
	}
	// 선택된 노드 부터 차례로 끝까지 검색.
	for (; pNode != NULL; pNode = pNode->pNext)
		if (pNode->data == searchValue)				// 값을 찾으면.
			return pNode;							// 그 노드 주소 리턴.

	return NULL;									// 못 찾으면 NULL 리턴.
}
/**
//*****************************************************************************
// 함수 이름 : FindIndex()\n
// 함수 설명 : head부터 nIndex번째 position 리턴.\n
// 매개 변수 : nIndex	: 찾을 position의 인덱스.
//*****************************************************************************/
NODE* CPList::FindIndex(int nIndex) const
{
	// 인자가 노드 수 이상이거나 음수일 때 NULL 리턴.
	if (nIndex >= m_nCount || nIndex < 0)
		return NULL;

	NODE* pNode = m_pNodeHead;						// head부터.
	while (nIndex--)								// 차례로 인자 만큼.
		pNode = pNode->pNext;						// 노드를 이동해서.

	return pNode;									// 그 노드 리턴.
}

// Name: Node Construction/Destruction 함수 정의
/**
//*****************************************************************************
// 함수 이름 : NewNode()\n
// 함수 설명 : 새 노드 동적 생성 후 그 position 리턴.\n
// 매개 변수 : pPrev	: 전의 position.\n
//			   pNext	: 후의 position.
//*****************************************************************************/
NODE* CPList::NewNode(NODE* pPrev, NODE* pNext)
{
	NODE* pNode = new NODE;			// 노드 구조체 크기 만큼 메모리 동적 할당.
	pNode->pPrev = pPrev;
	pNode->pNext = pNext;
	m_nCount++;						// list의 총 노드수 1증가.
	pNode->data = NULL;				// 새 노드의 값 초기화.

	return pNode;					// 새 노드 주소 리턴.
}
/**
//*****************************************************************************
// 함수 이름 : FreeNode()\n
// 함수 설명 : 이 position의 메모리 삭제.\n
// 매개 변수 : pNode	: 삭제할 position.
//*****************************************************************************/
void CPList::FreeNode(NODE* pNode)
{
	delete pNode;					// 노드 삭제.
	m_nCount--;						// list의 총 노드수 1감소.
	if (m_nCount == 0)				// list에 노드가 하나도 없으면.
		RemoveAll();
}
