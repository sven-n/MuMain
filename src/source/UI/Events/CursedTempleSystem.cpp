
#include "stdafx.h"
#include "UI/RmlBridge/RmlElementBox.h"
#include "UI/RmlBridge/RmlPointer.h"
#include "UI/RmlBridge/RmlNativeTextSize.h"
#include "UI/Events/CursedTempleSystem.h"
#include "UI/Events/EventPreview.h"
#include "UI/Dialogs/CommonMessageBox.h"
#include "UI/Dialogs/CustomMessageBox.h"
#include "UI/Widgets/UIBaseDef.h"
#include "Audio/DSPlaySound.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInventory.h"
#include "I18N/All.h"

#include "GameLogic/Items/CSItemOption.h"
#include "GameLogic/Events/CSChaosCastle.h"
#include "UI/Social/SocialWindowBase.h"
#include "Engine/AI/ZzzAI.h"
#include "Render/Effects/ZzzEffect.h"
#include "GameLogic/Events/w_CursedTemple.h"
#include "World/MapInfra/MapManager.h"
#include "UI/Inventory/InventoryCtrl.h"
#include "UI/Core/WindowSystem.h"
#include "GameLogic/Skills/SkillManager.h"
#include "UI/Scaling/UITransform.h"
#include "UI/Tooltip/LegacyTextListTooltip.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlDigitCells.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Render/Text/CUIRenderText.h"

#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/StringUtilities.h>

#include <string>

extern int TextNum;
extern wchar_t TextList[50][100];
extern int  TextListColor[50];

using namespace SEASON3B;
using namespace mu::ui::window;

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

    // RenderSkill()'s three hover tooltips are the only Tooltip::Show() callers in this file and
    // their hover regions are disjoint, so one shared owner token is enough.
    const int kSkillHoverTooltipOwner = 0;

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

    // A tile's place on the mini map, in reference px from the projection's origin (the original's
    // (464, 299) on the screen; the theme places #mini_map_markers there).
    float MiniMapPos(float pointX, float pointY, float scale, int aXis)
    {
        if (aXis == AXIS_X)
        {
            float ridY = posY[6] - pointY;
            return (pointX - ridY) / scale;
        }
        else
        {
            float ridX = posX[6] - pointX;
            return ((125 - (pointY + ridX))) / scale;
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
};

bool mu::ui::window::CCursedTempleSystem::Create(CManager* pNewUIMng)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_CURSEDTEMPLE_GAMESYSTEM, this);

    BuildRmlUi();

    Show(false);

    return true;
}

mu::ui::window::CCursedTempleSystem::CCursedTempleSystem() : m_pNewUIMng(NULL)
{
    Initialize();
}

mu::ui::window::CCursedTempleSystem::~CCursedTempleSystem()
{
    Destroy();
}

void mu::ui::window::CCursedTempleSystem::Initialize()
{
    LoadImages();

    ResetCursedTempleSystemInfo();
}

void mu::ui::window::CCursedTempleSystem::Destroy()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void mu::ui::window::CCursedTempleSystem::LoadImages()
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
        mu_swprintf(buff, L"Interface\\newui_ctscorealliednum%d.tga", i);
        LoadBitmap(buff, IMAGE_CURSEDTEMPLESYSTEM_SCORE_ALLIED_NUMBER + i, GL_LINEAR);
    }
    for (int j = 0; j < 10; ++j)
    {
        mu_swprintf(buff, L"Interface\\newui_ctscoreillusionnum%d.tga", j);
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

void mu::ui::window::CCursedTempleSystem::UnloadImages()
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

void mu::ui::window::CCursedTempleSystem::ResetCursedTempleSystemInfo()
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
        m_CursedTempleMyTeam[i].userIndex = 0xffff;
        m_CursedTempleMyTeam[i].x = 0;
        m_CursedTempleMyTeam[i].y = 0;
        m_CursedTempleMyTeam[i].mapNumber = 0xff;
    }
}

