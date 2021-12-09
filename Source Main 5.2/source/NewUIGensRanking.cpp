// NewUIGensRanking.cpp: implementation of the CNewUIGensRanking class.
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#ifdef PBG_ADD_GENSRANKING
#include "NewUIGensRanking.h"
#include "NewUISystem.h"
#include "wsclientinline.h"

using namespace SEASON3B;

#define TEMP_MAX_TEXT_LENGTH 1024

CNewUIGensRanking::CNewUIGensRanking() : m_fBooleanSize(0.8f)
{
	Init();
}

CNewUIGensRanking::~CNewUIGensRanking()
{
	if( m_pScrollBar ) 
		m_pScrollBar->Release();

	SAFE_DELETE( m_pScrollBar );

	if( m_pTextBox )
		m_pTextBox->Release();

	SAFE_DELETE( m_pTextBox );
	Destroy();
}

void CNewUIGensRanking::Init()
{
	m_nContribution = 0;
	memset(m_szRanking, 0, sizeof(char)*TEAMNAME_LENTH);

	m_Pos.x = 0; 
	m_Pos.y = 0;

	memset(m_szGensTeam, 0, sizeof(char)*TEAMNAME_LENTH);

	m_byGensInfluence = 0;
	m_ptRenderMarkPos.x = 0;
	m_ptRenderMarkPos.y = 0;

	m_nNextContribution = 0;

	memset(m_szTitleName, 0, sizeof(char)*TITLENAME_END*MAX_TITLELENGTH);
	SetTitleName();
}

void CNewUIGensRanking::Destroy()
{
	UnloadImages();

	if (m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}
}

bool CNewUIGensRanking::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if(pNewUIMng  == NULL)
		return false;
	
	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(INTERFACE_GENSRANKING, this);

	m_pScrollBar = new CNewUIScrollBar();
	m_pScrollBar->Create( x, y, 110 );

	m_pTextBox = new CNewUITextBox();
	m_pTextBox->Create( x, y, 200, 110 );

	SetPos(x, y);
	SetBtnInfo(450, 0);
	LoadImages();

	Show(false);
	
	return true;
}

void CNewUIGensRanking::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
	if( m_pScrollBar )
		m_pScrollBar->SetPos( x + 20 + 150, y + 273 );

	if( m_pTextBox )
		m_pTextBox->SetPos( x + 20, y + 280, 150, 110 );
}

bool CNewUIGensRanking::Render()
{
	EnableAlphaTest();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	RenderFrame();
	RenderTexts();
	RenderButtons();	
	DisableAlphaBlend();

	if( m_pScrollBar )
		m_pScrollBar->Render();

	if( m_pTextBox )
		m_pTextBox->Render();
	return true;
}

void CNewUIGensRanking::RenderFrame()
{
	RenderImage(IMAGE_BACK, m_Pos.x, m_Pos.y, 190.f, 429.f);
	RenderImage(IMAGE_TOP, m_Pos.x, m_Pos.y, 190.f, 64.f);
	RenderImage(IMAGE_LEFT, m_Pos.x, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_RIGHT, m_Pos.x+GENSRANKING_WIDTH-21, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_BOTTOM, m_Pos.x, m_Pos.y+GENSRANKING_HEIGHT-45, 190.f, 45.f);

	RenderImage(IMAGE_RANKBACK, m_Pos.x+13, m_Pos.y+60, GENSRANKBACK_WIDTH, GENSRANKBACK_HEIGHT);
	RenderMark(m_Pos.x+13+(85-50)*0.5f, m_Pos.y+60+(88-69)*0.5f, m_byGensInfluence);
	RenderInfoFrame(m_Pos.x+13, m_Pos.y+250, 168, 140);

	RenderImage(IMAGE_RANKBACK_TEXTBOX, m_Pos.x+13, m_Pos.y+GENSRANKBACK_HEIGHT+60+3, GENSRANKTEXTBACK_WIDTH, GENSRANKTEXTBACK_HEIGHT);
	RenderImage(IMAGE_RANKBACK_TEXTBOX, m_Pos.x+13, m_Pos.y+GENSRANKBACK_HEIGHT+GENSRANKTEXTBACK_HEIGHT+60+6, GENSRANKTEXTBACK_WIDTH, GENSRANKTEXTBACK_HEIGHT);
}

