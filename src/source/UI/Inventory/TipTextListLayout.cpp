#include "stdafx.h"

#include "UI/Inventory/TipTextListLayout.h"

#include "Engine/Object/ZzzInventory.h"
#include "I18N/All.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/Scaling/UITransform.h"

#include <cmath>

extern int TextNum;
extern int g_iItemInfo[16][17];

namespace
{
constexpr std::uint32_t Rgba(unsigned r, unsigned g, unsigned b, unsigned a)
{
    return (a << 24) | (b << 16) | (g << 8) | r;
}

// RenderText()'s reported size: the measured size, shrunk with the text when wider than its box.
SIZE MeasureInBox(const wchar_t* text, bool bold, float box)
{
    SIZE size = g_pRenderText->MeasureText(text, static_cast<int>(wcslen(text)));
    if (box > 0 && size.cx > box)
    {
        const auto role = bold ? UI::Scaling::FontRole::Bold : UI::Scaling::FontRole::Normal;
        const auto transform = UI::Scaling::GetActiveTransform();
        const float ratio = UI::Scaling::NativeTextPixelSizeInBox(role, transform, static_cast<float>(size.cx), box) /
                            UI::Scaling::NativeTextPixelSize(role, transform);
        size.cx = static_cast<LONG>(std::lround(static_cast<float>(size.cx) * ratio));
        size.cy = static_cast<LONG>(std::lround(static_cast<float>(size.cy) * ratio));
    }
    return size;
}
} // namespace

// The geometry and colours of RenderTipTextList() (ZzzInventory.cpp), line for line.
void UI::TipTextList::Record(TipTextListRecord& record, int sx, int sy, int textNum, int tab, int sort, int renderPoint,
                             bool useBackground)
{
    SIZE TextSize = {0, 0};
    float fWidth = 0;
    float fHeight = 0;
    for (int i = 0; i < textNum; ++i)
    {
        if (TextList[i][0] == '\0')
        {
            textNum = i;
            break;
        }

        g_pRenderText->SetFont(TextBold[i] ? g_hFontBold : g_hFont);
        TextSize = g_pRenderText->MeasureText(TextList[i], static_cast<int>(wcslen(TextList[i])));

        if (fWidth < TextSize.cx)
            fWidth = static_cast<float>(TextSize.cx);

        const bool halfLine = TextList[i][0] == '\n';
        const bool spacer = halfLine || (TextList[i][0] == L' ' && TextList[i][1] == L'\0');
        const SIZE lineSize = spacer ? g_pRenderText->MeasureText(L"Q", 1) : TextSize;
        fHeight += static_cast<float>(lineSize.cy) * (halfLine ? 0.55f : 1.1f);
    }

    if (tab > 0)
        fWidth = static_cast<float>(tab * 2);
    fWidth += 4;
    int iPos_x = static_cast<int>(static_cast<float>(sx) - fWidth / 2);
    if (iPos_x < 0)
        iPos_x = 0;
    if (static_cast<float>(iPos_x) + fWidth > REFERENCE_WIDTH)
        iPos_x = static_cast<int>(REFERENCE_WIDTH - fWidth - 1);

    const float fsx = static_cast<float>(iPos_x + 1);
    float fsy = renderPoint == STRP_BOTTOMCENTER ? static_cast<float>(sy) - fHeight : static_cast<float>(sy);

    if (useBackground && textNum > 0)
    {
        // RenderColor(..., 1.0f, 1): an opaque black frame; the fill black at 0.8.
        const float x = static_cast<float>(iPos_x);
        record.boxes.push_back({x - 1, fsy - 1, fWidth + 1, 1.f, TipTextListRecord::BoxKind::Border});
        record.boxes.push_back({x - 1, fsy - 1, 1.f, fHeight + 1, TipTextListRecord::BoxKind::Border});
        record.boxes.push_back({x - 1 + fWidth + 1, fsy - 1, 1.f, fHeight + 1, TipTextListRecord::BoxKind::Border});
        record.boxes.push_back({x - 1, fsy - 1 + fHeight + 1, fWidth + 2, 1.f, TipTextListRecord::BoxKind::Border});
        record.boxes.push_back({x, fsy, fWidth, fHeight, TipTextListRecord::BoxKind::Fill});
    }

    for (int i = 0; i < textNum; i++)
    {
        g_pRenderText->SetFont(TextBold[i] ? g_hFontBold : g_hFont);

        float lineHeight = 0;
        if (TextList[i][0] == 0x0a || (TextList[i][0] == ' ' && TextList[i][1] == 0x00))
        {
            TextSize = g_pRenderText->MeasureText(L"Q", 1);
            lineHeight = static_cast<float>(TextSize.cy) / (TextList[i][0] == 0x0a ? 2.0f : 1.0f);
        }
        else
        {
            std::uint32_t color = Rgba(255, 255, 255, 255);
            switch (TextListColor[i])
            {
            case TEXT_COLOR_BLUE:
                color = Rgba(128, 179, 255, 255);
                break;
            case TEXT_COLOR_GRAY:
                color = Rgba(102, 102, 102, 255);
                break;
            case TEXT_COLOR_RED:
                color = Rgba(255, 51, 26, 255);
                break;
            case TEXT_COLOR_YELLOW:
                color = Rgba(255, 204, 26, 255);
                break;
            case TEXT_COLOR_GREEN:
                color = Rgba(26, 255, 128, 255);
                break;
            case TEXT_COLOR_PURPLE:
                color = Rgba(255, 26, 255, 255);
                break;
            case TEXT_COLOR_REDPURPLE:
                color = Rgba(204, 128, 204, 255);
                break;
            case TEXT_COLOR_VIOLET:
                color = Rgba(179, 102, 255, 255);
                break;
            case TEXT_COLOR_ORANGE:
                color = Rgba(230, 107, 10, 255);
                break;
            default:
                break;
            }
            std::uint32_t bgColor = 0;
            if (TEXT_COLOR_DARKRED == TextListColor[i])
                bgColor = Rgba(160, 0, 0, 255);
            else if (TEXT_COLOR_DARKBLUE == TextListColor[i])
                bgColor = Rgba(0, 0, 160, 255);
            else if (TEXT_COLOR_DARKYELLOW == TextListColor[i])
                bgColor = Rgba(160, 102, 0, 255);
            else if (TEXT_COLOR_GREEN_BLUE == TextListColor[i])
            {
                bgColor = Rgba(60, 60, 200, 255);
                color = Rgba(0, 255, 0, 255);
            }
            const float box = fWidth - 2;
            TextSize = MeasureInBox(TextList[i], TextBold[i] != 0, box);
            record.lines.push_back(
                {TextList[i], fsx, fsy, box, static_cast<float>(TextSize.cy), sort, TextBold[i] != 0, color, bgColor});
            lineHeight = static_cast<float>(TextSize.cy);
        }
        fsy += lineHeight * 1.1f;
    }
}