void mu::ui::window::CCursedTempleSystem::StartScoreEffect()
{
    m_StartScoreEffectTime = 0;
    m_ScoreEffectAlph = 0.0f;
    m_ScoreEffectState = 0;
    m_IsScoreEffect = true;
}

void mu::ui::window::CCursedTempleSystem::EndScoreEffect()
{
    m_StartScoreEffectTime = 0;
    m_ScoreEffectAlph = 0.0f;
    m_ScoreEffectState = 0;
    m_IsScoreEffect = false;
}

void mu::ui::window::CCursedTempleSystem::StartTutorialStep()
{
    m_IsTutorialStep = true;
    m_TutorialStepState = 0;
    m_TutorialStepTime = timeGetTime();
}

void mu::ui::window::CCursedTempleSystem::EndTutorialStep()
{
    m_IsTutorialStep = false;
    m_TutorialStepState = 0;
    m_TutorialStepTime = 0;
}

SEASON3A::eCursedTempleTeam mu::ui::window::CCursedTempleSystem::GetMyTeam()
{
    return m_MyTeam;
}

bool mu::ui::window::CCursedTempleSystem::CheckInventoryHolyItem(CHARACTER* c)
{
    CPickedItem* pPickedItem = CInventoryCtrl::GetPickedItem();

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

bool mu::ui::window::CCursedTempleSystem::CheckTalkProgressNpc(DWORD npcindex, DWORD npckey)
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
                    mu::ui::window::CCursedTempleProgressMsgBox* pMsgBox = NULL;
                    mu::ui::window::CreateMessageBox(MSGBOX_LAYOUT_CLASS(mu::ui::window::CCursedTempleHolicItemSaveLayout), &pMsgBox);
                    if (pMsgBox)
                    {
                        pMsgBox->SetNpcIndex(npckey);
                    }
                }
                else
                {
                    g_pSystemLogBox->AddText(I18N::Game::NoItem, mu::ui::window::TYPE_ERROR_MESSAGE);
                }
            }
            else
            {
                mu::ui::window::CCursedTempleProgressMsgBox* pMsgBox = NULL;
                mu::ui::window::CreateMessageBox(MSGBOX_LAYOUT_CLASS(mu::ui::window::CCursedTempleHolicItemGetLayout), &pMsgBox);
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
                mu::ui::window::CreateOkMessageBox(I18N::Game::WeHaveEnteredTheHeartOf);

            if (npcindex == IllusionNpc)
                mu::ui::window::CreateOkMessageBox(I18N::Game::ListenToThisTheAlliesHave);
        }

        return true;
    }

    SocketClient->ToGameServer()->SendTalkToNpcRequest(npckey);
    return false;
}

bool mu::ui::window::CCursedTempleSystem::CheckHeroSkillType(int operatortype)
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

bool mu::ui::window::CCursedTempleSystem::CheckDragonRender()
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

bool mu::ui::window::CCursedTempleSystem::IsCursedTempleSkillKey(DWORD selectcharacterindex)
{
    if (Hero->m_CursedTempleCurSkillPacket) return false;

    return mu::ui::window::IsRepeat(VK_SHIFT);
}

bool mu::ui::window::CCursedTempleSystem::UpdateMouseEvent()
{
    // The wheel steps the skill wherever the pointer is; the buttons are the document's
    // (ct_alpha_click, ct_skill_up_click, ct_skill_down_click).
    if (MouseWheel >= 1)
    {
        if (CheckHeroSkillType())
            Hero->m_CursedTempleCurSkill += 1;
        MouseWheel = 0;
        return false;
    }
    if (MouseWheel <= -1)
    {
        if (CheckHeroSkillType(1))
            Hero->m_CursedTempleCurSkill -= 1;
        MouseWheel = 0;
        return false;
    }

    // The panels hold the pointer by their drawn area.
    Rml::ElementDocument* document = m_RmlView.Document();
    if (document != nullptr && UI::RmlBridge::IsPointerWithin(document->GetElementById("ct_hud_area")))
        return false;

    return true;
}

bool mu::ui::window::CCursedTempleSystem::UpdateKeyEvent()
{
    return true;
}

void mu::ui::window::CCursedTempleSystem::UpdateScore()
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

