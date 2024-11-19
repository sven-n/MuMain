# ifndef __BASE_CLASSES_H__
# define __BASE_CLASSES_H__

template <class T> class CList;

template <class T>
class CNode
{
protected:
    T m_Data;
    CNode<T>* m_pPrev;
    CNode<T>* m_pNext;

    void SetPrev(CNode<T>* pPrev) { m_pPrev = pPrev; }
    CNode<T>* GetPrev(void) { return (m_pPrev); }

    void SetNext(CNode<T>* pNext) { m_pNext = pNext; }
    CNode<T>* GetNext(void) { return (m_pNext); }

    friend class CList<T>;

public:
    CNode();
    CNode(T Data);
    ~CNode();
    void SetData(T Data) { m_Data = Data; }
    T& GetData(void) { return (m_Data); }
};

template <class T>
CNode<T>::CNode()
{
    m_pPrev = m_pNext = NULL;
}

template <class T>
CNode<T>::CNode(T Data)
{
    m_Data = Data;
    m_pPrev = m_pNext = NULL;
}

template <class T>
CNode<T>::~CNode()
{
}

template <class T>
class CList
{
private:
    long m_lCount;
    //T m_NullData;
    CNode<T>* m_pHead;
    CNode<T>* m_pTail;

public:
    CList();	// Constructor
    ~CList();	// Destructor

    LONG GetCount(void) { return (m_lCount); }

    CNode<T>* AddHead(T NewElement);
    CNode<T>* AddTail(T NewElement);
    CNode<T>* InsertBefore(CNode<T>* pNode, T NewElement);
    CNode<T>* InsertAfter(CNode<T>* pNode, T NewElement);

    T RemoveHead(void);
    T RemoveTail(void);
    T RemoveNode(CNode<T>*& pNode);
    void RemoveAll(void);

    BOOL IsEmpty(void) { return (m_pHead->GetNext() == m_pTail); }

    CNode<T>* FindHead(void);
    CNode<T>* FindTail(void);
    CNode<T>* FindNode(T SearchValue);

    CNode<T>* GetPrev(CNode<T>* pNode);
    CNode<T>* GetNext(CNode<T>* pNode);
    void SetData(CNode<T>* pNode, T Data);
    T& GetData(CNode<T>* pNode);
};

template <class T>
CList<T>::CList()
{
    m_pHead = new CNode<T>;
    m_pTail = new CNode<T>;
    m_pHead->SetNext(m_pTail);
    m_pTail->SetPrev(m_pHead);
    m_lCount = 0;
}

template <class T>
CList<T>::~CList()
{
    RemoveAll();
    delete m_pTail;
    delete m_pHead;
}

template <class T>
CNode<T>* CList<T>::AddHead(T NewElement)
{
    CNode<T>* pOne = new CNode<T>;

    if (pOne == NULL)
    {
        return (NULL);
    }

    pOne->SetData(NewElement);

    pOne->SetNext(m_pHead->GetNext());
    (m_pHead->GetNext())->SetPrev(pOne);
    pOne->SetPrev(m_pHead);
    m_pHead->SetNext(pOne);

    m_lCount++;

    return (pOne);
}

template <class T>
CNode<T>* CList<T>::AddTail(T NewElement)
{
    CNode<T>* pOne = new CNode<T>;

    if (pOne == NULL)
    {
        return (NULL);
    }

    pOne->SetData(NewElement);

    pOne->SetPrev(m_pTail->GetPrev());
    (m_pTail->GetPrev())->SetNext(pOne);
    pOne->SetNext(m_pTail);
    m_pTail->SetPrev(pOne);

    m_lCount++;

    return (pOne);
}

template <class T>
CNode<T>* CList<T>::InsertBefore(CNode<T>* pNode, T NewElement)
{
    CNode<T>* pOne = new CNode<T>;

    if (pOne == NULL)
    {
        return (NULL);
    }

    pOne->SetData(NewElement);
    pOne->SetPrev(pNode->GetPrev());
    (pNode->GetPrev())->SetNext(pOne);
    pOne->SetNext(pNode);
    pNode->SetPrev(pOne);

    m_lCount++;

    return (pOne);
}

