#include "stdafx.h"
#include "UIGuildInfo.h"
#include "PersonalShopTitleImp.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "ZzzInterface.h"

#include "UIManager.h"
#include "NewUISystem.h"

CPersonalShopTitleImp::CPersonalShopTitleImp() : m_iHighlightFrame(0), m_bShow(true)
{}
CPersonalShopTitleImp::~CPersonalShopTitleImp()
{
    RemoveAllShopTitle();
}

bool CPersonalShopTitleImp::AddShopTitle(int key, CHARACTER* pPlayer, const std::wstring& title)
{
    if (pPlayer == NULL)
        return false;
    if (pPlayer->Object.Kind != KIND_PLAYER)
    {
        g_ErrorReport.Write(L"@ AddShopTitle - there is NOT player object(id : %s) \n", pPlayer->ID);
        return false;
    }

    std::wstring full_name = pPlayer->ID;
    std::wstring topTitle, bottomTitle;

    auto mi = m_listShopTitleDrawObj.find(pPlayer);
    if (mi != m_listShopTitleDrawObj.end())
    {
        CShopTitleDrawObj* pDrawObj = (*mi).second;
        if (pDrawObj->GetKey() == key)
        {
            pDrawObj->SetBoxContent(full_name, title);
            pDrawObj->SetBoxPos(MakePos(-1, -1));
        }
        else
        {
            g_ErrorReport.Write(L"@ AddShopTitle - player key-value dismatch(id : %s) \n", pPlayer->ID);
        }
    }
    else
    {
        auto* pDrawObj = new CShopTitleDrawObj;
        pDrawObj->Create(key, full_name, title, MakePos(-1, -1));
        m_listShopTitleDrawObj.insert(type_drawobj_map::value_type(pPlayer, pDrawObj));
    }

    return true;
}
void CPersonalShopTitleImp::RemoveShopTitle(CHARACTER* pPlayer)
{
    auto mi = m_listShopTitleDrawObj.find(pPlayer);
    if (mi != m_listShopTitleDrawObj.end())
    {
        delete (*mi).second;
        m_listShopTitleDrawObj.erase(mi);
    }
}
void CPersonalShopTitleImp::RemoveAllShopTitle()
{
    auto mi = m_listShopTitleDrawObj.begin();
    for (; mi != m_listShopTitleDrawObj.end(); ++mi)
        delete (*mi).second;
    m_listShopTitleDrawObj.clear();
}
void CPersonalShopTitleImp::RemoveAllShopTitleExceptHero()
{
    auto mi = m_listShopTitleDrawObj.begin();
    for (; mi != m_listShopTitleDrawObj.end();)
    {
        if ((*mi).second->GetKey() != Hero->Key)
        {
            delete (*mi).second;
            mi = m_listShopTitleDrawObj.erase(mi);
        }
        else
        {
            mi++;
        }
    }
}

CHARACTER* CPersonalShopTitleImp::FindCharacter(int key) const
{
    auto mi = m_listShopTitleDrawObj.begin();
    for (; mi != m_listShopTitleDrawObj.end(); ++mi)
    {
        if ((*mi).second->GetKey() == key)
        {
            return (*mi).first;
        }
    }
    return NULL;
}

void CPersonalShopTitleImp::ShowShopTitles()
{
    m_bShow = true;
}

void CPersonalShopTitleImp::HideShopTitles()
{
    m_bShow = false;
}

bool CPersonalShopTitleImp::IsShowShopTitles() const
{
    return m_bShow;
}

void CPersonalShopTitleImp::EnableShopTitleDraw(CHARACTER* pPlayer)
{
    auto mi = m_listShopTitleDrawObj.find(pPlayer);
    if (mi != m_listShopTitleDrawObj.end())
    {
        (*mi).second->EnableDraw();
    }
}
void CPersonalShopTitleImp::DisableShopTitleDraw(CHARACTER* pPlayer)
{
    auto mi = m_listShopTitleDrawObj.find(pPlayer);
    if (mi != m_listShopTitleDrawObj.end()) {
        (*mi).second->DisableDraw();
    }
}
bool CPersonalShopTitleImp::IsShopTitleVisible(CHARACTER* pPlayer)
{
    type_drawobj_map::const_iterator mi = m_listShopTitleDrawObj.find(pPlayer);
    if (mi != m_listShopTitleDrawObj.end()) {
        return (*mi).second->IsVisible();
    }
    return false;
}