void CNewUIGensRanking::RenderInfoFrame(int iPosX, int iPosY, int iWidth, int iHeight, int iTitleWidth, int iTitleHeight)
{
	EnableAlphaTest();
	glColor4f ( 0.f, 0.f, 0.f, 0.6f );
	RenderColor( float(iPosX+3), float(iPosY+2), float(iTitleWidth-8), float(iTitleHeight) );
 	RenderColor( float(iPosX+3), float(iPosY+2+iTitleHeight), float(iWidth-7), float(iHeight-iTitleHeight-7) );
	EndRenderColor();
	
	RenderImage(IMAGE_GENSINFO_TOP_LEFT, iPosX, iPosY, 14, 14);
	RenderImage(IMAGE_GENSINFO_TOP_RIGHT, iPosX+iTitleWidth-14, iPosY, 14, 14);
	RenderImage(IMAGE_GENSINFO_TOP_RIGHT, iPosX+iWidth-14, iPosY+iTitleHeight, 14, 14);
	RenderImage(IMAGE_GENSINFO_BOTTOM_LEFT, iPosX, iPosY+iHeight-14, 14, 14);
	RenderImage(IMAGE_GENSINFO_BOTTOM_RIGHT, iPosX+iWidth-14, iPosY+iHeight-14, 14, 14);

	RenderImage(IMAGE_GENSINFO_TOP_PIXEL, iPosX+6, iPosY, iTitleWidth-12, 14);
	RenderImage(IMAGE_GENSINFO_RIGHT_PIXEL, iPosX+iTitleWidth-14, iPosY+6, 14, iTitleHeight-6);
	RenderImage(IMAGE_GENSINFO_TOP_PIXEL, iPosX+iTitleWidth-5, iPosY+iTitleHeight, iWidth-iTitleWidth-6, 14);
	RenderImage(IMAGE_GENSINFO_RIGHT_PIXEL, iPosX+iWidth-14, iPosY+iTitleHeight+6, 14, iHeight-iTitleHeight-14);
	RenderImage(IMAGE_GENSINFO_BOTTOM_PIXEL, iPosX+6, iPosY+iHeight-14, iWidth-12, 14);
	RenderImage(IMAGE_GENSINFO_LEFT_PIXEL, iPosX, iPosY+6, 14, iHeight-14);
}

void CNewUIGensRanking::RenderTexts()
{
	unicode::t_char szText[TEMP_MAX_TEXT_LENGTH];
	float _x = GetPos().x;
	float _y = GetPos().y+15;

	g_pRenderText->SetBgColor(0, 0, 0, 0);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->SetFont(g_hFontBold);
	sprintf(szText, GlobalText[3090]);
	g_pRenderText->RenderText(_x, _y, szText, GENSRANKING_WIDTH, 0, RT3_SORT_CENTER);

	_y += 75;
	sprintf(szText, GlobalText[3091]);
	g_pRenderText->SetTextColor(246, 209, 73, 255);
	g_pRenderText->RenderText(_x+102, _y, szText, GENSRANKING_WIDTH, 0, RT3_SORT_LEFT);

	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->SetFont(g_hFont);

	sprintf(szText, "%s", GetGensTeamName());
	_y += 20;
	g_pRenderText->RenderText(_x+102, _y, szText, GENSRANKING_WIDTH, 0, RT3_SORT_LEFT);

	sprintf(szText, GlobalText[3095]);
	_y += 24;
	g_pRenderText->RenderText(_x+100, _y, szText, GENSRANKING_WIDTH, 0, RT3_SORT_LEFT);

	sprintf(szText, "%s", GetTitleName(Hero->m_byRankIndex));
	g_pRenderText->RenderText(_x+66, _y, szText+1, GENSRANKING_WIDTH-16, 0, RT3_SORT_CENTER);

	g_pRenderText->SetTextColor(230, 230, 0, 255);
	g_pRenderText->SetFont(g_hFontBold);
	sprintf(szText, GlobalText[3098]);
	_y += 23;
	g_pRenderText->RenderText(_x+13, _y, szText, 74, 0, RT3_SORT_CENTER);

	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	sprintf(szText, GlobalText[3099], GetRanking());
	g_pRenderText->RenderText(_x, _y, szText, GENSRANKING_WIDTH-20, 0, RT3_SORT_RIGHT);

	g_pRenderText->SetTextColor(230, 230, 0, 255);
	g_pRenderText->SetFont(g_hFontBold);
	sprintf(szText, GlobalText[3096]);
	_y += 23;
	g_pRenderText->RenderText(_x+13, _y, szText, 74, 0, RT3_SORT_CENTER);
	g_pRenderText->SetFont(g_hFont);

	g_pRenderText->SetTextColor(255, 255, 255, 255);
	sprintf(szText, "%d", GetContribution());
	g_pRenderText->RenderText(_x, _y, szText, GENSRANKING_WIDTH-20, 0, RT3_SORT_RIGHT);

	
	unicode::t_char _szTempText[TEMP_MAX_TEXT_LENGTH];
	unicode::t_char _szText[NUM_LINE_CMB][MAX_TEXT_LENGTH];
	int _TextLineCnt = 0;
	float _fWidth=180, _fHeight=15;

	if(GetNextContribution()>0)
	{
		unicode::_sprintf(_szTempText, GlobalText[3097], GetNextContribution());
		_TextLineCnt = ::DivideStringByPixel(&_szText[0][0], NUM_LINE_CMB, MAX_TEXT_LENGTH, _szTempText, 140, true, '#');	
		for (int j=0; j<_TextLineCnt; ++j)
			g_pRenderText->RenderText(_x+20,  _y+(j*_fHeight)+20,  _szText[j], _fWidth, 0, RT3_SORT_LEFT);
	}

	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetTextColor(230, 230, 0, 255);
	sprintf(szText, GlobalText[3100]);
	
	_y += 78;
	g_pRenderText->RenderText(_x+13, _y, szText, 58, 0, RT3_SORT_CENTER);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->SetFont(g_hFont);

}