template <class T>
CNode<T>* CList<T>::InsertAfter(CNode<T>* pNode, T NewElement)
{
    CNode<T>* pOne = new CNode<T>;

    if (pOne == NULL)
    {
        return (NULL);
    }

    pOne->SetData(NewElement);

    pOne->SetNext(pNode->GetNext());
    (pNode->GetNext())->SetPrev(pOne);
    pOne->SetPrev(pNode);
    pNode->SetNext(pOne);

    m_lCount++;

    return (pOne);
}

template <class T>
T CList<T>::RemoveHead(void)
{
    CNode<T>* pOne = m_pHead->GetNext();

    if (pOne == m_pTail)
    {
        T NullData;
        memset(&NullData, 0, sizeof(T));

        return (NullData);
    }

    T Data = pOne->GetData();

    (pOne->GetNext())->SetPrev(m_pHead);
    m_pHead->SetNext(pOne->GetNext());

    delete pOne;

    m_lCount--;

    return (Data);
}

template <class T>
T CList<T>::RemoveTail(void)
{
    CNode<T>* pOne = m_pTail->GetPrev();

    if (pOne == m_pHead)
    {
        T NullData;
        memset(&NullData, 0, sizeof(T));

        return (NullData);
    }

    T Data = pOne->GetData();

    (pOne->GetPrev())->SetNext(m_pTail);
    m_pTail->SetPrev(pOne->GetPrev());

    delete pOne;

    m_lCount--;

    return (Data);
}

template <class T>
T CList<T>::RemoveNode(CNode<T>*& pNode)
{
    if (pNode == NULL)
    {
        T NullData;
        memset(&NullData, 0, sizeof(T));

        return (NullData);
    }

    T Data = pNode->GetData();
    CNode<T>* pNextNode = pNode->GetNext();

    (pNode->GetPrev())->SetNext(pNextNode);
    pNextNode->SetPrev(pNode->GetPrev());

    delete pNode;
    pNode = (pNextNode != m_pTail) ? pNextNode : NULL;

    m_lCount--;

    return (Data);
}

template <class T>
void CList<T>::RemoveAll(void)
{
    (m_pTail->GetPrev())->SetNext(NULL);

    CNode<T>* pToSeek, * pToDelete;

    pToDelete = pToSeek = m_pHead->GetNext();
    while (pToSeek)
    {
        pToSeek = pToSeek->GetNext();
        delete pToDelete;
        pToDelete = pToSeek;
    }

    m_pHead->SetNext(m_pTail);
    m_pTail->SetPrev(m_pHead);

    m_lCount = 0;
}

template <class T>
CNode<T>* CList<T>::FindHead(void)
{
    if (IsEmpty())
    {
        return (NULL);
    }
    else
    {
        return (m_pHead->GetNext());
    }
}

template <class T>
CNode<T>* CList<T>::FindTail(void)
{
    if (IsEmpty())
    {
        return (NULL);
    }
    else
    {
        return (m_pTail->GetPrev());
    }
}

template <class T>
CNode<T>* CList<T>::FindNode(T SearchValue)
{
    (m_pTail->GetPrev())->SetNext(NULL);

    CNode<T>* pToSeek = m_pHead->GetNext();

    while (pToSeek)
    {
        if (pToSeek->GetData() == SearchValue)
        {
            (m_pTail->GetPrev())->SetNext(m_pTail);
            return (pToSeek);
        }

        pToSeek = pToSeek->GetNext();
    }

    (m_pTail->GetPrev())->SetNext(m_pTail);
    return (NULL);
}

template <class T>
CNode<T>* CList<T>::GetPrev(CNode<T>* pNode)
{
    if (m_pHead == pNode->GetPrev())
    {
        return (NULL);
    }
    else
    {
        return (pNode->GetPrev());
    }
}

template <class T>
CNode<T>* CList<T>::GetNext(CNode<T>* pNode)
{
    if (m_pTail == (pNode->GetNext()))
    {
        return (NULL);
    }
    else
    {
        return (pNode->GetNext());
    }
}

template <class T>
void CList<T>::SetData(CNode<T>* pNode, T Data)
{
    pNode->SetData(Data);
}

template <class T>
T& CList<T>::GetData(CNode<T>* pNode)
{
    if (m_pHead == pNode || m_pTail == pNode)
    {
        T NullData;	// NULL 리턴에 필요한 값
        memset(&NullData, 0, sizeof(T));

        return (NullData);
    }
    else
    {
        return (pNode->GetData());
    }
}

