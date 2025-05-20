// NewUICursedTempleSystem.cpp: implementation of the CNewUICursedTempleSystem class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUICursedTempleSystem.h"
#include "NewUICommonMessageBox.h"
#include "NewUICustomMessageBox.h"
#include "UIBaseDef.h"
#include "DSPlaySound.h"
#include "ZzzInfomation.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"

#include "CSItemOption.h"
#include "CSChaosCastle.h"
#include "UIControls.h"
#include "ZzzAI.h"
#include "ZzzEffect.h"
#include "w_CursedTemple.h"
#include "MapManager.h"
#include "NewUIInventoryCtrl.h"
#include "NewUISystem.h"
#include "SkillManager.h"

extern int TextNum;
extern wchar_t TextList[50][100];
extern int  TextListColor[50];

using namespace SEASON3B;

namespace
{
    const int HolyItemNpc = 380;
    const int AlliedNpc = 381;
    const int IllusionNpc = 382;
    const int AlliedHolyItemBoxNpc = 383;
    const int IllusionHolyItemBoxNpc = 384;

    const int	AXIS_X = 0;
    const int	AXIS_Y = 1;
    const float PROGRESSTIME = 10000.0f;

    //#ifdef _DEBUG
    const float posX[7] =
    {
        146.0f, 192.0f, 200.0f, 138.0f, 128.0f, 210.0f, 170.0f,
    };

    const float posY[7] =
    {
        42.0f, 128.0f, 116.0f, 54.0f, 126.0f, 44.0f, 84.0f,
    };
    //#endif //_DEBUG

    float MiniMapPos(float pointX, float pointY, float scale, int aXis)
    {
        float minmapframeposX = 464.f, minmapframeposY = 299.f;

        if (aXis == AXIS_X)
        {
            float ridY = posY[6] - pointY;
            return ((pointX - ridY) / scale) + minmapframeposX;
        }
        else
        {
            float ridX = posX[6] - pointX;
            return (((125 - (pointY + ridX))) / scale) + minmapframeposY;
        }
    }

    bool CreateCursedTempleSkillEffect(CHARACTER* c, int skillindex, int SubType)
    {
        if (!c) return false;

        OBJECT* o = &c->Object;
        BMD* b = &Models[o->Type];

        vec3_t vRelativePos, vtaWorldPos, vLight, vAngle;

        switch (skillindex)
        {
        case AT_SKILL_CURSED_TEMPLE_PRODECTION:
        {
            DeleteEffect(MODEL_CURSEDTEMPLE_PRODECTION_SKILL, o);

            if (o->Live && !SearchEffect(MODEL_CURSEDTEMPLE_PRODECTION_SKILL, o))
            {
                Vector(0.3f, 0.3f, 0.8f, o->Light);
                CreateEffect(MODEL_CURSEDTEMPLE_PRODECTION_SKILL, o->Position, o->Angle, o->Light, 0, o);
                CreateEffect(MODEL_SHIELD_CRASH, o->Position, o->Angle, o->Light, 1, o);
                CreateEffect(BITMAP_SHOCK_WAVE, o->Position, o->Angle, o->Light, 10, o);
            }
        }
        return true;
        case AT_SKILL_CURSED_TEMPLE_RESTRAINT:
        {
            DeleteEffect(MODEL_CURSEDTEMPLE_RESTRAINT_SKILL, o);

            if (o->Live && !SearchEffect(MODEL_CURSEDTEMPLE_RESTRAINT_SKILL, o))
            {
                Vector(0.4f, 1.f, 0.8f, vLight);
                CreateEffect(BITMAP_SHOCK_WAVE, o->Position, o->Angle, vLight, 5);
                CreateEffect(MODEL_SHIELD_CRASH, o->Position, o->Angle, vLight, 1, o);

                for (int i = 1; i < 40; i++)
                {
                    vec3_t Position, Angle, Light;
                    VectorCopy(o->Position, Position);
                    Vector(0.f, 0.f, i * (90.f / 4.f), Angle);
                    Vector(0.6f, 1.f, 0.8f, Light);

                    Position[0] += cosf(Q_PI / 180.f * 10.0f * i) * (float)(rand() % 240 + 140);
                    Position[1] += sinf(Q_PI / 180.f * 10.0f * i) * (float)(rand() % 240 + 140);
                    Position[2] += rand() % 200 + 100;

                    Vector(0.f, 0.f, 0.f, vRelativePos);
                    VectorCopy(o->Position, b->BodyOrigin);

                    b->TransformPosition(o->BoneTransform[rand() % 50], vRelativePos, vtaWorldPos, true);

                    Vector(0.7f, 1.f, 0.4f, Light);

                    if (rand() % 2 == 1)
                    {
                        CreateJoint(BITMAP_JOINT_ENERGY, Position, vtaWorldPos, Angle, 44, o, 15.f, -1, 0, 0, -1, Light);
                    }
                }
            }
        }
        return true;
        case AT_SKILL_CURSED_TEMPLE_SUBLIMATION:
        {
            if (SubType == 0)
            {
                Vector(300.f, 0.f, 0.f, vAngle);

                Vector(0.8f, 0.3f, 0.3f, vLight);
                CreateEffect(MODEL_SKILL_INFERNO, o->Position, o->Angle, vLight, 8, o, 10, 0);

                Vector(0.8f, 0.3f, 0.3f, vLight);
                CreateEffect(MODEL_SHIELD_CRASH, o->Position, o->Angle, vLight, 2, o);

                Vector(0.3f, 0.3f, 0.8f, vLight);
                CreateEffect(BITMAP_SHOCK_WAVE, o->Position, o->Angle, vLight, 5, o);

                Vector(150.f, 0.f, 0.f, vRelativePos);
                Vector(0.8f, 0.3f, 0.3f, vLight);
                VectorCopy(o->Position, b->BodyOrigin);

                b->TransformPosition(o->BoneTransform[20], vRelativePos, vtaWorldPos, true);
                CreateParticle(BITMAP_CURSEDTEMPLE_EFFECT_MASKER, vtaWorldPos, o->Angle, vLight, 0, 1.5f);

                for (int i = 1; i < 40; i++)
                {
                    vec3_t Position, Angle, Light;
                    VectorCopy(o->Position, Position);
                    Vector(0.f, 0.f, 0.f, Angle);

                    Position[0] += cosf(Q_PI / 180.f * 10.0f * i) * (float)(rand() % 50 + 20);
                    Position[1] += sinf(Q_PI / 180.f * 10.0f * i) * (float)(rand() % 50 + 20);
                    Position[2] -= 120.f;

                    if (rand() % 3 == 1)
                    {
                        if (rand() % 2 == 1)
                        {
                            Vector(0.3f, 0.3f, 0.8f, Light);
                        }
                        else
                        {
                            Vector(1.f, 0.5f, 0.5f, Light);
                        }
                        CreateParticle(BITMAP_EFFECT, Position, Angle, Light, 2);
                    }
                }
            }
            else
            {
                Vector(150.f, 0.f, 0.f, vRelativePos);
                Vector(300.f, 0.f, 0.f, vAngle);

                Vector(0.8f, 0.3f, 0.3f, vLight);
                CreateEffect(MODEL_SKILL_INFERNO, o->Position, o->Angle, vLight, 8, o, 10, 0);

                Vector(0.8f, 0.3f, 0.3f, vLight);
                CreateEffect(MODEL_SHIELD_CRASH, o->Position, o->Angle, vLight, 2, o);

                Vector(0.3f, 0.3f, 0.8f, vLight);
                CreateEffect(BITMAP_SHOCK_WAVE, o->Position, o->Angle, vLight, 5, o);

                VectorCopy(o->Position, b->BodyOrigin);

                Vector(0.6f, 0.0f, 0.0f, vLight);
                b->TransformPosition(o->BoneTransform[31], vRelativePos, vtaWorldPos, true);
                CreateEffect(MODEL_FENRIR_THUNDER, vtaWorldPos, o->Angle, vLight, 2, o);
                CreateEffect(MODEL_FENRIR_THUNDER, vtaWorldPos, o->Angle, vLight, 2, o);
                CreateEffect(MODEL_FENRIR_THUNDER, vtaWorldPos, o->Angle, vLight, 2, o);
                CreateParticle(BITMAP_CLUD64, vtaWorldPos, o->Angle, vLight, 4, 1.f);

                b->TransformPosition(o->BoneTransform[40], vRelativePos, vtaWorldPos, true);
                CreateEffect(MODEL_FENRIR_THUNDER, vtaWorldPos, o->Angle, vLight, 2, o);
                CreateEffect(MODEL_FENRIR_THUNDER, vtaWorldPos, o->Angle, vLight, 2, o);
                CreateEffect(MODEL_FENRIR_THUNDER, vtaWorldPos, o->Angle, vLight, 2, o);
                CreateEffect(MODEL_FENRIR_THUNDER, vtaWorldPos, o->Angle, vLight, 2, o);
                CreateParticle(BITMAP_CLUD64, vtaWorldPos, o->Angle, vLight, 4, 1.f);
            }
        }
        return true;
        }
        return false;
    }

