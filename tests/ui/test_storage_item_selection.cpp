#include "doctest.h"

#include <cwchar>
#include <ostream>

#include "GameShop/StorageItemSelection.h"

using GameShop::StorageItem;
using GameShop::StorageItemSelection;

namespace
{
StorageItem Row(int seq, const wchar_t* name)
{
    StorageItem item;
    item.m_iStorageItemSeq = seq;
    wcsncpy_s(item.m_szName, name, _TRUNCATE);
    return item;
}
} // namespace

TEST_CASE("an empty storage page has nothing selected")
{
    StorageItemSelection sel;
    CHECK(sel.Empty());
    CHECK(sel.Selected() == nullptr);
    CHECK(sel.SelectedRow() == -1);
    sel.SelectLast();
    CHECK(sel.Selected() == nullptr);
}

TEST_CASE("the last row arriving is selected, as the native list box did")
{
    StorageItemSelection sel;
    sel.Add(Row(10, L"first"));
    sel.Add(Row(20, L"second"));
    sel.SelectLast();
    REQUIRE(sel.Selected() != nullptr);
    CHECK(sel.Selected()->m_iStorageItemSeq == 20);
    CHECK(sel.SelectedRow() == 1);
}

TEST_CASE("a selection follows its item, not its row, across a page refresh")
{
    StorageItemSelection sel;
    sel.Add(Row(10, L"first"));
    sel.Add(Row(20, L"second"));
    CHECK(sel.Select(10));
    CHECK(sel.SelectedRow() == 0);

    // The server resends the page with the first item gone: row 0 is now a different item.
    sel.Clear();
    CHECK(sel.Selected() == nullptr);
    sel.Add(Row(20, L"second"));
    sel.Add(Row(30, L"third"));
    CHECK(sel.Selected() == nullptr); // the picked item is no longer on the page
    sel.SelectLast();
    CHECK(sel.Selected()->m_iStorageItemSeq == 30);
}

TEST_CASE("a surviving selection is kept when the page is rebuilt")
{
    StorageItemSelection sel;
    sel.Add(Row(10, L"first"));
    sel.Add(Row(20, L"second"));
    CHECK(sel.Select(20));

    sel.Clear();
    sel.Add(Row(20, L"second"));
    sel.Add(Row(30, L"third"));
    CHECK(sel.Select(20));
    sel.SelectLast(); // must not steal the pick
    REQUIRE(sel.Selected() != nullptr);
    CHECK(sel.Selected()->m_iStorageItemSeq == 20);
}

TEST_CASE("selecting an absent item or an out-of-range row changes nothing")
{
    StorageItemSelection sel;
    sel.Add(Row(10, L"first"));
    CHECK(sel.Select(10));
    CHECK_FALSE(sel.Select(999));
    CHECK(sel.Selected()->m_iStorageItemSeq == 10);
    CHECK_FALSE(sel.SelectRow(-1));
    CHECK_FALSE(sel.SelectRow(5));
    CHECK(sel.Selected()->m_iStorageItemSeq == 10);
    CHECK(sel.SelectRow(0));
}
