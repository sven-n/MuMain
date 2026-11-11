#ifndef __BASE_CLASSES_H__
#define __BASE_CLASSES_H__

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

template <class T> class CList;

template <class T> class CNode
{
public:
    CNode() = default;
    explicit CNode(const T& data) : m_Data(data) {}

    void SetData(const T& data)
    {
        m_Data = data;
    }
    T& GetData()
    {
        return m_Data;
    }
    const T& GetData() const
    {
        return m_Data;
    }

private:
    void SetPrev(CNode<T>* previous)
    {
        m_pPrev = previous;
    }
    void SetNext(CNode<T>* next)
    {
        m_pNext = next;
    }
    CNode<T>* GetPrev()
    {
        return m_pPrev;
    }
    const CNode<T>* GetPrev() const
    {
        return m_pPrev;
    }
    CNode<T>* GetNext()
    {
        return m_pNext;
    }
    const CNode<T>* GetNext() const
    {
        return m_pNext;
    }

    friend class CList<T>;

    T m_Data{};
    CNode<T>* m_pPrev{nullptr};
    CNode<T>* m_pNext{nullptr};
};

template <class T> class CList
{
public:
    CList();
    ~CList();

    std::size_t GetCount() const
    {
        return m_Count;
    }
    bool IsEmpty() const
    {
        return m_Count == 0;
    }

    CNode<T>* AddHead(const T& newElement);
    CNode<T>* AddTail(const T& newElement);
    CNode<T>* InsertBefore(CNode<T>* node, const T& newElement);
    CNode<T>* InsertAfter(CNode<T>* node, const T& newElement);

    T RemoveHead();
    T RemoveTail();
    T RemoveNode(CNode<T>*& node);
    void RemoveAll();

    CNode<T>* FindHead();
    const CNode<T>* FindHead() const;
    CNode<T>* FindTail();
    const CNode<T>* FindTail() const;
    CNode<T>* FindNode(const T& searchValue);
    const CNode<T>* FindNode(const T& searchValue) const;

    CNode<T>* GetPrev(CNode<T>* node);
    const CNode<T>* GetPrev(const CNode<T>* node) const;
    CNode<T>* GetNext(CNode<T>* node);
    const CNode<T>* GetNext(const CNode<T>* node) const;

    void SetData(CNode<T>* node, const T& data);
    T& GetData(CNode<T>* node);
    const T& GetData(const CNode<T>* node) const;

private:
    CNode<T>* CreateDataNode(const T& value);
    bool IsSentinel(const CNode<T>* node) const
    {
        return node == m_pHead || node == m_pTail;
    }

    std::size_t m_Count{0};
    CNode<T>* m_pHead{nullptr};
    CNode<T>* m_pTail{nullptr};
    T m_NullData{};
};

// cppcheck-suppress [noCopyConstructor, noOperatorEq]
template <class T> CList<T>::CList() : m_pHead(new CNode<T>()), m_pTail(new CNode<T>())
{
    m_pHead->SetNext(m_pTail);
    m_pTail->SetPrev(m_pHead);
}

template <class T> CList<T>::~CList()
{
    RemoveAll();
    delete m_pTail;
    delete m_pHead;
}

template <class T> CNode<T>* CList<T>::CreateDataNode(const T& value)
{
    return new CNode<T>(value);
}

template <class T> CNode<T>* CList<T>::AddHead(const T& newElement)
{
    auto* node = CreateDataNode(newElement);
    auto* first = m_pHead->GetNext();
    node->SetNext(first);
    first->SetPrev(node);
    node->SetPrev(m_pHead);
    m_pHead->SetNext(node);
    ++m_Count;
    return node;
}

template <class T> CNode<T>* CList<T>::AddTail(const T& newElement)
{
    auto* node = CreateDataNode(newElement);
    auto* last = m_pTail->GetPrev();
    node->SetPrev(last);
    last->SetNext(node);
    node->SetNext(m_pTail);
    m_pTail->SetPrev(node);
    ++m_Count;
    return node;
}