    void DrawText(wchar_t* text, int textposx, int textposy, DWORD textcolor, DWORD textbackcolor, int textsort, float fontboxwidth, bool isbold)
    {
        if (isbold)
        {
            g_pRenderText->SetFont(g_hFontBold);
        }
        else
        {
            g_pRenderText->SetFont(g_hFont);
        }

        DWORD backuptextcolor = g_pRenderText->GetTextColor();
        DWORD backuptextbackcolor = g_pRenderText->GetBgColor();

        g_pRenderText->SetTextColor(textcolor);
        g_pRenderText->SetBgColor(textbackcolor);
        g_pRenderText->RenderText(textposx, textposy, text, fontboxwidth, 0, textsort);
        g_pRenderText->SetTextColor(backuptextcolor);
        g_pRenderText->SetBgColor(backuptextbackcolor);
    }
};

bool SEASON3B::CNewUICursedTempleSystem::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_CURSEDTEMPLE_GAMESYSTEM, this);

    SetPos(x, y);

    SetButtonInfo();

    Show(false);

    return true;
}

SEASON3B::CNewUICursedTempleSystem::CNewUICursedTempleSystem() : m_pNewUIMng(NULL)
{
    Initialize();
}

SEASON3B::CNewUICursedTempleSystem::~CNewUICursedTempleSystem()
{
    Destroy();
}

void SEASON3B::CNewUICursedTempleSystem::Initialize()
{
    LoadImages();

    ResetCursedTempleSystemInfo();
}

