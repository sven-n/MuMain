//*****************************************************************************
// File: UIMapName.cpp
//
// Desc: implementation of the CUIMapName class.
//
// producer: Ahn Sang-Kyu (2005. 6. 12)
//*****************************************************************************

#include "stdafx.h"
#include "UIMapName.h"

#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"

/*#include "UIControls.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
*/
#include "UIWindows.h"	// 윗 3개 파일 include 또는 "UIWindows.h" include.
#ifdef ASG_ADD_GENS_SYSTEM
#include "ZzzInventory.h"
#endif	// ASG_ADD_GENS_SYSTEM

extern float g_fScreenRate_x;
extern float g_fScreenRate_y;

//#define	ARGB(a,r,g,b)	(((DWORD)(a))<<24 | (DWORD)(r) | ((DWORD)(g))<<8 | ((DWORD)(b))<<16)

#define	UIMN_SHOW_TIME			5000	// 보여 주는 시간.
#define	UIMN_ALPHA_VARIATION	0.015f	// 알파 증감값.

#define	UIMN_IMG_WIDTH			166.0f	// 이미지 너비.
#define	UIMN_IMG_HEIGHT			90.0f	// 이미지 높이.

#ifdef ASG_ADD_GENS_SYSTEM
#define UIMN_STRIFE_HEIGHT		28.0f	// 분쟁지역 이미지 높이.
#endif	// ASG_ADD_GENS_SYSTEM

// 이미지 X 위치. 3D화면 너비에 따라 중앙 정렬.
#define	UIMN_IMG_POS_X		((::GetScreenWidth() * g_fScreenRate_x - UIMN_IMG_WIDTH) / 2.0f)
#define	UIMN_IMG_POS_Y		(220.0f * g_fScreenRate_y)		// 이미지 Y 위치.

#ifdef ASG_ADD_GENS_SYSTEM
#define UIMN_STRIFE_POS_Y	(UIMN_IMG_POS_Y - UIMN_STRIFE_HEIGHT)	// 분쟁지역 이미지 Y 위치.
#endif	// ASG_ADD_GENS_SYSTEM

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUIMapName::CUIMapName()
{
	InitImgPathMap();
}

CUIMapName::~CUIMapName()
{

}