template <class T> CNode<T>* CList<T>::InsertBefore(CNode<T>* node, const T& newElement)
{
    if (!node || node == m_pHead)
    {
        return nullptr;
    }

    auto* previous = node->GetPrev();
    auto* newNode = CreateDataNode(newElement);
    newNode->SetPrev(previous);
    newNode->SetNext(node);
    previous->SetNext(newNode);
    node->SetPrev(newNode);
    ++m_Count;
    return newNode;
}

template <class T> CNode<T>* CList<T>::InsertAfter(CNode<T>* node, const T& newElement)
{
    if (!node || node == m_pTail)
    {
        return nullptr;
    }

    auto* next = node->GetNext();
    auto* newNode = CreateDataNode(newElement);
    newNode->SetNext(next);
    newNode->SetPrev(node);
    next->SetPrev(newNode);
    node->SetNext(newNode);
    ++m_Count;
    return newNode;
}

template <class T> T CList<T>::RemoveHead()
{
    if (IsEmpty())
    {
        throw std::out_of_range("Cannot remove from an empty list.");
    }

    auto* node = m_pHead->GetNext();
    auto* next = node->GetNext();
    next->SetPrev(m_pHead);
    m_pHead->SetNext(next);
    T data = node->GetData();
    delete node;
    --m_Count;
    return data;
}

template <class T> T CList<T>::RemoveTail()
{
    if (IsEmpty())
    {
        throw std::out_of_range("Cannot remove from an empty list.");
    }

    auto* node = m_pTail->GetPrev();
    auto* previous = node->GetPrev();
    previous->SetNext(m_pTail);
    m_pTail->SetPrev(previous);
    T data = node->GetData();
    delete node;
    --m_Count;
    return data;
}

template <class T> T CList<T>::RemoveNode(CNode<T>*& node)
{
    if (!node || IsSentinel(node))
    {
        throw std::invalid_argument("Cannot remove a null or sentinel node.");
    }

    auto* next = node->GetNext();
    auto* prev = node->GetPrev();
    prev->SetNext(next);
    next->SetPrev(prev);
    T data = node->GetData();
    delete node;
    node = (next != m_pTail) ? next : nullptr;
    --m_Count;
    return data;
}

template <class T> void CList<T>::RemoveAll()
{
    CNode<T>* current = m_pHead->GetNext();
    while (current != m_pTail)
    {
        CNode<T>* toDelete = current;
        current = current->GetNext();
        delete toDelete;
    }

    m_pHead->SetNext(m_pTail);
    m_pTail->SetPrev(m_pHead);
    m_Count = 0;
}

template <class T> CNode<T>* CList<T>::FindHead()
{
    return IsEmpty() ? nullptr : m_pHead->GetNext();
}

template <class T> const CNode<T>* CList<T>::FindHead() const
{
    return IsEmpty() ? nullptr : m_pHead->GetNext();
}

template <class T> CNode<T>* CList<T>::FindTail()
{
    return IsEmpty() ? nullptr : m_pTail->GetPrev();
}

template <class T> const CNode<T>* CList<T>::FindTail() const
{
    return IsEmpty() ? nullptr : m_pTail->GetPrev();
}

template <class T> CNode<T>* CList<T>::FindNode(const T& searchValue)
{
    CNode<T>* current = m_pHead->GetNext();
    while (current != m_pTail)
    {
        if (current->GetData() == searchValue)
        {
            return current;
        }
        current = current->GetNext();
    }
    return nullptr;
}

template <class T> const CNode<T>* CList<T>::FindNode(const T& searchValue) const
{
    const CNode<T>* current = m_pHead->GetNext();
    while (current != m_pTail)
    {
        if (current->GetData() == searchValue)
        {
            return current;
        }
        current = current->GetNext();
    }
    return nullptr;
}

template <class T> CNode<T>* CList<T>::GetPrev(CNode<T>* node)
{
    if (!node)
    {
        return nullptr;
    }
    auto* prev = node->GetPrev();
    return (prev == m_pHead) ? nullptr : prev;
}