void SEASON3B::CNewUICursedTempleSystem::Destroy()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void SEASON3B::CNewUICursedTempleSystem::LoadImages()
{
    //minimap
    LoadBitmap(L"Interface\\newui_ctminmapframe.tga", IMAGE_CURSEDTEMPLESYSTEM_MINIMAPFRAME, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_ctminmap.jpg", IMAGE_CURSEDTEMPLESYSTEM_MINIMAP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_Bt_clearness_illusion.jpg", IMAGE_CURSEDTEMPLESYSTEM_MINIMAPALPBTN, GL_LINEAR);

    //minimapicon
    LoadBitmap(L"Interface\\newui_ctminmap_Relic.tga", IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_HOLYITEM_PC, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\newui_ctminmap_TeamA_box.tga", IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ILLUSION_HOLYITEM, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\newui_ctminmap_TeamA_member.tga", IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ILLUSION_PC, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\newui_ctminmap_TeamA_npc.tga", IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ILLUSION_NPC, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\newui_ctminmap_TeamB_box.tga", IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ALLIED_HOLYITEM, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\newui_ctminmap_TeamB_member.tga", IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ALLIED_PC, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\newui_ctminmap_TeamB_npc.tga", IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ALLIED_NPC, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\newui_ctminmap_Hero.tga", IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_HERO, GL_LINEAR, GL_CLAMP_TO_EDGE);

    //skill
    LoadBitmap(L"Interface\\newui_ctskillframe.tga", IMAGE_CURSEDTEMPLESYSTEM_SKILLFRAME, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_ctskillup.jpg", IMAGE_CURSEDTEMPLESYSTEM_SKILLUPBT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_ctskilldown.jpg", IMAGE_CURSEDTEMPLESYSTEM_SKILLDOWNBT, GL_LINEAR);

    //gametime
    LoadBitmap(L"Interface\\newui_ctgametimeframe.tga", IMAGE_CURSEDTEMPLESYSTEM_GAMETIME, GL_LINEAR);

    wchar_t buff[100];

    //score
    for (int i = 0; i < 10; ++i)
    {
        swprintf(buff, L"Interface\\newui_ctscorealliednum%d.tga", i);
        LoadBitmap(buff, IMAGE_CURSEDTEMPLESYSTEM_SCORE_ALLIED_NUMBER + i, GL_LINEAR);
    }
    for (int j = 0; j < 10; ++j)
    {
        swprintf(buff, L"Interface\\newui_ctscoreillusionnum%d.tga", j);
        LoadBitmap(buff, IMAGE_CURSEDTEMPLESYSTEM_SCORE_ILLUSION_NUMBER + j, GL_LINEAR);
    }
    LoadBitmap(L"Interface\\newui_ctscorevs0.tga", IMAGE_CURSEDTEMPLESYSTEM_SCORE_VS0, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_ctscorevs1.tga", IMAGE_CURSEDTEMPLESYSTEM_SCORE_VS1, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_ctscorealliedgaail.tga", IMAGE_CURSEDTEMPLESYSTEM_SCORE_ALLIED_GAAIL, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_ctscoreillsiongaail.tga", IMAGE_CURSEDTEMPLESYSTEM_SCORE_ILLUSION_GAAIL, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_ctscoreleft.tga", IMAGE_CURSEDTEMPLESYSTEM_SCORE_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_ctscoreright.tga", IMAGE_CURSEDTEMPLESYSTEM_SCORE_RIGHT, GL_LINEAR);

    //prorogress, npctalk
    LoadBitmap(L"Interface\\newui_msgbox_top.tga", IMAGE_CURSEDTEMPLESYSTEM_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_msgbox_middle.tga", IMAGE_CURSEDTEMPLESYSTEM_MIDDLE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_msgbox_bottom.tga", IMAGE_CURSEDTEMPLESYSTEM_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_CURSEDTEMPLESYSTEM_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_btn_empty_very_small.tga", IMAGE_CURSEDTEMPLESYSTEM_BTN, GL_LINEAR);

    LoadBitmap(L"Interface\\newui_skill2.jpg", IMAGE_SKILL2, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_non_skill2.jpg", IMAGE_NON_SKILL2, GL_LINEAR);
}

void SEASON3B::CNewUICursedTempleSystem::UnloadImages()
{
    //prorogress, npctalk
    DeleteBitmap(IMAGE_CURSEDTEMPLESYSTEM_BTN);
    DeleteBitmap(IMAGE_CURSEDTEMPLESYSTEM_BACK);
    DeleteBitmap(IMAGE_CURSEDTEMPLESYSTEM_BOTTOM);
    DeleteBitmap(IMAGE_CURSEDTEMPLESYSTEM_MIDDLE);
    DeleteBitmap(IMAGE_CURSEDTEMPLESYSTEM_TOP);

    //score
    DeleteBitmap(IMAGE_CURSEDTEMPLESYSTEM_SCORE_RIGHT);
    DeleteBitmap(IMAGE_CURSEDTEMPLESYSTEM_SCORE_LEFT);
    DeleteBitmap(IMAGE_CURSEDTEMPLESYSTEM_SCORE_ILLUSION_GAAIL);
    DeleteBitmap(IMAGE_CURSEDTEMPLESYSTEM_SCORE_ALLIED_GAAIL);
    DeleteBitmap(IMAGE_CURSEDTEMPLESYSTEM_SCORE_VS1);
    DeleteBitmap(IMAGE_CURSEDTEMPLESYSTEM_SCORE_VS0);
    for (int i = 0; i < 10; ++i)
    {
        DeleteBitmap(IMAGE_CURSEDTEMPLESYSTEM_SCORE_ALLIED_NUMBER + i);
    }

    for (int j = 0; j < 10; ++j)
    {
        DeleteBitmap(IMAGE_CURSEDTEMPLESYSTEM_SCORE_ALLIED_NUMBER + j);
    }

    //gametiem
    DeleteBitmap(IMAGE_CURSEDTEMPLESYSTEM_GAMETIME);

    //skill
    DeleteBitmap(IMAGE_CURSEDTEMPLESYSTEM_SKILLDOWNBT);
    DeleteBitmap(IMAGE_CURSEDTEMPLESYSTEM_SKILLUPBT);
    DeleteBitmap(IMAGE_CURSEDTEMPLESYSTEM_SKILLFRAME);

    //minmapicon
    DeleteBitmap(IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_HERO);
    DeleteBitmap(IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ALLIED_NPC);
    DeleteBitmap(IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ALLIED_PC);
    DeleteBitmap(IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ALLIED_HOLYITEM);
    DeleteBitmap(IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ILLUSION_NPC);
    DeleteBitmap(IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ILLUSION_PC);
    DeleteBitmap(IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ILLUSION_HOLYITEM);
    DeleteBitmap(IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_HOLYITEM_PC);

    //minimap
    DeleteBitmap(IMAGE_CURSEDTEMPLESYSTEM_MINIMAPALPBTN);
    DeleteBitmap(IMAGE_CURSEDTEMPLESYSTEM_MINIMAP);
    DeleteBitmap(IMAGE_CURSEDTEMPLESYSTEM_MINIMAPFRAME);

    DeleteBitmap(IMAGE_SKILL2);
    DeleteBitmap(IMAGE_NON_SKILL2);
}

void SEASON3B::CNewUICursedTempleSystem::ResetCursedTempleSystemInfo()
{
    m_EventMapTime = 0;
    m_HolyItemPlayerIndex = 0xffff;
    m_HolyItemPlayerPosX = 0;
    m_HolyItemPlayerPosY = 0;
    m_AlliedPoint = 0;
    m_IllusionPoint = 0;
    m_MyTeam = SEASON3A::eTeam_Count;
    m_CursedTempleMyTeamCount = 0;
    m_Scale = 2.5f;
    m_Alph = 1.0f;
    m_SkillPoint = 0;
    m_IsTutorialStep = false;
    m_TutorialStepState = 0;
    m_TutorialStepTime = 0;

    EndScoreEffect();

    for (int i = 0; i < MAX_PARTYS; ++i)
    {
        m_CursedTempleMyTeam[i].wPartyUserIndex = 0xffff;
        m_CursedTempleMyTeam[i].btX = 0;
        m_CursedTempleMyTeam[i].btY = 0;
        m_CursedTempleMyTeam[i].byMapNumber = 0xff;
    }
}

void SEASON3B::CNewUICursedTempleSystem::StartScoreEffect()
{
    m_StartScoreEffectTime = 0;
    m_ScoreEffectAlph = 0.0f;
    m_ScoreEffectState = 0;
    m_IsScoreEffect = true;
}

void SEASON3B::CNewUICursedTempleSystem::EndScoreEffect()
{
    m_StartScoreEffectTime = 0;
    m_ScoreEffectAlph = 0.0f;
    m_ScoreEffectState = 0;
    m_IsScoreEffect = false;
}

void SEASON3B::CNewUICursedTempleSystem::StartTutorialStep()
{
    m_IsTutorialStep = true;
    m_TutorialStepState = 0;
    m_TutorialStepTime = timeGetTime();
}

void SEASON3B::CNewUICursedTempleSystem::EndTutorialStep()
{
    m_IsTutorialStep = false;
    m_TutorialStepState = 0;
    m_TutorialStepTime = 0;
}

SEASON3A::eCursedTempleTeam SEASON3B::CNewUICursedTempleSystem::GetMyTeam()
{
    return m_MyTeam;
}

void SEASON3B::CNewUICursedTempleSystem::SetButtonInfo()
{
    m_Button[CURSEDTEMPLERESULT_ALPH].ChangeButtonImgState(true, IMAGE_CURSEDTEMPLESYSTEM_MINIMAPALPBTN, true);
    m_Button[CURSEDTEMPLERESULT_ALPH].ChangeButtonInfo(513, 238, 38, 24);

    m_Button[CURSEDTEMPLERESULT_SKILLUP].ChangeButtonImgState(true, IMAGE_CURSEDTEMPLESYSTEM_SKILLUPBT, true);
    m_Button[CURSEDTEMPLERESULT_SKILLUP].ChangeButtonInfo(0, 0, 15, 13);

    m_Button[CURSEDTEMPLERESULT_SKILLDOWN].ChangeButtonImgState(true, IMAGE_CURSEDTEMPLESYSTEM_SKILLDOWNBT, true);
    m_Button[CURSEDTEMPLERESULT_SKILLDOWN].ChangeButtonInfo(0, 0, 15, 13);
}

bool SEASON3B::CNewUICursedTempleSystem::CheckInventoryHolyItem(CHARACTER* c)
{
    CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();

    if (pPickedItem)
    {
        ITEM* pickitem = pPickedItem->GetItem();

        if (pickitem)
        {
            if (pickitem->Type == ITEM_POTION + 64)
            {
                return true;
            }
        }
    }

    if (c == Hero)
    {
        return g_pMyInventory->IsItem(ITEM_POTION + 64);
    }
    else
    {
        if (m_HolyItemPlayerIndex == 0xffff)
            return false;

        WORD holyitemcharacterindex = FindCharacterIndex(m_HolyItemPlayerIndex);

        if (holyitemcharacterindex == MAX_CHARACTERS_CLIENT)
        {
            return false;
        }

        CHARACTER* pc = &CharactersClient[holyitemcharacterindex];

        if (c == pc)
        {
            return true;
        }
    }

    return false;
}

bool SEASON3B::CNewUICursedTempleSystem::CheckTalkProgressNpc(DWORD npcindex, DWORD npckey)
{
    std::list<DWORD>				progressnpcindexlist;
    progressnpcindexlist.push_back(HolyItemNpc);
    progressnpcindexlist.push_back(AlliedHolyItemBoxNpc);
    progressnpcindexlist.push_back(IllusionHolyItemBoxNpc);

    for (auto iter = progressnpcindexlist.begin();
         iter != progressnpcindexlist.end();)
    {
        auto curiter = iter;
        ++iter;
        DWORD progressnpcindex = *curiter;

        if (progressnpcindex == npcindex)
        {
            if (progressnpcindex == AlliedHolyItemBoxNpc || progressnpcindex == IllusionHolyItemBoxNpc)
            {
                if (!(progressnpcindex == AlliedHolyItemBoxNpc && SEASON3A::eTeam_Allied == m_MyTeam)
                    && !(progressnpcindex == IllusionHolyItemBoxNpc && SEASON3A::eTeam_Illusion == m_MyTeam))
                {
                    return false;
                }

                if (CheckInventoryHolyItem(Hero))
                {
                    SEASON3B::CCursedTempleProgressMsgBox* pMsgBox = NULL;
                    SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCursedTempleHolicItemSaveLayout), &pMsgBox);
                    if (pMsgBox)
                    {
                        pMsgBox->SetNpcIndex(npckey);
                    }
                }
                else
                {
                    g_pSystemLogBox->AddText(GlobalText[2092], SEASON3B::TYPE_ERROR_MESSAGE);
                }
            }
            else
            {
                SEASON3B::CCursedTempleProgressMsgBox* pMsgBox = NULL;
                SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCursedTempleHolicItemGetLayout), &pMsgBox);
                if (pMsgBox)
                {
                    pMsgBox->SetNpcIndex(npckey);
                }
            }

            return true;
        }
    }

    if (npcindex == AlliedNpc || npcindex == IllusionNpc)
    {
        if (g_MessageBox->IsEmpty())
        {
            if (npcindex == AlliedNpc)
                SEASON3B::CreateOkMessageBox(GlobalText[2359]);

            if (npcindex == IllusionNpc)
                SEASON3B::CreateOkMessageBox(GlobalText[2362]);
        }

        return true;
    }

    SocketClient->ToGameServer()->SendTalkToNpcRequest(npckey);
    return false;
}