bool CPersonalShopTitleImp::IsShopTitleHighlight(CHARACTER* pPlayer) const
{
    auto mi = m_listShopTitleDrawObj.find(pPlayer);
    if (mi != m_listShopTitleDrawObj.end()) {
        return (*mi).second->IsHighlight();
    }
    return false;
}

bool CPersonalShopTitleImp::IsInViewport(CHARACTER* pPlayer)
{
    auto mi = m_listShopTitleDrawObj.find(pPlayer);
    if (mi != m_listShopTitleDrawObj.end())
    {
        return true;
    }
    return false;
}
void CPersonalShopTitleImp::GetShopTitle(CHARACTER* pPlayer, std::wstring& title)
{
    auto mi = m_listShopTitleDrawObj.find(pPlayer);
    if (mi != m_listShopTitleDrawObj.end()) {
        (*mi).second->GetFullTitle(title);
    }
}
void CPersonalShopTitleImp::GetShopTitleSummary(CHARACTER* pPlayer, std::wstring& summary)
{
    auto mi = m_listShopTitleDrawObj.find(pPlayer);
    if (mi != m_listShopTitleDrawObj.end()) {
        std::wstring full_title;
        (*mi).second->GetFullTitle(full_title);
        if (full_title.size() > 14) {
            int offset = 0;
            for (; offset < 12; ) {
                if (full_title[offset] & 0x80)
                    offset += 2;
                else
                    offset++;
            }
            summary.assign(full_title, 0, offset);
            summary += L"..";
        }
        else {
            summary = full_title;
        }
    }
}

DWORD CPersonalShopTitleImp::GetShopTextColor(CHARACTER* pPlayer)
{
    if (pPlayer->GuildRelationShip == GR_RIVAL || pPlayer->GuildRelationShip == GR_RIVALUNION ||
        pPlayer->PK == PVP_MURDERER2)
    {
        return RGBA(240, 20, 0, 255);
    }
    else
    {
        switch (pPlayer->PK)
        {
        case PVP_CAUTION:
            return RGBA(230, 180, 0, 255);
        case PVP_MURDERER1:
            return RGBA(230, 110, 0, 255);
        default:
            return IsShopTitleHighlight(pPlayer) ? (255 << 24) + (255 << 16) + (230 << 8) + (230) : (255 << 24) + (0 << 16) + (230 << 8) + (230);
        }
    }
}

DWORD CPersonalShopTitleImp::GetShopText2Color(CHARACTER* pPlayer)
{
    if (pPlayer->GuildRelationShip == GR_RIVAL || pPlayer->GuildRelationShip == GR_RIVALUNION ||
        pPlayer->PK == PVP_MURDERER2)
    {
        return RGBA(240, 20, 0, 255);
    }
    else
    {
        switch (pPlayer->PK)
        {
        case PVP_CAUTION:
            return RGBA(230, 180, 0, 255);
        case PVP_MURDERER1:
            return RGBA(230, 110, 0, 255);
        default:
            return IsShopTitleHighlight(pPlayer) ? (255 << 24) + (41 << 16) + (57 << 8) + (108) : (255 << 24) + (0 << 16) + (150 << 8) + (250);
        }
    }
}