//*****************************************************************************
// 함수 이름 : InitImgPathMap()
// 함수 설명 : 이미지 경로 맵 초기화.
//*****************************************************************************
void CUIMapName::InitImgPathMap()
{
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	string strFolderName = "Local\\" + g_strSelectedML + "\\ImgsMapName\\";

	m_mapImgPath[0] = strFolderName + "lorencia.tga";
	m_mapImgPath[1] = strFolderName + "dungeun.tga";
	m_mapImgPath[2] = strFolderName + "devias.tga";
	m_mapImgPath[3] = strFolderName + "noria.tga";
	m_mapImgPath[4] = strFolderName + "losttower.tga";

	m_mapImgPath[6] = strFolderName + "stadium.tga";
	m_mapImgPath[7] = strFolderName + "atlans.tga";
	m_mapImgPath[8] = strFolderName + "tarcan.tga";
	m_mapImgPath[9] = strFolderName + "devilsquare.tga";
	m_mapImgPath[10] = strFolderName + "Icarus.tga";
	m_mapImgPath[11] = strFolderName + "bloodcastle.tga";
	m_mapImgPath[12] = strFolderName + "bloodcastle.tga";
	m_mapImgPath[13] = strFolderName + "bloodcastle.tga";
	m_mapImgPath[14] = strFolderName + "bloodcastle.tga";
	m_mapImgPath[15] = strFolderName + "bloodcastle.tga";
	m_mapImgPath[16] = strFolderName + "bloodcastle.tga";
	m_mapImgPath[17] = strFolderName + "bloodcastle.tga";
	m_mapImgPath[18] = strFolderName + "chaoscastle.tga";
	m_mapImgPath[19] = strFolderName + "chaoscastle.tga";
	m_mapImgPath[20] = strFolderName + "chaoscastle.tga";
	m_mapImgPath[21] = strFolderName + "chaoscastle.tga";
	m_mapImgPath[22] = strFolderName + "chaoscastle.tga";
	m_mapImgPath[23] = strFolderName + "chaoscastle.tga";
	m_mapImgPath[24] = strFolderName + "Kalima.tga";
	m_mapImgPath[25] = strFolderName + "Kalima.tga";
	m_mapImgPath[26] = strFolderName + "Kalima.tga";
	m_mapImgPath[27] = strFolderName + "Kalima.tga";
	m_mapImgPath[28] = strFolderName + "Kalima.tga";
	m_mapImgPath[29] = strFolderName + "Kalima.tga";
	m_mapImgPath[30] = strFolderName + "loren.tga";
	m_mapImgPath[31] = strFolderName + "ordeal.tga";

	m_mapImgPath[33] = strFolderName + "aida.tga";
	m_mapImgPath[34] = strFolderName + "crywolffortress.tga";

	m_mapImgPath[36] = strFolderName + "lostkalima.tga";
	m_mapImgPath[37] = strFolderName + "kantru.tga";
	m_mapImgPath[38] = strFolderName + "kantru.tga";
	m_mapImgPath[39] = strFolderName + "kantru.tga";

	m_mapImgPath[41] = strFolderName + "BalgasBarrack.tga";
	m_mapImgPath[42] = strFolderName + "BalgasRefuge.tga";

	m_mapImgPath[45] = strFolderName + "IllusionTemple.tga";
	m_mapImgPath[46] = strFolderName + "IllusionTemple.tga";
	m_mapImgPath[47] = strFolderName + "IllusionTemple.tga";
	m_mapImgPath[48] = strFolderName + "IllusionTemple.tga";
	m_mapImgPath[49] = strFolderName + "IllusionTemple.tga";
	m_mapImgPath[50] = strFolderName + "IllusionTemple.tga";

	m_mapImgPath[51] = strFolderName + "Elbeland.tga";
	m_mapImgPath[52] = strFolderName + "bloodcastle.tga";
	m_mapImgPath[53] = strFolderName + "chaoscastle.tga";

	m_mapImgPath[56] = strFolderName + "SwampOfCalmness.tga";
#ifdef CSK_ADD_MAP_ICECITY
	m_mapImgPath[57] = strFolderName + "mapname_raklion.tga";
	m_mapImgPath[58] = strFolderName + "mapname_raklionboss.tga";
#endif // CSK_ADD_MAP_ICECITY
	
#ifdef YDG_ADD_MAP_SANTA_TOWN
	m_mapImgPath[62] = strFolderName + "santatown.tga";
#endif	// YDG_ADD_MAP_SANTA_TOWN
#ifdef PBG_ADD_PKFIELD
	m_mapImgPath[63] = strFolderName + "pkfield.tga";
#endif //PBG_ADD_PKFIELD
#ifdef YDG_ADD_MAP_DUEL_ARENA
	m_mapImgPath[64] = strFolderName + "duelarena.tga";
#endif	// YDG_ADD_MAP_DUEL_ARENA
#ifdef YDG_ADD_MAP_DOPPELGANGER1
	m_mapImgPath[65] = strFolderName + "doppelganger.tga";
#endif	// YDG_ADD_MAP_DOPPELGANGER1
#ifdef YDG_ADD_MAP_DOPPELGANGER2
	m_mapImgPath[66] = strFolderName + "doppelganger.tga";
#endif	// YDG_ADD_MAP_DOPPELGANGER2
#ifdef YDG_ADD_MAP_DOPPELGANGER3
	m_mapImgPath[67] = strFolderName + "doppelganger.tga";
#endif	// YDG_ADD_MAP_DOPPELGANGER3
#ifdef YDG_ADD_MAP_DOPPELGANGER4
	m_mapImgPath[68] = strFolderName + "doppelganger.tga";
#endif	// YDG_ADD_MAP_DOPPELGANGER4
#ifdef LDK_ADD_MAP_EMPIREGUARDIAN1
	m_mapImgPath[69] = strFolderName + "EmpireGuardian.tga";
#endif //LDK_ADD_MAP_EMPIREGUARDIAN1
#ifdef LDS_ADD_MAP_EMPIREGUARDIAN2
	m_mapImgPath[70] = strFolderName + "EmpireGuardian.tga";
#endif //LDS_ADD_MAP_EMPIREGUARDIAN2
#ifdef LDK_ADD_MAP_EMPIREGUARDIAN3
	m_mapImgPath[71] = strFolderName + "EmpireGuardian.tga";
#endif //LDK_ADD_MAP_EMPIREGUARDIAN3
#ifdef LDS_ADD_MAP_EMPIREGUARDIAN4
	m_mapImgPath[72] = strFolderName + "EmpireGuardian.tga";
#endif //LDS_ADD_MAP_EMPIREGUARDIAN4
#ifdef LDS_ADD_MAP_UNITEDMARKETPLACE
	m_mapImgPath[79] = strFolderName + "MapName_MarketRolen.tga";
#endif // LDS_ADD_MAP_UNITEDMARKETPLACE
#ifdef ASG_ADD_MAP_KARUTAN
	m_mapImgPath[80] = strFolderName + "MapName_Karutan.tga";
	m_mapImgPath[81] = strFolderName + "MapName_Karutan.tga";
#endif	// ASG_ADD_MAP_KARUTAN
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	 // 파일명 대입.ㅜㅜ
	m_mapImgPath[0] = "Interface\\lorencia.tga";
	m_mapImgPath[1] = "Interface\\dungeun.tga";
	m_mapImgPath[2] = "Interface\\devias.tga";
	m_mapImgPath[3] = "Interface\\noria.tga";
	m_mapImgPath[4] = "Interface\\losttower.tga";

	m_mapImgPath[6] = "Interface\\stadium.tga";
	m_mapImgPath[7] = "Interface\\atlans.tga";
	m_mapImgPath[8] = "Interface\\tarcan.tga";
	m_mapImgPath[9] = "Interface\\devilsquare.tga";
	m_mapImgPath[10] = "Interface\\Icarus.tga";
	m_mapImgPath[11] = "Interface\\bloodcastle.tga";
	m_mapImgPath[12] = "Interface\\bloodcastle.tga";
	m_mapImgPath[13] = "Interface\\bloodcastle.tga";
	m_mapImgPath[14] = "Interface\\bloodcastle.tga";
	m_mapImgPath[15] = "Interface\\bloodcastle.tga";
	m_mapImgPath[16] = "Interface\\bloodcastle.tga";
	m_mapImgPath[17] = "Interface\\bloodcastle.tga";
	m_mapImgPath[18] = "Interface\\chaoscastle.tga";
	m_mapImgPath[19] = "Interface\\chaoscastle.tga";
	m_mapImgPath[20] = "Interface\\chaoscastle.tga";
	m_mapImgPath[21] = "Interface\\chaoscastle.tga";
	m_mapImgPath[22] = "Interface\\chaoscastle.tga";
	m_mapImgPath[23] = "Interface\\chaoscastle.tga";
	m_mapImgPath[24] = "Interface\\Kalima.tga";
	m_mapImgPath[25] = "Interface\\Kalima.tga";
	m_mapImgPath[26] = "Interface\\Kalima.tga";
	m_mapImgPath[27] = "Interface\\Kalima.tga";
	m_mapImgPath[28] = "Interface\\Kalima.tga";
	m_mapImgPath[29] = "Interface\\Kalima.tga";
	m_mapImgPath[30] = "Interface\\loren.tga";
	m_mapImgPath[31] = "Interface\\ordeal.tga";				//  시련의 땅

	m_mapImgPath[33] = "Interface\\aida.tga";
	m_mapImgPath[34] = "Interface\\crywolffortress.tga";

	m_mapImgPath[36] = "Interface\\lostkalima.tga";
	m_mapImgPath[37] = "Interface\\kantru.tga";
	m_mapImgPath[38] = "Interface\\kantru.tga";
	m_mapImgPath[39] = "Interface\\kantru.tga";

	m_mapImgPath[41] = "Interface\\BalgasBarrack.tga";
	m_mapImgPath[42] = "Interface\\BalgasRefuge.tga";

	m_mapImgPath[45] = "Interface\\IllusionTemple.tga";
	m_mapImgPath[46] = "Interface\\IllusionTemple.tga";
	m_mapImgPath[47] = "Interface\\IllusionTemple.tga";
	m_mapImgPath[48] = "Interface\\IllusionTemple.tga";
	m_mapImgPath[49] = "Interface\\IllusionTemple.tga";
	m_mapImgPath[50] = "Interface\\IllusionTemple.tga";

	m_mapImgPath[51] = "Interface\\Elbeland.tga";
	m_mapImgPath[52] = "Interface\\bloodcastle.tga";
	m_mapImgPath[53] = "Interface\\chaoscastle.tga";

	m_mapImgPath[56] = "Interface\\SwampOfCalmness.tga";
#ifdef CSK_ADD_MAP_ICECITY
	m_mapImgPath[57] = "Interface\\mapname_raklion.tga";
	m_mapImgPath[58] = "Interface\\mapname_raklionboss.tga";
#endif // CSK_ADD_MAP_ICECITY
	
#ifdef YDG_ADD_MAP_SANTA_TOWN
	m_mapImgPath[62] = "Interface\\santatown.tga";
#endif	// YDG_ADD_MAP_SANTA_TOWN
#ifdef PBG_ADD_PKFIELD
	m_mapImgPath[63] = "Interface\\pkfield.tga";
#endif //PBG_ADD_PKFIELD
#ifdef YDG_ADD_MAP_DUEL_ARENA
	m_mapImgPath[64] = "Interface\\duelarena.tga";
#endif	// YDG_ADD_MAP_DUEL_ARENA
#ifdef YDG_ADD_MAP_DOPPELGANGER1
	m_mapImgPath[65] = "Interface\\doppelganger.tga";
#endif	// YDG_ADD_MAP_DOPPELGANGER1
#ifdef YDG_ADD_MAP_DOPPELGANGER2
	m_mapImgPath[66] = "Interface\\doppelganger.tga";
#endif	// YDG_ADD_MAP_DOPPELGANGER2
#ifdef YDG_ADD_MAP_DOPPELGANGER3
	m_mapImgPath[67] = "Interface\\doppelganger.tga";
#endif	// YDG_ADD_MAP_DOPPELGANGER3
#ifdef YDG_ADD_MAP_DOPPELGANGER4
	m_mapImgPath[68] = "Interface\\doppelganger.tga";
#endif	// YDG_ADD_MAP_DOPPELGANGER4
#ifdef LDK_ADD_MAP_EMPIREGUARDIAN1
	m_mapImgPath[69] = "Interface\\EmpireGuardian.tga";
#endif //LDK_ADD_MAP_EMPIREGUARDIAN1
#ifdef LDS_ADD_MAP_EMPIREGUARDIAN2
	m_mapImgPath[70] = "Interface\\EmpireGuardian.tga";
#endif //LDS_ADD_MAP_EMPIREGUARDIAN2
#ifdef LDK_ADD_MAP_EMPIREGUARDIAN3
	m_mapImgPath[71] = "Interface\\EmpireGuardian.tga";
#endif //LDK_ADD_MAP_EMPIREGUARDIAN3
#ifdef LDS_ADD_MAP_EMPIREGUARDIAN4
	m_mapImgPath[72] = "Interface\\EmpireGuardian.tga";
#endif //LDS_ADD_MAP_EMPIREGUARDIAN4
#ifdef LDS_ADD_MAP_UNITEDMARKETPLACE
	m_mapImgPath[79] = "Interface\\MapName_MarketRolen.tga";
#endif // LDS_ADD_MAP_UNITEDMARKETPLACE
#ifdef ASG_ADD_MAP_KARUTAN
	m_mapImgPath[80] = "Interface\\MapName_Karutan.tga";
	m_mapImgPath[81] = "Interface\\MapName_Karutan.tga";
#endif	// ASG_ADD_MAP_KARUTAN
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE

}

