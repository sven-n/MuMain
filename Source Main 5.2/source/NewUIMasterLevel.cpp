// NewUIGuildInfoWindow.cpp: implementation of the CNewUIGuildInfoWindow class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIMasterLevel.h"
#include "NewUISystem.h"
#include "NewUICommonMessageBox.h"
#include "NewUICustomMessageBox.h"
#include "DSPlaySound.h"
#include "wsclientinline.h"
#include "NewUIGuildInfoWindow.h"
#include "NewUIButton.h"
#include "NewUIMyInventory.h"
#include "CSitemOption.h"
#include "CharacterManager.h"
#include "SkillManager.h"

MASTER_LEVEL_DATA		m_MasterLevel[MAX_MASTER];
BYTE					Master_Skill_Data[9][5];
int						JobPoint[4] = {0,0,0,0};
char Need_Point = 0;
int	 In_Skill = 0;
using namespace SEASON3B;
extern char TextList[50][100];
extern int  TextListColor[50];
extern int  TextBold[50];
extern SIZE Size[50];

SEASON3B::CNewUIMasterLevel::CNewUIMasterLevel()
{
	m_pNewUIMng = NULL;
}

SEASON3B::CNewUIMasterLevel::~CNewUIMasterLevel() 
{ 
	Release(); 
}

bool SEASON3B::CNewUIMasterLevel::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if(NULL == pNewUIMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_MASTER_LEVEL, this);

	SetPos(x, y);

	LoadImages();


	m_BtnExit.ChangeButtonImgState( true, IMAGE_MASTER_EXIT, false );
	m_BtnExit.ChangeButtonInfo( m_Pos.x+591, m_Pos.y+390, 36, 29 );		
	m_BtnExit.ChangeToolTipText( GlobalText[1002], true );

	for(int i = 0; i < 4; i++)
	{
		m_EventState[i] = EVENT_NONE;
		m_Loc[i] = 0;
	}
	if( WindowWidth != 800 && WindowHeight != 600 )
	{
		int panelspaceX = 0;
		int panelspaceY = 7;
		int panelspace = 219+panelspaceY;

		if( WindowWidth == 1024 && WindowHeight == 768 )
		{
			panelspaceX = -127;
			panelspaceY = -120;
			panelspace = 219+panelspaceY;
		}
	}
	return true;
}

void SEASON3B::CNewUIMasterLevel::ClosingProcess()
{
	SendExitInventory();
}

float SEASON3B::CNewUIMasterLevel::GetLayerDepth()
{
	return 8.1f;
}

void SEASON3B::CNewUIMasterLevel::OpenningProcess()
{

}

void SEASON3B::CNewUIMasterLevel::Release()
{
	UnloadImages();

//	SAFE_DELETE_ARRAY(m_Button);

	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}
}

void SEASON3B::CNewUIMasterLevel::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
	m_BtnExit.ChangeButtonInfo( m_Pos.x+513, m_Pos.y+392, 36, 29 );
}

bool SEASON3B::CNewUIMasterLevel::UpdateKeyEvent()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MASTER_LEVEL))
	{
		if(SEASON3B::IsPress(VK_ESCAPE) == true || SEASON3B::IsPress('A') == true)
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_MASTER_LEVEL);
			PlayBuffer(SOUND_CLICK01);
			return false;
		}
	}

	return true;
}

bool SEASON3B::CNewUIMasterLevel::Render()
{
	//WindowWidth
	//WindowHeight

	EnableAlphaTest();
	glColor4f(1.f, 1.f, 1.f, 1.f);

	//RenderImage(IMAGE_MASTER_INTERFACE + 14, m_Pos.x, m_Pos.y, 800, 536);
	//RenderImage(IMAGE_MASTER_INTERFACE + 14, m_Pos.x, m_Pos.y, 640 * (640/WindowWidth), 480 * (,0.f,0.f,1.f,1.f);
//	float nx = float(640.f * (640.f/(float)WindowWidth));
//	float ny = float(429.f * (480.f/(float)WindowHeight));

	m_Width.x = float(800.f * (800.f/(float)1024));
	m_Width.y = float(540.f * (600.f/(float)768));
	m_Pos.x = (640 - m_Width.x)/2;
	m_Pos.y = (429 - m_Width.y)/2;
	RenderImage(IMAGE_MASTER_INTERFACE, m_Pos.x, m_Pos.y, m_Width.x,m_Width.y ,0.f,0.f,1.f,1.f);
	

	Render_Icon();
//	Render_Scroll();
	//RenderImage( IMAGE_SKILL_ICON, m_SkillIconPos.x+1, m_SkillIconPos.y, (float)SKILL_ICON_WIDTH, (float)SKILL_ICON_HEIGHT, src_x, src_y);
	m_BtnExit.Render();

	DisableAlphaBlend();

	Render_Text();
	return true;
}

void SEASON3B::CNewUIMasterLevel::Render_Scroll()
{
	int Scroll_Loc[4][2] = {{185,65},{308,65},{432,65},{590,65}};

	for(int i = 0; i < 4; i++)
	{
		if (m_EventState[i] == EVENT_SCROLL_BTN_DOWN) 
			glColor4f(0.7f, 0.7f, 0.7f, 1.0f);
		else 
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

		RenderImage(IMAGE_MASTER_SCROLLBAR_ON, m_Pos.x+Scroll_Loc[i][0], Scroll_Loc[i][1] + m_Loc[i], 15, 30);
	}
}

