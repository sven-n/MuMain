#include "GameShop/StorageItemSelection.h"

namespace GameShop
{
void StorageItemSelection::Clear()
{
    m_Items.clear();
    m_SelectedSeq = -1;
}

void StorageItemSelection::Add(const StorageItem& item)
{
    m_Items.push_back(item);
}

void StorageItemSelection::SelectLast()
{
    if (m_Items.empty())
    {
        m_SelectedSeq = -1;
        return;
    }
    if (Selected() != nullptr)
        return; // the pick survived the refresh
    m_SelectedSeq = m_Items.back().m_iStorageItemSeq;
}

bool StorageItemSelection::Select(int storageItemSeq)
{
    for (const StorageItem& item : m_Items)
    {
        if (item.m_iStorageItemSeq == storageItemSeq)
        {
            m_SelectedSeq = storageItemSeq;
            return true;
        }
    }
    return false;
}

bool StorageItemSelection::SelectRow(int displayIndex)
{
    if (displayIndex < 0 || displayIndex >= static_cast<int>(m_Items.size()))
        return false;
    m_SelectedSeq = m_Items[displayIndex].m_iStorageItemSeq;
    return true;
}

const StorageItem* StorageItemSelection::Selected() const
{
    if (m_SelectedSeq < 0)
        return nullptr;
    for (const StorageItem& item : m_Items)
    {
        if (item.m_iStorageItemSeq == m_SelectedSeq)
            return &item;
    }
    return nullptr;
}

int StorageItemSelection::SelectedRow() const
{
    for (size_t i = 0; i < m_Items.size(); ++i)
    {
        if (m_Items[i].m_iStorageItemSeq == m_SelectedSeq)
            return static_cast<int>(i);
    }
    return -1;
}
} // namespace GameShop
