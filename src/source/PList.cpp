//*****************************************************************************
// File: PList.cpp
//*****************************************************************************

#include "stdafx.h"
#include "PList.h"

CPList::CPList()
{
    m_nCount = 0;
    m_pNodeHead = m_pNodeTail = NULL;
}

CPList::~CPList()
{
    RemoveAll();
}

void* CPList::GetHead() const
{
    if (NULL == m_pNodeHead)
        return NULL;

    return m_pNodeHead->data;
}

void* CPList::GetTail() const
{
    if (NULL == m_pNodeTail)
        return NULL;

    return m_pNodeTail->data;
}

void* CPList::RemoveHead()
{
    if (NULL == m_pNodeHead)
        return NULL;

    if (::IsBadReadPtr(m_pNodeHead, sizeof(NODE)))
        return NULL;

    NODE* pOldNode = m_pNodeHead;
    void* returnValue = pOldNode->data;
    m_pNodeHead = pOldNode->pNext;
    if (m_pNodeHead != NULL)
        m_pNodeHead->pPrev = NULL;
    else
        m_pNodeTail = NULL;
    FreeNode(pOldNode);

    return returnValue;
}

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

NODE* CPList::AddHead(void* newElement)
{
    NODE* pNewNode = NewNode(NULL, m_pNodeHead);
    pNewNode->data = newElement;
    if (m_pNodeHead != NULL)
        m_pNodeHead->pPrev = pNewNode;
    else
        m_pNodeTail = pNewNode;
    m_pNodeHead = pNewNode;

    return pNewNode;
}

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

BOOL CPList::AddHead(CPList* pNewList)
{
    if (NULL == pNewList)
        return FALSE;

    NODE* pos = pNewList->GetTailPosition();
    while (pos != NULL)
        AddHead(pNewList->GetPrev(pos));

    return TRUE;
}

BOOL CPList::AddTail(CPList* pNewList)
{
    if (NULL == pNewList)
        return FALSE;

    NODE* pos = pNewList->GetHeadPosition();
    while (pos != NULL)
        AddTail(pNewList->GetNext(pos));

    return TRUE;
}

void CPList::RemoveAll()
{
    NODE* pNode;
    while (NULL != m_pNodeHead)
    {
        pNode = m_pNodeHead;
        m_pNodeHead = m_pNodeHead->pNext;
        delete pNode;
    }
    m_pNodeTail = NULL;
    m_nCount = 0;
}

void* CPList::GetNext(NODE*& rPosition) const
{
    NODE* pNode = rPosition;
    if (::IsBadReadPtr(pNode, sizeof(NODE)))
        return NULL;

    rPosition = pNode->pNext;

    return pNode->data;
}

void* CPList::GetPrev(NODE*& rPosition) const
{
    NODE* pNode = rPosition;
    if (::IsBadReadPtr(pNode, sizeof(NODE)))
        return NULL;

    rPosition = pNode->pPrev;

    return pNode->data;
}

void* CPList::GetAt(NODE* position) const
{
    NODE* pNode = position;
    if (::IsBadReadPtr(pNode, sizeof(NODE)))
        return NULL;

    return pNode->data;
}

BOOL CPList::SetAt(NODE* pos, void* newElement)
{
    NODE* pNode = pos;
    if (::IsBadReadPtr(pNode, sizeof(NODE)))
        return FALSE;

    pNode->data = newElement;

    return TRUE;
}

BOOL CPList::RemoveAt(NODE* position)
{
    NODE* pOldNode = position;
    if (::IsBadReadPtr(pOldNode, sizeof(NODE)))
        return FALSE;

    if (pOldNode == m_pNodeHead)
        m_pNodeHead = pOldNode->pNext;
    else
    {
        if (::IsBadReadPtr(pOldNode->pPrev, sizeof(NODE)))
            return FALSE;
        pOldNode->pPrev->pNext = pOldNode->pNext;
    }

    if (pOldNode == m_pNodeTail)
        m_pNodeTail = pOldNode->pPrev;
    else
    {
        if (::IsBadReadPtr(pOldNode->pNext, sizeof(NODE)))
            return FALSE;
        pOldNode->pNext->pPrev = pOldNode->pPrev;
    }
    FreeNode(pOldNode);

    return TRUE;
}

NODE* CPList::InsertBefore(NODE* position, void* newElement)
{
    if (position == NULL)
        return AddHead(newElement);

    NODE* pOldNode = position;
    NODE* pNewNode = NewNode(pOldNode->pPrev, pOldNode);
    pNewNode->data = newElement;
    if (pOldNode->pPrev != NULL)
        pOldNode->pPrev->pNext = pNewNode;
    else
        m_pNodeHead = pNewNode;
    pOldNode->pPrev = pNewNode;

    return pNewNode;
}

NODE* CPList::InsertAfter(NODE* position, void* newElement)
{
    if (position == NULL)
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

void CPList::Swap(NODE* pNode1, NODE* pNode2)
{
    if (pNode1 == NULL || pNode2 == NULL)
        return;

    void* temp;
    temp = pNode1->data;
    pNode1->data = pNode2->data;
    pNode2->data = temp;
}

NODE* CPList::Find(void* searchValue, NODE* startAfter) const
{
    NODE* pNode = startAfter;
    if (pNode == NULL)
        pNode = m_pNodeHead;
    else
    {
        if (::IsBadReadPtr(pNode, sizeof(NODE)))
            return NULL;

        pNode = pNode->pNext;
    }

    for (; pNode != NULL; pNode = pNode->pNext)
        if (pNode->data == searchValue)
            return pNode;

    return NULL;
}

NODE* CPList::FindIndex(int nIndex) const
{
    if (nIndex >= m_nCount || nIndex < 0)
        return NULL;

    NODE* pNode = m_pNodeHead;
    while (nIndex--)
        pNode = pNode->pNext;

    return pNode;
}

NODE* CPList::NewNode(NODE* pPrev, NODE* pNext)
{
    NODE* pNode = new NODE;
    pNode->pPrev = pPrev;
    pNode->pNext = pNext;
    m_nCount++;
    pNode->data = NULL;
    return pNode;
}

void CPList::FreeNode(NODE* pNode)
{
    delete pNode;
    m_nCount--;
    if (m_nCount == 0)
        RemoveAll();
}