void SEASON3B::CNewUIMasterLevel::Render_Icon()
{
	int Skill_Num = -1;
	float nx = 43.f;
	float ny = 42.f;
	BYTE Job = (Hero->Class >> 4) & 0x01;
	int GetJob = -1;

	if(Job == 1)
		Job = gCharacterManager.GetCharacterClass(Hero->Class);
	switch(Job)
	{
	case CLASS_WIZARD:
	case CLASS_SOULMASTER:
	case CLASS_GRANDMASTER:
		GetJob = 0;
		break;
	case CLASS_KNIGHT:
	case CLASS_BLADEKNIGHT:
	case CLASS_BLADEMASTER:
		GetJob = 4;
		break;
	case CLASS_ELF:
	case CLASS_MUSEELF:
	case CLASS_HIGHELF:
		GetJob = 8;
		break;
	case CLASS_DARK:
	case CLASS_DUELMASTER:
		GetJob = 12;
		break;
	case CLASS_DARK_LORD:
	case CLASS_LORDEMPEROR:
		GetJob = 16;
		break;
	case CLASS_SUMMONER:
	case CLASS_BLOODYSUMMONER:
	case CLASS_DIMENSIONMASTER:
		GetJob = 20;
		break;
	}

	int Loc[4] = {37,237,362,488};
	int Dir = -1;

	int Wid = SKILL_ICON_DATA_HEIGHT - 2;

	for(int i = 0; i < 30; i++)
	{
		if(i == 0)
			TextListColor[i] = TEXT_COLOR_GREEN;
		else
			TextListColor[i] = TEXT_COLOR_WHITE;
	}

int Skill_Icon;
int Point_R = 0,Point_Tot = 0;

	for(int Skill = 0; Skill < 4; Skill++)
	{
		JobPoint[Skill] = 0;
		int Over_Line = 0;
		int Count = 220/(SKILL_ICON_DATA_HEIGHT - Wid);
		int StartY = (m_Loc[Skill]/Count);
		for(int sy = StartY; sy < (Wid + StartY); sy++)
		{
			int nsy = sy-StartY;
			bool View_Kind = false;

			if(sy>0)
			{
				Point_R = 0;
				for(int sax = 0; sax < 5; sax++)
					Point_R += Master_Skill_Data[sy - 1][sax];
				if(Point_R > Over_Line)
					View_Kind = true;

				Over_Line = 0;
				Point_Tot = 0;
			}
			for(int sx = 0; sx < m_MasterLevel[GetJob + Skill].Width; sx++)
			{
				Dir = -1;
				Skill_Num = m_MasterLevel[GetJob + Skill].Ability[sy][sx];
				if(Skill_Num == -1)
					continue;
				else
				if(Skill_Num >= 10000)
				{
					Dir = Skill_Num/10000;
					Skill_Num = Skill_Num%10000;
				}

				Skill_Icon = 0;

				char Name[300];
				Skill_Icon = SkillAttribute[Skill_Num].Magic_Icon;
				strcpy(Name,SkillAttribute[Skill_Num].Name);

				DWORD Color;
				if(View_Kind == true || Skill_Num == 0
					|| (g_csItemOption.Special_Option_Check() == false && (AT_SKILL_POWER_SLASH_UP<= Skill_Num && AT_SKILL_POWER_SLASH_UP + 5 > Skill_Num))
					|| (g_csItemOption.Special_Option_Check(1) == false && (AT_SKILL_MANY_ARROW_UP<= Skill_Num && AT_SKILL_MANY_ARROW_UP + 5 > Skill_Num))
					)
					Color = RGBA(120,120,120,255);
				else
					Color = RGBA(255,255,255,255);

				RenderImage(IMAGE_MASTER_INTERFACE + 1, Loc[Skill] + ((SKILL_ICON_WIDTH + 19) * sx), 70 + ((SKILL_ICON_HEIGHT + 14) * nsy), 35,34 ,0.f,0.f,nx/64.f,ny/64.f,Color);
				RenderImage(IMAGE_MASTER_SKILL_ICON, Loc[Skill] + 5 + ((SKILL_ICON_WIDTH + 19) * sx), 75 + ((SKILL_ICON_HEIGHT + 14) * nsy), (float)15, (float)21 ,(20.f/256.f) * (Skill_Icon%12),((28.f/256.f)*((Skill_Icon/12)+4)),20.f/256.f,27.5f/256.f,Color);
				if(Skill_Num > 0)
				{
					int Cur_Skill = Skill_Num;
					int Get_Magic = 0;

					for(int i=0; i<MAX_MAGIC; ++i)
					{
						if((CharacterAttribute->Skill[i] >= Cur_Skill && CharacterAttribute->Skill[i] < (Cur_Skill + 5)))
						{
							Get_Magic = (CharacterAttribute->Skill[i] - Cur_Skill) + 1;
							break;
						}
					}
					unicode::t_char lpszStr[256] = {NULL, };

					if(View_Kind == true)
						g_pRenderText->SetTextColor(120, 120, 0, 255);
					else
						g_pRenderText->SetTextColor(255, 255, 0, 255);

					unicode::_sprintf( lpszStr, "%d", Get_Magic);
					g_pRenderText->RenderText(Loc[Skill] + 31 + ((SKILL_ICON_WIDTH + 19) * sx), 95 + ((SKILL_ICON_HEIGHT + 14) * nsy), lpszStr, 0, 0, RT3_WRITE_CENTER);

					Point_Tot += Get_Magic;
					for(int iu = 0; iu < Get_Magic; iu++)
					{
						JobPoint[Skill] += (Master_Skill_Data[sy][iu]);
						Over_Line += Master_Skill_Data[sy][iu];
					}
				}

				if(Dir == 1)
					RenderImage(IMAGE_MASTER_INTERFACE + 2, Loc[Skill] + ((SKILL_ICON_WIDTH + 19) * sx)+9, 70 + ((SKILL_ICON_HEIGHT + 14) * nsy) - 12, 8,12 ,0.f,18.f/32.f,14.8f/16.f,-(18.f/32.f));
				if(Dir == 2)
					RenderImage(IMAGE_MASTER_INTERFACE + 3, Loc[Skill] + ((SKILL_ICON_WIDTH + 19) * sx)+27, 70 + ((SKILL_ICON_HEIGHT + 14) * nsy)+11, 12,8 ,0.f,0.f,18.f/32.f,14.8f/16.f);
				if(Dir == 3)
					RenderImage(IMAGE_MASTER_INTERFACE + 2, Loc[Skill] + ((SKILL_ICON_WIDTH + 19) * sx)+9, 70 + ((SKILL_ICON_HEIGHT + 14) * nsy) + 31, 8,12 ,0.f,0.f,14.8f/16.f,18.f/32.f);
				if(Dir == 4)
					RenderImage(IMAGE_MASTER_INTERFACE + 3, Loc[Skill] + ((SKILL_ICON_WIDTH + 19) * sx)-12, 70 + ((SKILL_ICON_HEIGHT + 14) * nsy)+11, 12,8 ,18.f/32.f,0.f,-(18.f/32.f),14.8f/16.f);


				//IMAGE_MASTER_INTERFACE + 16
			}
		}
	}
	Render_Scroll();

	for(int Skill = 0; Skill < 4; Skill++)
	{
		int Count = 220/(SKILL_ICON_DATA_HEIGHT - Wid);
		int StartY = (m_Loc[Skill]/Count);
		for(int sy = StartY; sy < (Wid + StartY); sy++)
		{
			int nsy = sy-StartY;
			for(int sx = 0; sx < m_MasterLevel[GetJob + Skill].Width; sx++)
			{
				Dir = -1;
				Skill_Num = m_MasterLevel[GetJob + Skill].Ability[sy][sx];
				if(Skill_Num == -1)
					continue;
				else
				if(Skill_Num >= 10000)
				{
					Dir = Skill_Num/10000;
					Skill_Num = Skill_Num%10000;
				}
				char Name[300];
				Skill_Icon = SkillAttribute[Skill_Num].Magic_Icon;
				strcpy(Name,SkillAttribute[Skill_Num].Name);

				int Cur_Skill = Skill_Num;
				int Get_Magic = 0;

				for(int i=0; i<MAX_MAGIC; ++i)
				{
					if(CharacterAttribute->Skill[i] >= Cur_Skill && CharacterAttribute->Skill[i] < (Cur_Skill + 5))
					{
						Get_Magic = (CharacterAttribute->Skill[i] - Cur_Skill) + 1;
						break;
					}
				}
				if(Get_Magic <= 0)
					Get_Magic = 0;
				int Point = 0;

//				if(Point <= 0)
//					Point = Master_Skill_Data[sy][sx];

				if(!(SEASON3B::IsPress(VK_LBUTTON)))
				{
					int Data_Buff[] = { 
						AT_SKILL_DURABLE_MINUS1
						,AT_SKILL_DURABLE_MINUS2
						,AT_SKILL_RESIST_UP_POISON
						,AT_SKILL_RESIST_UP_LIGHT
						,AT_SKILL_RESIST_UP_ICE
						,AT_SKILL_AUTO_RECOVER_LIFE
						,AT_SKILL_GET_MONEY_UP
						,AT_SKILL_EXP_UP
						,AT_SKILL_SD_RECOVER_SPD_UP
					};
					int Tot_Buff = 9;

					if(CheckMouseIn(Loc[Skill] + 5 + ((SKILL_ICON_WIDTH + 19) * sx), 75 + ((SKILL_ICON_HEIGHT + 14) * nsy), 15, 21))
					{
						bool Check_View = false;
						for(int CV = 0; CV < Tot_Buff; CV++)
						{
							if(Data_Buff[CV] == Skill_Num)
							{
								Check_View = true;
								break;
							}
						}

						memset( TextBold, 0, sizeof(int) * 30 );

						for(int iu = 0; iu < Get_Magic; iu++)
						Point += Master_Skill_Data[sy][iu];

						for(int ib = 0; ib < 30; ib++)
							TextList[ib][0] = NULL;

						sprintf(TextList[0],"%s",Name);
						sprintf(TextList[1],GlobalText[1773],Point);
						

						int Dan = Get_Magic;
						bool Kind = false;

						for(int ia = 0; ia < Tot_Buff; ia++)
						{
							if(Cur_Skill == Data_Buff[ia])
							{
								Kind = true;
								break;
							}
						}

						int StartText = 2;
						int ib = 0;
						int Set_Master = 0,Num_Master = 0;
						int Add_Tex = 0;

						if(Get_Magic > 0)
						{
							sprintf(TextList[StartText],"         ");
							sprintf(TextList[StartText+1],GlobalText[1774],Dan);
							if(Set_Master <= 0)
							{
								Num_Master = Get_Magic -1;
								if(Num_Master < 0)
									Num_Master = 0;
								Set_Master++;
							}
							else
							{
								Num_Master = Get_Magic;
								if(Num_Master < 0)
									Num_Master = 0;
							}

							sprintf(TextList[StartText+2],GlobalText[1775],Master_Skill_Data[sy][Num_Master]);
							if(Kind == true)
							{
								if(Check_View == true)
									sprintf(TextList[StartText+Add_Tex+3],GlobalText[1776],SkillAttribute[Skill_Num + Num_Master].Damage);
								else
								{
									sprintf(TextList[StartText+Add_Tex+3],GlobalText[1780 + ((Skill_Num - AT_SKILL_MANA_RECOVER)/5)],SkillAttribute[Skill_Num + Num_Master].Damage);
								}
							}
							else
							{
								if(AT_SKILL_TORNADO_SWORDA_UP == Skill_Num || AT_SKILL_TORNADO_SWORDB_UP == Skill_Num || AT_SKILL_HELL_FIRE_UP == Skill_Num
									|| AT_SKILL_MANY_ARROW_UP == Skill_Num || AT_SKILL_BLAST_UP == Skill_Num || AT_SKILL_FIRE_BUST_UP == Skill_Num
									|| AT_SKILL_EVIL_SPIRIT_UP == Skill_Num
									|| AT_SKILL_EVIL_SPIRIT_UP_M == Skill_Num
									|| AT_SKILL_ICE_UP == Skill_Num
									)
								{
									if(SkillAttribute[Skill_Num + Num_Master].Distance > 0)
									{
										sprintf(TextList[StartText+Add_Tex+3],GlobalText[174],SkillAttribute[Skill_Num + Num_Master].Distance);
										Add_Tex++;
									}
									sprintf(TextList[StartText+Add_Tex+3],GlobalText[175],SkillAttribute[Skill_Num + Num_Master].Mana);
								}
								else
								if(AT_SKILL_HP_RECOVER == Skill_Num)
								{
									sprintf(TextList[StartText+Add_Tex+3],GlobalText[1781],SkillAttribute[Skill_Num + Num_Master].Damage);
								}
								else
								if(AT_SKILL_SD_RECOVER == Skill_Num)
								{
									sprintf(TextList[StartText+Add_Tex+3],GlobalText[1782],SkillAttribute[Skill_Num + Num_Master].Damage);
								}
								else
								if(AT_SKILL_MANA_RECOVER == Skill_Num)
								{
									sprintf(TextList[StartText+Add_Tex+3],GlobalText[1780],SkillAttribute[Skill_Num + Num_Master].Damage);
								}
								else
								if( AT_SKILL_ALICE_CHAINLIGHTNING_UP == Skill_Num
									||	AT_SKILL_ALICE_DRAINLIFE_UP == Skill_Num)
								{
									sprintf(TextList[StartText+Add_Tex+3],GlobalText[175],SkillAttribute[Skill_Num + Num_Master].Mana);
								}
								else
								if(AT_SKILL_ASHAKE_UP == Skill_Num)
								{
									if(SkillAttribute[Skill_Num + Num_Master].Distance > 0)
									{
										sprintf(TextList[StartText+Add_Tex+3],GlobalText[174],SkillAttribute[Skill_Num + Num_Master].Distance);
										Add_Tex++;
									}
									sprintf(TextList[StartText+Add_Tex+3],GlobalText[360],SkillAttribute[Skill_Num + Num_Master].AbilityGuage);
								}
								else
								if(AT_SKILL_SOUL_UP == Skill_Num
									|| AT_SKILL_BLOW_UP == Skill_Num
									|| AT_SKILL_ANGER_SWORD_UP == Skill_Num
									|| AT_SKILL_DEF_POWER_UP == Skill_Num
									|| AT_SKILL_ATT_POWER_UP == Skill_Num
									|| AT_SKILL_BLOOD_ATT_UP == Skill_Num
									|| AT_SKILL_POWER_SLASH_UP == Skill_Num
									|| AT_SKILL_FIRE_SCREAM_UP == Skill_Num
									)
								{
									if(SkillAttribute[Skill_Num + Num_Master].Distance > 0)
									{
										sprintf(TextList[StartText+Add_Tex+3],GlobalText[174],SkillAttribute[Skill_Num + Num_Master].Distance);
										Add_Tex++;
									}
									sprintf(TextList[StartText+Add_Tex+3],GlobalText[175],SkillAttribute[Skill_Num + Num_Master].Mana);
									Add_Tex++;
									sprintf(TextList[StartText+Add_Tex+3],GlobalText[360],SkillAttribute[Skill_Num + Num_Master].AbilityGuage);
								}
								else
								if(AT_SKILL_LIFE_UP == Skill_Num || AT_SKILL_ALICE_SLEEP_UP == Skill_Num ||	AT_SKILL_LIGHTNING_SHOCK_UP == Skill_Num)
								{
									sprintf(TextList[StartText+Add_Tex+3],GlobalText[175],SkillAttribute[Skill_Num + Num_Master].Mana);
									Add_Tex++;
									sprintf(TextList[StartText+Add_Tex+3],GlobalText[360],SkillAttribute[Skill_Num + Num_Master].AbilityGuage);
								}
								else
								if(AT_SKILL_HEAL_UP == Skill_Num)
								{
									sprintf(TextList[StartText+Add_Tex+3],GlobalText[1785],SkillAttribute[Skill_Num + Num_Master].Damage);
									Add_Tex++;
									sprintf(TextList[StartText+Add_Tex+3],GlobalText[175],SkillAttribute[Skill_Num + Num_Master].Mana);
								}
								else
								if(AT_SKILL_MANA_MINUS_ADD == Skill_Num)
								{
									sprintf(TextList[StartText+Add_Tex+3],GlobalText[1788],SkillAttribute[Skill_Num + Num_Master].Damage);
								}
								else
								sprintf(TextList[StartText+Add_Tex+3],GlobalText[1777],SkillAttribute[Skill_Num + Num_Master].Damage);
							}
							StartText = 6 + Add_Tex;

							if(Get_Magic == 5)
								ib = StartText;
							else
								ib = StartText+4 + Add_Tex;
							Add_Tex = 0;
						}
						Dan++;
						if(Get_Magic != 5)
						{
							if(ib == 6 + Add_Tex)
								ib = 10 + Add_Tex;
							else
							 ib = 6 + Add_Tex;

							if(Get_Magic == 0)
								ib = 6 + Add_Tex;
							sprintf(TextList[StartText],"         ");
							sprintf(TextList[StartText+1],GlobalText[1774],Dan);
							if(Set_Master <= 0)
							{
								Num_Master = Get_Magic -1;
								if(Num_Master < 0)
									Num_Master = 0;
								Set_Master++;
							}
							else
							{
								Num_Master = Get_Magic;
								if(Num_Master < 0)
									Num_Master = 0;
							}
							sprintf(TextList[StartText+Add_Tex+2],GlobalText[1775],Master_Skill_Data[sy][Num_Master]);
							if(Kind == true)
							{
								if(Check_View == true)
								sprintf(TextList[StartText+Add_Tex+3],GlobalText[1776],SkillAttribute[Skill_Num + Num_Master].Damage);
								else
								{
									sprintf(TextList[StartText+Add_Tex+3],GlobalText[1780 + ((Skill_Num - AT_SKILL_MANA_RECOVER)/5)],SkillAttribute[Skill_Num + Num_Master].Damage);
								}
							}
							else
							{
								if(AT_SKILL_TORNADO_SWORDA_UP == Skill_Num || AT_SKILL_TORNADO_SWORDB_UP == Skill_Num || AT_SKILL_HELL_FIRE_UP == Skill_Num
									|| AT_SKILL_MANY_ARROW_UP == Skill_Num || AT_SKILL_BLAST_UP == Skill_Num || AT_SKILL_FIRE_BUST_UP == Skill_Num
									|| AT_SKILL_EVIL_SPIRIT_UP == Skill_Num
									|| AT_SKILL_EVIL_SPIRIT_UP_M == Skill_Num
									|| AT_SKILL_ICE_UP == Skill_Num
									)
								{
									if(SkillAttribute[Skill_Num + Num_Master].Distance > 0)
									{
										sprintf(TextList[StartText+Add_Tex+3],GlobalText[174],SkillAttribute[Skill_Num + Num_Master].Distance);
										Add_Tex++;
									}
									sprintf(TextList[StartText+Add_Tex+3],GlobalText[175],SkillAttribute[Skill_Num + Num_Master].Mana);
								}
								else
								if(AT_SKILL_HP_RECOVER == Skill_Num)
								{
									sprintf(TextList[StartText+Add_Tex+3],GlobalText[1781],SkillAttribute[Skill_Num + Num_Master].Damage);
								}
								else
								if(AT_SKILL_SD_RECOVER == Skill_Num)
								{
									sprintf(TextList[StartText+Add_Tex+3],GlobalText[1782],SkillAttribute[Skill_Num + Num_Master].Damage);
								}
								else
								if(AT_SKILL_MANA_RECOVER == Skill_Num)
								{
									sprintf(TextList[StartText+Add_Tex+3],GlobalText[1780],SkillAttribute[Skill_Num + Num_Master].Damage);
								}
								else
								if(AT_SKILL_ASHAKE_UP == Skill_Num)
								{
									if(SkillAttribute[Skill_Num + Num_Master].Distance > 0)
									{
										sprintf(TextList[StartText+Add_Tex+3],GlobalText[174],SkillAttribute[Skill_Num + Num_Master].Distance);
										Add_Tex++;
									}
									sprintf(TextList[StartText+Add_Tex+3],GlobalText[360],SkillAttribute[Skill_Num + Num_Master].AbilityGuage);
								}
								else
								if(AT_SKILL_SOUL_UP == Skill_Num
									|| AT_SKILL_BLOW_UP == Skill_Num
									|| AT_SKILL_ANGER_SWORD_UP == Skill_Num
									|| AT_SKILL_DEF_POWER_UP == Skill_Num
									|| AT_SKILL_ATT_POWER_UP == Skill_Num
									|| AT_SKILL_BLOOD_ATT_UP == Skill_Num
									|| AT_SKILL_POWER_SLASH_UP == Skill_Num
									|| AT_SKILL_FIRE_SCREAM_UP == Skill_Num
									)
								{
//									sprintf(TextList[StartText+Add_Tex+3],GlobalText[1783],SkillAttribute[Skill_Num + Num_Master].Damage);
//									Add_Tex++;
									if(SkillAttribute[Skill_Num + Num_Master].Distance > 0)
									{
										sprintf(TextList[StartText+Add_Tex+3],GlobalText[174],SkillAttribute[Skill_Num + Num_Master].Distance);
										Add_Tex++;
									}
									sprintf(TextList[StartText+Add_Tex+3],GlobalText[175],SkillAttribute[Skill_Num + Num_Master].Mana);
									Add_Tex++;
									sprintf(TextList[StartText+Add_Tex+3],GlobalText[360],SkillAttribute[Skill_Num + Num_Master].AbilityGuage);
								}
								else
								if(AT_SKILL_LIFE_UP == Skill_Num || AT_SKILL_ALICE_SLEEP_UP == Skill_Num ||	AT_SKILL_LIGHTNING_SHOCK_UP == Skill_Num)
								{
									sprintf(TextList[StartText+Add_Tex+3],GlobalText[175],SkillAttribute[Skill_Num + Num_Master].Mana);
									Add_Tex++;
									sprintf(TextList[StartText+Add_Tex+3],GlobalText[360],SkillAttribute[Skill_Num + Num_Master].AbilityGuage);
								}
								else
								if(AT_SKILL_HEAL_UP == Skill_Num)
								{
									sprintf(TextList[StartText+Add_Tex+3],GlobalText[1785],SkillAttribute[Skill_Num + Num_Master].Damage);
									Add_Tex++;
									sprintf(TextList[StartText+Add_Tex+3],GlobalText[175],SkillAttribute[Skill_Num + Num_Master].Mana);
								}
								else
								if(	AT_SKILL_ALICE_CHAINLIGHTNING_UP == Skill_Num
									||	AT_SKILL_ALICE_DRAINLIFE_UP == Skill_Num)
								{
									sprintf(TextList[StartText+Add_Tex+3],GlobalText[175],SkillAttribute[Skill_Num + Num_Master].Mana);
								}
								else
								if(AT_SKILL_MANA_MINUS_ADD == Skill_Num)
								{
									sprintf(TextList[StartText+Add_Tex+3],GlobalText[1788],SkillAttribute[Skill_Num + Num_Master].Damage);
								}
								else
								sprintf(TextList[StartText+Add_Tex+3],GlobalText[1777],SkillAttribute[Skill_Num + Num_Master].Damage);
							}
							if(ib == 6 + Add_Tex)
								ib = 10 + Add_Tex;
							else
								ib = StartText+4 + Add_Tex;
							if(Get_Magic == 0)
								ib = 6 + Add_Tex;

						}

						if(AT_SKILL_LIFE_UP == Skill_Num || AT_SKILL_TORNADO_SWORDA_UP == Skill_Num || AT_SKILL_TORNADO_SWORDB_UP == Skill_Num
							|| AT_SKILL_SOUL_UP == Skill_Num || AT_SKILL_MANY_ARROW_UP == Skill_Num || AT_SKILL_ASHAKE_UP == Skill_Num
							|| AT_SKILL_FIRE_BUST_UP == Skill_Num || AT_SKILL_BLAST_UP == Skill_Num || AT_SKILL_HELL_FIRE_UP == Skill_Num
							|| AT_SKILL_PET_DURABLE_SPD_DN  == Skill_Num
							|| AT_SKILL_BLOW_UP == Skill_Num
							|| AT_SKILL_ANGER_SWORD_UP == Skill_Num
							|| AT_SKILL_EVIL_SPIRIT_UP == Skill_Num
							|| AT_SKILL_EVIL_SPIRIT_UP_M == Skill_Num
							|| AT_SKILL_ICE_UP == Skill_Num
							|| AT_SKILL_DEF_POWER_UP == Skill_Num
							|| AT_SKILL_ATT_POWER_UP == Skill_Num
							|| AT_SKILL_BLOOD_ATT_UP == Skill_Num
							|| AT_SKILL_POWER_SLASH_UP == Skill_Num
							|| AT_SKILL_FIRE_SCREAM_UP == Skill_Num
							|| AT_SKILL_ALICE_SLEEP_UP == Skill_Num
							|| AT_SKILL_ALICE_CHAINLIGHTNING_UP == Skill_Num
							|| AT_SKILL_LIGHTNING_SHOCK_UP == Skill_Num
							|| AT_SKILL_ALICE_DRAINLIFE_UP == Skill_Num
							)
						{
							sprintf(TextList[ib],"");
							ib++;
							switch(Skill_Num)
							{
							case AT_SKILL_LIFE_UP:
								sprintf(TextList[ib],GlobalText[1786]);
								break;
							case AT_SKILL_SOUL_UP:
								sprintf(TextList[ib],GlobalText[1783]);
								break;
							case AT_SKILL_BLAST_UP:
							case AT_SKILL_FIRE_BUST_UP:
							case AT_SKILL_ASHAKE_UP:
							case AT_SKILL_MANY_ARROW_UP:
							case AT_SKILL_TORNADO_SWORDA_UP:
							case AT_SKILL_TORNADO_SWORDB_UP:
							case AT_SKILL_HELL_FIRE_UP:
							case AT_SKILL_ICE_UP:
							case AT_SKILL_EVIL_SPIRIT_UP:
							case AT_SKILL_EVIL_SPIRIT_UP_M:
							case AT_SKILL_BLOW_UP:
							case AT_SKILL_ANGER_SWORD_UP:
							case AT_SKILL_BLOOD_ATT_UP:
							case AT_SKILL_POWER_SLASH_UP:
							case AT_SKILL_FIRE_SCREAM_UP:
							case AT_SKILL_ALICE_CHAINLIGHTNING_UP:
							case AT_SKILL_LIGHTNING_SHOCK_UP:
							case AT_SKILL_ALICE_DRAINLIFE_UP:
								sprintf(TextList[ib],GlobalText[1784]);
								break;
							case AT_SKILL_ALICE_SLEEP_UP:
							case AT_SKILL_PET_DURABLE_SPD_DN:
							case AT_SKILL_DEF_POWER_UP:
							case AT_SKILL_ATT_POWER_UP:
								sprintf(TextList[ib],GlobalText[1787]);
								break;
							};
							ib++;
						}
						RenderTipTextList(Loc[Skill] + 5 + ((SKILL_ICON_WIDTH + 19) * sx), 75 + ((SKILL_ICON_HEIGHT + 14) * nsy), ib, 0, RT3_SORT_CENTER, FALSE);
					}
				}
			}
		}
	}
}