void mu::ui::window::CCursedTempleSystem::UpdateTutorialStep()
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

bool mu::ui::window::CCursedTempleSystem::Update()
{
    UpdateScore();
    UpdateTutorialStep();
    SyncView();

    return true;
}

namespace
{
    // Shared by RenderSkill()'s three hover tooltips below -- each builds TextList/TextListColor
    // the legacy way first, then hands off here; (x, y) in window pixels.
    void ShowSkillHoverTooltip(float x, float y, int textNum)
    {
        UI::Tooltip::ShowLegacyTextList(textNum, x, y, UI::Tooltip::Placement::Below, &kSkillHoverTooltipOwner);
    }
}

namespace
{
// A file of Data/Interface, relative to cursed_temple_system.rml.
Rml::String InterfaceImage(const std::string& file)
{
    return "../../../" + file;
}

Rml::String TexelRect(float x, float y, float width, float height)
{
    return Rml::CreateString("%g %g %g %g", x, y, width, height);
}

void AddSprite(std::vector<CursedTempleSpriteEntry>& sprites, const Rml::Vector4f& box, const std::string& file,
               const Rml::String& rect, float opacity = 1.f)
{
    sprites.push_back({box.x, box.y, box.z, box.w, InterfaceImage(file), rect, opacity});
}
} // namespace

void mu::ui::window::CCursedTempleSystem::BindRmlModel(Rml::DataModelConstructor& c, CursedTempleSystemRmlModel& model)
{
    auto sprite = c.RegisterStruct<CursedTempleSpriteEntry>();
    sprite.RegisterMember("left", &CursedTempleSpriteEntry::left);
    sprite.RegisterMember("top", &CursedTempleSpriteEntry::top);
    sprite.RegisterMember("width", &CursedTempleSpriteEntry::width);
    sprite.RegisterMember("height", &CursedTempleSpriteEntry::height);
    sprite.RegisterMember("src", &CursedTempleSpriteEntry::src);
    sprite.RegisterMember("rect", &CursedTempleSpriteEntry::rect);
    sprite.RegisterMember("opacity", &CursedTempleSpriteEntry::opacity);
    c.RegisterArray<std::vector<CursedTempleSpriteEntry>>();
    auto line = c.RegisterStruct<CursedTempleTextEntry>();
    line.RegisterMember("text", &CursedTempleTextEntry::text);
    line.RegisterMember("text_px", &CursedTempleTextEntry::textPx);
    line.RegisterMember("title", &CursedTempleTextEntry::title);
    c.RegisterArray<std::vector<CursedTempleTextEntry>>();

    c.Bind("panels_shown", &model.panelsShown);
    c.Bind("score_shown", &model.scoreShown);
    c.Bind("skill_icon_src", &model.skillIconSrc);
    c.Bind("skill_icon_rect", &model.skillIconRect);
    c.Bind("allied_tens_src", &model.alliedTensSrc);
    c.Bind("allied_ones_src", &model.alliedOnesSrc);
    c.Bind("illusion_tens_src", &model.illusionTensSrc);
    c.Bind("illusion_ones_src", &model.illusionOnesSrc);
    c.Bind("allied_two_digits", &model.alliedTwoDigits);
    c.Bind("illusion_two_digits", &model.illusionTwoDigits);
    c.RegisterArray<std::vector<Rml::String>>();
    c.Bind("minute_digits", &model.minuteDigits);
    c.Bind("second_digits", &model.secondDigits);
    c.Bind("alpha_digits", &model.alphaDigits);
    c.Bind("allied_digits", &model.alliedDigits);
    c.Bind("illusion_digits", &model.illusionDigits);
    c.Bind("kills_needed_digits", &model.killsNeededDigits);
    c.Bind("kills_digits", &model.killsDigits);
    c.Bind("sprites", &model.sprites);
    c.Bind("alpha", &model.alpha);
    // The original's transparency button: the panels' buttons half see-through, or opaque again.
    c.BindEventCallback("ct_alpha_click", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        { m_Alph = m_Alph != 1.0f ? 1.0f : 0.51f; });
    c.BindEventCallback("ct_skill_up_click", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        {
                            if (CheckHeroSkillType())
                                Hero->m_CursedTempleCurSkill += 1;
                        });
    c.BindEventCallback("ct_skill_down_click", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        {
                            if (CheckHeroSkillType(1))
                                Hero->m_CursedTempleCurSkill -= 1;
                        });
    c.Bind("tutorial_lines", &model.tutorialLines);
}

