//*****************************************************************************
// File: PList.h
//
// Desc: Pointer list class header file.
//		 포인터 리스트 클래스.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#ifndef _PLIST_H_
#define _PLIST_H_

#include <windows.h>

//*****************************************************************************
// Name: NODE struct
// Desc: 이중 연결 포인터 리스트의 노드 구조체 선언.
//*****************************************************************************
struct NODE
{
	NODE* pNext;						///< 뒷 노드 주소.						
	NODE* pPrev;						///< 앞 노드 주소.
	void* data;							///< 자료.
};

//*****************************************************************************
// Name: CPList Class
// Desc: 포인터 리스트 클래스 선언.
//*****************************************************************************
class CPList
{
public:
	CPList();							// 생성자.
	~CPList();							// 소멸자.

// Head/Tail Access
	void* GetHead() const;				///< head의 값을 구하는 함수.
	void* GetTail() const;				///< tail의 값을 구하는 함수.

// Operations
	void* RemoveHead();					///< Head를 메모리에서 삭제하는 함수.
	void* RemoveTail();					///< Tail을 메모리에서 삭제하는 함수.
	NODE* AddHead(void* newElement);	///< head에 새로운 노드, 값을 삽입하는 함수.
	NODE* AddTail(void* newElement);	///< tail에 새로운 노드, 값을 삽입하는 함수.
	BOOL AddHead(CPList* pNewList);		///< head에 새로운 list 삽입하는 함수.
	BOOL AddTail(CPList* pNewList);		///< tail에 새로운 list 삽입하는 함수.
	void RemoveAll();					///< list의 모든 메모리를 삭제하는 함수.

// Iteration
	/// head의 position를 구하는 함수.
	NODE* GetHeadPosition() const { return m_pNodeHead; }
	/// tail의 position를 구하는 함수.
	NODE* GetTailPosition() const { return m_pNodeTail; }
	void* GetNext(NODE*& rPosition) const;///<이 node 값 리턴, 다음 노드 주소로.
    void* GetPrev(NODE*& rPosition) const;///<이 node 값 리턴, 이전 노드 주소로.

// Retrieval/Modification
	void* GetAt(NODE* position) const;	///< 이 node 값을 구하는 함수.
	BOOL SetAt(NODE* pos, void* newElement);///< 이 위치에서 새로운 값을 set.
	BOOL RemoveAt(NODE* position);		///< 이 위치의 값을 삭제하는 함수.

// Insertion
	/// 이 위치 앞에 새로운 노드, 값 삽입.
	NODE* InsertBefore(NODE* position, void* newElement);
	///<이 위치 전에 새로운 노드, 값 삽입.
	NODE* InsertAfter(NODE* position, void* newElement);

// Swap
	void Swap(NODE* pNode1, NODE* pNode2);	///< 스왑 함수.

// Searching
	/// 찾을 값의 주소를 구하는 함수.
	NODE* Find(void* searchValue, NODE* startAfter = NULL) const;
	NODE* FindIndex(int nIndex) const;	///< head부터 nIndex번째 주소 리턴.

// Status 
	int GetCount() const { return m_nCount; }		///< node의 갯수를 구하는 함수.
	BOOL IsEmpty() const { return 0 == m_nCount; }	///< list가 비었는가?

protected:
// Data Member
	NODE*	m_pNodeHead;				///< head node 주소.
	NODE*	m_pNodeTail;				///< tail node 주소.
	int		m_nCount;					///< 총 node 수.

// Node Construction/Destruction
	NODE* NewNode(NODE* pPrev, NODE* pNext);///< 새 노드 동적 생성 함수.
	void FreeNode(NODE* pNode);			///< 노드 삭제 함수.
};

#endif