//*****************************************************************************
// 함수 이름 : Init()
// 함수 설명 : 멤버변수 초기화. Main Scene 시작시 초기화 해주어야됨.
//*****************************************************************************
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

//*****************************************************************************
// 함수 이름 : ShowMapName()
// 함수 설명 : 맵 이미지를 보여준다.
//			  (처음 시작할 때, 죽었다가 다시 부활할 때, 텔레포트(일반 이동 포함
//			  할 때 이 함수 호출)
//*****************************************************************************
void CUIMapName::ShowMapName()
{
	m_eState = FADEIN;
	m_fAlpha = 0.2f;
	m_dwDeltaTickSum = 0;

	//. GM맵은 맵이름을 출력하지 않는다. 
	if(World == WD_40AREA_FOR_GM)
	{
		m_eState = HIDE;
		return;
	}

	if (m_nOldWorld != World)
	{
	// 맵 이름 이미지 교체.
		char szImgPath[128];
		::strcpy(szImgPath, m_mapImgPath[World].data());

#ifdef LDS_ADD_EMPIRE_GUARDIAN
		DeleteBitmap(BITMAP_INTERFACE_EX+45);
#endif	// LDS_ADD_EMPIRE_GUARDIAN
		LoadBitmap(szImgPath, BITMAP_INTERFACE_EX+45);
		
		m_nOldWorld = World;

#ifdef ASG_ADD_GENS_SYSTEM
		m_bStrife = ::IsStrifeMap(World);
#endif	// ASG_ADD_GENS_SYSTEM
	}
}