void CNewUIGensRanking::RenderButtons()
{
	m_BtnExit.Render();
}

void CNewUIGensRanking::RenderMark(int _x, int _y, BYTE _GensInfluence)
{
	RanderMark(_x, _y, (GENS_TYPE)_GensInfluence, Hero->m_byRankIndex);
}

bool CNewUIGensRanking::Update()
{
	if( !IsVisible() )
		return true;

	if( m_pTextBox )
	{
		m_pTextBox->ClearText();
		m_pTextBox->AddText( GlobalText[3101] );
		m_pTextBox->AddText( GlobalText[3102] );

		if( m_pTextBox->GetMoveableLine() > 0 )
		{
			if( m_pScrollBar ) 
			{
				m_pScrollBar->Show( true );

				int iMaxPos = m_pTextBox->GetMoveableLine();

				m_pScrollBar->SetMaxPos( iMaxPos );
				m_pScrollBar->Update();
				m_pTextBox->SetCurLine( m_pScrollBar->GetCurPos() );
			}
		}
		else
		{
			if( m_pScrollBar ) 
			{
				m_pScrollBar->Show( false );
			}
		}
	}
	return true;
}

bool CNewUIGensRanking::UpdateMouseEvent()
{
	if(!g_pNewUISystem->IsVisible(INTERFACE_GENSRANKING))
		return true;

	if( m_pScrollBar ) 
		m_pScrollBar->UpdateMouseEvent();

	if(BtnProcess())
		return false;
	
	if(CheckMouseIn(m_Pos.x, m_Pos.y, GENSRANKING_WIDTH, GENSRANKING_HEIGHT))
	{
		if(SEASON3B::IsPress(VK_RBUTTON))
		{
			MouseRButton = false;
			MouseRButtonPop = false;
			MouseRButtonPush = false;
			return false;
		}
		if(!SEASON3B::IsNone(VK_LBUTTON))
			return false;
	}
	return true;
}

bool CNewUIGensRanking::UpdateKeyEvent()
{
	if(g_pNewUISystem->IsVisible(INTERFACE_GENSRANKING))
	{
		if(SEASON3B::IsPress(VK_ESCAPE))
		{
			g_pNewUISystem->Hide(INTERFACE_GENSRANKING);
			return false;
		}
	}
	return true;
}

bool CNewUIGensRanking::BtnProcess()
{
	POINT pClose = {GetPos().x+169, GetPos().y+7};
		
	if(SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(pClose.x, pClose.y, 13, 12))
	{
		g_pNewUISystem->Hide(INTERFACE_GENSRANKING);
		return false;
	}

	if(m_BtnExit.UpdateMouseEvent())
	{
		if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_GENSRANKING))
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_GENSRANKING);
			return true;
		}
		return false;
	}

	return false;
}

void CNewUIGensRanking::SetBtnInfo(float _PosX, float _PosY)
{
	m_BtnExit.ChangeButtonImgState(true, IMAGE_GENS_EXIT_BTN, false);
	m_BtnExit.ChangeButtonInfo(13+_PosX, 392+_PosY, 36, 29);
	m_BtnExit.ChangeToolTipText(GlobalText[1002], true);
}

void CNewUIGensRanking::OpenningProcess()
{
	SendRequestGensInfo_Open();
}

void CNewUIGensRanking::ClosingProcess()
{
	//n/a
}

void CNewUIGensRanking::LoadImages()
{
	LoadBitmap("Interface\\newui_gens_rankback.tga", IMAGE_RANKBACK);
}

void CNewUIGensRanking::UnloadImages()
{
	DeleteBitmap(IMAGE_RANKBACK);
}