template <class T>
class CQueue : private CList<T>
{
public:
    CQueue();
    ~CQueue();

    BOOL Insert(T NewElement);
    T Remove(void);
    void CleanUp(void);

    BOOL Find(T Element);

    LONG GetCount(void) { return (CList<T>::GetCount()); }
};

template <class T>
CQueue<T>::CQueue() : CList<T>()
{
}

template <class T>
CQueue<T>::~CQueue()
{
}

template <class T>
BOOL CQueue<T>::Insert(T NewElement)
{
    if (NULL == AddTail(NewElement))
    {	// 실패하면
        return (FALSE);
    }

    return (TRUE);
}

template <class T>
T CQueue<T>::Remove(void)
{
    return(RemoveHead());
}

template <class T>
void CQueue<T>::CleanUp(void)
{
    RemoveAll();
}

template <class T>
BOOL CQueue<T>::Find(T Element)
{
    if (NULL == FindNode(Element))
    {	// 찾지 못하면
        return (FALSE);
    }

    return (TRUE);
}

template <class T, class S> class CBTree;

template <class T, class S>
class CBNode
{
protected:
    T m_Data;
    S m_CompValue;
    CBNode<T, S>* m_pLeft = NULL;
    CBNode<T, S>* m_pRight = NULL;
    CBNode<T, S>* m_pParent = NULL;

    void SetLeft(CBNode<T, S>* pLeft) { m_pLeft = pLeft; pLeft->m_pParent = this; }
    void SetRight(CBNode<T, S>* pRight) { m_pRight = pRight; pRight->m_pParent = this; }
    CBNode<T, S>* GetLeft(void) { return (m_pLeft); }
    CBNode<T, S>* GetRight(void) { return (m_pRight); }
    CBNode<T, S>* GetParent(void) { return (m_pParent); }

    friend class CBTree<T, S>;
    friend CBTree<T, S>& operator + (const CBTree<T, S>& Value1, const CBTree<T, S>& Value2);

public:

    CBNode();
    CBNode(T Data, S CompValue);
    ~CBNode();

    // 데이터 처리
    void SetData(T Data) { m_Data = Data; }
    void SetValue(S CompValue) { m_CompValue = CompValue; }
    T& GetData(void) { return (m_Data); }
    S GetValue(void) { return (m_CompValue); }
};

template <class T, class S>
CBNode<T, S>::CBNode()
{
    m_pLeft = m_pRight = m_pParent = NULL;
}

template <class T, class S>
CBNode<T, S>::CBNode(T Data, S CompValue)
{
    m_pLeft = m_pRight = m_pParent = NULL;
    m_Data = Data;
    m_CompValue = CompValue;
}

template <class T, class S>
CBNode<T, S>::~CBNode()
{
}

template <class T, class S>
class CBTree
{
protected:
    long m_lCount = 0;
    CBNode<T, S>* m_pHead = NULL;

    void RemoveFrom(CBNode<T, S>* pNode);
    void CycleFrom(CBNode<T, S>* pNode, DWORD dwParam);
    void (*m_Process)(T Data, S CompValue, DWORD dwParam);

public:
    CBTree();	// Constructor
    CBTree(const CBTree<T, S>& RValue);
    ~CBTree();	// Destructor

    CBTree<T, S>& operator = (const CBTree<T, S>& RValue);
    friend CBTree<T, S>& operator + (const CBTree<T, S>& Value1, const CBTree<T, S>& Value2);

    CBNode<T, S>* Add(T NewElement, S CompValue);
    void RemoveNode(CBNode<T, S>*& pNode);
    void RemoveAll(void);

    BOOL IsEmpty(void) { return (m_lCount == 0); }
    virtual LONG GetCount(void) { return (m_lCount); }

    CBNode<T, S>* FindNode(S CompValue);

    CBNode<T, S>* FindHead(void);
    CBNode<T, S>* GetLeft(CBNode<T, S>* pNode);
    CBNode<T, S>* GetRight(CBNode<T, S>* pNode);
    CBNode<T, S>* GetParent(CBNode<T, S>* pNode);