// RenderHelpCategory() (ZzzInventory.cpp): the column's heading, right-aligned, with the
// background its call left on (RenderTipTextList()'s default).
void UI::TipTextList::RecordHelpCategory(TipTextListRecord& record, int columnType, int x, int y)
{
    const wchar_t* pText = nullptr;
    switch (columnType)
    {
    case _COLUMN_TYPE_LEVEL:
        pText = I18N::Game::LV;
        break;
    case _COLUMN_TYPE_ATTMIN:
    case _COLUMN_TYPE_ATTMAX:
        pText = I18N::Game::ATKDmg;
        break;
    case _COLUMN_TYPE_MAGIC:
        pText = I18N::Game::WIZDmg;
        break;
    case _COLUMN_TYPE_CURSE:
        pText = I18N::Game::Curse;
        break;
    case _COLUMN_TYPE_PET_ATTACK:
        pText = I18N::Game::Attack;
        break;
    case _COLUMN_TYPE_DEFENCE:
        pText = I18N::Game::DEF;
        break;
    case _COLUMN_TYPE_DEFRATE:
        pText = I18N::Game::DEFRate;
        break;
    case _COLUMN_TYPE_REQSTR:
        pText = I18N::Game::STR;
        break;
    case _COLUMN_TYPE_REQDEX:
        pText = I18N::Game::AGI;
        break;
    case _COLUMN_TYPE_REQENG:
        pText = I18N::Game::ENG;
        break;
    case _COLUMN_TYPE_REQCHA:
        pText = I18N::Game::Command;
        break;
    case _COLUMN_TYPE_REQVIT:
        pText = I18N::Game::STA;
        break;
    case _COLUMN_TYPE_REQNLV:
        pText = I18N::Game::ReqLV;
        break;
    default:
        break;
    }
    mu_swprintf(TextList[TextNum], pText);
    TextListColor[TextNum] = TEXT_COLOR_BLUE;
    TextNum++;
    Record(record, x, y, TextNum, 0, RT3_SORT_RIGHT, FALSE, true);
    TextNum = 0;
}

// RenderHelpLine() (ZzzInventory.cpp): a column's value per item level, white where the hero can
// equip the item, red where not, then the column's advance.
void UI::TipTextList::RecordHelpLine(TipTextListRecord& record, int columnType, const wchar_t* printStyle,
                                     int& tabSpace, const wchar_t* gapText, int y, int itemType)
{
    const int maxLevel = itemType == 5 ? 0 : iMaxLevel;
    for (int Level = 0; Level <= maxLevel; ++Level)
    {
        mu_swprintf(TextList[TextNum], printStyle, g_iItemInfo[Level][columnType]);
        TextListColor[Level] = g_iItemInfo[Level][_COLUMN_TYPE_CAN_EQUIP] == TRUE ? TEXT_COLOR_WHITE : TEXT_COLOR_RED;
        TextBold[Level] = false;
        ++TextNum;
    }

    Record(record, tabSpace, y, TextNum, 0, RT3_SORT_CENTER, FALSE, true);

    const SIZE size = gapText == nullptr ? g_pRenderText->MeasureText(TextList[TextNum - 1],
                                                                      static_cast<int>(wcslen(TextList[TextNum - 1])))
                                         : g_pRenderText->MeasureText(gapText, static_cast<int>(wcslen(gapText)));
    tabSpace += size.cx;
    if (itemType == 6)
        tabSpace += 5;
    TextNum -= maxLevel + 1;
}