bool SEASON3B::CNewUICursedTempleSystem::CheckHeroSkillType(int operatortype)
{
    if (operatortype == 0)
    {
        if (Hero->m_CursedTempleCurSkill >= AT_SKILL_CURSED_TEMPLE_SUBLIMATION) return false;
        else return true;
    }
    else
    {
        if (Hero->m_CursedTempleCurSkill <= AT_SKILL_CURSED_TEMPLE_PRODECTION) return false;
        else return true;
    }
}

bool SEASON3B::CNewUICursedTempleSystem::CheckDragonRender()
{
    if (IsVisible())
    {
        return Hero->SafeZone ? false : true;
    }
    else
    {
        return false;
    }
}

bool SEASON3B::CNewUICursedTempleSystem::IsCursedTempleSkillKey(DWORD selectcharacterindex)
{
    if (Hero->m_CursedTempleCurSkillPacket) return false;

    return SEASON3B::IsRepeat(VK_SHIFT);
}

bool SEASON3B::CNewUICursedTempleSystem::UpdateMouseEvent()
{
    if (m_Button[CURSEDTEMPLERESULT_ALPH].UpdateMouseEvent())
    {
        if (m_Alph != 1.0f)
        {
            m_Alph = 1.0f;
        }
        else
        {
            m_Alph = 0.51f;
        }

        return false;
    }

    if (MouseWheel >= 1 || m_Button[CURSEDTEMPLERESULT_SKILLUP].UpdateMouseEvent())
    {
        if (CheckHeroSkillType())
        {
            Hero->m_CursedTempleCurSkill += 1;
        }

        MouseWheel = 0;

        return false;
    }

    if (MouseWheel <= -1 || m_Button[CURSEDTEMPLERESULT_SKILLDOWN].UpdateMouseEvent())
    {
        if (CheckHeroSkillType(1))
        {
            Hero->m_CursedTempleCurSkill -= 1;
        }

        MouseWheel = 0;

        return false;
    }

    if (CheckMouseIn(512, 232.f - 53.f, 128, 255))
    {
        return false;
    }

    return true;
}

bool SEASON3B::CNewUICursedTempleSystem::UpdateKeyEvent()
{
    return true;
}

void SEASON3B::CNewUICursedTempleSystem::UpdateScore()
{
    if (!m_IsScoreEffect) return;

    switch (m_ScoreEffectState)
    {
    case 0:
    {
        m_ScoreEffectAlph += 0.015f;
        if (1.0f < m_ScoreEffectAlph)
        {
            m_ScoreEffectState = 1;
            m_ScoreEffectAlph = 1.0f;
            m_StartScoreEffectTime = timeGetTime();
        }
    }
    break;
    case 1:
    {
        DWORD curTime = timeGetTime();
        if (curTime - m_StartScoreEffectTime >= 5000)
        {
            m_ScoreEffectState = 2;
        }
    }
    break;
    case 2:
    {
        m_ScoreEffectAlph -= 0.015f;
        if (0.0f > m_ScoreEffectAlph)
        {
            EndScoreEffect();
        }
    }
    break;
    }
}

void SEASON3B::CNewUICursedTempleSystem::UpdateTutorialStep()
{
    if (!m_IsTutorialStep) return;

    DWORD curTime = timeGetTime();
    if (curTime - m_TutorialStepTime >= 10000)
    {
        m_TutorialStepState += 1;
        m_TutorialStepTime = curTime;

        if (m_TutorialStepState == 3)
        {
            EndTutorialStep();
        }
    }
}

bool SEASON3B::CNewUICursedTempleSystem::Update()
{
    UpdateScore();
    UpdateTutorialStep();

    return true;
}