DWORD CPersonalShopTitleImp::GetShopBGColor(CHARACTER* pPlayer)
{
    if (pPlayer->GuildRelationShip == GR_RIVAL || pPlayer->GuildRelationShip == GR_RIVALUNION ||
        pPlayer->PK == PVP_MURDERER2)
    {
        return IsShopTitleHighlight(pPlayer) ? RGBA(182, 122, 82, 128) : RGBA(108, 57, 41, 128);
    }
    else
    {
        switch (pPlayer->PK)
        {
        case PVP_CAUTION:
            return IsShopTitleHighlight(pPlayer) ? RGBA(182, 122, 82, 128) : RGBA(108, 57, 41, 128);
        case PVP_MURDERER1:
            return IsShopTitleHighlight(pPlayer) ? RGBA(182, 122, 82, 128) : RGBA(108, 57, 41, 128);
        default:
            return IsShopTitleHighlight(pPlayer) ? (128 << 24) + (0 << 16) + (150 << 8) + (250) : (128 << 24) + (41 << 16) + (57 << 8) + (108);
        }
    }
}

void CPersonalShopTitleImp::Update()
{
    if (!m_listShopTitleDrawObj.empty()) {
        CheckKeyIntegrity();
    }
}
void CPersonalShopTitleImp::Draw()
{
    if (!m_listShopTitleDrawObj.empty())
    {
        UpdatePosition();
        RevisionPosition();

        auto isHighlightTime = static_cast<INT64>(WorldTime / 265) % 2 == 0;

        auto mi = m_listShopTitleDrawObj.begin();
        for (; mi != m_listShopTitleDrawObj.end(); ++mi)
        {
            CShopTitleDrawObj* pDrawObj = mi->second;
            if (SelectedCharacter != -1
                && isHighlightTime
                && g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_COMMAND)
                && g_pCommandWindow->GetCurCommandType() == COMMAND_PURCHASE)
            {
                CHARACTER* selectedPlayer = &CharactersClient[SelectedCharacter];
                if (mi->first == selectedPlayer)
                {
                    pDrawObj->EnableHighlight();
                }
            }
            else
            {
                pDrawObj->DisableHighlight();
            }

            if (pDrawObj->IsVisible() && IsShowShopTitles())
            {
                pDrawObj->Draw(mi->first->PK);
            }
        }
    }
}

CPersonalShopTitleImp* CPersonalShopTitleImp::GetObjPtr()
{
    static CPersonalShopTitleImp s_Instance;
    return &s_Instance;
}

