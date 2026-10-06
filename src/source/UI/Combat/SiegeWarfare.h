
#if !defined(AFX_CNewUISiegeWarfare_H__6810678B_808B_4765_B9AC_AC34344E7E2D__INCLUDED_)
#define AFX_CNewUISiegeWarfare_H__6810678B_808B_4765_B9AC_AC34344E7E2D__INCLUDED_

#pragma once

#include "UI/Core/WindowObject.h"
#include "Core/Globals/_struct.h"
#include "UI/Core/WindowManager.h"
#include "UI/Combat/SiegeWarBase.h"
#include "UI/Combat/SiegeWarfareRmlModel.h"
#include "UI/RmlBridge/RmlThemedView.h"

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
// The castle siege HUD on the Valley of Loren: the observer, soldier or commander variant
// (m_pSiegeWarUI) keeps the state and the input; siege_warfare.rml draws it.
class CSiegeWarfare : public CObject
{
public:
    enum SIEGEWAR_TYPE
    {
        SIEGEWAR_TYPE_NONE = -1,
        SIEGEWAR_TYPE_OBSERVER = 0,
        SIEGEWAR_TYPE_COMMANDER,
        SIEGEWAR_TYPE_SOLDIER,
    };

private:
    CManager* m_pNewUIMng;
    POINT m_Pos;

    CSiegeWarBase* m_pSiegeWarUI;
    short m_sGuildMarkIndex;
    BYTE m_byGuildStatus;
    int m_iCurSiegeWarType;

    int m_iHour;
    int m_iMinute;
    int m_iSecond;
    DWORD m_dwSyncTime;

    bool m_bCreated;

public:
    CSiegeWarfare();
    virtual ~CSiegeWarfare();

    bool Create(CManager* pNewUIMng, int x, int y);
    bool CreateMiniMapUI();
    void InitMiniMapUI();
    void SetGuildData(const CHARACTER* pCharacter);
    void Release();

    void SetPos(int x, int y);

    bool UpdateMouseEvent();
    bool UpdateKeyEvent();
    bool Update();
    bool Render();

    bool BtnProcess();

    float GetLayerDepth(); //. 1.6f

    void OpenningProcess();
    void ClosingProcess();

    void ClearGuildMemberLocation(void);
    void SetGuildMemberLocation(BYTE type, int x, int y);
    // The variant `type` instead of the one the hero's guild status picks (UI::EventPreview).
    void CreatePreviewMiniMapUI(SIEGEWAR_TYPE type);
    void SetTime(BYTE byHour, BYTE byMinute);

    void SetMapInfo(GuildCommander& data);

public:
    inline CSiegeWarBase* GetBase()
    {
        if (!m_pSiegeWarUI)
            return NULL;

        return m_pSiegeWarUI;
    }

    inline int GetCurSiegeWarType()
    {
        return m_iCurSiegeWarType;
    };
    inline bool IsCreated()
    {
        return m_bCreated;
    };

    void InitSkillUI();
    void ReleaseSkillUI();


private:
    void BuildRmlUi();
    void SyncRmlModel();
    void ApplyRmlModel(const SiegeWarfareRmlModel& next);

    void BindRmlModel(Rml::DataModelConstructor& c, SiegeWarfareRmlModel& model);
    // The original drew the HUD under nearly every other window (layer depth 1.6), and a docked
    // panel's frame is painted in the background context before the native windows: only a document
    // in that same context, behind the others, stays under them (as the duel and battle-soccer boards
    // do). The durability warnings, the logs and every native window then draw over the HUD.
    UI::RmlBridge::ThemedView<SiegeWarfareRmlModel> m_RmlView{"siege_warfare",
        [this](Rml::DataModelConstructor& c, SiegeWarfareRmlModel& model) { BindRmlModel(c, model); },
        {{"Data/Interface/RmlUi/siege_warfare.rml"}}};
    // The frame's values, filled in place every frame: its collections keep their storage, and it
    // holds last frame's values where FillRmlModel() leaves a field alone.
    SiegeWarfareRmlModel m_NextRmlModel;
};
}

#endif // !defined(AFX_CNewUISiegeWarfare_H__6810678B_808B_4765_B9AC_AC34344E7E2D__INCLUDED_)