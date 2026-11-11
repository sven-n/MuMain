#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace GameShop
{
// Mirrors _define.h's MAX_TEXT_LENGTH / MAX_USERNAME_SIZE / MAX_GIFT_MESSAGE_SIZE so this header
// stays free of the engine's Windows-typed globals and can be unit tested on its own.
// InGameShop.cpp static_asserts that they still agree.
inline constexpr int kStorageTextLength = 255;
inline constexpr int kStorageUserNameSize = 10;
inline constexpr int kStorageMessageSize = 200;

// One row of the cash shop's storage or gift box. This is IGS_StorageItem moved out of the widget
// toolkit unchanged, field names and fixed buffers included, so the ~120 lines that fill one in
// CInGameShop::AddStorageItem() keep working as they were. Only m_bIsSelected is gone: which row
// is picked belongs to StorageItemSelection below, not to the row.
struct StorageItem
{
    int m_iStorageSeq = 0;
    int m_iStorageItemSeq = 0;
    int m_iStorageGroupCode = 0;
    int m_iProductSeq = 0;
    int m_iPriceSeq = 0;
    int m_iCashPoint = 0;
    int m_iNum = 0;
    std::uint16_t m_wItemCode = 0;

    wchar_t m_szName[kStorageTextLength] = {};
    wchar_t m_szNum[kStorageTextLength] = {};
    wchar_t m_szPeriod[kStorageTextLength] = {};
    wchar_t m_szSendUserName[kStorageUserNameSize + 1] = {};
    wchar_t m_szMessage[kStorageMessageSize] = {};
    wchar_t m_szType = 0;
};

// The rows of one storage page and which one is picked. The selection is held as a storage item
// sequence rather than a row index: the server resends a whole page on every refresh, so an index
// can come back pointing at a different item.
class StorageItemSelection
{
public:
    void Clear();
    void Add(const StorageItem& item);
    // Selects the last row, as the native list box did while rows arrived -- but leaves a pick
    // that survived the refresh alone.
    void SelectLast();
    bool Select(int storageItemSeq);
    bool SelectRow(int displayIndex);

    std::span<const StorageItem> Items() const { return m_Items; }
    const StorageItem* Selected() const;
    int SelectedSeq() const { return m_SelectedSeq; }
    int SelectedRow() const;
    bool Empty() const { return m_Items.empty(); }
    int Count() const { return static_cast<int>(m_Items.size()); }

private:
    std::vector<StorageItem> m_Items;
    int m_SelectedSeq = -1;
};
} // namespace GameShop