bool SEASON3B::CNewUIMasterLevel::Update()
{

	return true;
}

void SEASON3B::CNewUIMasterLevel::Render_Text()
{
	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 0);
	unicode::t_char lpszStr[256] = {NULL, };
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	unicode::_sprintf( lpszStr, GlobalText[1746],Master_Level_Data.nMLevel );
	g_pRenderText->RenderText(m_Pos.x+170, m_Pos.y+8, lpszStr, 0, 0, RT3_WRITE_CENTER);

	unicode::_sprintf( lpszStr, GlobalText[1747],Master_Level_Data.nMLevelUpMPoint );
	g_pRenderText->RenderText(m_Pos.x+265, m_Pos.y+8, lpszStr, 0, 0, RT3_WRITE_CENTER);

	unicode::_sprintf( lpszStr, GlobalText[1748],Master_Level_Data.lMasterLevel_Experince,Master_Level_Data.lNext_MasterLevel_Experince );
	g_pRenderText->RenderText(m_Pos.x + 410, m_Pos.y+8, lpszStr, 0, 0, RT3_WRITE_CENTER);

	//unicode::_sprintf( lpszStr, GlobalText[20]);
	int Job = 0;// = GetCharacterClass(Hero->Class);

	switch(gCharacterManager.GetCharacterClass(Hero->Class))
	{
	case CLASS_WIZARD:
	case CLASS_SOULMASTER:
	case CLASS_GRANDMASTER:
		Job = 1751;
		break;
	case CLASS_KNIGHT:
	case CLASS_BLADEKNIGHT:
	case CLASS_BLADEMASTER:
		Job = 1755;
		break;
	case CLASS_ELF:
	case CLASS_MUSEELF:
	case CLASS_HIGHELF:
		Job = 1759;
		break;
	case CLASS_DARK:
	case CLASS_DUELMASTER:
		Job = 1763;
		break;
	case CLASS_DARK_LORD:
	case CLASS_LORDEMPEROR:
		Job = 1767;
		break;
	case CLASS_SUMMONER:
	case CLASS_BLOODYSUMMONER:
	case CLASS_DIMENSIONMASTER:
		Job = 3136;
		break;
	}

	g_pRenderText->SetTextColor(255, 155, 0, 255);
	unicode::_sprintf( lpszStr, GlobalText[Job],JobPoint[0]);
	g_pRenderText->RenderText(m_Pos.x + 91, m_Pos.y+37, lpszStr, 0, 0, RT3_SORT_LEFT);
	unicode::_sprintf( lpszStr, GlobalText[Job + 1],JobPoint[1]);
	g_pRenderText->RenderText(m_Pos.x + 251, m_Pos.y+37, lpszStr, 0, 0, RT3_SORT_LEFT);
	unicode::_sprintf( lpszStr, GlobalText[Job + 2],JobPoint[2]);
	g_pRenderText->RenderText(m_Pos.x + 375, m_Pos.y+37, lpszStr, 0, 0, RT3_SORT_LEFT);
	unicode::_sprintf( lpszStr, GlobalText[Job + 3],JobPoint[3]);
	g_pRenderText->RenderText(m_Pos.x + 518, m_Pos.y+37, lpszStr, 0, 0, RT3_SORT_LEFT);
}


