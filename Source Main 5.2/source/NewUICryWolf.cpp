// NewUIGuildInfoWindow.cpp: implementation of the CNewUIGuildInfoWindow class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUICryWolf.h"
#include "NewUISystem.h"
#include "NewUICommonMessageBox.h"
#include "NewUICustomMessageBox.h"
#include "DSPlaySound.h"
#include "UIGuildInfo.h"
#include "UIControls.h"
#include "UIPopup.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"

extern bool	View_Bal;
extern char	Suc_Or_Fail;
extern char	View_Suc_Or_Fail;
extern float Deco_Insert;
extern char Message_Box;
extern wchar_t   Box_String[2][200];
extern int  Dark_elf_Num;
extern int Button_Down;
extern int BackUp_Key;
extern int Val_Hp;
extern int m_iHour, m_iMinute;
extern DWORD m_dwSyncTime;
extern int Delay;
extern int Add_Num;
extern bool Dark_Elf_Check;
extern int iNextNotice;

extern BYTE Rank;
extern int Exp;
extern BYTE Ranking[5];
extern BYTE HeroClass[5];
extern int HeroScore[5];
extern wchar_t HeroName[5][MAX_ID_SIZE + 1];

extern int BackUpMin;
extern bool TimeStart;
extern int Delay_Add_inter;
extern bool View_End_Result;
extern int nPastTick;
extern int BackUpTick;

extern BYTE m_OccupationState;
extern BYTE m_CrywolfState;
extern int m_StatueHP;

using namespace SEASON3B;

SEASON3B::CNewUICryWolf::CNewUICryWolf()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;

    m_iHour = 0;
    m_iMinute = 0;
    m_iSecond = 0;
    m_dwSyncTime = 0;
    m_icntTime = 0;
    m_bTimeStart = false;
}

SEASON3B::CNewUICryWolf::~CNewUICryWolf()
{
    Release();
}

bool SEASON3B::CNewUICryWolf::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_CRYWOLF, this);

    SetPos(x, y);

    LoadImages();
    return true;
}

void SEASON3B::CNewUICryWolf::ClosingProcess()
{
    SocketClient->ToGameServer()->SendCloseNpcRequest();
}

float SEASON3B::CNewUICryWolf::GetLayerDepth()
{
    return 10.0f;
}

void SEASON3B::CNewUICryWolf::OpenningProcess()
{
    m_iHour = 0;
    m_iMinute = 0;
    m_iSecond = 0;
    m_dwSyncTime = 0;
    m_icntTime = 0;
    m_bTimeStart = false;
}

void SEASON3B::CNewUICryWolf::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void SEASON3B::CNewUICryWolf::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool SEASON3B::CNewUICryWolf::UpdateMouseEvent()
{
    return true;
}
bool SEASON3B::CNewUICryWolf::UpdateKeyEvent()
{
    return true;
}

