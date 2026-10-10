#include "GameShop/BuyOptionSelection.h"

namespace GameShop
{
void BuyOptionSelection::Clear()
{
    m_Options.clear();
    m_SelectedPriceSeq = -1;
    m_ReportedPriceSeq = -1;
}

void BuyOptionSelection::Add(const BuyOption& option)
{
    m_Options.push_back(option);
}

void BuyOptionSelection::SelectLast()
{
    if (m_Options.empty())
    {
        m_SelectedPriceSeq = -1;
        return;
    }
    m_SelectedPriceSeq = m_Options.back().m_iPriceSeq;
}

bool BuyOptionSelection::Select(int priceSeq)
{
    for (const BuyOption& option : m_Options)
    {
        if (option.m_iPriceSeq == priceSeq)
        {
            m_SelectedPriceSeq = priceSeq;
            return true;
        }
    }
    return false;
}

bool BuyOptionSelection::SelectRow(int displayIndex)
{
    if (displayIndex < 0 || displayIndex >= static_cast<int>(m_Options.size()))
        return false;
    m_SelectedPriceSeq = m_Options[displayIndex].m_iPriceSeq;
    return true;
}

const BuyOption* BuyOptionSelection::Selected() const
{
    if (m_SelectedPriceSeq < 0)
        return nullptr;
    for (const BuyOption& option : m_Options)
    {
        if (option.m_iPriceSeq == m_SelectedPriceSeq)
            return &option;
    }
    return nullptr;
}

int BuyOptionSelection::SelectedRow() const
{
    for (size_t i = 0; i < m_Options.size(); ++i)
    {
        if (m_Options[i].m_iPriceSeq == m_SelectedPriceSeq)
            return static_cast<int>(i);
    }
    return -1;
}

bool BuyOptionSelection::TakeSelectionChanged()
{
    if (m_ReportedPriceSeq == m_SelectedPriceSeq)
        return false;
    m_ReportedPriceSeq = m_SelectedPriceSeq;
    return true;
}
} // namespace GameShop
