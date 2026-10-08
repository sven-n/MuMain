#pragma once
#include "Core/Globals/_TextureIndex.h"

#ifdef PBG_ADD_GENSRANKING

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/Inventory/MyInventory.h"
#include "UI/Widgets/Window/Button.h"
#include "UI/Widgets/Window/TextBox.h"
#include "UI/HUD/GensRankingRmlModel.h"
#include "UI/RmlBridge/RmlThemedView.h"
#define MAX_TITLELENGTH		32

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
// The Gens ranking window, docked right. gens_ranking.rml draws it; the rewards text box and
// its reward lines keep the native wrapping while RmlUi owns scrolling and drawing.
// RanderMark() still draws the small family marks over players (UI/Chat).
class CGensRanking : public CObject
{
    enum IMAGE_LIST
    {
        IMAGE_NEWMARK_DUPRIAN = BITMAP_GENS_MARK_DUPRIAN,
        IMAGE_NEWMARK_BARNERT = BITMAP_GENS_MARK_BARNERT,
    };

    static constexpr float GENSRANKING_WIDTH = 190.0f;
    static constexpr float GENSRANKING_HEIGHT = 429.0f;
    static constexpr int TEAMNAME_LENTH = 10;
    static constexpr float GENSMARK_WIDTH = 50.0f;
    static constexpr float GENSMARK_HEIGHT = 69.0f;
    static constexpr float GENSRANKBACK_WIDTH = 170.0f;
    static constexpr float GENSRANKBACK_HEIGHT = 88.0f;
    static constexpr float GENSRANKTEXTBACK_WIDTH = 170.0f;
    static constexpr float GENSRANKTEXTBACK_HEIGHT = 21.0f;

    enum IMAGE_INDEX
    {
        TITLENAME_NONE = 0,
        TITLENAME_START = 1,
        TITLENAME_END = 14,
    };

public:
    enum IMAGE_AREA
    {
        MARK_UIINFO = 0,
        MARK_BOOLEAN,
        MARK_RANKINFOWIN,
    };

    enum GENS_TYPE
    {
        GENSTYPE_NONE = 0,
        GENSTYPE_DUPRIAN,
        GENSTYPE_BARNERT,
    };

private:
    void Init();
    void Destroy();
    POINT m_Pos;

    FLOAT m_fBooleanSize;

    int m_nContribution;
    wchar_t m_szRanking[TEAMNAME_LENTH];
    wchar_t m_szGensTeam[TEAMNAME_LENTH];

    GENS_TYPE m_byGensInfluence;
    POINT m_ptRenderMarkPos;

    void BindRmlModel(Rml::DataModelConstructor& c, GensRankingRmlModel& model);
    UI::RmlBridge::ThemedView<GensRankingRmlModel> m_RmlView{"gens_ranking",
        [this](Rml::DataModelConstructor& c, GensRankingRmlModel& model) { BindRmlModel(c, model); },
        {{"Data/Interface/RmlUi/gens_ranking.rml"}}};
    bool m_PendingExit = false;

    void BuildRmlUi();
    void SyncRmlModel();
    void SyncContent();

    wchar_t m_szTitleName[TITLENAME_END][MAX_TITLELENGTH];

    int m_nNextContribution;

public:
    CManager* m_pNewUIMng;
    CTextBox* m_pTextBox;
    CGensRanking();
    virtual ~CGensRanking();

    bool Create(CManager* pNewUIMng, int x, int y);
    Rml::ElementDocument* GetFillDocument() const override { return m_RmlView.Document(); }
    Rml::ElementDocument* GetPlacedDocument() const override { return m_RmlView.Document(); }
    void SetPos(int x, int y);
    const POINT& GetPos()
    {
        return m_Pos;
    }

    bool Render();

    bool Update();
    bool UpdateMouseEvent();
    bool UpdateKeyEvent();


    void OpenningProcess();
    void ClosingProcess();

    float GetLayerDepth()
    {
        return 4.2f;
    }

    void SetContribution(int _Contribution);
    int GetContribution();

    void SetNextContribution(int _NextContribution);
    int GetNextContribution();

    bool SetRanking(int _Ranking);
    wchar_t* GetRanking();

    bool SetGensInfo();
    bool SetGensTeamName(const wchar_t* _pTeamName);
    wchar_t* GetGensTeamName();

    void SetTitleName();
    wchar_t* GetTitleName(BYTE _index);

    void RanderMark(float _x, float _y, GENS_TYPE _GensInfluence, BYTE _GensRankInfo,
                    IMAGE_AREA _ImageArea = MARK_RANKINFOWIN, float _RenderY = 0);
    int GetImageIndex(BYTE _index);
};
}

#endif //PBG_ADD_GENSRANKING
