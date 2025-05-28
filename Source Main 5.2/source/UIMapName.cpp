//*****************************************************************************
// File: UIMapName.cpp
//*****************************************************************************

#include "stdafx.h"
#include "UIMapName.h"
#include "MapManager.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"

#include "UIWindows.h"
#ifdef ASG_ADD_GENS_SYSTEM
#include "ZzzInventory.h"
#endif	// ASG_ADD_GENS_SYSTEM




#define	UIMN_SHOW_TIME			5000
#define	UIMN_ALPHA_VARIATION	0.015f

#define	UIMN_IMG_WIDTH			166.0f
#define	UIMN_IMG_HEIGHT			90.0f

#ifdef ASG_ADD_GENS_SYSTEM
#define UIMN_STRIFE_HEIGHT		28.0f
#endif	// ASG_ADD_GENS_SYSTEM

#define	UIMN_IMG_POS_X		((::GetScreenWidth() * g_fScreenRate_x - UIMN_IMG_WIDTH) / 2.0f)
#define	UIMN_IMG_POS_Y		(220.0f * g_fScreenRate_y)

#ifdef ASG_ADD_GENS_SYSTEM
#define UIMN_STRIFE_POS_Y	(UIMN_IMG_POS_Y - UIMN_STRIFE_HEIGHT)
#endif	// ASG_ADD_GENS_SYSTEM

CUIMapName::CUIMapName()
{
    InitImgPathMap();
}

CUIMapName::~CUIMapName()
{
}

void CUIMapName::InitImgPathMap()
{
    std::wstring strFolderName = L"Local\\" + g_strSelectedML + L"\\ImgsMapName\\";

    m_mapImgPath[0] = strFolderName + L"lorencia.tga";
    m_mapImgPath[1] = strFolderName + L"dungeun.tga";
    m_mapImgPath[2] = strFolderName + L"devias.tga";
    m_mapImgPath[3] = strFolderName + L"noria.tga";
    m_mapImgPath[4] = strFolderName + L"losttower.tga";
    m_mapImgPath[6] = strFolderName + L"stadium.tga";
    m_mapImgPath[7] = strFolderName + L"atlans.tga";
    m_mapImgPath[8] = strFolderName + L"tarcan.tga";
    m_mapImgPath[9] = strFolderName + L"devilsquare.tga";
    m_mapImgPath[10] = strFolderName + L"Icarus.tga";
    m_mapImgPath[11] = strFolderName + L"bloodcastle.tga";
    m_mapImgPath[12] = strFolderName + L"bloodcastle.tga";
    m_mapImgPath[13] = strFolderName + L"bloodcastle.tga";
    m_mapImgPath[14] = strFolderName + L"bloodcastle.tga";
    m_mapImgPath[15] = strFolderName + L"bloodcastle.tga";
    m_mapImgPath[16] = strFolderName + L"bloodcastle.tga";
    m_mapImgPath[17] = strFolderName + L"bloodcastle.tga";
    m_mapImgPath[18] = strFolderName + L"chaoscastle.tga";
    m_mapImgPath[19] = strFolderName + L"chaoscastle.tga";
    m_mapImgPath[20] = strFolderName + L"chaoscastle.tga";
    m_mapImgPath[21] = strFolderName + L"chaoscastle.tga";
    m_mapImgPath[22] = strFolderName + L"chaoscastle.tga";
    m_mapImgPath[23] = strFolderName + L"chaoscastle.tga";
    m_mapImgPath[24] = strFolderName + L"Kalima.tga";
    m_mapImgPath[25] = strFolderName + L"Kalima.tga";
    m_mapImgPath[26] = strFolderName + L"Kalima.tga";
    m_mapImgPath[27] = strFolderName + L"Kalima.tga";
    m_mapImgPath[28] = strFolderName + L"Kalima.tga";
    m_mapImgPath[29] = strFolderName + L"Kalima.tga";
    m_mapImgPath[30] = strFolderName + L"loren.tga";
    m_mapImgPath[31] = strFolderName + L"ordeal.tga";

    m_mapImgPath[33] = strFolderName + L"aida.tga";
    m_mapImgPath[34] = strFolderName + L"crywolffortress.tga";

    m_mapImgPath[36] = strFolderName + L"lostkalima.tga";
    m_mapImgPath[37] = strFolderName + L"kantru.tga";
    m_mapImgPath[38] = strFolderName + L"kantru.tga";
    m_mapImgPath[39] = strFolderName + L"kantru.tga";

    m_mapImgPath[41] = strFolderName + L"BalgasBarrack.tga";
    m_mapImgPath[42] = strFolderName + L"BalgasRefuge.tga";

    m_mapImgPath[45] = strFolderName + L"IllusionTemple.tga";
    m_mapImgPath[46] = strFolderName + L"IllusionTemple.tga";
    m_mapImgPath[47] = strFolderName + L"IllusionTemple.tga";
    m_mapImgPath[48] = strFolderName + L"IllusionTemple.tga";
    m_mapImgPath[49] = strFolderName + L"IllusionTemple.tga";
    m_mapImgPath[50] = strFolderName + L"IllusionTemple.tga";

    m_mapImgPath[51] = strFolderName + L"Elbeland.tga";
    m_mapImgPath[52] = strFolderName + L"bloodcastle.tga";
    m_mapImgPath[53] = strFolderName + L"chaoscastle.tga";

    m_mapImgPath[56] = strFolderName + L"SwampOfCalmness.tga";
    m_mapImgPath[57] = strFolderName + L"mapname_raklion.tga";
    m_mapImgPath[58] = strFolderName + L"mapname_raklionboss.tga";

    m_mapImgPath[62] = strFolderName + L"santatown.tga";
    m_mapImgPath[63] = strFolderName + L"pkfield.tga";
    m_mapImgPath[64] = strFolderName + L"duelarena.tga";
    m_mapImgPath[65] = strFolderName + L"doppelganger.tga";
    m_mapImgPath[66] = strFolderName + L"doppelganger.tga";
    m_mapImgPath[67] = strFolderName + L"doppelganger.tga";
    m_mapImgPath[68] = strFolderName + L"doppelganger.tga";
    m_mapImgPath[69] = strFolderName + L"EmpireGuardian.tga";
    m_mapImgPath[70] = strFolderName + L"EmpireGuardian.tga";
    m_mapImgPath[71] = strFolderName + L"EmpireGuardian.tga";
    m_mapImgPath[72] = strFolderName + L"EmpireGuardian.tga";
    m_mapImgPath[79] = strFolderName + L"MapName_MarketRolen.tga";

#ifdef ASG_ADD_MAP_KARUTAN
    m_mapImgPath[80] = strFolderName + L"MapName_Karutan.tga";
    m_mapImgPath[81] = strFolderName + L"MapName_Karutan.tga";
#endif	// ASG_ADD_MAP_KARUTAN
}