template <class T> const CNode<T>* CList<T>::GetPrev(const CNode<T>* node) const
{
    if (!node)
    {
        return nullptr;
    }
    auto* prev = node->GetPrev();
    return (prev == m_pHead) ? nullptr : prev;
}

template <class T> CNode<T>* CList<T>::GetNext(CNode<T>* node)
{
    if (!node)
    {
        return nullptr;
    }
    auto* next = node->GetNext();
    return (next == m_pTail) ? nullptr : next;
}

template <class T> const CNode<T>* CList<T>::GetNext(const CNode<T>* node) const
{
    if (!node)
    {
        return nullptr;
    }
    auto* next = node->GetNext();
    return (next == m_pTail) ? nullptr : next;
}

template <class T> void CList<T>::SetData(CNode<T>* node, const T& data)
{
    if (!node || IsSentinel(node))
    {
        return;
    }
    node->SetData(data);
}

template <class T> T& CList<T>::GetData(CNode<T>* node)
{
    if (IsSentinel(node))
    {
        throw std::out_of_range("Attempted to get data from a sentinel node.");
    }
    return node->GetData();
}

template <class T> const T& CList<T>::GetData(const CNode<T>* node) const
{
    if (IsSentinel(node))
    {
        throw std::out_of_range("Attempted to get data from a sentinel node.");
    }
    return node->GetData();
}

template <class T> class CQueue : private CList<T>
{
public:
    CQueue() = default;
    ~CQueue() = default;

    bool Insert(const T& newElement);
    T Remove();
    void CleanUp();

    bool Find(const T& element);

    // cppcheck-suppress duplInheritedMember
    std::size_t GetCount() const
    {
        return CList<T>::GetCount();
    }
};

template <class T> bool CQueue<T>::Insert(const T& newElement)
{
    return this->AddTail(newElement) != nullptr;
}

template <class T> T CQueue<T>::Remove()
{
    return this->RemoveHead();
}

template <class T> void CQueue<T>::CleanUp()
{
    this->RemoveAll();
}

template <class T> bool CQueue<T>::Find(const T& element)
{
    return this->FindNode(element) != nullptr;
}

template <class T, class S> class CBTree;

template <class T, class S> class CBNode
{
public:
    CBNode() = default;
    CBNode(const T& data, const S& value) : m_Data(data), m_CompValue(value) {}

    void SetData(const T& data)
    {
        m_Data = data;
    }
    void SetValue(const S& value)
    {
        m_CompValue = value;
    }
    T& GetData()
    {
        return m_Data;
    }
    const T& GetData() const
    {
        return m_Data;
    }
    S GetValue() const
    {
        return m_CompValue;
    }

private:
    void SetLeft(CBNode<T, S>* left)
    {
        m_pLeft = left;
        if (m_pLeft)
        {
            m_pLeft->m_pParent = this;
        }
    }

    void SetRight(CBNode<T, S>* right)
    {
        m_pRight = right;
        if (m_pRight)
        {
            m_pRight->m_pParent = this;
        }
    }

    CBNode<T, S>* GetLeft()
    {
        return m_pLeft;
    }
    const CBNode<T, S>* GetLeft() const
    {
        return m_pLeft;
    }
    CBNode<T, S>* GetRight()
    {
        return m_pRight;
    }
    const CBNode<T, S>* GetRight() const
    {
        return m_pRight;
    }
    CBNode<T, S>* GetParent()
    {
        return m_pParent;
    }
    const CBNode<T, S>* GetParent() const
    {
        return m_pParent;
    }

    friend class CBTree<T, S>;

    T m_Data{};
    S m_CompValue{};
    CBNode<T, S>* m_pLeft{nullptr};
    CBNode<T, S>* m_pRight{nullptr};
    CBNode<T, S>* m_pParent{nullptr};
};