bool SEASON3B::CNewUICryWolf::Render()
{
    if (M34CryWolf1st::IsCyrWolf1st() == false)
        return true;

    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    wchar_t Text[300];

    float Main[] = { 518.f,278.f,122.f,119.f,120.f / 128.f,118.f / 128.f };
    float Number[5][6] = { {565.f,280.f,13.f,13.f,12.f / 16.f,12.f / 16.f},
                            {582.f,282.f,13.f,13.f,12.f / 16.f,12.f / 16.f},
                            {598.f,286.f,13.f,13.f,12.f / 16.f,12.f / 16.f},
                            {613.f,294.f,13.f,13.f,12.f / 16.f,12.f / 16.f},
                            {625.f,306.f,13.f,13.f,12.f / 16.f,12.f / 16.f} };
    float Dark_Elf_Icon[] = { 623.f,358.f,15.f,15.f,14.f / 16.f,14.f / 16.f };
    float Val_Icon[] = { 623.f,379.f,15.f,15.f,14.f / 16.f,14.f / 16.f };
    int TotDelay = 400;

    if (Suc_Or_Fail == 1)
    {
        Delay++;
        if (Delay >= TotDelay)
        {
            Delay = 0;
            Suc_Or_Fail = 0;
        }
    }

    if (Suc_Or_Fail >= 0)
    {
        float A_Value = 0.f;
        int aa = (Delay * 2) % 140;

        if (aa > 70)
            A_Value = 1.f - ((aa - 70) * 0.01f);
        else
            A_Value = 0.3f + (aa * 0.01f);

        if (Delay_Add_inter <= 0)
            Delay_Add_inter = 0;
        else
            Delay_Add_inter -= 15;

        if ((Delay * 15) > 479)
        {
            g_pCryWolfInterface->Render(150, 50, 329, 94, 0.f, 0.f, 328.f / 512.f, 93.f / 128.f, Add_Num, false, false, A_Value);
        }
        else if (Suc_Or_Fail == 0)
        {
            g_pCryWolfInterface->Render(150 + (Delay * 15), 50, 329, 94, 0.f, 0.f, 328.f / 512.f, 93.f / 128.f, Add_Num, false, false, A_Value);

            Delay++;
            if ((Delay * 15) > 479)
            {
                Delay = 0;
                Suc_Or_Fail = -1;

                Delay_Add_inter = 390;
                View_End_Result = true;
                SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCry_Wolf_Result_Set_Temple));
            }
        }
        else
        {
            //Delay_Add_inter
            g_pCryWolfInterface->Render(-329 + (Delay * 15), 50, 329, 94, 0.f, 0.f, 328.f / 512.f, 93.f / 128.f, Add_Num, false, false, A_Value);
        }

        g_pCryWolfInterface->Render(230, 150, 196, 141, 0.f, 0.f, 195.f / 256.f, 140.f / 256.f, 26);
        g_pCryWolfInterface->Render(250 + Delay_Add_inter, 188, 110, 28, 0.f, 0.f, 110.f / 128.f, 27.f / 32.f, 27);

        if (Delay_Add_inter == 0)
        {
            int Exp_val[9] = { 0,0,0,0,0,0,0,0,0 }, Exp_Dummy = 0, Val = 0;

            g_pCryWolfInterface->Render(250 + 120, 188, 29, 28, 0.f, 0.f, 29.f / 32.f, 27.f / 32.f, 28 + Rank);

            for (int i = 0; i < 9; i++)
            {
                if (Exp >= Val)
                {
                    if (Val > 0)
                    {
                        Exp_Dummy = Exp / Val;
                        Exp_val[8 - i] = Exp_Dummy % 10;
                    }
                    else
                    {
                        Exp_val[8 - i] = Exp % 10;
                        Val = 1;
                    }
                }
                else
                    break;

                Val *= 10;
            }
            int Move_X = 29;

            g_pCryWolfInterface->Render(200 + Move_X, 235, 60, 19, 0.f, 0.f, 60.f / 64.f, 19.f / 32.f, 43);
            g_pCryWolfInterface->Render(250 + 130 + Move_X, 235, 15, 19, 0.f, 0.f, 15.f / 16.f, 19.f / 32.f, 33 + Exp_val[8]);
            g_pCryWolfInterface->Render(250 + 115 + Move_X, 235, 15, 19, 0.f, 0.f, 15.f / 16.f, 19.f / 32.f, 33 + Exp_val[7]);
            g_pCryWolfInterface->Render(250 + 100 + Move_X, 235, 15, 19, 0.f, 0.f, 15.f / 16.f, 19.f / 32.f, 33 + Exp_val[6]);
            g_pCryWolfInterface->Render(250 + 85 + Move_X, 235, 15, 19, 0.f, 0.f, 15.f / 16.f, 19.f / 32.f, 33 + Exp_val[5]);
            g_pCryWolfInterface->Render(250 + 70 + Move_X, 235, 15, 19, 0.f, 0.f, 15.f / 16.f, 19.f / 32.f, 33 + Exp_val[4]);
            g_pCryWolfInterface->Render(250 + 55 + Move_X, 235, 15, 19, 0.f, 0.f, 15.f / 16.f, 19.f / 32.f, 33 + Exp_val[3]);
            g_pCryWolfInterface->Render(250 + 40 + Move_X, 235, 15, 19, 0.f, 0.f, 15.f / 16.f, 19.f / 32.f, 33 + Exp_val[2]);
            g_pCryWolfInterface->Render(250 + 25 + Move_X, 235, 15, 19, 0.f, 0.f, 15.f / 16.f, 19.f / 32.f, 33 + Exp_val[1]);
            g_pCryWolfInterface->Render(250 + 10 + Move_X, 235, 15, 19, 0.f, 0.f, 15.f / 16.f, 19.f / 32.f, 33 + Exp_val[0]);
        }
    }

    if (M34CryWolf1st::Get_State_Only_Elf() == false || M34CryWolf1st::IsCyrWolf1st() == false)
        return true;

    g_pCryWolfInterface->Render(Main[0], Main[1], Main[2], Main[3], 0.f, 0.f, Main[4], Main[5], 3);

    //23,24;

    for (int ia = 0; ia < 5; ia++)
    {
        BYTE Use = (m_AltarState[ia] & 0xf0) >> 4;
        BYTE State = (m_AltarState[ia] & 0x0f);
        if (Use == CRYWOLF_ALTAR_STATE_CONTRACTED)
        {
            if (State == 1)
            {
                g_pCryWolfInterface->Render(Number[ia][0], Number[ia][1], Number[ia][2], Number[ia][3], 0.f, 0.f, Number[ia][4], Number[ia][5], 23);
            }
            else if (State == 2)
            {
                g_pCryWolfInterface->Render(Number[ia][0], Number[ia][1], Number[ia][2], Number[ia][3], 0.f, 0.f, Number[ia][4], Number[ia][5], 24);
            }
            else
            {
                g_pCryWolfInterface->Render(Number[ia][0], Number[ia][1], Number[ia][2], Number[ia][3], 0.f, 0.f, Number[ia][4], Number[ia][5], 25);
            }
        }
        else
        {
            if (State == 1)
            {
                g_pCryWolfInterface->Render(Number[ia][0], Number[ia][1], Number[ia][2], Number[ia][3], 0.f, 0.f, Number[ia][4], Number[ia][5], 7);
            }
            else if (State == 2)
            {
                g_pCryWolfInterface->Render(Number[ia][0], Number[ia][1], Number[ia][2], Number[ia][3], 0.f, 0.f, Number[ia][4], Number[ia][5], 8);
            }
        }
    }

    if (Dark_elf_Num == 0)
    {
        g_pCryWolfInterface->Render(Dark_Elf_Icon[0], Dark_Elf_Icon[1], Dark_Elf_Icon[2], Dark_Elf_Icon[3], 0.f, 0.f, Dark_Elf_Icon[4], Dark_Elf_Icon[5], 6);
    }
    else
    {
        g_pCryWolfInterface->Render(Dark_Elf_Icon[0], Dark_Elf_Icon[1], Dark_Elf_Icon[2], Dark_Elf_Icon[3], 0.f, 0.f, Dark_Elf_Icon[4], Dark_Elf_Icon[5], 5);
    }

    g_pCryWolfInterface->Render(538, 392, 104, 37, 0.f, 0.f, 104.f / 128.f, 36.f / 64.f, 12);

    glColor3f(1.f, 0.6f, 0.3f);
    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(255, 148, 21, 255);
    g_pRenderText->SetBgColor(0);
    swprintf(Text, GlobalText[1948], Dark_elf_Num);
    g_pRenderText->RenderText(582, 359, Text, 0, 0, RT3_WRITE_CENTER);

    if (View_Bal == true)
    {
        if (Deco_Insert < 21.f)
        {
            Deco_Insert += 1.f;
        }
        else
        {
            g_pCryWolfInterface->Render(Val_Icon[0], Val_Icon[1], Val_Icon[2], Val_Icon[3], 0.f, 0.f, Val_Icon[4], Val_Icon[5], 4);

            swprintf(Text, GlobalText[1949]);
            g_pRenderText->RenderText(600, 380, Text, 0, 0, RT3_WRITE_CENTER);

            float Hp = ((67.f / 100.f) * (float)Val_Hp);
            float nx = ((68.f / 100.f) * (float)Val_Hp);

            g_pCryWolfInterface->Render(548, 388, nx, 8, 0.f, 0.f, Hp / 128.f, 8.f / 8.f, 1);
        }
    }
    if (View_Bal == false)
    {
        if (Deco_Insert > 0.f)
        {
            Deco_Insert -= 1.f;
        }
    }

    if (m_bTimeStart == true && m_CrywolfState == CRYWOLF_STATE_START)
    {
        m_iSecond = m_iSecond - (GetTickCount() - m_dwSyncTime);

        if (View_Bal == false)
        {
            glColor3f(1.f, 1.0f, 1.0f);
        }
        else
        {
            glColor3f(1.f, 0.3f, 0.3f);
        }

        if (m_iMinute < 10)
        {
            RenderNumber2D(510 + 60, 384 + 18, 0, 14, 14);
        }
        RenderNumber2D(510 + 70, 384 + 18, m_iMinute, 14, 14);
        if (m_iSecond / 1000 < 10)
        {
            RenderNumber2D(520 + 77, 384 + 18, 0, 14, 14);
        }
        RenderNumber2D(520 + 87, 384 + 18, m_iSecond / 1000, 14, 14);

        m_dwSyncTime = GetTickCount();

        if (m_iMinute <= 0 && m_iSecond <= 0)
        {
            m_bTimeStart = false;
            m_iSecond = 0;
            m_icntTime = 0;
        }
    }
    else
    {
        RenderNumber2D(510 + 60, 384 + 18, 0, 14, 14);
        RenderNumber2D(510 + 70, 384 + 18, 0, 14, 14);
        RenderNumber2D(520 + 77, 384 + 18, 0, 14, 14);
        RenderNumber2D(520 + 87, 384 + 18, 0, 14, 14);
    }

    int HpS = 100 - m_StatueHP;
    float Hp = ((88.f / 100.f) * (float)HpS);
    float nx = ((89.f / 100.f) * (float)HpS);
    RenderImage(IMAGE_MVP_INTERFACE + 9, 548 + nx, 323, 89.f - nx, 30, Hp / 128.f, 0.f, ((((88.f / 100.f) * (float)(100.f - HpS))) / 128.f), 29.f / 32.f);

    M34CryWolf1st::RenderNoticesCryWolf();

    if (M34CryWolf1st::Get_State_Only_Elf() == false || M34CryWolf1st::IsCyrWolf1st() == false)
        return true;

    int Yes = 250;
    int No = 330;

    g_pRenderText->SetTextColor(255, 148, 21, 255);
    g_pRenderText->SetBgColor(0x00000000);

    if (Message_Box == 1)
    {
        g_pCryWolfInterface->Render(212, 206, 209, 80, 0.f, 0.f, 206.f / 256.f, 77.f / 128.f, 22);
        if (MouseX > No && MouseX < No + 54 && MouseY > 250 && MouseY < 250 + 30)
        {
            if (Button_Down == 1)
            {
                g_pCryWolfInterface->Render(No, 250, 54, 30, 0.f, 0.f, 53.f / 64.f, 30.f / 32.f, 15);

                Message_Box = 0;
                Button_Down = 0;
            }
            else
            {
                g_pCryWolfInterface->Render(No, 250, 54, 30, 0.f, 0.f, 53.f / 64.f, 30.f / 32.f, 14);
            }
        }
        else
        {
            g_pCryWolfInterface->Render(No, 250, 54, 30, 0.f, 0.f, 53.f / 64.f, 30.f / 32.f, 13);
        }

        if (MouseX > Yes && MouseX < Yes + 54 && MouseY > 250 && MouseY < 250 + 30)
        {
            if (Button_Down == 2)
            {
                g_pCryWolfInterface->Render(Yes, 250, 54, 30, 0.f, 0.f, 53.f / 64.f, 30.f / 32.f, 21);
                Message_Box = 0;
                Button_Down = 0;
            }
            else
            {
                g_pCryWolfInterface->Render(Yes, 250, 54, 30, 0.f, 0.f, 53.f / 64.f, 30.f / 32.f, 20);
            }
        }
        else
        {
            g_pCryWolfInterface->Render(Yes, 250, 54, 30, 0.f, 0.f, 53.f / 64.f, 30.f / 32.f, 19);
        }

        if (Box_String[0][0] != NULL)
        {
            int Y_loc = 239;
            if (Box_String[1][0] != NULL)
                Y_loc = 227;
            g_pRenderText->RenderText(317, Y_loc, Box_String[0], 0, 0, RT3_WRITE_CENTER);
        }
        if (Box_String[1][0] != NULL)
        {
            g_pRenderText->RenderText(317, 238, Box_String[1], 0, 0, RT3_WRITE_CENTER);
        }
    }
    else if (Message_Box == 2)
    {
        g_pCryWolfInterface->Render(212, 206, 209, 80, 0.f, 0.f, 206.f / 256.f, 77.f / 128.f, 22);

        if (MouseX > 290 && MouseX < 290 + 54 && MouseY > 250 && MouseY < 250 + 30)
        {
            if (Button_Down == 3)
            {
                g_pCryWolfInterface->Render(290, 250, 54, 30, 0.f, 0.f, 53.f / 64.f, 30.f / 32.f, 18);

                Message_Box = 0;
                Button_Down = 0;
            }
            else
            {
                g_pCryWolfInterface->Render(290, 250, 54, 30, 0.f, 0.f, 53.f / 64.f, 30.f / 32.f, 17);
            }
        }
        else
        {
            g_pCryWolfInterface->Render(290, 250, 54, 30, 0.f, 0.f, 53.f / 64.f, 30.f / 32.f, 16);
        }

        if (Box_String[0][0] != NULL)
        {
            int Y_loc = 239;
            if (Box_String[1][0] != NULL)
                Y_loc = 227;
            g_pRenderText->RenderText(317, Y_loc, Box_String[0], 0, 0, RT3_WRITE_CENTER);
        }

        if (Box_String[1][0] != NULL)
        {
            g_pRenderText->RenderText(317, 238, Box_String[1], 0, 0, RT3_WRITE_CENTER);
        }
    }

    DisableAlphaBlend();

    return true;
}