void mu::ui::window::CCursedTempleSystem::BuildRmlUi()
{
    m_RmlView.Ensure();
}

// The original RenderSkill(): the current skill's icon (grey until enough kill points), the kill
// points it needs and has, the skill up and down buttons, and the three hover tooltips.
void mu::ui::window::CCursedTempleSystem::SyncSkill()
{
    const int CursedTempleCurSkillType = Hero->m_CursedTempleCurSkill;
    const int MaxKillCount = SkillAttribute[CursedTempleCurSkillType].KillCount;

    // The icon's own place is the theme's; which sheet and cell it shows is the current skill and
    // whether the hero has the kill points for it.
    SyncField(m_RmlView.Binder(), &CursedTempleSystemRmlModel::skillIconSrc, "skill_icon_src",
              InterfaceImage(m_SkillPoint >= MaxKillCount ? "newui_skill2.jpg" : "newui_non_skill2.jpg"));
    SyncField(m_RmlView.Binder(), &CursedTempleSystemRmlModel::skillIconRect, "skill_icon_rect",
              TexelRect(static_cast<float>((8 + (CursedTempleCurSkillType - 210)) * 20), 0.f, 20.f, 28.f));
    SyncField(m_RmlView.Binder(), &CursedTempleSystemRmlModel::killsNeededDigits, "kills_needed_digits",
              UI::RmlBridge::DigitCells(MaxKillCount, 12.f, 14.f));
    SyncField(m_RmlView.Binder(), &CursedTempleSystemRmlModel::killsDigits, "kills_digits",
              UI::RmlBridge::DigitCells(m_SkillPoint, 12.f, 14.f));

    // The theme's hover zones (#ct_hint_*): a tooltip shows 20 units above the hovered one.
    Rml::ElementDocument* document = m_RmlView.Document();
    Rml::Vector2f anchor;
    const auto hovered = [&](const char* id)
    {
        Rml::Element* zone = document != nullptr ? document->GetElementById(id) : nullptr;
        Rml::Vector2f point;
        if (!UI::RmlBridge::IsPointerWithin(zone) || !UI::RmlBridge::DrawnTopLeft(*zone, point))
            return false;
        anchor = {point.x, point.y - 20.f * UI::RmlBridge::DrawnScale(*zone)};
        return true;
    };

    bool anyTooltipHovered = false;
    if (hovered("ct_hint_skill"))
    {
        anyTooltipHovered = true;
        TextNum = 0;
        ZeroMemory(TextListColor, 20 * sizeof(int));
        for (int i = 0; i < 30; i++)
        {
            TextList[i][0] = 0;
        }

        SKILL_ATTRIBUTE* p = &SkillAttribute[CursedTempleCurSkillType];
        mu_swprintf(TextList[TextNum], L"%ls", p->Name);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextNum++;

        mu_swprintf(TextList[TextNum], L"\n");
        TextNum++;

        mu_swprintf(TextList[TextNum], L"%ls",
                    I18N::Game::Lookup(2379 + (CursedTempleCurSkillType - AT_SKILL_CURSED_TEMPLE_PRODECTION)));
        TextListColor[TextNum] = TEXT_COLOR_DARKBLUE;
        TextNum++;

        ShowSkillHoverTooltip(anchor.x, anchor.y, TextNum);
    }

    if (hovered("ct_hint_kills_needed"))
    {
        anyTooltipHovered = true;
        TextNum = 0;
        ZeroMemory(TextListColor, 20 * sizeof(int));
        for (int i = 0; i < 30; i++)
        {
            TextList[i][0] = 0;
        }

        mu_swprintf(TextList[TextNum], L"%ls", I18N::Game::RequiredKillPoint);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextNum++;

        ShowSkillHoverTooltip(anchor.x, anchor.y, TextNum);
    }

    if (hovered("ct_hint_kills"))
    {
        anyTooltipHovered = true;
        TextNum = 0;
        ZeroMemory(TextListColor, 20 * sizeof(int));
        for (int i = 0; i < 30; i++)
        {
            TextList[i][0] = 0;
        }

        mu_swprintf(TextList[TextNum], L"%ls", I18N::Game::AchievedKillPoint);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextNum++;

        ShowSkillHoverTooltip(anchor.x, anchor.y, TextNum);
    }

    if (!anyTooltipHovered)
    {
        UI::Tooltip::HideLegacyTextList(&kSkillHoverTooltipOwner);
    }
}