void CPersonalShopTitleImp::UpdatePosition()
{
    float Width = GetScreenWidth(), Height = 480;

    if (!CameraTopViewEnable)
        Height = 480 - 48;

    EndBitmap();
    BeginOpengl(0, 0, Width, Height);

    auto mi = m_listShopTitleDrawObj.begin();
    for (; mi != m_listShopTitleDrawObj.end(); ++mi) {
        CShopTitleDrawObj* pDrawObj = (*mi).second;

        SIZE size;
        pDrawObj->GetBoxSize(size);

        POINT pos;
        CalculateBooleanPos((*mi).first, size, pos);	//. real position

        pDrawObj->SetBoxPos(pos);
    }

    EndOpengl();
    BeginBitmap();
}
void CPersonalShopTitleImp::RevisionPosition()
{
    //. Fit to screen
    auto mi_x = m_listShopTitleDrawObj.begin();
    for (; mi_x != m_listShopTitleDrawObj.end(); ++mi_x) {
        auto mi_y = m_listShopTitleDrawObj.begin();
        for (; mi_y != m_listShopTitleDrawObj.end(); ++mi_y) {
            if (mi_x != mi_y) {
                RECT rcX, rcY;
                (*mi_x).second->GetBoxRect(rcX);
                (*mi_y).second->GetBoxRect(rcY);

                if (rcX.right > rcY.left && rcX.left < rcY.right &&
                    rcX.bottom > rcY.top && rcX.top < rcY.bottom)
                {
                    POINT pos;
                    if (rcX.bottom < (rcY.top + rcY.bottom) / 2) {
                        pos.x = rcX.left;
                        pos.y = rcY.top - (rcX.bottom - rcX.top);
                    }
                    else {
                        pos.x = rcX.left;
                        pos.y = rcY.bottom;
                    }
                    (*mi_x).second->SetBoxPos(pos);
                }
            }
        }
    }

    for (mi_x = m_listShopTitleDrawObj.begin(); mi_x != m_listShopTitleDrawObj.end(); ++mi_x) {
        POINT pos; SIZE size;
        (*mi_x).second->GetBoxPos(pos);
        (*mi_x).second->GetBoxSize(size);
        if (pos.x < 0) pos.x = 0;
        if ((unsigned int)pos.x >= WindowWidth - (unsigned int)size.cx) pos.x = WindowWidth - (size.cx + 4);
        if (pos.y < 0) pos.y = 0;
        if ((unsigned int)pos.y >= WindowHeight - (unsigned int)size.cy) pos.y = WindowHeight - size.cy;
        (*mi_x).second->SetBoxPos(pos);
    }
}
void CPersonalShopTitleImp::CheckKeyIntegrity()
{
    auto mi = m_listShopTitleDrawObj.begin();

    for (; mi != m_listShopTitleDrawObj.end();)
    {
        CHARACTER* pPlayer = (*mi).first;
        CShopTitleDrawObj* pDrawObj = (*mi).second;

        if (pPlayer->Key != pDrawObj->GetKey())
        {
            delete pDrawObj;
            mi = m_listShopTitleDrawObj.erase(mi);
            g_ErrorReport.Write(L"@ CheckKeyIntegrity - player key-value dismatch(id : %s, server's key : %d, client array's key : %d) \n",
                pPlayer->ID, pDrawObj->GetKey(), pPlayer->Key);
        }
        else if (pPlayer->Object.Kind != KIND_PLAYER)
        {
            delete pDrawObj;
            mi = m_listShopTitleDrawObj.erase(mi);
            g_ErrorReport.Write(L"@ CheckKeyIntegrity - player type invalid(id : %s, server's key : %d, client array's key : %d) \n",
                pPlayer->ID, pDrawObj->GetKey(), pPlayer->Key);
        }
        else
        {
            mi++;
        }
    }
}

void CPersonalShopTitleImp::CalculateBooleanPos(IN CHARACTER* pPlayer, IN const SIZE& size, OUT POINT& pos)
{
    vec3_t posTemp;
    OBJECT* pObject = &pPlayer->Object;
    Vector(pObject->Position[0], pObject->Position[1], pObject->Position[2] + pObject->BoundingBoxMax[2] + 60.f, posTemp);

    POINT ptFloating;
    Projection(posTemp, (int*)&ptFloating.x, (int*)&ptFloating.y);		//. logical position

    //. pos : real position
    pos.x = (ptFloating.x * (int)WindowWidth / 640) - size.cx / 2;
    pos.y = (ptFloating.y * (int)WindowHeight / 480) - (12 * 3 * (int)WindowHeight / 480);
}

CPersonalShopTitleImp::CShopTitleDrawObj::CShopTitleDrawObj() { Init(); }
CPersonalShopTitleImp::CShopTitleDrawObj::~CShopTitleDrawObj() {}

void CPersonalShopTitleImp::CShopTitleDrawObj::Init()
{
    //. initialize
    m_fullname = L"";
    m_topTitle = L"";
    m_bottomTitle = L"";

    m_key = -1;
    m_bDraw = false;
    m_pos.x = m_pos.y = 0;
    m_size.cx = m_size.cy = 0;
    m_icon.cx = m_icon.cy = 0;
    m_bHighlight = false;
}

bool CPersonalShopTitleImp::CShopTitleDrawObj::Create(int key, const std::wstring& name, const std::wstring& title, POINT& pos)
{
    m_key = key & 0x7FFF;
    SetBoxContent(name, title);
    SetBoxPos(pos);
    EnableDraw();

    return true;
}
void CPersonalShopTitleImp::CShopTitleDrawObj::Release()
{
    Init();
}

int CPersonalShopTitleImp::CShopTitleDrawObj::GetKey() const
{
    return m_key;
}