bool SEASON3B::CNewUICryWolf::Update()
{
    return true;
}

float SEASON3B::CNewUICryWolf::ConvertX(float x)
{
    return x * (float)WindowWidth / 640.f;
}

float SEASON3B::CNewUICryWolf::ConvertY(float y)
{
    return y * (float)WindowHeight / 480.f;
}

bool SEASON3B::CNewUICryWolf::Render(int Posx, int Posy, int nPosx, int nPosy, float u, float v, float su, float sv, int Index, bool Scale, bool StartScale, float Alpha)
{
    glColor4f(1.f, 1.f, 1.f, Alpha);

    RenderImage(IMAGE_MVP_INTERFACE + Index, Posx, Posy, nPosx, nPosy, u, v, su, sv);

    return true;
}

void SEASON3B::CNewUICryWolf::SetTime(int iHour, int iMinute)
{
    m_iHour = iHour;
    if (m_iMinute != iMinute)
    {
        m_iSecond = 60000;
        m_icntTime = 1;
        m_bTimeStart = true;
    }
    else if (m_icntTime == 1)
    {
        m_iSecond = 40000;
        m_icntTime++;
    }
    else if (m_icntTime == 2)
    {
        m_iSecond = 20000;
        m_icntTime = 0;
    }
    m_iMinute = iMinute;
}