// The original RenderGameTime(): the frame, the colon dot, the minutes and the seconds.
void mu::ui::window::CCursedTempleSystem::SyncGameTime()
{
    SyncField(m_RmlView.Binder(), &CursedTempleSystemRmlModel::minuteDigits, "minute_digits",
              UI::RmlBridge::DigitCells(static_cast<int>(m_EventMapTime / 60), 12.f, 14.f));
    SyncField(m_RmlView.Binder(), &CursedTempleSystemRmlModel::secondDigits, "second_digits",
              UI::RmlBridge::DigitCells(static_cast<int>(m_EventMapTime % 60), 12.f, 14.f));
}

// The original RenderMiniMap(): the skill panel's frame, the map and its frame, the fixed NPC and
// box markers, the party, the sacred item's carrier, the transparency button, the hero, the
// transparency and the two teams' points.
void mu::ui::window::CCursedTempleSystem::SyncMiniMap(std::vector<CursedTempleSpriteEntry>& sprites)
{
    m_Scale = 1.56f;

    const auto marker = [&](float tileX, float tileY, const Rml::Vector2f& size, const char* file)
    {
        AddSprite(
            sprites,
            {MiniMapPos(tileX, tileY, m_Scale, AXIS_X), MiniMapPos(tileX, tileY, m_Scale, AXIS_Y), size.x, size.y},
            file, TexelRect(0.f, 0.f, size.x, size.y));
    };
    marker(138, 44, {9.f, 9.f}, "newui_ctminmap_TeamB_npc.tga");
    marker(138, 58, {9.f, 8.f}, "newui_ctminmap_TeamB_box.tga");
    marker(192, 113, {9.f, 8.f}, "newui_ctminmap_TeamA_box.tga");
    marker(193, 126, {9.f, 9.f}, "newui_ctminmap_TeamA_npc.tga");

    for (int k = 0; k < m_CursedTempleMyTeamCount; ++k)
    {
        const UI::CursedTemple::PartyPosition* p = &m_CursedTempleMyTeam[k];

        if (p->userIndex == 0xffff)
            continue;

        if (p->userIndex != Hero->Key && p->userIndex != m_HolyItemPlayerIndex)
        {
            const float pcX = MiniMapPos(p->x, p->y, m_Scale, AXIS_X);
            const float pcY = MiniMapPos(p->x, p->y, m_Scale, AXIS_Y);
            AddSprite(sprites, {pcX - 3.f, pcY - 3.f, 7.f, 7.f},
                      m_MyTeam == SEASON3A::eTeam_Allied ? "newui_ctminmap_TeamB_member.tga"
                                                         : "newui_ctminmap_TeamA_member.tga",
                      TexelRect(0.f, 0.f, 7.f, 7.f));
        }
    }

    if (m_HolyItemPlayerIndex != 0xffff && m_HolyItemPlayerIndex != Hero->Key)
    {
        const float holypcX = MiniMapPos(m_HolyItemPlayerPosX, m_HolyItemPlayerPosY, m_Scale, AXIS_X);
        const float holypcY = MiniMapPos(m_HolyItemPlayerPosX, m_HolyItemPlayerPosY, m_Scale, AXIS_Y);
        AddSprite(sprites, {holypcX - 5.f, holypcY - 5.f, 14.f, 14.f}, "newui_ctminmap_Relic.tga",
                  TexelRect(0.f, 0.f, 14.f, 14.f));
    }

    const auto heroX = static_cast<float>(Hero->PositionX);
    const auto heroY = static_cast<float>(Hero->PositionY);
    const float hero_x = MiniMapPos(heroX, heroY, m_Scale, AXIS_X);
    const float hero_y = MiniMapPos(heroX, heroY, m_Scale, AXIS_Y);
    AddSprite(sprites, {hero_x - 4, hero_y - 4, 11.f, 11.f}, "newui_ctminmap_Hero.tga",
              TexelRect(0.f, 0.f, 11.f, 11.f));

    SyncField(m_RmlView.Binder(), &CursedTempleSystemRmlModel::alphaDigits, "alpha_digits",
              UI::RmlBridge::DigitCells(static_cast<int>(m_Alph * 100), 16.f, 16.f));
    SyncField(m_RmlView.Binder(), &CursedTempleSystemRmlModel::alliedDigits, "allied_digits",
              UI::RmlBridge::DigitCells(m_AlliedPoint, 16.f, 16.f));
    SyncField(m_RmlView.Binder(), &CursedTempleSystemRmlModel::illusionDigits, "illusion_digits",
              UI::RmlBridge::DigitCells(m_IllusionPoint, 16.f, 16.f));
}