void CPersonalShopTitleImp::CShopTitleDrawObj::SetBoxContent(const std::wstring& name, const std::wstring& title)
{
    m_fullname = name;
    m_fulltitle = title;

    g_pRenderText->SetFont(g_hFontBold);
    GetTextExtentPoint32(g_pRenderText->GetFontDC(), GlobalText[1104], GlobalText.GetStringSize(1104), &m_icon);

    SeparateShopTitle(title, m_topTitle, m_bottomTitle);
    CalculateBooleanSize(name, m_topTitle, m_bottomTitle, m_size);
}
void CPersonalShopTitleImp::CShopTitleDrawObj::SetBoxPos(POINT& pos)
{
    m_pos = pos;
}

void CPersonalShopTitleImp::CShopTitleDrawObj::GetBoxSize(SIZE& size)
{
    size = m_size;
}

void CPersonalShopTitleImp::CShopTitleDrawObj::GetBoxPos(POINT& pos)
{
    pos = m_pos;
}

void CPersonalShopTitleImp::CShopTitleDrawObj::GetBoxRect(RECT& rect)
{
    rect.left = m_pos.x;
    rect.top = m_pos.y;
    rect.right = m_pos.x + m_size.cx;
    rect.bottom = m_pos.y + m_size.cy;
}
void CPersonalShopTitleImp::CShopTitleDrawObj::GetFullTitle(std::wstring& title)
{
    title = m_fulltitle;
}

void CPersonalShopTitleImp::CShopTitleDrawObj::EnableDraw()
{
    m_bDraw = true;
}
void CPersonalShopTitleImp::CShopTitleDrawObj::DisableDraw()
{
    m_bDraw = false;
}
bool CPersonalShopTitleImp::CShopTitleDrawObj::IsVisible() const
{
    return m_bDraw;
}

void CPersonalShopTitleImp::CShopTitleDrawObj::EnableHighlight()
{
    m_bHighlight = true;
}
void CPersonalShopTitleImp::CShopTitleDrawObj::DisableHighlight()
{
    m_bHighlight = false;
}
bool CPersonalShopTitleImp::CShopTitleDrawObj::IsHighlight() const
{
    return m_bHighlight;
}

