
#if !defined(AFX_NEWUISIEGEWARBASE_H__13F2F04C_290F_41FC_A7A5_2F56F22B3478__INCLUDED_)
#define AFX_NEWUISIEGEWARBASE_H__13F2F04C_290F_41FC_A7A5_2F56F22B3478__INCLUDED_

#pragma once

#define MAX_COMMANDGROUP ( 7 )

#include "UI/HUD/MainFrameWindow.h"
#include "UI/Combat/SiegeWarfareRmlModel.h"

namespace Rml
{
class Element;
class ElementDocument;
}

namespace mu::ui::window
{

class CSiegeWarBase
{
public:
    enum FRAME_SIZE
    {
        MINIMAP_FRAME_WIDTH = 154,
        MINIMAP_FRAME_HEIGHT = 162,
        TIME_FRAME_WIDTH = 134,
        TIME_FRAME_HEIGHT = 37,
        MINIMAP_BTN_ALPHA_WIDTH = 30,
        MINIMAP_BTN_ALPHA_HEIGHT = 22,
        COMMAND_ATTACK_WIDTH = 13,
        COMMAND_ATTACK_HEIGHT = 13,
        COMMAND_DEFENCE_WIDTH = 18,
        COMMAND_DEFENCE_HEIGHT = 15,
        COMMAND_WAIT_WIDTH = 11,
        COMMAND_WAIT_HEIGHT = 12,
        BATTLESKILL_FRAME_WIDTH = 128,
        BATTLESKILL_FRAME_HEIGHT = 53,
        SKILL_BTN_SCROLL_WIDTH = 15,
        SKILL_BTN_SCROLL_HEIGHT = 13,
        SKILL_ICON_WIDTH = 20,
        SKILL_ICON_HEIGHT = 28,
        SKILL_TOOLTIP_WIDTH = 128,
        SKILL_TOOLTIP_HEIGHT = 32,
        BTN_ALPHA_WIDTH = 38,
        BTN_ALPHA_HEIGHT = 23,
    };

protected:
    POINT m_MiniMapFramePos;
    POINT m_MiniMapPos;
    POINT m_SkillFramePos;

    POINT m_HeroPosInWorld;
    POINT m_HeroPosInMiniMap;
    POINT m_MiniMapScaleOffset;

    float m_fMiniMapTexU;
    float m_fMiniMapTexV;

    int m_iMiniMapScale;
    float m_fMiniMapAlpha;
    bool m_bRenderSkillUI;
    bool m_bRenderToolTip;

    bool m_bSecond;
    int m_iHour;
    int m_iMinute;
    float m_fTime;

    DWORD m_dwBuffState;

    GuildCommander m_CmdBuffer[MAX_COMMANDGROUP];
    // The HUD's document (CSiegeWarfare's): the HUD hit-tests its drawn elements.
    Rml::ElementDocument* m_Document = nullptr;

    std::list<int> m_listBattleSkill;
    std::list<int>::iterator m_iterCurBattleSkill;

public:
    CSiegeWarBase();
    virtual ~CSiegeWarBase();

protected:
    virtual bool OnCreate(int x, int y) = 0;
    virtual bool OnUpdate() = 0;
    virtual void OnRelease() = 0;
    virtual bool OnUpdateMouseEvent() = 0;
    virtual bool OnUpdateKeyEvent() = 0;
    virtual bool OnBtnProcess() = 0;
    virtual void OnSetPos(int x, int y) = 0;
    // The variant's own parts of the picture (dots, commands, the commander's buttons), in
    // the original's OnRender() order.
    virtual void OnFillRmlModel(SiegeWarfareRmlModel& model) = 0;

    // A point of the world (tile x, y) on the mini map, reference px.
    POINT MiniMapPoint(int x, int y) const;
    // An element of the HUD's document, or null.
    Rml::Element* Element(const char* id) const;
    void FillCommands(SiegeWarfareRmlModel& model);

public:
    bool Create(int x, int y);
    bool Update();
    // Everything the original's Render() drew, for siege_warfare.rml; also shows or hides
    // the battle skill's tooltip as the original's RenderSkillIcon() did.
    void FillRmlModel(SiegeWarfareRmlModel& model);
    void Release();

    bool UpdateMouseEvent();
    bool UpdateKeyEvent();
    void SetPos(int x, int y);
    void SetTime(int iHour, int iMinute);
    void SetMapInfo(GuildCommander& data);
    void SetRenderSkillUI(bool bRenderSkillUI);

    bool InitBattleSkill();
    void ReleaseBattleSkill();

    void SetDocument(Rml::ElementDocument* document) { m_Document = document; }
    // The document's buttons (siege_warfare.rml): the transparency, the map's zoom, the battle
    // skill's scroll, and the commander's team and command buttons.
    void ToggleAlpha();
    void ToggleMiniMapScale();
    void ScrollSkillUp();
    void ScrollSkillDown();
    virtual void OnTeamClick(int team) {}
    virtual void OnOrderClick(int order) {}

private:
    bool BtnProcess();
    void UpdateBuffState();
    void UpdateHeroPos();
    void FillSkill(SiegeWarfareRmlModel& model);

    void SetSkillScrollUp();
    void SetSkillScrollDn();
};
}

#endif // !defined(AFX_NEWUISIEGEWARBASE_H__13F2F04C_290F_41FC_A7A5_2F56F22B3478__INCLUDED_)