void CUIMapName::Init()
{
    m_eState = HIDE;
    m_nOldWorld = -1;
    m_dwOldTime = ::timeGetTime();
    m_dwDeltaTickSum = 0;
    m_fAlpha = 1.0f;
#ifdef ASG_ADD_GENS_SYSTEM
    m_bStrife = false;
#endif	// ASG_ADD_GENS_SYSTEM
}

void CUIMapName::ShowMapName()
{
    m_eState = FADEIN;
    m_fAlpha = 0.2f;
    m_dwDeltaTickSum = 0;

    if (gMapManager.WorldActive == WD_40AREA_FOR_GM)
    {
        m_eState = HIDE;
        return;
    }

    if (m_nOldWorld != gMapManager.WorldActive)
    {
        wchar_t szImgPath[128];
        ::wcscpy(szImgPath, m_mapImgPath[gMapManager.WorldActive].data());

        DeleteBitmap(BITMAP_INTERFACE_EX + 45);
        LoadBitmap(szImgPath, BITMAP_INTERFACE_EX + 45);

        m_nOldWorld = gMapManager.WorldActive;

#ifdef ASG_ADD_GENS_SYSTEM
        m_bStrife = ::IsStrifeMap(gMapManager.WorldActive);
#endif	// ASG_ADD_GENS_SYSTEM
    }
}

void CUIMapName::Update()
{
    DWORD dwNowTime = ::timeGetTime();
    DWORD dwDeltaTick = dwNowTime - m_dwOldTime;

    switch (m_eState)
    {
    case FADEIN:
        m_fAlpha += UIMN_ALPHA_VARIATION;
        if (1.0f <= m_fAlpha)
        {
            m_eState = SHOW;
            m_fAlpha = 1.0f;
        }
        break;

    case SHOW:
        m_dwDeltaTickSum += dwDeltaTick;
        if (m_dwDeltaTickSum > UIMN_SHOW_TIME)
        {
            m_eState = FADEOUT;
            m_dwDeltaTickSum = 0;
        }
        break;

    case FADEOUT:
        m_fAlpha -= UIMN_ALPHA_VARIATION;
        if (0.0f >= m_fAlpha)
        {
            m_eState = HIDE;
            m_fAlpha = 0.0f;
        }
        break;
    }

    m_dwOldTime = dwNowTime;
}

void CUIMapName::Render()
{
    Update();

    if (HIDE == m_eState)
        return;

    ::EnableAlphaTest();
    ::glColor4f(1.0f, 1.0f, 1.0f, m_fAlpha);

#ifdef ASG_ADD_GENS_SYSTEM
    if (m_bStrife)
        ::RenderBitmap(BITMAP_INTERFACE_EX + 47, UIMN_IMG_POS_X, UIMN_STRIFE_POS_Y,
            UIMN_IMG_WIDTH, UIMN_STRIFE_HEIGHT, 0.0f, 0.0f, UIMN_IMG_WIDTH / 256.0f, UIMN_STRIFE_HEIGHT / 32.0f, false, false);
#endif	// ASG_ADD_GENS_SYSTEM
    ::RenderBitmap(BITMAP_INTERFACE_EX + 45, UIMN_IMG_POS_X, UIMN_IMG_POS_Y,
        UIMN_IMG_WIDTH, UIMN_IMG_HEIGHT, 0.0f, 0.0f, UIMN_IMG_WIDTH / 256.0f, UIMN_IMG_HEIGHT / 128.0f, false, false);

    ::glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    ::DisableAlphaBlend();
}