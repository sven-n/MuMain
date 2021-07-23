/*-*-*-*                                                   *-*-*-*/
/*-*-*-*     Base Classes Library  Ver 1.31 Release 018    *-*-*-*/
/*-*-*-*                                                   *-*-*-*/
//- BaseCls.h
//  Template Library of [[ Doubly Linked List ]] and [[ Queue ]] and [[ Stack ]] and [[ Binary Tree ]]
//	and [[ Elastic Dimension ]] and [[ Sorted Queue ]]
//  By SeongMin Hong ( 1999. 3. 18 - 2002. 4. 19)


# ifndef __BASE_CLASSES_H__
# define __BASE_CLASSES_H__



//___________________________________________________________________________________________//
//___________________________________________________________________________________________//
//___________________________________________________________________________________________//
//
//									CNode template class
//___________________________________________________________________________________________//
//___________________________________________________________________________________________//
//___________________________________________________________________________________________//



template <class T> class CList;	// 전방 선언 - CNode에서 사용하기 위함


template <class T>
class CNode
{
	protected:
		T m_Data;	// 기본 데이터
		CNode<T>* m_pPrev;	// 앞에 링크된 노드
		CNode<T>* m_pNext;	// 뒤에 링크된 노드

		// 앞쪽 노드 처리
		void SetPrev( CNode<T>* pPrev)	{ m_pPrev = pPrev; }
		CNode<T>* GetPrev( void)		{ return ( m_pPrev); }

		// 뒤쪽 노드 처리
		void SetNext( CNode<T>* pNext)	{ m_pNext = pNext; }
		CNode<T>* GetNext( void)		{ return ( m_pNext); }

		friend class CList<T>;

	public:
		CNode();
		CNode( T Data);
		~CNode();
		//void* operator new( size_t stAllocate);