    void SetData(CBNode<T, S>* pNode, T Data);
    T& GetData(CBNode<T, S>* pNode);
    S GetValue(CBNode<T, S>* pNode);

    void Cycle(void (*Process)(T Data, S CompValue, DWORD dwParam), DWORD dwParam);

protected:
    void CopyFrom(CBNode<T, S>* pNode);
    void UnlinkOneBottomNode(CBNode<T, S>*& pNode);

public:
    BOOL Optimize(void);
protected:
    BOOL GetOptimizeList(T* pData, S* pCompValue);
    void AddDataByOptimization(T* pData, S* pCompValue, int nLeft, int nRight);
};

template <class T, class S>
CBTree<T, S>::CBTree()
{
    m_lCount = 0;
    m_pHead = NULL;
}

template <class T, class S>
CBTree<T, S>::CBTree(const CBTree<T, S>& RValue)
{
    m_lCount = 0;
    m_pHead = NULL;

    if (RValue.m_pHead)
    {
        CopyFrom(RValue.m_pHead);
    }
}

template <class T, class S>
CBTree<T, S>::~CBTree()
{
    RemoveAll();
}

template <class T, class S>
CBTree<T, S>& CBTree<T, S>::operator = (const CBTree<T, S>& RValue)
{
    if (&RValue == this)
    {
        return (*this);
    }

    RemoveAll();
    if (RValue.m_pHead)
    {
        CopyFrom(RValue.m_pHead);
    }

    return (*this);
}

template <class T, class S>
CBTree<T, S>& operator + (const CBTree<T, S>& Value1, const CBTree<T, S>& Value2)
{
    CBTree<T, S> Result;
    CBTree<T, S> Left;
    CBTree<T, S> Right;
    if (Value1.m_pHead->GetValue() <= Value2.m_pHead->GetValue())
    {
        Left = Value1;
        Right = Value2;
    }
    else
    {
        Left = Value2;
        Right = Value1;
    }

    T NullData;
    memset(&NullData, 0, sizeof(T));
    S CompValue = 0;
    if (Left.m_pHead != NULL && Right.m_pHead != NULL)
    {
        CompValue = (Left.m_pHead->GetValue() + Left.m_pHead->GetValue() + 1) / 2;
    }
    Result.Add(NullData, CompValue);
    Result.m_pHead->SetLeft(Left.m_pHead);
    Result.m_pHead->SetRight(Right.m_pHead);
    Result.m_lCount = Left.m_lCount + Right.m_lCount + 1;
    Left.m_pHead = NULL;
    Right.m_pHead = NULL;

    return (CBTree<T, S>(Result));
}

template <class T, class S>
void CBTree<T, S>::CopyFrom(CBNode<T, S>* pNode)
{
    Add(pNode->GetData(), pNode->GetValue());
    if (pNode->GetLeft())
    {
        CopyFrom(pNode->GetLeft());
    }
    if (pNode->GetLeft())
    {
        CopyFrom(pNode->GetRight());
    }
}

