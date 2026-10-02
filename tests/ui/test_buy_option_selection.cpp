#include "doctest.h"

#include <cwchar>
#include <ostream>

#include "GameShop/BuyOptionSelection.h"

using GameShop::BuyOption;
using GameShop::BuyOptionSelection;

namespace
{
BuyOption Option(int priceSeq, const wchar_t* name)
{
    BuyOption option;
    option.m_iPriceSeq = priceSeq;
    wcsncpy_s(option.m_szItemName, name, _TRUNCATE);
    return option;
}
} // namespace

TEST_CASE("an empty option list has nothing selected")
{
    BuyOptionSelection sel;
    CHECK(sel.Empty());
    CHECK(sel.Selected() == nullptr);
    CHECK(sel.SelectedRow() == -1);
    sel.SelectLast();
    CHECK(sel.Selected() == nullptr);
}

TEST_CASE("the last option added is the one picked, as the native list did")
{
    BuyOptionSelection sel;
    sel.Add(Option(7, L"30 days"));
    sel.Add(Option(9, L"90 days"));
    sel.SelectLast();
    REQUIRE(sel.Selected() != nullptr);
    CHECK(sel.Selected()->m_iPriceSeq == 9);
    CHECK(sel.SelectedRow() == 1);
}

TEST_CASE("a pick is held by price sequence, so rebuilding for another package drops it")
{
    BuyOptionSelection sel;
    sel.Add(Option(7, L"30 days"));
    sel.Add(Option(9, L"90 days"));
    CHECK(sel.Select(7));
    CHECK(sel.SelectedRow() == 0);

    sel.Clear();
    CHECK(sel.Selected() == nullptr);
    sel.Add(Option(11, L"7 days"));
    sel.Add(Option(13, L"14 days"));
    CHECK(sel.Selected() == nullptr); // row 0 is a different option now
    CHECK_FALSE(sel.Select(7));
    sel.SelectLast();
    CHECK(sel.Selected()->m_iPriceSeq == 13);
}

TEST_CASE("selection changes are reported once each, as IsChangeLine did")
{
    BuyOptionSelection sel;
    sel.Add(Option(7, L"30 days"));
    sel.Add(Option(9, L"90 days"));

    sel.SelectLast();
    CHECK(sel.TakeSelectionChanged());
    CHECK_FALSE(sel.TakeSelectionChanged());

    CHECK(sel.Select(7));
    CHECK(sel.TakeSelectionChanged());
    CHECK_FALSE(sel.TakeSelectionChanged());

    CHECK(sel.Select(7)); // same option again
    CHECK_FALSE(sel.TakeSelectionChanged());
}

TEST_CASE("an absent sequence or an out-of-range row changes nothing")
{
    BuyOptionSelection sel;
    sel.Add(Option(7, L"30 days"));
    CHECK(sel.Select(7));
    CHECK_FALSE(sel.Select(999));
    CHECK(sel.Selected()->m_iPriceSeq == 7);
    CHECK_FALSE(sel.SelectRow(-1));
    CHECK_FALSE(sel.SelectRow(3));
    CHECK(sel.Selected()->m_iPriceSeq == 7);
    CHECK(sel.SelectRow(0));
}