// The original RenderScore(): the two teams' points in big digits between their banners, shown for
// a while after a team scores. Each theme places the digits and the banners; which digit image a
// team shows is its points.
void mu::ui::window::CCursedTempleSystem::SyncScore()
{
    const auto digit = [](const char* team, int value)
    { return InterfaceImage(std::string("newui_ctscore") + team + "num" + std::to_string(value) + ".tga"); };

    const bool alliedTwo = m_AlliedPoint / 10 != 0;
    const bool illusionTwo = m_IllusionPoint / 10 != 0;
    SyncField(m_RmlView.Binder(), &CursedTempleSystemRmlModel::alliedTwoDigits, "allied_two_digits", alliedTwo);
    SyncField(m_RmlView.Binder(), &CursedTempleSystemRmlModel::illusionTwoDigits, "illusion_two_digits", illusionTwo);
    SyncField(m_RmlView.Binder(), &CursedTempleSystemRmlModel::alliedTensSrc, "allied_tens_src",
              digit("allied", m_AlliedPoint / 10 % 10));
    SyncField(m_RmlView.Binder(), &CursedTempleSystemRmlModel::alliedOnesSrc, "allied_ones_src",
              digit("allied", m_AlliedPoint % 10));
    SyncField(m_RmlView.Binder(), &CursedTempleSystemRmlModel::illusionTensSrc, "illusion_tens_src",
              digit("illusion", m_IllusionPoint / 10 % 10));
    SyncField(m_RmlView.Binder(), &CursedTempleSystemRmlModel::illusionOnesSrc, "illusion_ones_src",
              digit("illusion", m_IllusionPoint % 10));
}