template <class T, class S>
CBNode<T, S>* CBTree<T, S>::Add(T NewElement, S CompValue)
{
    if (m_pHead == NULL)
    {
        m_lCount++;
        m_pHead = new CBNode<T, S>(NewElement, CompValue);
        m_pHead->m_pParent = NULL;
        m_pHead->m_pLeft = NULL;
        m_pHead->m_pRight = NULL;

        return (m_pHead);
    }

    CBNode<T, S>* pNode = m_pHead;
    CBNode<T, S>* pPrev = m_pHead;
    while (pNode != NULL)
    {
        if (CompValue < pNode->GetValue())
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

    if (pNode == NULL)
    {
        m_lCount++;
        auto* pOne = new CBNode<T, S>(NewElement, CompValue);

        if (CompValue < pPrev->GetValue())
        {
            pPrev->SetLeft(pOne);
        }
        else
        {
            pPrev->SetRight(pOne);
        }

        return (pOne);
    }
    else
    {
        return (pNode);
    }
}

template <class T, class S>
void CBTree<T, S>::RemoveNode(CBNode<T, S>*& pNode)
{
    if (!pNode)
    {
        return;
    }

    CBNode<T, S>* pOld = NULL;

    CBNode<T, S>* pSeek = pNode->GetLeft();
    if (pSeek)
    {
        if (!pNode->GetRight())
        {
            if (pNode == m_pHead)
            {
                m_pHead = pSeek;
                m_pHead->m_pParent = NULL;
            }
            else
            {
                if (pNode == pNode->GetParent()->m_pRight)
                {
                    pNode->GetParent()->SetRight(pSeek);
                }
                else
                {
                    pNode->GetParent()->SetLeft(pSeek);
                }
            }
            delete pNode;
            m_lCount--;
            return;
        }

        pOld = pSeek;
        while (pSeek)
        {
            pOld = pSeek;
            pSeek = pSeek->GetRight();
        }

        T Data = pOld->m_Data;
        S CompValue = pOld->m_CompValue;
        //UnlinkOneBottomNode( pOld);
        RemoveNode(pOld);

        pNode->m_Data = Data;
        pNode->m_CompValue = CompValue;

        return;
    }
    else
    {
        pSeek = pNode->GetRight();
        if (pSeek)
        {
            if (pNode == m_pHead)
            {
                m_pHead = pSeek;
                m_pHead->m_pParent = NULL;
            }
            else
            {
                if (pNode == pNode->GetParent()->m_pRight)
                {
                    pNode->GetParent()->SetRight(pSeek);
                }
                else
                {
                    pNode->GetParent()->SetLeft(pSeek);
                }
            }
            delete pNode;
            m_lCount--;
            return;
        }
    }
    UnlinkOneBottomNode(pNode);
}

template <class T, class S>
void CBTree<T, S>::UnlinkOneBottomNode(CBNode<T, S>*& pNode)
{
    if (pNode->GetParent())
    {
        if (pNode->GetParent()->GetLeft() == pNode)
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

    m_lCount--;
    delete pNode;
    pNode = NULL;
}

template <class T, class S>
void CBTree<T, S>::RemoveAll(void)
{
    if (m_pHead != NULL)
    {
        RemoveFrom(m_pHead);
        m_pHead = NULL;
    }
    m_lCount = 0;
}

template <class T, class S>
void CBTree<T, S>::RemoveFrom(CBNode<T, S>* pNode)
{
    if (!pNode)
    {
        return;
    }

    if (pNode->GetLeft() != NULL)
    {
        RemoveFrom(pNode->GetLeft());
    }

    if (pNode->GetRight() != NULL)
    {
        RemoveFrom(pNode->GetRight());
    }

    RemoveNode(pNode);
}

template <class T, class S>
CBNode<T, S>* CBTree<T, S>::FindNode(S CompValue)
{
    CBNode<T, S>* pNode = m_pHead;
    while (pNode != NULL && pNode->GetValue() != CompValue)
    {
        if (CompValue < pNode->GetValue())
        {
            pNode = pNode->GetLeft();
        }
        else
        {
            pNode = pNode->GetRight();
        }
    }

    return (pNode);
}

template <class T, class S>
CBNode<T, S>* CBTree<T, S>::FindHead(void)
{
    return (m_pHead);
}

template <class T, class S>
CBNode<T, S>* CBTree<T, S>::GetLeft(CBNode<T, S>* pNode)
{
    return (pNode->GetLeft());
}

template <class T, class S>
CBNode<T, S>* CBTree<T, S>::GetRight(CBNode<T, S>* pNode)
{
    return (pNode->GetRight());
}

template <class T, class S>
CBNode<T, S>* GetParent(CBNode<T, S>* pNode)
{
    return (pNode->GetParent());
}

template <class T, class S>
void CBTree<T, S>::SetData(CBNode<T, S>* pNode, T Data)
{
    pNode->SetData(Data);
}

template <class T, class S>
T& CBTree<T, S>::GetData(CBNode<T, S>* pNode)
{
    return (pNode->GetData());
}

template <class T, class S>
S CBTree<T, S>::GetValue(CBNode<T, S>* pNode)
{
    return (pNode->GetValue());
}

template <class T, class S>
void CBTree<T, S>::Cycle(void (*Process)(T Data, S CompValue, DWORD dwParam), DWORD dwParam)
{
    m_Process = Process;

    if (m_pHead == NULL)
    {
        return;
    }

    if (m_pHead->GetLeft() != NULL)
    {
        CycleFrom(m_pHead->GetLeft(), dwParam);
    }

    (*m_Process)(m_pHead->GetData(), m_pHead->GetValue(), dwParam);

    if (m_pHead->GetRight() != NULL)
    {
        CycleFrom(m_pHead->GetRight(), dwParam);
    }
}

template <class T, class S>
void CBTree<T, S>::CycleFrom(CBNode<T, S>* pNode, DWORD dwParam)
{
    if (pNode->GetLeft() != NULL)
    {
        CycleFrom(pNode->GetLeft(), dwParam);
    }

    (*m_Process)(pNode->GetData(), pNode->GetValue(), dwParam);

    if (pNode->GetRight() != NULL)
    {
        CycleFrom(pNode->GetRight(), dwParam);
    }
}

template <class T, class S>
BOOL CBTree<T, S>::Optimize(void)
{
    T* pData = new T[GetCount()];
    S* pCompValue = new S[GetCount()];
    if (!GetOptimizeList(pData, pCompValue))
    {
        delete[] pCompValue;
        delete[] pData;
        return (FALSE);
    }

    long lCount = GetCount();
    RemoveAll();
    AddDataByOptimization(pData, pCompValue, 0, lCount);
    delete[] pCompValue;
    delete[] pData;

    return (TRUE);
}

template <class T, class S>
void ProcessGetOptimizeList(T Data, S CompValue, DWORD dwParam)
{
    DWORD* pdwParam = (DWORD*)dwParam;
    long* pCount = (long*)pdwParam[0];
    T* pData = (T*)pdwParam[1];
    S* pCompValue = (S*)pdwParam[2];
    pData[*pCount] = Data;
    pCompValue[(*pCount)++] = CompValue;
}

template <class T, class S>
BOOL CBTree<T, S>::GetOptimizeList(T* pData, S* pCompValue)
{
    long lOptimizeCount = 0;
    DWORD dwOptimizeData[3] = { (DWORD)&lOptimizeCount, (DWORD)pData, (DWORD)pCompValue };
    Cycle(ProcessGetOptimizeList, (DWORD)dwOptimizeData);
    if (GetCount() != lOptimizeCount)
    {
        return (FALSE);
    }

    return (TRUE);
}

template <class T, class S>
void CBTree<T, S>::AddDataByOptimization(T* pData, S* pCompValue, int nLeft, int nRight)
{
    if (nLeft == nRight)
    {
        return;
    }

    Add(pData[(nLeft + nRight) / 2], pCompValue[(nLeft + nRight) / 2]);

    AddDataByOptimization(pData, pCompValue, nLeft, (nLeft + nRight) / 2);
    AddDataByOptimization(pData, pCompValue, (nLeft + nRight) / 2 + 1, nRight);
}

template <class T>
class CDimension
{
protected:
    int m_nSize;
    T* m_pData;

public:
    CDimension();	// Constructor
    ~CDimension();	// Destructor

    T Set(int nIndex, T Data);
    T Get(int nIndex);

protected:
    void CheckDimensionSize(int nIndex);
};

template <class T>
CDimension<T>::CDimension()
{
    m_nSize = 16;
    m_pData = new T[m_nSize];
}

template <class T>
CDimension<T>::~CDimension()
{
    delete[] m_pData;
}

template <class T>
T CDimension<T>::Set(int nIndex, T Data)
{
    CheckDimensionSize(nIndex);
    m_pData[nIndex] = Data;

    return (Data);
}

template <class T>
T CDimension<T>::Get(int nIndex)
{
    CheckDimensionSize(nIndex);
    return (m_pData[nIndex]);
}

template <class T>
void CDimension<T>::CheckDimensionSize(int nIndex)
{
    if (nIndex >= m_nSize)
    {
        for (int nNewSize = m_nSize; nNewSize <= nIndex; nNewSize *= 2)
        {
        }

        T* pTempBuffer = new T[m_nSize];
        memcpy(pTempBuffer, m_pData, m_nSize * sizeof(T));
        delete[] m_pData;
        m_pData = new T[nNewSize];
        memcpy(m_pData, pTempBuffer, m_nSize * sizeof(T));
        m_nSize = nNewSize;
        delete[] pTempBuffer;
    }
}

# endif //__BASE_CLASSES_H__