void CNewUIGensRanking::SetContribution(int _Contribution)
{
	if(_Contribution < 0)
	{
		m_nContribution = 0;
		return;
	}

	m_nContribution = _Contribution;
}

int CNewUIGensRanking::GetContribution()
{
	if(m_nContribution <= 0)
		return 0;

	return m_nContribution;
}

bool CNewUIGensRanking::SetRanking(int _Ranking)
{
	if(_Ranking <= 0)
	{
		sprintf(m_szRanking, "-");
		return false;
	}

	itoa(_Ranking, m_szRanking, 10);
	return true;
}

unicode::t_char* CNewUIGensRanking::GetRanking()
{
	return m_szRanking;
}

void CNewUIGensRanking::SetNextContribution(int _NextContribution)
{
	if(_NextContribution < 0)
	{
		m_nNextContribution = -1;
		return;
	}
	m_nNextContribution = _NextContribution;
}

int CNewUIGensRanking::GetNextContribution()
{
	return m_nNextContribution;
}

bool CNewUIGensRanking::SetGensInfo()
{
	m_byGensInfluence = Hero->m_byGensInfluence;

	if((m_byGensInfluence & GENSTYPE_DUPRIAN)==GENSTYPE_DUPRIAN)
	{
		SetGensTeamName(GlobalText[3092]);
		return true;
	}
	else if((m_byGensInfluence & GENSTYPE_BARNERT)==GENSTYPE_BARNERT)
	{
		SetGensTeamName(GlobalText[3093]);
		return true;
	}
	else
	{
		g_pChatListBox->AddText("", GlobalText[3094], SEASON3B::TYPE_SYSTEM_MESSAGE);
		return false;
	}
	return false;
}

bool CNewUIGensRanking::SetGensTeamName(const char* _pTeamName)
{
	if(_pTeamName)
	{
		strcpy(m_szGensTeam, _pTeamName);
		return true;
	}
	return false;
}

char* CNewUIGensRanking::GetGensTeamName()
{
	return m_szGensTeam;
}

void CNewUIGensRanking::SetTitleName()
{
	unicode::t_char _szTempText[256] = {0,};
	unicode::_sprintf(_szTempText, GlobalText[3104]);
	::DivideStringByPixel(&m_szTitleName[0][0], TITLENAME_END, MAX_TITLELENGTH, _szTempText, 240, true, '#');
}

const char* CNewUIGensRanking::GetTitleName(BYTE _index)
{
	if(TITLENAME_START <= _index && TITLENAME_END >= _index)
		return m_szTitleName[_index-1];
	else
		return m_szTitleName[TITLENAME_END-1];
}

void CNewUIGensRanking::RanderMark(float _x, float _y, BYTE _GensInfluence, BYTE _GensRankInfo, int _ImageArea, float _RenderY)
{
	if(!_GensInfluence)
		return;

	if(_GensRankInfo < TITLENAME_START || _GensRankInfo > TITLENAME_END)
		_GensRankInfo = TITLENAME_END;

	int _Image_Type = (_GensInfluence == 1) ? IMAGE_NEWMARK_DUPRIAN : IMAGE_NEWMARK_BARNERT;

	float _width= GENSMARK_WIDTH;
	float _height = GENSMARK_HEIGHT;
	BITMAP_t* pBitmap = Bitmaps.GetTexture(_Image_Type);
	float _BitmapWidth = pBitmap->Width;
	float _BitmapHeight = pBitmap->Height;
	
	if(_ImageArea == MARK_BOOLEAN)
	{
		_width= GENSMARK_WIDTH * m_fBooleanSize;
		_height = GENSMARK_HEIGHT * m_fBooleanSize;
		_BitmapWidth = pBitmap->Width * m_fBooleanSize;
		_BitmapHeight = pBitmap->Height * m_fBooleanSize;
		_y = (_RenderY - _y - _height) / 2 + _y;
		_x = (float)(_x - _width + 1);
	}

	int _ImgIndex = GetImageIndex(_GensRankInfo);
	float iWidthIndex = _ImgIndex % 5;
	float iHeightIndex = _ImgIndex / 5;
	float u = iWidthIndex * _width / _BitmapWidth;
	float v = iHeightIndex * _height / _BitmapHeight;
	
	RenderBitmap(_Image_Type, _x, _y, _width, _height, u, v, _width/_BitmapWidth, _height/_BitmapHeight);	
}

int CNewUIGensRanking::GetImageIndex(BYTE _index)
{
	if(_index > TITLENAME_END || _index <= TITLENAME_NONE)
		return -1;

	return TITLENAME_END - _index;
}
#endif //PBG_ADD_GENSRANKING