void SEASON3B::CNewUIMasterLevel::OpenMasterLevel ( const char* filename )
{
//	MASTER_LEVEL_DATA		m_MasterLevel[MAX_MASTER];
	FILE* fp = fopen ( filename, "rb" );
	if(fp != NULL)
	{
		int Size = sizeof(MASTER_LEVEL_DATA);
		BYTE *Buffer = new BYTE [Size*MAX_MASTER + 45];
		fread(Buffer,(Size*MAX_MASTER) + 45,1,fp);

		DWORD dwCheckSum;
		fread(&dwCheckSum,sizeof ( DWORD),1,fp);
		fclose(fp);

		if ( dwCheckSum != GenerateCheckSum2( Buffer, (Size*MAX_MASTER) + 45, 0x2BC1))
		{
			char Text[256];
    		sprintf(Text,"%s - File corrupted.",filename);
			g_ErrorReport.Write( Text);
			MessageBox(g_hWnd,Text,NULL,MB_OK);
			SendMessage(g_hWnd,WM_DESTROY,0,0);
		}
		else
		{
			BYTE *pSeek = Buffer;

			for(int i = 0; i < 9; i++)
			{
				Master_Skill_Data[i][0] = 0;
				BuxConvert(pSeek,5);
				memcpy(&Master_Skill_Data[i],pSeek,5);
				pSeek += 5;
			}
			for(int i=0;i<MAX_MASTER;i++)
			{
				BuxConvert(pSeek,Size);
				memcpy(&(m_MasterLevel[i]),pSeek,Size);

				pSeek += Size;
			}
		}
		delete [] Buffer;
	}
	else
	{
		char Text[256];
    	sprintf(Text,"%s - File not exist.",filename);
		g_ErrorReport.Write( Text);
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
	}
}

