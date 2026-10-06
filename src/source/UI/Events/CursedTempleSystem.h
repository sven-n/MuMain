
#if !defined(AFX_NEWUICURSEDTEMPLESYSTEM_H__3018484F_9F75_48EB_8D76_31103617DCB7__INCLUDED_)
#define AFX_NEWUICURSEDTEMPLESYSTEM_H__3018484F_9F75_48EB_8D76_31103617DCB7__INCLUDED_

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/Widgets/Window/Button.h"
#include "UI/Events/CursedTempleUpdates.h"
#include "UI/HUD/MainFrameWindow.h"
#include "UI/Events/CursedTempleSystemRmlModel.h"
#include "UI/RmlBridge/RmlThemedView.h"

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
// The Illusion Temple HUD: the time, the mini map, the skill panel, the score effect and the
// tutorial step. cursed_temple_system.rml draws it; C++ keeps the state, the buttons' input and
// the hover tooltips.
class CCursedTempleSystem : public CObject
{
public:
    enum IMAGE_LIST
    {
        IMAGE_CURSEDTEMPLESYSTEM_TOP = CMessageBoxMng::IMAGE_MSGBOX_TOP,
        IMAGE_CURSEDTEMPLESYSTEM_MIDDLE = CMessageBoxMng::IMAGE_MSGBOX_MIDDLE,
        IMAGE_CURSEDTEMPLESYSTEM_BOTTOM = CMessageBoxMng::IMAGE_MSGBOX_BOTTOM,
        IMAGE_CURSEDTEMPLESYSTEM_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,
        IMAGE_CURSEDTEMPLESYSTEM_BTN = CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_VERY_SMALL,

        IMAGE_CURSEDTEMPLESYSTEM_MINIMAPFRAME = BITMAP_CURSEDTEMPLE_BEGIN + 2,
        IMAGE_CURSEDTEMPLESYSTEM_MINIMAP,
        IMAGE_CURSEDTEMPLESYSTEM_MINIMAPALPBTN,
        IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_HOLYITEM_PC,
        IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ILLUSION_HOLYITEM,
        IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ILLUSION_PC,
        IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ILLUSION_NPC,
        IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ALLIED_HOLYITEM,
        IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ALLIED_PC,
        IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ALLIED_NPC,
        IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_HERO,
        IMAGE_CURSEDTEMPLESYSTEM_SKILLFRAME,
        IMAGE_CURSEDTEMPLESYSTEM_SKILLUPBT,
        IMAGE_CURSEDTEMPLESYSTEM_SKILLDOWNBT,
        IMAGE_CURSEDTEMPLESYSTEM_GAMETIME,
        IMAGE_CURSEDTEMPLESYSTEM_SCORE_ALLIED_NUMBER,
        IMAGE_CURSEDTEMPLESYSTEM_SCORE_ILLUSION_NUMBER = IMAGE_CURSEDTEMPLESYSTEM_SCORE_ALLIED_NUMBER + 10,
        IMAGE_CURSEDTEMPLESYSTEM_SCORE_VS0 = IMAGE_CURSEDTEMPLESYSTEM_SCORE_ILLUSION_NUMBER + 10,
        IMAGE_CURSEDTEMPLESYSTEM_SCORE_VS1,
        IMAGE_CURSEDTEMPLESYSTEM_SCORE_ALLIED_GAAIL,
        IMAGE_CURSEDTEMPLESYSTEM_SCORE_ILLUSION_GAAIL,
        IMAGE_CURSEDTEMPLESYSTEM_SCORE_LEFT,
        IMAGE_CURSEDTEMPLESYSTEM_SCORE_RIGHT,
        IMAGE_SKILL2 = CSkillList::IMAGE_SKILL2,
        IMAGE_NON_SKILL2 = CSkillList::IMAGE_NON_SKILL2,
    };

    enum
    {
        CURSEDTEMPLERESULT_ALPH = 0,
        CURSEDTEMPLERESULT_SKILLUP,
        CURSEDTEMPLERESULT_SKILLDOWN,
        CURSEDTEMPLERESULT_MAXBUTTONCOUNT,
    };

public:
    CCursedTempleSystem();
    virtual ~CCursedTempleSystem();