// The original RenderTutorialStep(): the step's title and three lines from (140, 50), 14 px apart
// with the second row left empty, each in a 300 px box. The theme places and colours them.
void mu::ui::window::CCursedTempleSystem::SyncTutorialStep(std::vector<CursedTempleTextEntry>& lines)
{
    if (!m_IsTutorialStep)
        return;

    const wchar_t* texts[5] = {};
    if (m_TutorialStepState == 0)
    {
        texts[0] = I18N::Game::STEP1BattleBegins;
        texts[2] = I18N::Game::TheStoneStatueAppearsRandomlyFromOneOfTheTwoLocations;
        texts[3] = I18N::Game::TheSacredItemMayBeAchievedByClickingOnTheStoneStatue;
        texts[4] = I18N::Game::BeCautiousOfTheFactThat;
    }
    else if (m_TutorialStepState == 1)
    {
        texts[0] = I18N::Game::STEP2StorageOfTheSacredItem;
        texts[2] = I18N::Game::ClickOnTheStorageOfThe;
        texts[3] = I18N::Game::TheGoalIsToAchieveAsManyPointsAsPossibleWithinTheGivenPeriod;
        texts[4] = I18N::Game::TheStoneStatueReappearsAfterTheStorageLookForTheStatue;
    }
    else if (m_TutorialStepState == 2)
    {
        texts[0] = I18N::Game::STEP3OfficialSkills;
        texts[2] = I18N::Game::YouMayAchieveTheKillPoints;
        texts[3] = I18N::Game::MouseWheelButtonChangeSkillTypesShiftMouseRightClickUse;
        texts[4] = I18N::Game::ThereAre4TypesOfSkills;
    }

    g_pRenderText->SetFont(g_hFont);
    for (int j = 0; j < 5; ++j)
    {
        if (texts[j] == nullptr)
            continue;
        const std::wstring text = texts[j];
        const int width = g_pRenderText->MeasureText(text.c_str(), static_cast<int>(text.size())).cx;
        lines.push_back(
            {StringUtils::WideToNarrow(text.c_str()),
             UI::RmlBridge::NativeTextPxInBox(UI::Scaling::FontRole::Normal, static_cast<float>(width), 300.f),
             j == 0});
    }
}

void mu::ui::window::CCursedTempleSystem::SyncView()
{
    BuildRmlUi();
    if (!m_RmlView.Document())
        return;

    // The original's check for a map change out of the event without the event's own hide.
    if (IsVisible() && gMapManager.IsCursedTemple() == false &&
        !UI::EventPreview::IsShowing(UI::EventPreview::Event::Temple))
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_CURSEDTEMPLE_GAMESYSTEM);

    const bool visible = IsVisible();
    UI::RmlBridge::SyncDocumentVisibilityBehind(m_RmlView.Document(), visible);
    if (!visible)
        return;

    std::vector<CursedTempleSpriteEntry> sprites;
    // The original's condition: drawn unless every one of these windows is open.
    const bool panelsShown = !g_pCharacterInfoWindow->IsVisible() || !g_pMyInventory->IsVisible() ||
                             !g_pGuildInfoWindow->IsVisible() || !g_pWindowMgr->IsVisible() ||
                             !g_pPartyInfoWindow->IsVisible() || !g_pMyQuestInfoWindow->IsVisible();
    SyncField(m_RmlView.Binder(), &CursedTempleSystemRmlModel::panelsShown, "panels_shown", panelsShown);
    if (panelsShown)
    {
        SyncGameTime();
        SyncMiniMap(sprites);
        SyncSkill();
    }
    SyncField(m_RmlView.Binder(), &CursedTempleSystemRmlModel::alpha, "alpha", m_Alph);
    SyncField(m_RmlView.Binder(), &CursedTempleSystemRmlModel::scoreShown, "score_shown", m_IsScoreEffect);
    if (m_IsScoreEffect)
        SyncScore();
    std::vector<CursedTempleTextEntry> lines;
    SyncTutorialStep(lines);

    CursedTempleSystemRmlModel& model = m_RmlView.GetModel();
    SyncField(m_RmlView.Binder(), &CursedTempleSystemRmlModel::sprites, "sprites", std::move(sprites));
    SyncField(m_RmlView.Binder(), &CursedTempleSystemRmlModel::tutorialLines, "tutorial_lines", std::move(lines));
}

bool mu::ui::window::CCursedTempleSystem::Render()
{
    // Nothing native left: the HUD is RmlUi (SyncView()). Kept because CObject requires the
    // override.
    return true;
}

void mu::ui::window::CCursedTempleSystem::SetCursedTempleSkill(CHARACTER* c, OBJECT* o, DWORD selectcharacterindex)
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
        g_pSystemLogBox->AddText(I18N::Game::KillPointIsnTSufficient, mu::ui::window::TYPE_ERROR_MESSAGE);
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
                if (m_CursedTempleMyTeam[i].userIndex == CharactersClient[selectcharacterindex].Key)
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