void CPersonalShopTitleImp::CShopTitleDrawObj::Draw(int iPkLevel)
{
    // Render who's shop
    int iIconBkColor = IsHighlight() ? RGBA(250, 150, 0, 128) : RGBA(108, 57, 41, 128);
    int iIconTextColor = IsHighlight() ? RGBA(108, 57, 41, 255) : RGBA(250, 150, 0, 255);
    int iNameBkColor = IsHighlight() ? RGBA(250, 150, 0, 128) : RGBA(108, 57, 41, 128);
    int iNameTextColor = IsHighlight() ? RGBA(230, 230, 255, 255) : RGBA(230, 230, 0, 255);

    switch (iPkLevel) {
    case PVP_CAUTION:
    {
        iIconBkColor = iNameBkColor = IsHighlight() ? RGBA(182, 122, 82, 128) : RGBA(108, 57, 41, 128);
        iIconTextColor = iNameTextColor = RGBA(230, 180, 0, 255);
    }
    break;
    case PVP_MURDERER1:
    {
        iIconBkColor = iNameBkColor = IsHighlight() ? RGBA(182, 122, 82, 128) : RGBA(108, 57, 41, 128);
        iIconTextColor = iNameTextColor = RGBA(230, 110, 0, 255);
    }
    break;
    case PVP_MURDERER2:
    {
        iIconBkColor = iNameBkColor = IsHighlight() ? RGBA(182, 122, 82, 128) : RGBA(108, 57, 41, 128);
        iIconTextColor = iNameTextColor = RGBA(240, 20, 0, 255);
    }
    break;
    }

    
    
    POINT RenderPos = { m_pos.x / g_fScreenRate_x, m_pos.y / g_fScreenRate_y };
    SIZE RenderBoxSize = { m_size.cx / g_fScreenRate_x, m_size.cy / g_fScreenRate_y };
    SIZE RenderIconSize = { m_icon.cx / g_fScreenRate_x, m_icon.cy / g_fScreenRate_y };
    int iLineHeight = FontHeight / g_fScreenRate_y;

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetBgColor(iIconBkColor);
    g_pRenderText->SetTextColor(iIconTextColor);
    g_pRenderText->RenderText(RenderPos.x, RenderPos.y, GlobalText[1104], RenderIconSize.cx, iLineHeight);

    g_pRenderText->SetBgColor(iNameBkColor);
    g_pRenderText->SetTextColor(iNameTextColor);
    g_pRenderText->RenderText(RenderPos.x + RenderIconSize.cx, RenderPos.y, m_fullname.c_str(),
        RenderBoxSize.cx - RenderIconSize.cx, iLineHeight);
    RenderPos.y += iLineHeight;

    g_pRenderText->SetFont(g_hFont);

    // Render shop title
    if (!m_bottomTitle.empty())
    {
        g_pRenderText->SetBgColor(108, 57, 41, 200);
        g_pRenderText->SetTextColor(255, 255, 255, 255);
        g_pRenderText->RenderText(RenderPos.x, RenderPos.y, m_topTitle.c_str(), RenderBoxSize.cx, iLineHeight);
        RenderPos.y += iLineHeight;
        g_pRenderText->RenderText(RenderPos.x, RenderPos.y, m_bottomTitle.c_str(), RenderBoxSize.cx, iLineHeight);
    }
    else if (!m_topTitle.empty())
    {
        g_pRenderText->SetBgColor(108, 57, 41, 200);
        g_pRenderText->SetTextColor(255, 255, 255, 255);
        g_pRenderText->RenderText(RenderPos.x, RenderPos.y, m_topTitle.c_str(), RenderBoxSize.cx, iLineHeight);
    }
}
void CPersonalShopTitleImp::CShopTitleDrawObj::SeparateShopTitle(const std::wstring& title, std::wstring& topTitle, std::wstring& bottomTitle)
{
    wchar_t pszTopTitle[MAX_SHOPTITLE] = { 0 };
    wchar_t pszBottomTitle[MAX_SHOPTITLE] = { 0 };
    CutText(title.c_str(), pszTopTitle, pszBottomTitle, MAX_SHOPTITLE);

    topTitle = pszTopTitle;
    bottomTitle = pszBottomTitle;
}
void CPersonalShopTitleImp::CShopTitleDrawObj::CalculateBooleanSize(IN const std::wstring& name, IN const std::wstring& topTitle, IN const std::wstring& bottomTitle, OUT SIZE& size)
{
    SIZE text_size[3];
    g_pRenderText->SetFont(g_hFontBold);
    GetTextExtentPoint32(g_pRenderText->GetFontDC(), name.c_str(), name.size(), &text_size[0]);
    g_pRenderText->SetFont(g_hFont);
    GetTextExtentPoint32(g_pRenderText->GetFontDC(), topTitle.c_str(), topTitle.size(), &text_size[1]);
    GetTextExtentPoint32(g_pRenderText->GetFontDC(), bottomTitle.c_str(), bottomTitle.size(), &text_size[2]);

    int maxWidth, maxHeight;
    if (!bottomTitle.empty())
    {
        maxWidth = max(text_size[0].cx + m_icon.cx, max(text_size[1].cx, text_size[2].cx));
        maxHeight = FontHeight * 3;
    }
    else if (!topTitle.empty())
    {
        maxWidth = max(text_size[0].cx + m_icon.cx, text_size[1].cx);
        maxHeight = FontHeight * 2;
    }
    else
    {
        maxWidth = text_size[0].cx + m_icon.cx;
        maxHeight = FontHeight;
    }

    size.cx = maxWidth;
    size.cy = maxHeight;
}

// CPersonalItemPriceTable static member initialize
CPersonalItemPriceTable* CPersonalItemPriceTable::ms_pBuyer = NULL;
CPersonalItemPriceTable* CPersonalItemPriceTable::ms_pSeller = NULL;