    bool Create(CManager* pNewUIMng, int x, int y);

private:
    void LoadImages();
    void UnloadImages();
    void SetButtonInfo();

public:
    bool UpdateMouseEvent();
    bool UpdateKeyEvent();
    bool Update();

public:
    bool CheckInventoryHolyItem(CHARACTER* c);
    bool CheckTalkProgressNpc(DWORD npcindex, DWORD npckey);
    bool CheckHeroSkillType(int operatortype = 0);
    bool CheckDragonRender();
    bool IsCursedTempleSkillKey(DWORD selectcharacterindex);

private:
    void UpdateScore();
    void UpdateTutorialStep();

public:
    bool Render();


private:
    void BuildRmlUi();
    // Builds the HUD's images and texts in the original Render()'s order and syncs the document.
    void SyncView();
    void SyncGameTime(std::vector<CursedTempleSpriteEntry>& sprites);
    void SyncMiniMap(std::vector<CursedTempleSpriteEntry>& sprites);
    void SyncSkill(std::vector<CursedTempleSpriteEntry>& sprites);
    void SyncScore();
    void SyncTutorialStep(std::vector<CursedTempleTextEntry>& lines);

public:
    const POINT& GetPos() const;
    float GetLayerDepth(); //. 1.5f

public:
    void SetPos(int x, int y);
    void ResetCursedTempleSystemInfo();
    void StartScoreEffect();
    void StartTutorialStep();
    void EndScoreEffect();
    void EndTutorialStep();

    SEASON3A::eCursedTempleTeam GetMyTeam();

public:
    void SetCursedTempleSkill(CHARACTER* c, OBJECT* o, DWORD selectcharacterindex);
    void ResolveSkill(const UI::CursedTemple::SkillResult& result);
    void EndSkill(std::uint16_t skill, std::uint16_t targetKey);
    void SetMatchStatus(const UI::CursedTemple::MatchStatus& status);
    void SetSkillPoints(std::uint8_t points);

private:
    void Initialize();
    void Destroy();

private:
    CManager* m_pNewUIMng;
    POINT m_Pos;
    CButton m_Button[CURSEDTEMPLERESULT_MAXBUTTONCOUNT];
    // EventTime
    DWORD m_EventMapTime; // Total event time
    // MiniMap
    WORD m_HolyItemPlayerIndex; // Index of the player holding the Holy Item
    WORD m_HolyItemPlayerPosX;  // Sacred Item X-coordinate
    WORD m_HolyItemPlayerPosY;  // Sacred Item Y-coordinate
    wchar_t m_HolyItemPlayerName[MAX_USERNAME_SIZE];

    float m_Scale;
    float m_Alph;

    // HolyItemCount
    WORD m_AlliedPoint;   // Allied Forces score
    WORD m_IllusionPoint; // Illusion Cult score

    WORD m_CursedTempleMyTeamCount;
    UI::CursedTemple::PartyPosition m_CursedTempleMyTeam[MAX_PARTYS];
    // Team
    SEASON3A::eCursedTempleTeam m_MyTeam;
    // skillpoint
    WORD m_SkillPoint;
    // Score
    bool m_IsScoreEffect;
    DWORD m_StartScoreEffectTime;
    WORD m_ScoreEffectState;
    float m_ScoreEffectAlph;
    // Tutorial Step
    bool m_IsTutorialStep;
    WORD m_TutorialStepState;
    DWORD m_TutorialStepTime;

    void BindRmlModel(Rml::DataModelConstructor& c, CursedTempleSystemRmlModel& model);
    // The original drew the HUD at layer depth 1.5, under nearly every panel: the document sits in the
    // background context, behind its other documents.
    UI::RmlBridge::ThemedView<CursedTempleSystemRmlModel> m_RmlView{"cursed_temple_system",
        [this](Rml::DataModelConstructor& c, CursedTempleSystemRmlModel& model) { BindRmlModel(c, model); },
        {{"Data/Interface/RmlUi/cursed_temple_system.rml"}}};
};

    inline
        const POINT& CCursedTempleSystem::GetPos() const
    {
        return m_Pos;
    }

    inline
        float CCursedTempleSystem::GetLayerDepth()	//. 1.5f
    {
        return 1.5f;
    }

    inline
        void CCursedTempleSystem::SetPos(int x, int y)
    {
        m_Pos.x = x; m_Pos.y = y;
    }
};

#endif // !defined(AFX_NEWUICURSEDTEMPLESYSTEM_H__3018484F_9F75_48EB_8D76_31103617DCB7__INCLUDED_)