void SEASON3B::CNewUICursedTempleSystem::RenderSkill()
{
    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, m_Alph);

    float Width, Height, fU, fV, x, y;
    int iSkillIndex;

    int CursedTempleCurSkillType = Hero->m_CursedTempleCurSkill;

    int MaxKillCount = SkillAttribute[CursedTempleCurSkillType].KillCount;

    if (m_SkillPoint >= MaxKillCount)
    {
        iSkillIndex = IMAGE_SKILL2;
    }
    else
    {
        iSkillIndex = IMAGE_NON_SKILL2;
    }

    x = 512.f + 27.f;
    y = 258.f - 58.f;
    Width = 20.f;
    Height = 28.f;
    fU = (8 + (CursedTempleCurSkillType - 210)) * (Width / 256.f);
    fV = 0;
    RenderBitmap(iSkillIndex, x, y, Width, Height, fU, fV, Width / 256.f, Height / 256.f);

    glColor4f(0.9f, 0.4f, 0.1f, m_Alph);
    RenderNumber(x + 55.f, y + 8.f, MaxKillCount, 1.f);

    glColor4f(1.f, 1.f, 1.f, m_Alph);
    RenderNumber(x + 77.f, y + 8.f, m_SkillPoint, 1.f);

    x = 512.f + 50; y = 201.f;
    m_Button[CURSEDTEMPLERESULT_SKILLUP].SetPos(x, y);
    m_Button[CURSEDTEMPLERESULT_SKILLUP].ChangeAlpha(m_Alph);
    m_Button[CURSEDTEMPLERESULT_SKILLUP].Render();

    x = 512.f + 50; y = 203.f + 11;
    m_Button[CURSEDTEMPLERESULT_SKILLDOWN].SetPos(x, y);
    m_Button[CURSEDTEMPLERESULT_SKILLDOWN].ChangeAlpha(m_Alph);
    m_Button[CURSEDTEMPLERESULT_SKILLDOWN].Render();

    x = 512.f + 28; y = 258.f - 55.f; Width = 18; Height = 24;
    if (CheckMouseIn(x, y, Width, Height))
    {
        TextNum = 0;
        ZeroMemory(TextListColor, 20 * sizeof(int));
        for (int i = 0; i < 30; i++)
        {
            TextList[i][0] = NULL;
        }

        wchar_t skillname[100];
        memset(&skillname, 0, sizeof(char));

        SKILL_ATTRIBUTE* p = &SkillAttribute[CursedTempleCurSkillType];
        wcscpy(skillname, p->Name);
        swprintf(TextList[TextNum], L"%s", skillname);
        TextListColor[TextNum] = TEXT_COLOR_BLUE; TextNum++;

        swprintf(TextList[TextNum], L"\n"); TextNum++;

        swprintf(TextList[TextNum], L"%s", GlobalText[2379 + (CursedTempleCurSkillType - AT_SKILL_CURSED_TEMPLE_PRODECTION)]);
        TextListColor[TextNum] = TEXT_COLOR_DARKBLUE; TextNum++;

        RenderTipTextList(x, y - 20, TextNum, 0);
    }

    x = 512.f + 28 + 55; y = 258.f - 55.f; Width = 18; Height = 24;
    if (CheckMouseIn(x, y, Width, Height))
    {
        TextNum = 0;
        ZeroMemory(TextListColor, 20 * sizeof(int));
        for (int i = 0; i < 30; i++)
        {
            TextList[i][0] = NULL;
        }

        swprintf(TextList[TextNum], L"%s", GlobalText[2378]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextNum++;

        RenderTipTextList(x, y - 20, TextNum, 0);
    }

    x = 512.f + 28 + 77; y = 258.f - 55.f; Width = 18; Height = 24;
    if (CheckMouseIn(x, y, Width, Height))
    {
        TextNum = 0;
        ZeroMemory(TextListColor, 20 * sizeof(int));
        for (int i = 0; i < 30; i++)
        {
            TextList[i][0] = NULL;
        }

        swprintf(TextList[TextNum], L"%s", GlobalText[2377]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextNum++;

        RenderTipTextList(x, y - 20, TextNum, 0);
    }

    glColor4f(1.f, 1.f, 1.f, 1.f);
    DisableAlphaBlend();
}

void SEASON3B::CNewUICursedTempleSystem::RenderGameTime()
{
    float x, y, Width, Height;

    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, m_Alph);

    x = 506.f; y = 393.f; Width = 134.f; Height = 37.f;
    RenderBitmap(IMAGE_CURSEDTEMPLESYSTEM_GAMETIME, x, y, Width, Height, 0.f, 0.f, Width / 256.f, Height / 64.f);

    x = 507.5f + (134.f / 2); y = 407.5f; Width = 3.f; Height = 9.0f;
    glColor4f(1.f, 0.6f, 0.3f, m_Alph);
    RenderBitmap(BITMAP_INTERFACE_EX + 44, x, y, Width, Height, 0.f, 0.f, Width / 4.f, Height / 16.f);

    int minute = m_EventMapTime / 60;
    int second = m_EventMapTime % 60;

    glColor4f(0.f, 0.f, 0.f, m_Alph);
    x = 507.5f + (134.f / 2); y = 404.5f;
    RenderNumber(x - 15.f, y, minute, 1.1f);
    RenderNumber(x + 20.f, y, second, 1.1f);

    glColor4f(1.f, 0.6f, 0.3f, m_Alph);
    x = 507.5f + (134.f / 2); y = 404.5f;
    RenderNumber(x - 15.f, y, minute, 1.1f);
    RenderNumber(x + 20.f, y, second, 1.1f);

    glColor4f(1.f, 1.f, 1.f, 1.f);
    DisableAlphaBlend();
}

void SEASON3B::CNewUICursedTempleSystem::RenderMiniMap()
{
    float x, y, Width, Height;

    m_Scale = 1.56f;

    EnableAlphaTest();

    glColor4f(1.f, 1.f, 1.f, m_Alph);

    x = 512.f; y = 232.f - 53.f; Width = 128.f; Height = 53.f;
    RenderBitmap(IMAGE_CURSEDTEMPLESYSTEM_SKILLFRAME, x, y, Width, Height, 0.f, 0.f, Width / 128.f, Height / 64.f);

    x = 512.f; y = 263.f; Width = 128.f; Height = 128.f;
    RenderBitmap(IMAGE_CURSEDTEMPLESYSTEM_MINIMAP, x, y, Width, Height, 0.f, 0.f, 1.f, 1.f);

    x = 512.f; y = 232.f; Width = 128.f; Height = 165.f;
    RenderBitmap(IMAGE_CURSEDTEMPLESYSTEM_MINIMAPFRAME, x, y, Width, Height, 0.f, 0.f, Width / 128.f, Height / 256.f);

    float npc_x = MiniMapPos(138, 44, m_Scale, AXIS_X);
    float npc_y = MiniMapPos(138, 44, m_Scale, AXIS_Y);
    RenderBitmap(IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ALLIED_NPC,
        npc_x, npc_y, 9.0f, 9.0f, 0.f, 0.f, 9.f / 16.f, 9.f / 16.f);

    npc_x = MiniMapPos(138, 58, m_Scale, AXIS_X);
    npc_y = MiniMapPos(138, 58, m_Scale, AXIS_Y);
    RenderBitmap(IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ALLIED_HOLYITEM,
        npc_x, npc_y, 9.0f, 8.0f, 0.f, 0.f, 9.f / 16.f, 8.f / 8.f);

    npc_x = MiniMapPos(192, 113, m_Scale, AXIS_X);
    npc_y = MiniMapPos(192, 113, m_Scale, AXIS_Y);
    RenderBitmap(IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ILLUSION_HOLYITEM,
        npc_x, npc_y, 9.0f, 8.0f, 0.f, 0.f, 9.f / 16.f, 8.f / 8.f);

    npc_x = MiniMapPos(193, 126, m_Scale, AXIS_X);
    npc_y = MiniMapPos(193, 126, m_Scale, AXIS_Y);
    RenderBitmap(IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ILLUSION_NPC,
        npc_x, npc_y, 9.0f, 9.0f, 0.f, 0.f, 9.f / 16.f, 9.f / 16.f);

    for (int k = 0; k < m_CursedTempleMyTeamCount; ++k)
    {
        PMSG_CURSED_TAMPLE_PARTY_POS* p = &m_CursedTempleMyTeam[k];

        if (p->wPartyUserIndex == 0xffff) continue;

        if (p->wPartyUserIndex != Hero->Key && p->wPartyUserIndex != m_HolyItemPlayerIndex)
        {
            float pcX = MiniMapPos(p->btX, p->btY, m_Scale, AXIS_X);
            float pcY = MiniMapPos(p->btX, p->btY, m_Scale, AXIS_Y);

            if (m_MyTeam == SEASON3A::eTeam_Allied)
            {
                RenderBitmap(IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ALLIED_PC,
                    pcX - 3.f, pcY - 3.f, 7.0f, 7.0f, 0.f, 0.f, 7.f / 8.f, 7.f / 8.f);
            }
            else
            {
                RenderBitmap(IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ILLUSION_PC,
                    pcX - 3.f, pcY - 3.f, 7.0f, 7.0f, 0.f, 0.f, 7.f / 8.f, 7.f / 8.f);
            }
        }
    }

    //  성물 위치
    if (m_HolyItemPlayerIndex != 0xffff && m_HolyItemPlayerIndex != Hero->Key)
    {
        float holypcX = MiniMapPos(m_HolyItemPlayerPosX, m_HolyItemPlayerPosY, m_Scale, AXIS_X);
        float holypcY = MiniMapPos(m_HolyItemPlayerPosX, m_HolyItemPlayerPosY, m_Scale, AXIS_Y);
        RenderBitmap(IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_HOLYITEM_PC,
            holypcX - 5.f, holypcY - 5.f, 14.0f, 14.0f, 0.f, 0.f, 14.f / 16.f, 14.f / 16.f);
    }

    m_Button[CURSEDTEMPLERESULT_ALPH].ChangeAlpha(m_Alph);
    m_Button[CURSEDTEMPLERESULT_ALPH].Render();

    glColor4f(1.f, 1.f, 1.f, m_Alph);

    // 히어로
    x = (Hero->PositionX);
    y = (Hero->PositionY);
    float hero_x = MiniMapPos(x, y, m_Scale, AXIS_X);
    float hero_y = MiniMapPos(x, y, m_Scale, AXIS_Y);
    RenderBitmap(IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_HERO,
        hero_x - 4, hero_y - 4, 11.0f, 11.0f, 0.f, 0.f, 11.f / 16.f, 11.f / 16.f);

    // 알파 값
    RenderNumber2D(517.f + 15.f, 246.f, static_cast<int>(m_Alph * 100), 8, 8);
    // 점수
    RenderNumber2D(517.f + 66.f, 246.f, m_AlliedPoint, 8, 8);
    RenderNumber2D(517.f + 110.f, 246.f, m_IllusionPoint, 8, 8);

    glColor4f(1.f, 1.f, 1.f, 1.f);

    DisableAlphaBlend();

#ifdef _DEBUG
    // 미니맵 좌표 수정 할때 필요 하니..놔 둘것...
/*
    for ( int j = 0; j < 7; ++j )
    {
        glColor3f ( 1.f, 0.f, 0.f );
        RenderColor( MiniMapPos( posX[j], posY[j], m_Scale, AXIS_X ),
                     MiniMapPos( posX[j], posY[j], m_Scale, AXIS_Y ), 3, 3 );
        glColor3f ( 1.f, 1.f, 1.f );
    }
    DisableAlphaBlend();
*/
#endif //_DEBUG
}