void SEASON3B::CNewUIMasterLevel::LoadImages()
{
	//IMAGE_MASTER_INTERFACE
	LoadBitmap("Interface\\newui_exit_00.tga", IMAGE_MASTER_EXIT, GL_LINEAR);				// Exit Button
	LoadBitmap("Interface\\Master_back.jpg", IMAGE_MASTER_INTERFACE, GL_LINEAR);
	LoadBitmap("Interface\\Master_skillbox.tga", IMAGE_MASTER_INTERFACE + 1, GL_LINEAR);
	LoadBitmap("Interface\\Master_arrow(D).tga", IMAGE_MASTER_INTERFACE + 2, GL_LINEAR);
	LoadBitmap("Interface\\Master_arrow(R).tga", IMAGE_MASTER_INTERFACE + 3, GL_LINEAR);
}

void SEASON3B::CNewUIMasterLevel::UnloadImages()
{
	for(int i = 0; i < 4; i++)
	{
		DeleteBitmap(IMAGE_MASTER_INTERFACE + i);
	}
	DeleteBitmap( IMAGE_MASTER_EXIT );		// Exit Button
}

bool SEASON3B::CNewUIMasterLevel::UpdateMouseEvent()
{
	bool ret = true;
//g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MASTER_LEVEL);
	if(m_BtnExit.UpdateMouseEvent() == true)
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_MASTER_LEVEL);
		return true;
	}

	if(SEASON3B::IsPress(VK_LBUTTON))
	{
		ret = Check_Mouse(MouseX, MouseY);
		if(ret == false)
		{
			PlayBuffer( SOUND_CLICK01 );
		}
		
	}

	for(int i = 0; i < 4; i++)
	{
		if(m_EventState[i] == EVENT_SCROLL_BTN_DOWN)
		{
			if(SEASON3B::IsRelease(VK_LBUTTON))
			{
				m_EventState[i] = EVENT_NONE;
				return true;
			}
		}
	}

	Check_Btn();

	if(CheckMouseIn(m_Pos.x, m_Pos.y, m_Width.x, m_Width.y))
	{
		return false;
	}

	return ret;
}