void mu::ui::window::CCursedTempleSystem::ResolveSkill(const UI::CursedTemple::SkillResult& result)
{
    WORD magNumber = result.skill;
    WORD sourceobjkey = result.sourceKey;
    WORD targetobjkey = result.targetKey;

    WORD sourceobjindex = FindCharacterIndex(sourceobjkey);
    WORD targetobjindex = FindCharacterIndex(targetobjkey);

    if (sourceobjindex == MAX_CHARACTERS_CLIENT || targetobjindex == MAX_CHARACTERS_CLIENT)
        return;

    CHARACTER* sc = &CharactersClient[sourceobjindex];
    OBJECT* sco = &sc->Object;

    CHARACTER* tc = &CharactersClient[targetobjindex];
    OBJECT* tco = &tc->Object;

    if (!result.succeeded)
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

void mu::ui::window::CCursedTempleSystem::EndSkill(std::uint16_t skill, std::uint16_t targetKey)
{
    WORD magNumber = skill;
    WORD targetobjkey = targetKey;
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

void mu::ui::window::CCursedTempleSystem::SetMatchStatus(const UI::CursedTemple::MatchStatus& status)
{
    m_EventMapTime = status.remainingSeconds;

    if (status.relicHolderIndex == 0xffff)
    {
        memset(&m_HolyItemPlayerName, 0, sizeof(char));
    }

    m_HolyItemPlayerIndex = status.relicHolderIndex;
    m_HolyItemPlayerPosX = status.relicX;
    m_HolyItemPlayerPosY = status.relicY;
    m_MyTeam = status.localTeam;

    wchar_t message[200];
    memset(&message, 0, sizeof(char));

    if (m_MyTeam == SEASON3A::eTeam_Allied)
    {
        if (m_AlliedPoint != status.alliedPoints)
        {
            PlayBuffer(SOUND_CURSEDTEMPLE_GAMESYSTEM4);
            StartScoreEffect();
            g_pSystemLogBox->AddText(I18N::Game::TheAlliesAreAdvancingOnWeAreNotFarFromTheVictoryChargeOn, mu::ui::window::TYPE_ERROR_MESSAGE);
        }
        else if (m_IllusionPoint != status.illusionPoints)
        {
            PlayBuffer(SOUND_CURSEDTEMPLE_GAMESYSTEM4);
            StartScoreEffect();
            g_pSystemLogBox->AddText(I18N::Game::AlthoughWeHaveLostThisBattle, mu::ui::window::TYPE_ERROR_MESSAGE);
        }
    }
    else
    {
        if (m_IllusionPoint != status.illusionPoints)
        {
            PlayBuffer(SOUND_CURSEDTEMPLE_GAMESYSTEM4);
            StartScoreEffect();
            g_pSystemLogBox->AddText(I18N::Game::HoorayForTheIllusionSorceryWe, mu::ui::window::TYPE_ERROR_MESSAGE);
        }
        else if (m_AlliedPoint != status.alliedPoints)
        {
            PlayBuffer(SOUND_CURSEDTEMPLE_GAMESYSTEM4);
            StartScoreEffect();
            g_pSystemLogBox->AddText(I18N::Game::YouMustNotLoseTheTemple, mu::ui::window::TYPE_ERROR_MESSAGE);
        }
    }

    m_AlliedPoint = status.alliedPoints;
    m_IllusionPoint = status.illusionPoints;

    m_CursedTempleMyTeamCount = static_cast<WORD>(std::min(status.party.size(), std::size(m_CursedTempleMyTeam)));
    for (int i = 0; i < m_CursedTempleMyTeamCount; ++i)
    {
        if (status.party[i].userIndex != 0xffff)
            m_CursedTempleMyTeam[i] = status.party[i];
    }
}

void mu::ui::window::CCursedTempleSystem::SetSkillPoints(std::uint8_t points)
{
    if (m_SkillPoint < points)
    {
        wchar_t message[100];
        memset(&message, 0, sizeof(char));
        mu_swprintf(message, I18N::Game::KillPointDAchieved, points - m_SkillPoint);
        g_pSystemLogBox->AddText(message, mu::ui::window::TYPE_SYSTEM_MESSAGE);
    }

    m_SkillPoint = points;
}