void SEASON3B::CNewUICursedTempleSystem::RenderScore()
{
    if (!m_IsScoreEffect) return;

    ::EnableAlphaTest();
    ::glColor4f(1.0f, 1.0f, 1.0f, m_ScoreEffectAlph);

    // 뮤연합군 점수
    if (m_AlliedPoint / 10 != 0)
    {
        RenderBitmap(IMAGE_CURSEDTEMPLESYSTEM_SCORE_ALLIED_NUMBER + (m_AlliedPoint / 10),
            196, 160.f, 56.0f, 66.0f, 0.f, 0.f, 56.f / 64.f, 66.f / 128.f);
        RenderBitmap(IMAGE_CURSEDTEMPLESYSTEM_SCORE_ALLIED_NUMBER + (m_AlliedPoint % 10),
            253, 160.f, 56.0f, 66.0f, 0.f, 0.f, 56.f / 64.f, 66.f / 128.f);
    }
    else
    {
        RenderBitmap(IMAGE_CURSEDTEMPLESYSTEM_SCORE_ALLIED_NUMBER + (m_AlliedPoint % 10),
            224, 160.f, 56.0f, 66.0f, 0.f, 0.f, 56.f / 64.f, 66.f / 128.f);
    }

    // :
    RenderBitmap(IMAGE_CURSEDTEMPLESYSTEM_SCORE_VS1 + (m_IllusionPoint / 10),
        310, 168.f, 20.0f, 45.0f, 0.f, 0.f, 20.f / 32.f, 45.f / 64.f);

    // 환영교단 점수
    if (m_IllusionPoint / 10 != 0)
    {
        RenderBitmap(IMAGE_CURSEDTEMPLESYSTEM_SCORE_ILLUSION_NUMBER + (m_IllusionPoint / 10),
            331, 160.f, 56.0f, 66.0f, 0.f, 0.f, 56.f / 64.f, 66.f / 128.f);
        RenderBitmap(IMAGE_CURSEDTEMPLESYSTEM_SCORE_ILLUSION_NUMBER + (m_IllusionPoint % 10),
            388, 160.f, 56.0f, 66.0f, 0.f, 0.f, 56.f / 64.f, 66.f / 128.f);
    }
    else
    {
        RenderBitmap(IMAGE_CURSEDTEMPLESYSTEM_SCORE_ILLUSION_NUMBER + (m_IllusionPoint % 10),
            358, 160.f, 56.0f, 66.0f, 0.f, 0.f, 56.f / 64.f, 66.f / 128.f);
    }

    RenderBitmap(IMAGE_CURSEDTEMPLESYSTEM_SCORE_ALLIED_GAAIL, 232.f, 115.f, 40.0f, 36.0f, 0.f, 0.f, 40.f / 64.f, 36.f / 64.f);
    RenderBitmap(IMAGE_CURSEDTEMPLESYSTEM_SCORE_VS0, 292.f, 123.f, 49.0f, 27.0f, 0.f, 0.f, 49.f / 64.f, 27.f / 32.f);
    RenderBitmap(IMAGE_CURSEDTEMPLESYSTEM_SCORE_ILLUSION_GAAIL, 367.f, 115.f, 40.0f, 36.0f, 0.f, 0.f, 40.f / 64.f, 36.f / 64.f);

    RenderBitmap(IMAGE_CURSEDTEMPLESYSTEM_SCORE_LEFT, 133.f, 115.f, 67.0f, 125.0f, 0.f, 0.f, 67.f / 128.f, 125.f / 128.f);
    RenderBitmap(IMAGE_CURSEDTEMPLESYSTEM_SCORE_RIGHT, 445.f, 115.f, 67.0f, 125.0f, 0.f, 0.f, 67.f / 128.f, 125.f / 128.f);

    ::glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    ::DisableAlphaBlend();
}