bool SEASON3B::CNewUIMasterLevel::Check_Mouse(int mx,int my)
{
	/*
	if(mx > m_Pos.x && mx < (m_Pos.x+m_Width.x) && my > m_Pos.y && my < (m_Pos.y+m_Width.y))
	{
		if(mx > (m_Pos.x+12+Tab_Pos) && mx < (m_Pos.x+12+Tab_Pos+56) && my > m_Pos.y && my < (m_Pos.y+90))
		{
			return false;
		}
	}
	*/
	int Scroll_Loc[4][2] = {{185,65},{308,65},{432,65},{590,65}};
	for(int i = 0; i < 4; i++)
	{
		if(SEASON3B::CheckMouseIn(m_Pos.x+Scroll_Loc[i][0], m_Pos.y+Scroll_Loc[i][1]+m_Loc[i], 15, 30) == true && m_EventState[i] == EVENT_NONE)
		{
			m_EventState[i] = EVENT_SCROLL_BTN_DOWN;
			/*
			if(m_BackUp == 0)
			{
				m_BackUp	= MouseY;
			}
			*/
			return false;
		}
	}
	return true;
}

bool SEASON3B::CNewUIMasterLevel::Check_Btn()
{
	int Scroll_Lenth = 285 - 65;
	/*
	if(SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_GUILDINFO);
		m_EventState = EVENT_NONE;
		return false;
	}
*/
	for(int i = 0; i< 4; i++)
	{
		if(m_EventState[i] == EVENT_SCROLL_BTN_DOWN && SEASON3B::IsRelease(VK_LBUTTON))
		{
			m_EventState[i] = EVENT_NONE;
			return false;
		}
		else
		if(m_EventState[i] == EVENT_SCROLL_BTN_DOWN)
		{
			m_Loc[i] = MouseY - 65;
			if(m_Loc[i] > Scroll_Lenth)
				m_Loc[i] = Scroll_Lenth;
			else
			if(m_Loc[i] <= 0)
				m_Loc[i] = 0;
		}
	}

	int Skill_Num = -1;

	BYTE Job = (Hero->Class >> 4) & 0x01;
	int GetJob = -1;

	if(Job == 1)
		Job = gCharacterManager.GetCharacterClass(Hero->Class);
	switch(Job)
	{
	case CLASS_WIZARD:
	case CLASS_SOULMASTER:
	case CLASS_GRANDMASTER:
		GetJob = 0;
		break;
	case CLASS_KNIGHT:
	case CLASS_BLADEKNIGHT:
	case CLASS_BLADEMASTER:
		GetJob = 4;
		break;
	case CLASS_ELF:
	case CLASS_MUSEELF:
	case CLASS_HIGHELF:
		GetJob = 8;
		break;
	case CLASS_DARK:
	case CLASS_DUELMASTER:
		GetJob = 12;
		break;
	case CLASS_DARK_LORD:
	case CLASS_LORDEMPEROR:
		GetJob = 16;
		break;
	case CLASS_SUMMONER:
	case CLASS_BLOODYSUMMONER:
	case CLASS_DIMENSIONMASTER:
		GetJob = 20;
		break;
	}

	int Loc[4] = {37,237,362,488};
	int Dir = -1;

	int Wid = SKILL_ICON_DATA_HEIGHT - 2;
	int Over_Line = 0;

	
	for(int Skill = 0; Skill < 4; Skill++)
	{
		int Count = 220/(SKILL_ICON_DATA_HEIGHT - Wid);
		int StartY = (m_Loc[Skill]/Count);
		for(int sy = StartY; sy < (Wid + StartY); sy++)
		{
			int nsy = sy-StartY;
			for(int sx = 0; sx < m_MasterLevel[GetJob + Skill].Width; sx++)
			{
				Dir = -1;
				Skill_Num = m_MasterLevel[GetJob + Skill].Ability[sy][sx];
				if(Skill_Num == -1)
					continue;
				else
				if(Skill_Num >= 10000)
				{
					Dir = Skill_Num/10000;
					Skill_Num = Skill_Num%10000;
				}


				if(SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(Loc[Skill] + 5 + ((SKILL_ICON_WIDTH + 19) * sx), 75 + ((SKILL_ICON_HEIGHT + 14) * nsy), 15, 21))
				{
					if(sy > 0 && (m_MasterLevel[GetJob + Skill].Ability[sy - 1][sx]/10000) == 3)
					{
						int Before_Skill = m_MasterLevel[GetJob + Skill].Ability[sy - 1][sx]%10000;

						int icntMagic = 0;
						for(int i=0; i<MAX_MAGIC; ++i)
						{
							icntMagic = i;
							if(CharacterAttribute->Skill[i] == (Before_Skill + 4))
							{
								break;
							}
						}
						if(icntMagic == MAX_MAGIC)
							continue;
					}
					else
					if(sx > 0 && (m_MasterLevel[GetJob + Skill].Ability[sy][sx - 1]/10000) == 2)
					{
						int Before_Skill = m_MasterLevel[GetJob + Skill].Ability[sy][sx - 1]%10000;
						int icntMagic = 0;
						for(int i=0; i<MAX_MAGIC; ++i)
						{
							icntMagic = i;
							if(CharacterAttribute->Skill[i] == (Before_Skill + 4))
							{
								break;
							}
						}
						if(icntMagic == MAX_MAGIC)
							continue;
					}
					else
					if(sx < (m_MasterLevel[GetJob + Skill].Width - 1) && (m_MasterLevel[GetJob + Skill].Ability[sy][sx + 1]/10000) == 4)
					{
						int Before_Skill = m_MasterLevel[GetJob + Skill].Ability[sy][sx + 1]%10000;
						int icntMagic = 0;
						for(int i=0; i<MAX_MAGIC; ++i)
						{
							icntMagic = i;
							if(CharacterAttribute->Skill[i] == (Before_Skill + 4))
							{
								break;
							}
						}
						if(icntMagic == MAX_MAGIC)
							continue;
					}
					PlayBuffer(SOUND_CLICK01);

					bool Get_Skill = false;
					if(Skill_Num > 0)
					{
						int Cur_Skill = Skill_Num;
						int Get_Magic = -1;

						for(int i=0; i<MAX_MAGIC; ++i)
						{
							if((CharacterAttribute->Skill[i] + 1) > Cur_Skill && (CharacterAttribute->Skill[i] + 1) < (Cur_Skill + 5))
							{
								if(Master_Skill_Data[sy][(CharacterAttribute->Skill[i] + 1) - Cur_Skill] <= Master_Level_Data.nMLevelUpMPoint)
								{
									Get_Magic = (CharacterAttribute->Skill[i] - Cur_Skill) + 1;
									Cur_Skill = CharacterAttribute->Skill[i] + 1;
									Get_Skill = true;
								}
								else
									Get_Skill = false;
								break;
							}
						}

						if(Get_Skill == false)
						{
							Get_Skill = true;

							for(int i=0; i<MAX_MAGIC; ++i)
							{
								if(CharacterAttribute->Skill[i] == (Cur_Skill + 4))
								{
									Get_Skill = false;
									break;
								}
							}
							if(Get_Skill == true)
							{
								if(Master_Skill_Data[0][0] > Master_Level_Data.nMLevelUpMPoint)
									Get_Skill = false;
							}
						}

						if(Get_Skill == true)
						{
							int Point_R = 0,Point_Tot = 0,Point_Before = 0,Before_Line = 0;

							if(sy > 0)
							{
								for(int sax = 0; sax < 5; sax++)
								{
									Point_Before += Master_Skill_Data[sy - 1][sax];
									Point_R += Master_Skill_Data[sy][sax];
								}
								for(int sax = 0; sax < m_MasterLevel[GetJob + Skill].Width; sax++)
								{
									int Skill_Number = m_MasterLevel[GetJob + Skill].Ability[sy-1][sax];

									if(Skill_Number == -1)
										continue;
									else
									if(Skill_Number >= 10000)
									{
//										Dir = Skill_Number/10000;
										Skill_Number = Skill_Number%10000;

									}

									int Skill_Icon_Local = 0;

									Skill_Icon_Local = SkillAttribute[Skill_Number].Magic_Icon;

									if(Skill_Number > 0)
									{
						
										int Get_Magic_Local = 0;

										for(int it=0; it<MAX_MAGIC; ++it)
										{
											if(CharacterAttribute->Skill[it] >= Skill_Number && CharacterAttribute->Skill[it] < (Skill_Number + 5))
											{
												Get_Magic_Local = (CharacterAttribute->Skill[it] - Skill_Number) + 1;
												break;
											}
										}
					
					
										for(int iu = 0; iu < Get_Magic_Local; iu++)
										{
											Before_Line += Master_Skill_Data[sy-1][iu];
										}
									}
								}

								Dir = -1;
								if(Skill_Num > 0)
								{
									int Cur_Skill = Skill_Num;

									for(int i=0; i<MAX_MAGIC; ++i)
									{
										if(CharacterAttribute->Skill[i] >= Cur_Skill && CharacterAttribute->Skill[i] < (Cur_Skill + 5))
										{
											Get_Magic = (CharacterAttribute->Skill[i] - Cur_Skill) + 1;
											break;
										}
									}
									if(Get_Magic > 0)
									{
										Point_Tot += Get_Magic;
										for(int iu = 0; iu < Get_Magic; iu++)
											Over_Line += Master_Skill_Data[sy][iu];
									}
									else
										Get_Magic = 0;
								}
							}

							if((Point_Before <= Before_Line) && (Point_R >= Over_Line) && Get_Magic <= 5 && Master_Skill_Data[sy][Get_Magic] <= Master_Level_Data.nMLevelUpMPoint)
							{
								int Pot = Get_Magic;
								if(Pot < 0)
									Pot = 0;
								else
								if(Pot >= 5)
									Pot = 4;
								Need_Point = Master_Skill_Data[sy][Pot];

								if(g_csItemOption.Special_Option_Check() == false && (Cur_Skill == AT_SKILL_ICE_BLADE||(AT_SKILL_POWER_SLASH_UP <= Cur_Skill && AT_SKILL_POWER_SLASH_UP+4 >= Cur_Skill)))
									return true;

								if(g_csItemOption.Special_Option_Check(1) == false && (Cur_Skill == AT_SKILL_CROSSBOW||(AT_SKILL_MANY_ARROW_UP <= Cur_Skill && AT_SKILL_MANY_ARROW_UP+4 >= Cur_Skill)))
									return true;

								In_Skill = Cur_Skill;
								SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CMaster_Level_Interface));
								Over_Line = 0;
								MouseLButton = false;
								MouseLButtonPop = false;
								MouseLButtonPush = false;
							}
						}
					}
				}
			}
		}
	}
	return true;
}