void SEASON3B::CNewUICryWolf::InitTime()
{
    m_dwSyncTime = GetTickCount();
}

void SEASON3B::CNewUICryWolf::LoadImages()
{
    LoadBitmap(L"Interface\\in_bar.tga", IMAGE_MVP_INTERFACE, GL_LINEAR);
    LoadBitmap(L"Interface\\in_bar2.jpg", IMAGE_MVP_INTERFACE + 1, GL_LINEAR);
    LoadBitmap(L"Interface\\in_deco.tga", IMAGE_MVP_INTERFACE + 2, GL_LINEAR);
    LoadBitmap(L"Interface\\in_main-New.tga", IMAGE_MVP_INTERFACE + 3, GL_LINEAR);
    LoadBitmap(L"Interface\\in_main_icon_bal1.tga", IMAGE_MVP_INTERFACE + 4, GL_LINEAR);
    LoadBitmap(L"Interface\\in_main_icon_dl1.tga", IMAGE_MVP_INTERFACE + 5, GL_LINEAR);
    LoadBitmap(L"Interface\\in_main_icon_dl2.tga", IMAGE_MVP_INTERFACE + 6, GL_LINEAR);
    LoadBitmap(L"Interface\\in_main_number1.tga", IMAGE_MVP_INTERFACE + 7, GL_LINEAR);
    LoadBitmap(L"Interface\\in_main_number2.tga", IMAGE_MVP_INTERFACE + 8, GL_LINEAR);
    LoadBitmap(L"Interface\\in_main2-New.tga", IMAGE_MVP_INTERFACE + 9, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_failure.tga", IMAGE_MVP_INTERFACE + 10, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_success.tga", IMAGE_MVP_INTERFACE + 11, GL_LINEAR);
    LoadBitmap(L"Interface\\t_main-New.tga", IMAGE_MVP_INTERFACE + 12, GL_LINEAR);
    LoadBitmap(L"Interface\\m_b_no1.tga", IMAGE_MVP_INTERFACE + 13, GL_LINEAR);
    LoadBitmap(L"Interface\\m_b_no2.tga", IMAGE_MVP_INTERFACE + 14, GL_LINEAR);
    LoadBitmap(L"Interface\\m_b_no3.tga", IMAGE_MVP_INTERFACE + 15, GL_LINEAR);
    LoadBitmap(L"Interface\\m_b_ok1.tga", IMAGE_MVP_INTERFACE + 16, GL_LINEAR);
    LoadBitmap(L"Interface\\m_b_ok2.tga", IMAGE_MVP_INTERFACE + 17, GL_LINEAR);
    LoadBitmap(L"Interface\\m_b_ok3.tga", IMAGE_MVP_INTERFACE + 18, GL_LINEAR);
    LoadBitmap(L"Interface\\m_b_yes1.tga", IMAGE_MVP_INTERFACE + 19, GL_LINEAR);
    LoadBitmap(L"Interface\\m_b_yes2.tga", IMAGE_MVP_INTERFACE + 20, GL_LINEAR);
    LoadBitmap(L"Interface\\m_b_yes3.tga", IMAGE_MVP_INTERFACE + 21, GL_LINEAR);
    LoadBitmap(L"Interface\\m_main.tga", IMAGE_MVP_INTERFACE + 22, GL_LINEAR);
    LoadBitmap(L"Interface\\in_main_number1_1.tga", IMAGE_MVP_INTERFACE + 23, GL_LINEAR);
    LoadBitmap(L"Interface\\in_main_number2_1.tga", IMAGE_MVP_INTERFACE + 24, GL_LINEAR);
    LoadBitmap(L"Interface\\in_main_number0_2.tga", IMAGE_MVP_INTERFACE + 25, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_table.tga", IMAGE_MVP_INTERFACE + 26, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_rank.tga", IMAGE_MVP_INTERFACE + 27, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_D.tga", IMAGE_MVP_INTERFACE + 28, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_C.tga", IMAGE_MVP_INTERFACE + 29, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_B.tga", IMAGE_MVP_INTERFACE + 30, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_A.tga", IMAGE_MVP_INTERFACE + 31, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_S.tga", IMAGE_MVP_INTERFACE + 32, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_0.tga", IMAGE_MVP_INTERFACE + 33, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_1.tga", IMAGE_MVP_INTERFACE + 34, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_2.tga", IMAGE_MVP_INTERFACE + 35, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_3.tga", IMAGE_MVP_INTERFACE + 36, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_4.tga", IMAGE_MVP_INTERFACE + 37, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_5.tga", IMAGE_MVP_INTERFACE + 38, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_6.tga", IMAGE_MVP_INTERFACE + 39, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_7.tga", IMAGE_MVP_INTERFACE + 40, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_8.tga", IMAGE_MVP_INTERFACE + 41, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_9.tga", IMAGE_MVP_INTERFACE + 42, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_exp.tga", IMAGE_MVP_INTERFACE + 43, GL_LINEAR);
    LoadBitmap(L"Interface\\m_main_rank.tga", IMAGE_MVP_INTERFACE + 44, GL_LINEAR);
}

void SEASON3B::CNewUICryWolf::UnloadImages()
{
    DeleteBitmap(IMAGE_MVP_INTERFACE);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 1);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 2);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 3);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 4);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 5);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 6);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 7);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 8);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 9);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 10);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 11);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 12);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 13);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 14);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 15);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 16);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 17);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 18);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 19);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 20);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 21);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 22);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 23);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 24);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 25);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 26);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 27);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 28);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 29);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 30);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 31);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 32);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 33);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 34);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 35);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 36);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 37);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 38);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 39);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 40);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 41);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 42);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 43);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 44);
}