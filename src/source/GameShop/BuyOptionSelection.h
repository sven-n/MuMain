#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace GameShop
{
// Mirrors _define.h's MAX_TEXT_LENGTH so this header stays free of the engine's Windows-typed
// globals and can be unit tested on its own. MsgBoxIGSBuySelectItem.cpp static_asserts it.
inline constexpr int kBuyOptionTextLength = 255;

// One purchasable option of a package -- a price/period/cash-type choice. This is
// IGS_SelectBuyItem moved out of the widget toolkit, minus the per-row ::CRadioButton it carried
// to draw its own indicator and minus m_bIsSelected: which option is picked belongs to
// BuyOptionSelection below, and the indicator is the theme's.
struct BuyOption
{
    int m_iPackageSeq = 0;
    int m_iDisplaySeq = 0;
    int m_iPriceSeq = 0;
    std::uint16_t m_wItemCode = 0;
    int m_iCashType = 0;

    wchar_t m_szItemName[kBuyOptionTextLength] = {};
    wchar_t m_szItemPrice[kBuyOptionTextLength] = {};
    wchar_t m_szItemPeriod[kBuyOptionTextLength] = {};
    wchar_t m_szAttribute[kBuyOptionTextLength] = {};
};

// The options offered for one package and which is picked. Held as a price sequence rather than a
// row index: the dialog is rebuilt whenever the package changes, and an index would survive into
// a different option.
class BuyOptionSelection
{
public:
    void Clear();
    void Add(const BuyOption& option);
    // The native list selected each row as it arrived, leaving the last one picked.
    void SelectLast();
    bool Select(int priceSeq);
    bool SelectRow(int displayIndex);

    std::span<const BuyOption> Options() const { return m_Options; }
    const BuyOption* Selected() const;
    int SelectedRow() const;
    bool Empty() const { return m_Options.empty(); }
    int Count() const { return static_cast<int>(m_Options.size()); }
    // True once per change, as the native list box's IsChangeLine() reported it.
    bool TakeSelectionChanged();

private:
    std::vector<BuyOption> m_Options;
    int m_SelectedPriceSeq = -1;
    int m_ReportedPriceSeq = -1;
};
} // namespace GameShop