template <class T, class S> class CBTree
{
public:
    CBTree();
    ~CBTree();

    CBNode<T, S>* Add(const T& newElement, const S& value);
    void RemoveNode(CBNode<T, S>*& node);
    void RemoveAll();

    bool IsEmpty() const
    {
        return m_Count == 0;
    }
    std::size_t GetCount() const
    {
        return m_Count;
    }

    CBNode<T, S>* FindHead();
    CBNode<T, S>* FindNode(const S& value);
    const CBNode<T, S>* FindNode(const S& value) const;
    CBNode<T, S>* GetLeft(CBNode<T, S>* node);
    CBNode<T, S>* GetRight(CBNode<T, S>* node);
    CBNode<T, S>* GetParent(CBNode<T, S>* node);

    void SetData(CBNode<T, S>* node, const T& data);
    T& GetData(CBNode<T, S>* node);
    S GetValue(CBNode<T, S>* node);

    void Cycle(std::function<void(const T&, const S&)> process);

private:
    CBNode<T, S>* CreateNode(const T& value, const S& compValue);
    void RemoveFrom(CBNode<T, S>* node);
    void CycleFrom(CBNode<T, S>* node, std::function<void(const T&, const S&)> process);

    std::size_t m_Count{0};
    CBNode<T, S>* m_pHead{nullptr};
};

template <class T, class S> CBTree<T, S>::CBTree() : m_pHead(nullptr) {}

template <class T, class S> CBTree<T, S>::~CBTree()
{
    RemoveAll();
}

template <class T, class S> CBNode<T, S>* CBTree<T, S>::CreateNode(const T& value, const S& compValue)
{
    return new CBNode<T, S>(value, compValue);
}

template <class T, class S> CBNode<T, S>* CBTree<T, S>::Add(const T& newElement, const S& value)
{
    if (!m_pHead)
    {
        m_pHead = CreateNode(newElement, value);
        ++m_Count;
        return m_pHead;
    }

    CBNode<T, S>* current = m_pHead;
    while (true)
    {
        if (value < current->GetValue())
        {
            if (current->GetLeft())
            {
                current = current->GetLeft();
            }
            else
            {
                current->SetLeft(CreateNode(newElement, value));
                ++m_Count;
                return current->GetLeft();
            }
        }
        else
        {
            if (current->GetRight())
            {
                current = current->GetRight();
            }
            else
            {
                current->SetRight(CreateNode(newElement, value));
                ++m_Count;
                return current->GetRight();
            }
        }
    }
}

template <class T, class S> CBNode<T, S>* CBTree<T, S>::FindNode(const S& value)
{
    CBNode<T, S>* current = m_pHead;
    while (current)
    {
        if (value == current->GetValue())
        {
            return current;
        }
        if (value < current->GetValue())
        {
            current = current->GetLeft();
        }
        else
        {
            current = current->GetRight();
        }
    }
    return nullptr;
}

template <class T, class S> const CBNode<T, S>* CBTree<T, S>::FindNode(const S& value) const
{
    const CBNode<T, S>* current = m_pHead;
    while (current)
    {
        if (value == current->GetValue())
        {
            return current;
        }
        if (value < current->GetValue())
        {
            current = current->GetLeft();
        }
        else
        {
            current = current->GetRight();
        }
    }
    return nullptr;
}

template <class T, class S> void CBTree<T, S>::RemoveNode(CBNode<T, S>*& node)
{
    if (!node)
    {
        return;
    }

    if (node->GetLeft() && node->GetRight())
    {
        CBNode<T, S>* minNode = node->GetRight();
        while (minNode->GetLeft())
        {
            minNode = minNode->GetLeft();
        }

        node->SetData(minNode->GetData());
        node->SetValue(minNode->GetValue());
        RemoveNode(minNode);
    }
    else if (node->GetLeft())
    {
        CBNode<T, S>* left = node->GetLeft();
        if (node == m_pHead)
        {
            m_pHead = left;
        }
        else if (node->GetParent()->GetLeft() == node)
        {
            node->GetParent()->SetLeft(left);
        }
        else
        {
            node->GetParent()->SetRight(left);
        }
        delete node;
        --m_Count;
    }
    else if (node->GetRight())
    {
        CBNode<T, S>* right = node->GetRight();
        if (node == m_pHead)
        {
            m_pHead = right;
        }
        else if (node->GetParent()->GetLeft() == node)
        {
            node->GetParent()->SetLeft(right);
        }
        else
        {
            node->GetParent()->SetRight(right);
        }
        delete node;
        --m_Count;
    }
    else
    {
        if (node == m_pHead)
        {
            m_pHead = nullptr;
        }
        else if (node->GetParent()->GetLeft() == node)
        {
            node->GetParent()->SetLeft(nullptr);
        }
        else
        {
            node->GetParent()->SetRight(nullptr);
        }
        delete node;
        --m_Count;
    }
}