void SEASON3B::CNewUICursedTempleSystem::RenderTutorialStep()
{
    if (!m_IsTutorialStep) return;

    TextNum = 0;
    ZeroMemory(TextListColor, 20 * sizeof(int));
    for (int i = 0; i < 30; i++)
    {
        TextList[i][0] = NULL;
    }

    if (m_TutorialStepState == 0)
    {
        wcscpy(TextList[TextNum], GlobalText[2400]);
        TextListColor[TextNum] = 0xFF49B0FF; ++TextNum;
        swprintf(TextList[TextNum], L"");
        TextListColor[TextNum] = 0xFF000000; ++TextNum;
        wcscpy(TextList[TextNum], GlobalText[2401]);
        TextListColor[TextNum] = 0xFFffffff; ++TextNum;
        wcscpy(TextList[TextNum], GlobalText[2402]);
        TextListColor[TextNum] = 0xFFffffff; ++TextNum;
        wcscpy(TextList[TextNum], GlobalText[2403]);
        TextListColor[TextNum] = 0xFFffffff; ++TextNum;
    }
    else if (m_TutorialStepState == 1)
    {
        wcscpy(TextList[TextNum], GlobalText[2404]);
        TextListColor[TextNum] = 0xFF49B0FF; ++TextNum;
        swprintf(TextList[TextNum], L"");
        TextListColor[TextNum] = 0xFF000000; ++TextNum;
        wcscpy(TextList[TextNum], GlobalText[2405]);
        TextListColor[TextNum] = 0xFFffffff; ++TextNum;
        wcscpy(TextList[TextNum], GlobalText[2406]);
        TextListColor[TextNum] = 0xFFffffff; ++TextNum;
        wcscpy(TextList[TextNum], GlobalText[2407]);
        TextListColor[TextNum] = 0xFFffffff; ++TextNum;
    }
    else if (m_TutorialStepState == 2)
    {
        wcscpy(TextList[TextNum], GlobalText[2408]);
        TextListColor[TextNum] = 0xFF49B0FF; ++TextNum;
        swprintf(TextList[TextNum], L"");
        TextListColor[TextNum] = 0xFF000000; ++TextNum;
        wcscpy(TextList[TextNum], GlobalText[2409]);
        TextListColor[TextNum] = 0xFFffffff; ++TextNum;
        wcscpy(TextList[TextNum], GlobalText[2410]);
        TextListColor[TextNum] = 0xFFffffff; ++TextNum;
        wcscpy(TextList[TextNum], GlobalText[2411]);
        TextListColor[TextNum] = 0xFFffffff; ++TextNum;
    }

    ::EnableAlphaTest();
    for (int j = 0; j < TextNum; ++j)
    {
        DrawText(TextList[j], 140, 50 + (j * 14), TextListColor[j], 0x00000000, RT3_SORT_LEFT, 300, false);
    }
    ::DisableAlphaBlend();
}

bool SEASON3B::CNewUICursedTempleSystem::Render()
{
    // 환영사원 이벤트 도중 비정상적으로 맵 이동 됐을 경우를 위한 예외 처리
    if (gMapManager.IsCursedTemple() == false)
    {
        if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CURSEDTEMPLE_GAMESYSTEM) == true)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_CURSEDTEMPLE_GAMESYSTEM);
        }

        return true;
    }

    if (!g_pCharacterInfoWindow->IsVisible() || !g_pMyInventory->IsVisible()
        || !g_pGuildInfoWindow->IsVisible() || !g_pWindowMgr->IsVisible()
        || !g_pPartyInfoWindow->IsVisible() || !g_pMyQuestInfoWindow->IsVisible())
    {
        RenderGameTime();
        RenderMiniMap();
        RenderSkill();
    }

    RenderScore();
    RenderTutorialStep();

    return true;
}

void SEASON3B::CNewUICursedTempleSystem::SetCursedTempleSkill(CHARACTER* c, OBJECT* o, DWORD selectcharacterindex)
{
    if (Hero->m_CursedTempleCurSkillPacket)
    {
        MouseRButtonPush = false;
        return;
    }

    // 스킬 사용 거리
    int CursedTempleCurSkillType = c->m_CursedTempleCurSkill;

    int MaxKillCount = SkillAttribute[CursedTempleCurSkillType].KillCount;

    if (m_SkillPoint < MaxKillCount)
    {
        g_pSystemLogBox->AddText(GlobalText[2392], SEASON3B::TYPE_ERROR_MESSAGE);
        MouseRButtonPush = false;
        return;
    }

    if (CursedTempleCurSkillType == AT_SKILL_CURSED_TEMPLE_TELEPORT)
    {
        if (m_HolyItemPlayerIndex == 0xffff)
        {
            MouseRButtonPush = false;
            return;
        }
    }

    float Distance = gSkillManager.GetSkillDistance(CursedTempleCurSkillType, c);

    bool checktile = true;

    if (CursedTempleCurSkillType == AT_SKILL_CURSED_TEMPLE_RESTRAINT
        || CursedTempleCurSkillType == AT_SKILL_CURSED_TEMPLE_SUBLIMATION)
    {
        checktile = CheckTile(c, o, Distance);
    }

    if (checktile)
    {
        CHARACTER* tc = NULL;

        if (CursedTempleCurSkillType == AT_SKILL_CURSED_TEMPLE_RESTRAINT
            || CursedTempleCurSkillType == AT_SKILL_CURSED_TEMPLE_SUBLIMATION)
        {
            for (int i = 0; i < m_CursedTempleMyTeamCount; ++i)
            {
                if (m_CursedTempleMyTeam[i].wPartyUserIndex == CharactersClient[selectcharacterindex].Key)
                {
                    MouseRButtonPush = false;
                    return;
                }
            }

            if (selectcharacterindex == -1)
            {
                MouseRButtonPush = false;
                return;
            }
            else if (getTargetCharacterKey(c, selectcharacterindex) == -1)
            {
                MouseRButtonPush = false;
                return;
            }

            tc = &CharactersClient[selectcharacterindex];
        }
        else
        {
            tc = Hero;
        }

        if (tc == NULL)
        {
            MouseRButtonPush = false;
            return;
        }

        SocketClient->ToGameServer()->SendIllusionTempleSkillRequest(CursedTempleCurSkillType, static_cast<BYTE>(tc->Key), Distance);
        Hero->m_CursedTempleCurSkillPacket = true;
        MouseRButtonPush = false;

        //에니메이션 설정
        switch (CursedTempleCurSkillType)
        {
        case AT_SKILL_CURSED_TEMPLE_PRODECTION:
            SetAction(o, PLAYER_ATTACK_REMOVAL);
            break;
        case AT_SKILL_CURSED_TEMPLE_RESTRAINT:
            SetAction(o, PLAYER_ATTACK_REMOVAL);
            break;
        case AT_SKILL_CURSED_TEMPLE_TELEPORT:
            SetAction(o, PLAYER_ATTACK_REMOVAL);
            break;
        case AT_SKILL_CURSED_TEMPLE_SUBLIMATION:
            SetAction(o, PLAYER_ATTACK_REMOVAL);
            break;
        }
    }
}