		// 데이터 처리
		void SetData( T Data)	{ m_Data = Data; }
		T& GetData( void)		{ return ( m_Data);	}
};


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	Constructor
//
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
CNode<T>::CNode()
{
	m_pPrev = m_pNext = NULL;
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	Constructor	; not tested
//
//	<< Data : 노드가 소유할 데이터
//
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
CNode<T>::CNode( T Data)
{
	m_Data = Data;
	m_pPrev = m_pNext = NULL;
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	Destructor
//
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
CNode<T>::~CNode()
{
}

/*template <class T>
void* CNode<T>::operator new( size_t stAllocate)
{
	return ( new char [stAllocate]);
}*/

//___________________________________________________________________________________________//
//___________________________________________________________________________________________//
//___________________________________________________________________________________________//
//
//									CList template class
//___________________________________________________________________________________________//
//___________________________________________________________________________________________//
//___________________________________________________________________________________________//

template <class T>
class CList
{
	private:
		long m_lCount;	// Stack 안에 들어있는 Node 개수
		//T m_NullData;	// NULL 리턴에 쓰일 값
		CNode<T>* m_pHead;	// Head 노드
		CNode<T>* m_pTail;	// Tail 노드

	public:
		CList();	// Constructor
		~CList();	// Destructor

		LONG GetCount( void)	{ return ( m_lCount); }	// Stack 안에 들어있는 Node 개수를 얻는다.

		CNode<T>* AddHead( T NewElement);	// Head 노드로 추가
		CNode<T>* AddTail( T NewElement);	// Tail 노드로 추가
		CNode<T>* InsertBefore( CNode<T>* pNode, T NewElement);
		CNode<T>* InsertAfter( CNode<T>* pNode, T NewElement);

		T RemoveHead( void);	// Head 노드 제거
		T RemoveTail( void);	// Tail 노드 제거
		T RemoveNode( CNode<T>* &pNode);	// 노드를 찾아서 제거
		void RemoveAll( void);	// 전부 제거

		BOOL IsEmpty( void)	{ return ( m_pHead->GetNext() == m_pTail); }	// 리스트가 비어있는지 체크

		CNode<T>* FindHead( void);	// Head 노드 찾기
		CNode<T>* FindTail( void);	// Tail 노드 찾기
		CNode<T>* FindNode( T SearchValue);	// SearchValue를 Data로 갖고 있는 노드 찾기

		CNode<T>* GetPrev( CNode<T>* pNode);
		CNode<T>* GetNext( CNode<T>* pNode);
		void SetData( CNode<T>* pNode, T Data);
		T& GetData( CNode<T>* pNode);
};



//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	Constructor
//
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
CList<T>::CList()
{
	// 헤더와 테일러를 만든다.
	m_pHead = new CNode<T>;
	m_pTail = new CNode<T>;

	// 헤더와 테일러 연결
	m_pHead->SetNext( m_pTail);
	m_pTail->SetPrev( m_pHead);

	m_lCount = 0;
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	Destructor
//
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
CList<T>::~CList()
{
	RemoveAll();
	delete m_pTail;
	delete m_pHead;
}


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	AddHead - 새로운 값을 가지는 노드를 생성해서 Head로 붙인다.	; not tested
//
//	<< NewElement : 새로 생성되는 노드가 가질 값
//
//	>> CNode<T>* : 새로 생성된 노드
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
CNode<T>* CList<T>::AddHead( T NewElement)
{
	CNode<T>* pOne = new CNode<T>;

	if ( pOne == NULL)
	{	// 메모리 할당이 실패하면
		return ( NULL);
	}

	pOne->SetData( NewElement);

	// 새로운 노드의 뒤에 붙을 것 설정
	pOne->SetNext( m_pHead->GetNext());
	( m_pHead->GetNext())->SetPrev( pOne);
	// 새로운 노드의 앞에 헤더 붙이기
	pOne->SetPrev( m_pHead);
	m_pHead->SetNext( pOne);

	m_lCount++;

	return ( pOne);
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	AddTail - 새로운 값을 가지는 노드를 생성해서 Tail로 붙인다.
//
//	<< NewElement : 새로 생성되는 노드가 가질 값
//
//	>> CNode<T>* : 새로 생성된 노드
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
CNode<T>* CList<T>::AddTail( T NewElement)
{
	CNode<T>* pOne = new CNode<T>;

	if ( pOne == NULL)
	{	// 메모리 할당이 실패하면
		return ( NULL);
	}

	pOne->SetData( NewElement);

	// 새로운 노드의 앞에 붙을 것 설정
	pOne->SetPrev( m_pTail->GetPrev());
	( m_pTail->GetPrev())->SetNext( pOne);
	// 새로운 노드의 뒤에 테일러 붙이기
	pOne->SetNext( m_pTail);
	m_pTail->SetPrev( pOne);

	m_lCount++;

	return ( pOne);
}


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	InsertBefore - 새로운 값을 가지는 노드를 생성해서 pNode 앞에 끼워 넣는다.	; not tested
//
//	<< pNode : 새로운 노드 뒤에 붙을 노드		NewElement : 새로 생성되는 노드가 가질 값
//
//	>> CNode<T>* : 새로 생성된 노드
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
CNode<T>* CList<T>::InsertBefore( CNode<T>* pNode, T NewElement)
{
	CNode<T>* pOne = new CNode<T>;

	if ( pOne == NULL)
	{	// 메모리 할당이 실패하면
		return ( NULL);
	}

	pOne->SetData( NewElement);

	// 새로운 노드 앞에 붙을 것을 설정
	pOne->SetPrev( pNode->GetPrev());
	( pNode->GetPrev())->SetNext( pOne);
	// 새로운 노드 뒤에 pNode 붙이기
	pOne->SetNext( pNode);
	pNode->SetPrev( pOne);

	m_lCount++;

	return ( pOne);
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	InsertAfter - 새로운 값을 가지는 노드를 생성해서 pNode 뒤에 끼워 넣는다.	; not tested
//
//	<< pNode : 새로운 노드 앞에 붙을 노드		NewElement : 새로 생성되는 노드가 가질 값
//
//	>> CNode<T>* : 새로 생성된 노드
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
CNode<T>* CList<T>::InsertAfter( CNode<T>* pNode, T NewElement)
{
	CNode<T>* pOne = new CNode<T>;

	if ( pOne == NULL)
	{	// 메모리 할당이 실패하면
		return ( NULL);
	}

	pOne->SetData( NewElement);

	// 새로운 노드 뒤에 붙을 것을 설정
	pOne->SetNext( pNode->GetNext());
	( pNode->GetNext())->SetPrev( pOne);
	// 새로운 노드 앞에 pNode 붙이기
	pOne->SetPrev( pNode);
	pNode->SetNext( pOne);

	m_lCount++;

	return ( pOne);
}


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	RemoveHead - Head 노드를 제거한다.	; not tested
//
//	>> T : Head 노드에 있던 값
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
T CList<T>::RemoveHead( void)
{
	CNode<T>* pOne = m_pHead->GetNext();
	
	if ( pOne == m_pTail)
	{	// 리스트가 비었으면 끝낸다.
		T NullData;	// NULL 리턴에 필요한 값
		memset( &NullData, 0, sizeof( T));

		return ( NullData);
	}

	// Head에 있던 값 저장
	T Data = pOne->GetData();

	// 헤더가 뒤에 Head의 뒤의 것을 붙이게 설정
	( pOne->GetNext())->SetPrev( m_pHead);
	m_pHead->SetNext( pOne->GetNext());
	
	// 노드 제거
	delete pOne;

	m_lCount--;

	return ( Data);
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	RemoveTail - Tail 노드를 제거한다.
//
//	>> T : Tail 노드에 있던 값
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
T CList<T>::RemoveTail( void)
{
	CNode<T>* pOne = m_pTail->GetPrev();
	
	if ( pOne == m_pHead)
	{	// 리스트가 비었으면 끝낸다.
		T NullData;	// NULL 리턴에 필요한 값
		memset( &NullData, 0, sizeof( T));

		return ( NullData);
	}

	// Tail에 있던 값 저장
	T Data = pOne->GetData();

	// 테일러가 앞에 Tail의 앞의 것을 붙이게 설정
	( pOne->GetPrev())->SetNext( m_pTail);
	m_pTail->SetPrev( pOne->GetPrev());
	
	// 노드 제거
	delete pOne;

	m_lCount--;

	return ( Data);
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	RemoveNode - 해당 노드를 제거한다.	; not tested
//
//	<< &pNode : 제거할 노드 ( 이 포인터는 노드가 제거된 후, 그 뒤에 붙었던 노드를 가리킨다.)
//
//	>> T : 노드에 있던 값
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
T CList<T>::RemoveNode( CNode<T>* &pNode)
{
	if ( pNode == NULL)
	{
		T NullData;	// NULL 리턴에 필요한 값
		memset( &NullData, 0, sizeof( T));

		return ( NullData);
	}

	T Data = pNode->GetData();
	CNode<T>* pNextNode = pNode->GetNext();

	// pNode의 앞 뒤 노드 연결
	( pNode->GetPrev())->SetNext( pNextNode);
	pNextNode->SetPrev( pNode->GetPrev());

	// pNode를 제거하고 그 뒤에 연결된 것을 갖도록 한다.
	delete pNode;
	pNode = ( pNextNode != m_pTail) ? pNextNode : NULL;

	m_lCount--;

	return ( Data);
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	RemoveAll - 모든 노드를 제거한다.
//
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
void CList<T>::RemoveAll( void)
{
	// 테일러쪽으로의 링크를 해제
	( m_pTail->GetPrev())->SetNext( NULL);

	// Head 다음 노드부터 삭제
	CNode<T> *pToSeek, *pToDelete;

	pToDelete = pToSeek = m_pHead->GetNext();
	while ( pToSeek)
	{
		pToSeek = pToSeek->GetNext();
		delete pToDelete;
		pToDelete = pToSeek;
	}

	// 헤더와 테일러 연결 ( 헤더와 테일러는 제거되지 않는다.)
	m_pHead->SetNext( m_pTail);
	m_pTail->SetPrev( m_pHead);

	m_lCount = 0;
}


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	FindHead - Head 노드를 얻는다.
//
//	>> CNode<T>* : Head 노드의 포인터
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
CNode<T>* CList<T>::FindHead( void)
{
	if ( IsEmpty())
	{
		return ( NULL);
	}
	else
	{
		return ( m_pHead->GetNext());
	}
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	FindTail - Tail 노드를 얻는다.	; not tested
//
//	>> CNode<T>* : Tail 노드의 포인터
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
CNode<T>* CList<T>::FindTail( void)
{
	if( IsEmpty())
	{
		return ( NULL);
	}
	else
	{
		return ( m_pTail->GetPrev());
	}
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	FindNode - 특정값을 가지고 있는 노드를 찾는다.	; not tested
//
//	<< SearchValue : 찾고 싶은 노드가 가지고 있는 값
//
//	>> CNode<T>* : 발견한 노드의 포인터
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
CNode<T>* CList<T>::FindNode( T SearchValue)
{
	// 테일러쪽으로의 링크 해제
	( m_pTail->GetPrev())->SetNext( NULL);

	// Head부터 검색
	CNode<T> *pToSeek = m_pHead->GetNext();

	while( pToSeek)
	{
		if ( pToSeek->GetData() == SearchValue)
		{
			// 테일러쪽으로의 링크 복구
			( m_pTail->GetPrev())->SetNext( m_pTail);
			return ( pToSeek);
		}

		pToSeek = pToSeek->GetNext();
	}

	// 테일러쪽으로의 링크 복구
	( m_pTail->GetPrev())->SetNext( m_pTail);

	// 찾지 못한 경우 NULL 리턴
	return ( NULL);
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	GetPrev - 노드의 앞에 있는 노드를 얻는다.
//
//	<< pNode : 기준 노드
//
//	>> CNode<T>* : 기준 노드의 앞 노드. 없으면 NULL
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
CNode<T>* CList<T>::GetPrev( CNode<T>* pNode)
{
	if ( m_pHead == pNode->GetPrev())
	{	// 헤더인 경우를 조사해서 앞쪽 노드가 없는 것으로 처리
		return ( NULL);
	}
	else
	{
		return ( pNode->GetPrev());
	}
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	GetNext - 노드의 뒤에 있는 노드를 얻는다.
//
//	<< pNode : 기준 노드
//
//	>> CNode<T>* : 기준 노드의 뒤에 있는 노드. 없으면 NULL
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
CNode<T>* CList<T>::GetNext( CNode<T>* pNode)
{
	if ( m_pTail == ( pNode->GetNext()))
	{	// 테일러인 경우를 조사해서 뒤쪽 노드가 없는 것으로 처리
		return ( NULL);
	}
	else
	{
		return ( pNode->GetNext());
	}
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	SetData - 노드의 값을 지정한다.
//
//	<< pNode : 변경할 노드의 포인터		Data : 새로 대입될 값
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
void CList<T>::SetData( CNode<T>* pNode, T Data)
{
	pNode->SetData( Data);
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	GetData - 노드가 가지고 있는 값을 얻는다. ( 참조값이므로 값이 클래스인 경우 멤버 함수 이용 가능)
//
//	<< pNode : 값을 얻고자 하는 노드의 포인터	T& : 노드가 가지고 있는 값의 참조값
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
T& CList<T>::GetData( CNode<T>* pNode)
{
	if ( m_pHead == pNode || m_pTail == pNode)
	{
		T NullData;	// NULL 리턴에 필요한 값
		memset( &NullData, 0, sizeof( T));

		return ( NullData);
	}
	else
	{
		return ( pNode->GetData());
	}
}


//___________________________________________________________________________________________//
//___________________________________________________________________________________________//
//___________________________________________________________________________________________//
//
//									CQueue template class
//___________________________________________________________________________________________//
//___________________________________________________________________________________________//
//___________________________________________________________________________________________//

template <class T>
class CQueue : private CList<T>
{
	public:
		CQueue();
		~CQueue();

		BOOL Insert( T NewElement);
		T Remove( void);
		void CleanUp( void);

		BOOL Find( T Element);

		LONG GetCount( void)	{ return ( CList<T>::GetCount()); }	// Stack 안에 들어있는 Node 개수를 얻는다.
};



//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	Constructor
//
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
CQueue<T>::CQueue() : CList<T>()
{
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	Destructor
//
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
CQueue<T>::~CQueue()
{
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	Insert - 큐에 새로운 값을 넣는다.	; not tested
//
//	<< NewElement : 큐에 넣어질 값
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
BOOL CQueue<T>::Insert( T NewElement)
{
	if ( NULL == AddTail( NewElement))
	{	// 실패하면
		return ( FALSE);
	}

	return ( TRUE);
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	Remove - 큐에 들어있는 값을 꺼낸다. ( 큐가 비었으면 NULL을 리턴한다.)	; not tested
//
//	>> T : 큐에서 꺼내진 값
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
T CQueue<T>::Remove( void)
{
	return( RemoveHead());
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	CleanUp - 큐를 비운다.	; not tested
//
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
void CQueue<T>::CleanUp( void)
{
	RemoveAll();
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	Find - 큐에 해당하는 값이 들어있는지 검사한다.	; not tested
//
//	<< Element : 검사할 값
//
//	>> BOOL : 큐에 값이 들어있는지 여부
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
BOOL CQueue<T>::Find( T Element)
{
	if ( NULL == FindNode( Element))
	{	// 찾지 못하면
		return ( FALSE);
	}

	return ( TRUE);
}


//___________________________________________________________________________________________//
//___________________________________________________________________________________________//
//___________________________________________________________________________________________//
//
//									CStack template class
//___________________________________________________________________________________________//
//___________________________________________________________________________________________//
//___________________________________________________________________________________________//

/*template <class T>
class CStack : private CList<T>
{
	public:
		CStack();
		~CStack();

		BOOL Push(T NewElement);
		T Pop(void);
		void CleanUp( void);

		BOOL Find( T Element);

		LONG GetCount( void)	{ return ( m_lCount); }	// Stack 안에 들어있는 Node 개수를 얻는다.
};



//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	Constructor
//
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
CStack<T>::CStack() : CList()
{
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	Destructor
//
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
CStack<T>::~CStack()
{
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	Push - 스택에 새로운 값을 넣는다.	; not tested
//
//	<< NewElement : 스택에 넣어질 값
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
BOOL CStack<T>::Push( T NewElement)
{
	if ( NULL == AddTail( T))
	{	// 실패하면
		return ( FALSE);
	}

	return ( TRUE);
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	Pop - 스택에 들어있는 값을 꺼낸다. ( 스택이 비었으면 NULL을 리턴한다.)	; not tested
//
//	>> T : 스택에서 꺼내진 값
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
T CStack<T>::Pop( void)
{
	return( RemoveTail());
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	CleanUp - 스택을 비운다.	; not tested
//
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
void CStack<T>::CleanUp( void)
{
	RemoveAll();
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//	Find - 스택에 해당하는 값이 들어있는지 검사한다.	; not tested
//
//	<< Element : 검사할 값
//
//	>> BOOL : 스택에 값이 들어있는지 여부
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
template <class T>
BOOL CStack<T>::Find( T Element)
{
	if ( NULL == FindNode( Element))
	{	// 찾지 못하면
		return ( FALSE);
	}

	return ( TRUE);
}
*/


//___________________________________________________________________________________________//
//___________________________________________________________________________________________//
//___________________________________________________________________________________________//
//
//									CBNode template class
//___________________________________________________________________________________________//
//___________________________________________________________________________________________//
//___________________________________________________________________________________________//


template <class T, class S> class CBTree;	// 전방 선언 - CBNode에서 사용하기 위함

template <class T, class S>
class CBNode
{
	protected:
		T m_Data;	// 기본 데이터
		S m_CompValue;	// 비교시에 사용하는 값
		CBNode<T, S>* m_pLeft;		// 왼쪽 자식 노드
		CBNode<T, S>* m_pRight;	// 오른쪽 자식 노드
		CBNode<T, S>* m_pParent;

		void SetLeft( CBNode<T, S>* pLeft)	{ m_pLeft = pLeft; pLeft->m_pParent = this; }
		void SetRight( CBNode<T, S>* pRight) { m_pRight = pRight; pRight->m_pParent = this;  }
		CBNode<T, S>* GetLeft( void)	{ return ( m_pLeft); }
		CBNode<T, S>* GetRight( void)	{ return ( m_pRight); }
		CBNode<T, S>* GetParent( void)	{ return ( m_pParent); }

		friend class CBTree<T, S>;
		friend CBTree<T, S>& operator + ( const CBTree<T, S>& Value1, const CBTree<T, S>& Value2);

	public:

		CBNode();
		CBNode( T Data, S CompValue);
		~CBNode();

		// 데이터 처리
		void SetData( T Data)	{ m_Data = Data; }
		void SetValue( S CompValue)	{ m_CompValue = CompValue; }
		T& GetData( void)		{ return ( m_Data);	}
		S GetValue( void)	{ return ( m_CompValue); }
};


template <class T, class S>
CBNode<T, S>::CBNode()
{
	m_pLeft = m_pRight = m_pParent = NULL;
}

template <class T, class S>
CBNode<T, S>::CBNode( T Data, S CompValue)
{
	m_pLeft = m_pRight = m_pParent = NULL;
	m_Data = Data;
	m_CompValue = CompValue;
}

template <class T, class S>
CBNode<T, S>::~CBNode()
{
}


//___________________________________________________________________________________________//
//___________________________________________________________________________________________//
//___________________________________________________________________________________________//
//
//								CBTree template class
//___________________________________________________________________________________________//
//___________________________________________________________________________________________//
//___________________________________________________________________________________________//

template <class T, class S>		// S 는 < 연산이 있는 것이어야 한다.
class CBTree
{
	protected:
		long m_lCount;	// List 안에 들어있는 Node 개수
		CBNode<T, S>* m_pHead;	// Head 노드

		void RemoveFrom( CBNode<T, S>* pNode);	// 뒤의 것 다 제거
		void CycleFrom( CBNode<T, S>* pNode, DWORD dwParam);
		void ( *m_Process)( T Data, S CompValue, DWORD dwParam);

	public:
		CBTree();	// Constructor
		CBTree( const CBTree<T, S>& RValue);	// 복사 컨스트럭터
		~CBTree();	// Destructor

		// 연산자 오버로딩
		CBTree<T, S>& operator = ( const CBTree<T, S>& RValue);
		friend CBTree<T, S>& operator + ( const CBTree<T, S>& Value1, const CBTree<T, S>& Value2);


		// 일반 함수
		CBNode<T, S>* Add( T NewElement, S CompValue);	// 추가
		T RemoveNode( CBNode<T, S>* &pNode);	// 노드를 찾아서 제거
		void RemoveAll( void);	// 전부 제거

		BOOL IsEmpty( void)	{ return ( m_lCount == 0); }	// 리스트가 비어있는지 체크
		virtual LONG GetCount( void)	{ return ( m_lCount); }	// List 안에 들어있는 Node 개수를 얻는다.

		//CBNode<T, S>* FindNode( T SearchValue);	// SearchValue를 Data로 갖고 있는 노드 찾기
		CBNode<T, S>* FindNode( S CompValue);	// Value를 갖고 있는 노드 찾기

		CBNode<T, S>* FindHead( void);
		CBNode<T, S>* GetLeft( CBNode<T, S>* pNode);
		CBNode<T, S>* GetRight( CBNode<T, S>* pNode);
		CBNode<T, S>* GetParent( CBNode<T, S>* pNode);

		void SetData( CBNode<T, S>* pNode, T Data);
		T& GetData( CBNode<T, S>* pNode);
		S GetValue( CBNode<T, S>* pNode);

		void Cycle( void ( *Process)( T Data, S CompValue, DWORD dwParam), DWORD dwParam);

	protected:
		void CopyFrom( CBNode<T, S>* pNode);	// 해당 노드에서부터의 내용을 현재 트리에 추가시킨다.
		void UnlinkOneBottomNode( CBNode<T, S>* &pNode);

	public:
		BOOL Optimize( void);
	protected:
		BOOL GetOptimizeList( T *pData, S *pCompValue);
		void AddDataByOptimization( T *pData, S *pCompValue, int nLeft, int nRight);
};


template <class T, class S>
CBTree<T, S>::CBTree()
{
	// 노드 개수 초기화
	m_lCount = 0;
	m_pHead = NULL;
}

template <class T, class S>
CBTree<T, S>::CBTree( const CBTree<T, S>& RValue)
{
	// 노드 개수 초기화
	m_lCount = 0;
	m_pHead = NULL;

	// 복사
	if ( RValue.m_pHead)
	{
		CopyFrom( RValue.m_pHead);
	}
}

template <class T, class S>
CBTree<T, S>::~CBTree()
{
	RemoveAll();
}

template <class T, class S>
CBTree<T, S>& CBTree<T, S>::operator = ( const CBTree<T, S>& RValue)
{
	if ( &RValue == this)
	{
		return ( *this);
	}

	RemoveAll();
	if ( RValue.m_pHead)
	{
		CopyFrom( RValue.m_pHead);
	}

	return ( *this);
}

template <class T, class S>
CBTree<T, S>& operator + ( const CBTree<T, S>& Value1, const CBTree<T, S>& Value2)
{	// 쓰지 말 것
	CBTree<T, S> Result;
	CBTree<T, S> Left;
	CBTree<T, S> Right;
	if ( Value1.m_pHead->GetValue() <= Value2.m_pHead->GetValue())
	{
		Left = Value1;
		Right = Value2;
	}
	else
	{
		Left = Value2;
		Right = Value1;
	}

	// 머리 생성
	T NullData;
	memset( &NullData, 0, sizeof( T));
	S CompValue = 0;
	if ( Left.m_pHead != NULL && Right.m_pHead != NULL)
	{
		CompValue = ( Left.m_pHead->GetValue() + Left.m_pHead->GetValue() + 1) / 2;
	}
	Result.Add( NullData, CompValue);

	// 좌 우 연결
	Result.m_pHead->SetLeft( Left.m_pHead);
	Result.m_pHead->SetRight( Right.m_pHead);
	Result.m_lCount = Left.m_lCount + Right.m_lCount + 1;

	// 노드들이 제거되지 않게 하려면 이 작업이 필요하다.
	Left.m_pHead = NULL;
	Right.m_pHead = NULL;

	return ( CBTree<T, S>( Result));
}


template <class T, class S>
void CBTree<T, S>::CopyFrom( CBNode<T, S>* pNode)
{
	Add( pNode->GetData(), pNode->GetValue());
	if ( pNode->GetLeft())
	{
		CopyFrom( pNode->GetLeft());
	}
	if ( pNode->GetLeft())
	{
		CopyFrom( pNode->GetRight());
	}
}


template <class T, class S>
CBNode<T, S>* CBTree<T, S>::Add( T NewElement, S CompValue)
{
	if ( m_pHead == NULL)
	{	// 머리에 추가
		m_lCount++;
		m_pHead = new CBNode<T, S>( NewElement, CompValue);

		return ( m_pHead);
	}

	CBNode<T, S>* pNode = m_pHead;
	CBNode<T, S>* pPrev = m_pHead;
	while( pNode != NULL)
	{
		if ( CompValue < pNode->GetValue())
		{
			pPrev = pNode;
			pNode = pNode->GetLeft();
		}
		else
		{
			pPrev = pNode;
			pNode = pNode->GetRight();
		}
	}

	if ( pNode == NULL)
	{	// 추가
		m_lCount++;
		CBNode<T, S>* pOne = new CBNode<T, S>( NewElement, CompValue);

		if ( CompValue < pPrev->GetValue())
		{	// 왼쪽
			pPrev->SetLeft( pOne);
		}
		else
		{
			pPrev->SetRight( pOne);
		}

		return ( pOne);
	}
	else
	{	// 값이 있으므로
		return ( pNode);
	}
}

template <class T, class S>
T CBTree<T, S>::RemoveNode( CBNode<T, S>* &pNode)
{	// 노드 제거는 트리 없앨 때만 쓸 수 있음 ( 아직은 )
	// 노드 값 얻어 놓기
	T ResultData = pNode->GetData();

	// 트리 구조 재정비
	// 왼쪽 자식의 가장 오른쪽에 있는 거나 오른쪽 자식의 가장 왼쪽에 있는 걸로 바꾼다.

	// 1. 대체할 것을 얻는다.
	CBNode<T, S> *pOld = NULL;

	CBNode<T, S> *pSeek = pNode->GetLeft();
	if ( pSeek)
	{
		if ( !pNode->GetRight())
		{	// 왼쪽만 남은 경우
			if ( pNode == m_pHead)
			{
				m_pHead = pSeek;
				m_pHead->m_pParent = NULL;
			}
			else
			{
				if ( pNode == pNode->GetParent()->m_pRight)
				{
					pNode->GetParent()->SetRight( pSeek);
				}
				else
				{
					pNode->GetParent()->SetLeft( pSeek);
				}
			}
			delete pNode;
			m_lCount--;
			return ( ResultData);
		}
		// 둘 다 남은 경우
		pOld = pSeek;
		while ( pSeek)
		{
			pOld = pSeek;
			pSeek = pSeek->GetRight();
		}

		// 2. 그 값을 제거한다.
		T Data = pOld->m_Data;
		S CompValue = pOld->m_CompValue;
		//UnlinkOneBottomNode( pOld);
		RemoveNode( pOld);

		// 3. 없애려는 값에 새로운 값을 넣어준다.
		pNode->m_Data = Data;
		pNode->m_CompValue = CompValue;

		return ( ResultData);
	}
	else
	{
		pSeek = pNode->GetRight();
		if ( pSeek)
		{	// 오른쪽만 남은 경우
			if ( pNode == m_pHead)
			{
				m_pHead = pSeek;
				m_pHead->m_pParent = NULL;
			}
			else
			{
				if ( pNode == pNode->GetParent()->m_pRight)
				{
					pNode->GetParent()->SetRight( pSeek);
				}
				else
				{
					pNode->GetParent()->SetLeft( pSeek);
				}
			}
			delete pNode;
			m_lCount--;
			return ( ResultData);
		}
	}

	// 둘다 없는 경우
	UnlinkOneBottomNode( pNode);
	return ( ResultData);
}

template <class T, class S>
void CBTree<T, S>::UnlinkOneBottomNode( CBNode<T, S>* &pNode)
{
	if ( pNode->GetParent())
	{
		if ( pNode->GetParent()->GetLeft() == pNode)
		{
			pNode->GetParent()->m_pLeft = NULL;
		}
		else
		{
			pNode->GetParent()->m_pRight = NULL;
		}
	}
	else
	{
		m_pHead = NULL;
	}

	// 제거
	m_lCount--;
	delete pNode;
	pNode = NULL;
}

template <class T, class S>
void CBTree<T, S>::RemoveAll( void)
{
	if ( m_pHead != NULL)
	{
		RemoveFrom( m_pHead);
		m_pHead = NULL;
	}
	m_lCount = 0;
}

template <class T, class S>
void CBTree<T, S>::RemoveFrom( CBNode<T, S>* pNode)
{
	if ( pNode->GetLeft() != NULL)
	{
		RemoveFrom( pNode->GetLeft());
	}

	if ( pNode->GetRight() != NULL)
	{
		RemoveFrom( pNode->GetRight());
	}

	RemoveNode( pNode);
}


template <class T, class S>
CBNode<T, S>* CBTree<T, S>::FindNode( S CompValue)
{
	CBNode<T, S>* pNode = m_pHead;
	while( pNode != NULL && pNode->GetValue() != CompValue)
	{
		if ( CompValue < pNode->GetValue())
		{
			pNode = pNode->GetLeft();
		}
		else
		{
			pNode = pNode->GetRight();
		}
	}

	return ( pNode);
}

template <class T, class S>
CBNode<T, S>* CBTree<T, S>::FindHead( void)
{
	return ( m_pHead);
}

template <class T, class S>
CBNode<T, S>* CBTree<T, S>::GetLeft( CBNode<T, S>* pNode)
{
	return ( pNode->GetLeft());
}

template <class T, class S>
CBNode<T, S>* CBTree<T, S>::GetRight( CBNode<T, S>* pNode)
{
	return ( pNode->GetRight());
}

template <class T, class S>
CBNode<T, S>* GetParent( CBNode<T, S>* pNode)
{
	return ( pNode->GetParent());
}

template <class T, class S>
void CBTree<T, S>::SetData( CBNode<T, S>* pNode, T Data)
{
	pNode->SetData( Data);
}

template <class T, class S>
T& CBTree<T, S>::GetData( CBNode<T, S>* pNode)
{
	return ( pNode->GetData());
}

template <class T, class S>
S CBTree<T, S>::GetValue( CBNode<T, S>* pNode)
{
	return ( pNode->GetValue());
}

template <class T, class S>
void CBTree<T, S>::Cycle( void ( *Process)( T Data, S CompValue, DWORD dwParam), DWORD dwParam)
{
	m_Process = Process;

	if ( m_pHead == NULL)
	{
		return;
	}

	if ( m_pHead->GetLeft() != NULL)
	{
		CycleFrom( m_pHead->GetLeft(), dwParam);
	}

	( *m_Process)( m_pHead->GetData(), m_pHead->GetValue(), dwParam);

	if ( m_pHead->GetRight() != NULL)
	{
		CycleFrom( m_pHead->GetRight(), dwParam);
	}
}

template <class T, class S>
void CBTree<T, S>::CycleFrom( CBNode<T, S>* pNode, DWORD dwParam)
{
	if ( pNode->GetLeft() != NULL)
	{
		CycleFrom( pNode->GetLeft(), dwParam);
	}

	( *m_Process)( pNode->GetData(), pNode->GetValue(), dwParam);

	if ( pNode->GetRight() != NULL)
	{
		CycleFrom( pNode->GetRight(), dwParam);
	}
}

template <class T, class S>
BOOL CBTree<T, S>::Optimize( void)
{
	T *pData = new T[GetCount()];
	S *pCompValue = new S[GetCount()];
	if ( !GetOptimizeList( pData, pCompValue))
	{
		delete [] pCompValue;
		delete [] pData;
		return ( FALSE);
	}

	long lCount = GetCount();
	RemoveAll();
	AddDataByOptimization( pData, pCompValue, 0, lCount);
	delete [] pCompValue;
	delete [] pData;

	return ( TRUE);
}

template <class T, class S>
void ProcessGetOptimizeList( T Data, S CompValue, DWORD dwParam)
{
	DWORD *pdwParam = ( DWORD *)dwParam;
	long *pCount = ( long*)pdwParam[0];
	T *pData = ( T *)pdwParam[1];
	S *pCompValue = ( S *)pdwParam[2];
	pData[*pCount] = Data;
	pCompValue[( *pCount)++] = CompValue;
}

template <class T, class S>
BOOL CBTree<T, S>::GetOptimizeList( T *pData, S *pCompValue)
{
	long lOptimizeCount = 0;
	DWORD dwOptimizeData[3] = { ( DWORD)&lOptimizeCount, ( DWORD)pData, ( DWORD)pCompValue};
	Cycle( ProcessGetOptimizeList, ( DWORD)dwOptimizeData);
	if ( GetCount() != lOptimizeCount)
	{
		return ( FALSE);
	}

	return ( TRUE);
}

template <class T, class S>
void CBTree<T, S>::AddDataByOptimization( T *pData, S *pCompValue, int nLeft, int nRight)
{
	if ( nLeft == nRight)
	{
		return;
	}

	Add( pData[( nLeft + nRight) / 2], pCompValue[( nLeft + nRight) / 2]);

	AddDataByOptimization( pData, pCompValue, nLeft, ( nLeft + nRight) / 2);
	AddDataByOptimization( pData, pCompValue, ( nLeft + nRight) / 2 + 1, nRight);
}


//___________________________________________________________________________________________//
//___________________________________________________________________________________________//
//___________________________________________________________________________________________//
//
//									CList template class
//___________________________________________________________________________________________//
//___________________________________________________________________________________________//
//___________________________________________________________________________________________//
// 크기가 자유롭게 늘어나는 배열

template <class T>
class CDimension
{
	protected:
		int m_nSize;
		T *m_pData;

	public:
		CDimension();	// Constructor
		~CDimension();	// Destructor

		T Set( int nIndex, T Data);
		T Get( int nIndex);

	protected:
		void CheckDimensionSize( int nIndex);
};


template <class T>
CDimension<T>::CDimension()
{
	m_nSize = 16;
	m_pData = new T [m_nSize];
}

template <class T>
CDimension<T>::~CDimension()
{
	delete [] m_pData;
}

template <class T>
T CDimension<T>::Set( int nIndex, T Data)
{
	CheckDimensionSize( nIndex);
	m_pData[nIndex] = Data;

	return ( Data);
}

template <class T>
T CDimension<T>::Get( int nIndex)
{
	CheckDimensionSize( nIndex);
	return ( m_pData[nIndex]);
}

template <class T>
void CDimension<T>::CheckDimensionSize( int nIndex)
{
	if ( nIndex >= m_nSize)
	{	// 넘쳤으면 크기를 늘려준다.
		for ( int nNewSize = m_nSize; nNewSize <= nIndex; nNewSize *= 2)
		{
		}

		// 크기를 늘린 다음 복사
		T *pTempBuffer = new T [m_nSize];
		memcpy( pTempBuffer, m_pData, m_nSize * sizeof ( T));
		delete [] m_pData;
		m_pData = new T [nNewSize];
		memcpy( m_pData, pTempBuffer, m_nSize * sizeof ( T));
		m_nSize = nNewSize;
		delete [] pTempBuffer;
	}
}


# endif //__BASE_CLASSES_H__


/*-*-*-*                    End of Code                      *-*-*-*/