template <class T, class S> void CBTree<T, S>::RemoveAll()
{
    RemoveFrom(m_pHead);
    m_pHead = nullptr;
    m_Count = 0;
}

template <class T, class S> void CBTree<T, S>::RemoveFrom(CBNode<T, S>* node)
{
    if (!node)
    {
        return;
    }

    std::vector<std::pair<CBNode<T, S>*, bool>> stack;
    stack.emplace_back(node, false);

    while (!stack.empty())
    {
        auto currentEntry = stack.back();
        stack.pop_back();

        CBNode<T, S>* current = currentEntry.first;
        bool visited = currentEntry.second;

        if (!current)
        {
            continue;
        }

        if (visited)
        {
            delete current;
            continue;
        }

        stack.emplace_back(current, true);
        if (current->GetRight())
        {
            stack.emplace_back(current->GetRight(), false);
        }
        if (current->GetLeft())
        {
            stack.emplace_back(current->GetLeft(), false);
        }
    }
}

template <class T, class S> CBNode<T, S>* CBTree<T, S>::FindHead()
{
    return m_pHead;
}

template <class T, class S> CBNode<T, S>* CBTree<T, S>::GetLeft(CBNode<T, S>* node)
{
    return node->GetLeft();
}

template <class T, class S> CBNode<T, S>* CBTree<T, S>::GetRight(CBNode<T, S>* node)
{
    return node->GetRight();
}

template <class T, class S> CBNode<T, S>* CBTree<T, S>::GetParent(CBNode<T, S>* node)
{
    return node->GetParent();
}

template <class T, class S> void CBTree<T, S>::SetData(CBNode<T, S>* node, const T& data)
{
    node->SetData(data);
}

template <class T, class S> T& CBTree<T, S>::GetData(CBNode<T, S>* node)
{
    return node->GetData();
}

template <class T, class S> S CBTree<T, S>::GetValue(CBNode<T, S>* node)
{
    return node->GetValue();
}

template <class T, class S> void CBTree<T, S>::Cycle(std::function<void(const T&, const S&)> process)
{
    CycleFrom(m_pHead, process);
}

template <class T, class S>
void CBTree<T, S>::CycleFrom(CBNode<T, S>* node, std::function<void(const T&, const S&)> process)
{
    if (!node)
    {
        return;
    }

    CycleFrom(node->GetLeft(), process);
    process(node->GetData(), node->GetValue());
    CycleFrom(node->GetRight(), process);
}

template <class T> class CDimension
{
protected:
    int m_nSize;
    T* m_pData;

public:
    CDimension();  // Constructor
    ~CDimension(); // Destructor

    T Set(int nIndex, T Data);
    T Get(int nIndex);

protected:
    void CheckDimensionSize(int nIndex);
};

template <class T> CDimension<T>::CDimension()
{
    m_nSize = 16;
    // cppcheck-suppress [noCopyConstructor, noOperatorEq]
    m_pData = new T[m_nSize];
}

template <class T> CDimension<T>::~CDimension()
{
    delete[] m_pData;
}

template <class T> T CDimension<T>::Set(int nIndex, T Data)
{
    CheckDimensionSize(nIndex);
    m_pData[nIndex] = Data;

    return (Data);
}

template <class T> T CDimension<T>::Get(int nIndex)
{
    CheckDimensionSize(nIndex);
    return (m_pData[nIndex]);
}

template <class T> void CDimension<T>::CheckDimensionSize(int nIndex)
{
    if (nIndex >= m_nSize)
    {
        int nNewSize = m_nSize;
        for (; nNewSize <= nIndex; nNewSize *= 2)
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

#endif //__BASE_CLASSES_H__