//*****************************************************************************
// 함수 이름 : Update()
// 함수 설명 : 업데이트. 매 프레임 마다 Render()전에 호출.
//*****************************************************************************
void CUIMapName::Update()
{
	// 이전 프레임과의 시간차 구함.
	DWORD dwNowTime = ::timeGetTime();
	DWORD dwDeltaTick = dwNowTime - m_dwOldTime;

	switch (m_eState)
	{
	case FADEIN:	// 서서히 나타나는 경우.
		m_fAlpha += UIMN_ALPHA_VARIATION;
		if (1.0f <= m_fAlpha)
		{
			m_eState = SHOW;
			m_fAlpha = 1.0f;
		}
		break;

	case SHOW:		// 완전히 보이는 경우.
		m_dwDeltaTickSum += dwDeltaTick;
		if (m_dwDeltaTickSum > UIMN_SHOW_TIME)
		{
			m_eState = FADEOUT;
			m_dwDeltaTickSum = 0;
		}
		break;

	case FADEOUT:	// 서서히 사라지는 경우.
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

//*****************************************************************************
// 함수 이름 : Render()
// 함수 설명 : 렌더.
//*****************************************************************************
void CUIMapName::Render()
{
	Update();

	if (HIDE == m_eState)
		return;

	::EnableAlphaTest();						// 칼라키 투명.
	::glColor4f(1.0f, 1.0f, 1.0f, m_fAlpha);	// 알파값 조정.

	// 이미지 렌더. 위치와 영역은 실제 픽셀 값.
#ifdef ASG_ADD_GENS_SYSTEM
	if (m_bStrife)
		::RenderBitmap(BITMAP_INTERFACE_EX+47, UIMN_IMG_POS_X, UIMN_STRIFE_POS_Y,
			UIMN_IMG_WIDTH, UIMN_STRIFE_HEIGHT, 0.0f, 0.0f,
			UIMN_IMG_WIDTH / 256.0f, UIMN_STRIFE_HEIGHT / 32.0f, false, false);
#endif	// ASG_ADD_GENS_SYSTEM
	::RenderBitmap(BITMAP_INTERFACE_EX+45, UIMN_IMG_POS_X, UIMN_IMG_POS_Y,
		UIMN_IMG_WIDTH, UIMN_IMG_HEIGHT, 0.0f, 0.0f,
		UIMN_IMG_WIDTH / 256.0f, UIMN_IMG_HEIGHT / 128.0f, false, false);

	::glColor4f(1.0f, 1.0f, 1.0f, 1.0f);	// 이후 렌더에 영향을 안주기 위해.
	::DisableAlphaBlend();
}