void SEASON3B::CNewUICursedTempleSystem::ReceiveCursedTempRegisterSkill(const BYTE* ReceiveBuffer)
{
    auto data = (LPPMSG_CURSED_TEMPLE_USE_MAGIC_RESULT)ReceiveBuffer;

    WORD magNumber = ((WORD)(data->MagicH) << 8) + data->MagicL;

    WORD sourceobjkey = data->wSourceObjIndex;
    WORD targetobjkey = data->wTargetObjIndex;

    WORD sourceobjindex = FindCharacterIndex(sourceobjkey);
    WORD targetobjindex = FindCharacterIndex(targetobjkey);

    if (sourceobjindex == MAX_CHARACTERS_CLIENT || targetobjindex == MAX_CHARACTERS_CLIENT)
        return;

    CHARACTER* sc = &CharactersClient[sourceobjindex];
    OBJECT* sco = &sc->Object;

    CHARACTER* tc = &CharactersClient[targetobjindex];
    OBJECT* tco = &tc->Object;

    if (data->MagicResult == 0)
    {
        if (sc == Hero) Hero->m_CursedTempleCurSkillPacket = false;
        return;
    }

    if (sc != Hero && magNumber != AT_SKILL_TELEPORT && magNumber != AT_SKILL_TELEPORT_ALLY && tco->Visible)
    {
        sco->Angle[2] = CreateAngle2D(sco->Position, tco->Position);
    }

    if (sc == Hero)
    {
        Hero->m_CursedTempleCurSkillPacket = false;
    }

    bool effectresult = false;

    switch (magNumber)
    {
    case AT_SKILL_CURSED_TEMPLE_PRODECTION:
    {
        // _buffwani_
        g_CharacterRegisterBuff(tco, eBuff_CursedTempleProdection);

        if (sc != Hero) SetAction(sco, PLAYER_ATTACK_REMOVAL);

        effectresult = CreateCursedTempleSkillEffect(tc, AT_SKILL_CURSED_TEMPLE_PRODECTION, 0);
    }
    break;
    case AT_SKILL_CURSED_TEMPLE_RESTRAINT:
    {
        tc->Movement = false;

        SetPlayerStop(tc);

        g_CharacterRegisterBuff(tco, eDeBuff_CursedTempleRestraint);

        if (sc != Hero) SetAction(sco, PLAYER_ATTACK_REMOVAL);

        sc->AttackTime = 1;
        sc->TargetCharacter = targetobjindex;
        sc->SkillSuccess = true;
        sc->Skill = magNumber;

        effectresult = CreateCursedTempleSkillEffect(tc, AT_SKILL_CURSED_TEMPLE_RESTRAINT, 0);
    }
    break;
    case AT_SKILL_CURSED_TEMPLE_TELEPORT:
    {
        if (sc != Hero) SetAction(sco, PLAYER_ATTACK_REMOVAL);
    }
    break;
    case AT_SKILL_CURSED_TEMPLE_SUBLIMATION:
    {
        SetAction(tco, PLAYER_SHOCK);
        effectresult = CreateCursedTempleSkillEffect(tc, AT_SKILL_CURSED_TEMPLE_SUBLIMATION, 0);

        if (sc != Hero) SetAction(sco, PLAYER_ATTACK_REMOVAL);
        effectresult = CreateCursedTempleSkillEffect(sc, AT_SKILL_CURSED_TEMPLE_SUBLIMATION, 1);
    }
    break;
    }
}

void SEASON3B::CNewUICursedTempleSystem::ReceiveCursedTempUnRegisterSkill(const BYTE* ReceiveBuffer)
{
    auto data = (LPPMSG_CURSED_TEMPLE_SKILL_END)ReceiveBuffer;

    WORD magNumber = ((WORD)(data->MagicH) << 8) + data->MagicL;

    WORD targetobjkey = data->wObjIndex;
    WORD targetobjindex = FindCharacterIndex(targetobjkey);

    if (targetobjindex == MAX_CHARACTERS_CLIENT)
        return;

    CHARACTER* tc = &CharactersClient[targetobjindex];
    OBJECT* tco = &tc->Object;

    switch (magNumber)
    {
    case AT_SKILL_CURSED_TEMPLE_PRODECTION:
    {
        if (g_isCharacterBuff(tco, eBuff_CursedTempleProdection))
        {
            g_CharacterUnRegisterBuff(tco, eBuff_CursedTempleProdection);

            DeleteEffect(MODEL_CURSEDTEMPLE_PRODECTION_SKILL, tco);
        }
    }
    break;
    case AT_SKILL_CURSED_TEMPLE_RESTRAINT:
    {
        if (g_isCharacterBuff(tco, eDeBuff_CursedTempleRestraint))
        {
            g_CharacterUnRegisterBuff(tco, eDeBuff_CursedTempleRestraint);

            DeleteEffect(MODEL_CURSEDTEMPLE_RESTRAINT_SKILL, tco);
        }
    }
    break;
    }
}

void SEASON3B::CNewUICursedTempleSystem::ReceiveCursedTempleInfo(const BYTE* ReceiveBuffer)
{
    auto data = (LPPMSG_CURSED_TAMPLE_STATE)ReceiveBuffer;

    m_EventMapTime = data->wRemainSec;

    if (data->btUserIndex == 0xffff)
    {
        memset(&m_HolyItemPlayerName, 0, sizeof(char));
    }

    m_HolyItemPlayerIndex = data->btUserIndex;
    m_HolyItemPlayerPosX = data->btX;
    m_HolyItemPlayerPosY = data->btY;
    m_MyTeam = static_cast<SEASON3A::eCursedTempleTeam>(data->btMyTeam);

    wchar_t message[200];
    memset(&message, 0, sizeof(char));

    if (m_MyTeam == SEASON3A::eTeam_Allied)
    {
        if (m_AlliedPoint != data->btAlliedPoint)
        {
            PlayBuffer(SOUND_CURSEDTEMPLE_GAMESYSTEM4);
            StartScoreEffect();
            g_pSystemLogBox->AddText(GlobalText[2360], SEASON3B::TYPE_ERROR_MESSAGE);
        }
        else if (m_IllusionPoint != data->btIllusionPoint)
        {
            PlayBuffer(SOUND_CURSEDTEMPLE_GAMESYSTEM4);
            StartScoreEffect();
            g_pSystemLogBox->AddText(GlobalText[2361], SEASON3B::TYPE_ERROR_MESSAGE);
        }
    }
    else
    {
        if (m_IllusionPoint != data->btIllusionPoint)
        {
            PlayBuffer(SOUND_CURSEDTEMPLE_GAMESYSTEM4);
            StartScoreEffect();
            g_pSystemLogBox->AddText(GlobalText[2363], SEASON3B::TYPE_ERROR_MESSAGE);
        }
        else if (m_AlliedPoint != data->btAlliedPoint)
        {
            PlayBuffer(SOUND_CURSEDTEMPLE_GAMESYSTEM4);
            StartScoreEffect();
            g_pSystemLogBox->AddText(GlobalText[2364], SEASON3B::TYPE_ERROR_MESSAGE);
        }
    }

    m_AlliedPoint = data->btAlliedPoint;
    m_IllusionPoint = data->btIllusionPoint;

    m_CursedTempleMyTeamCount = data->btPartyCount;

    int Offset = sizeof(PMSG_CURSED_TAMPLE_STATE);

    for (int i = 0; i < m_CursedTempleMyTeamCount; i++)
    {
        auto data2 = (LPPMSG_CURSED_TAMPLE_PARTY_POS)(ReceiveBuffer + Offset);

        if (data2->wPartyUserIndex != 0xffff)
        {
            PMSG_CURSED_TAMPLE_PARTY_POS* p = &m_CursedTempleMyTeam[i];

            p->wPartyUserIndex = data2->wPartyUserIndex;
            p->byMapNumber = data2->byMapNumber;
            p->btX = data2->btX;
            p->btY = data2->btY;
        }

        Offset += sizeof(PMSG_CURSED_TAMPLE_PARTY_POS);
    }
}

void SEASON3B::CNewUICursedTempleSystem::ReceiveCursedTempSkillPoint(const BYTE* ReceiveBuffer)
{
    auto data = (LPPMSG_CURSED_TEMPLE_SKILL_POINT)ReceiveBuffer;

    if (m_SkillPoint < data->btSkillPoint)
    {
        wchar_t message[100];
        memset(&message, 0, sizeof(char));
        swprintf(message, GlobalText[2391], data->btSkillPoint - m_SkillPoint);
        g_pSystemLogBox->AddText(message, SEASON3B::TYPE_SYSTEM_MESSAGE);
    }

    m_SkillPoint = data->btSkillPoint;
}

void SEASON3B::CNewUICursedTempleSystem::ReceiveCursedTempleHolyItemRelics(const BYTE* ReceiveBuffer)
{
    auto data = (LPPMSG_RELICS_GET_USER)ReceiveBuffer;
}