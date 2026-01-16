//*****************************************************************************
// File: PList.h
//*****************************************************************************

#ifndef _PLIST_H_
#define _PLIST_H_

struct NODE
{
    NODE* pNext;
    NODE* pPrev;
    void* data;
};

class CPList
{
public:
    CPList();
    ~CPList();

    // Head/Tail Access
    void* GetHead() const;
    void* GetTail() const;

    void* RemoveHead();
    void* RemoveTail();
    NODE* AddHead(void* newElement);
    NODE* AddTail(void* newElement);
    BOOL AddHead(CPList* pNewList);
    BOOL AddTail(CPList* pNewList);
    void RemoveAll();

    NODE* GetHeadPosition() const { return m_pNodeHead; }
    NODE* GetTailPosition() const { return m_pNodeTail; }
    void* GetNext(NODE*& rPosition) const;
    void* GetPrev(NODE*& rPosition) const;

    void* GetAt(NODE* position) const;
    BOOL SetAt(NODE* pos, void* newElement);
    BOOL RemoveAt(NODE* position);

    NODE* InsertBefore(NODE* position, void* newElement);
    NODE* InsertAfter(NODE* position, void* newElement);

    void Swap(NODE* pNode1, NODE* pNode2);

    NODE* Find(void* searchValue, NODE* startAfter = NULL) const;
    NODE* FindIndex(int nIndex) const;

    int GetCount() const { return m_nCount; }
    BOOL IsEmpty() const { return 0 == m_nCount; }

protected:
    NODE* m_pNodeHead;
    NODE* m_pNodeTail;
    int		m_nCount;

    NODE* NewNode(NODE* pPrev, NODE* pNext);
    void FreeNode(NODE* pNode);
};

#endif