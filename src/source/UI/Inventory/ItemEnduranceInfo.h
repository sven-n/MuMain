
#if !defined(AFX_NEWUIITEMENDURANCEINFO_H__ADB04FC1_C3E3_47B5_8026_C78C5800500C__INCLUDED_)
#define AFX_NEWUIITEMENDURANCEINFO_H__ADB04FC1_C3E3_47B5_8026_C78C5800500C__INCLUDED_

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/Inventory/ItemEnduranceRmlModel.h"
#include "UI/RmlBridge/RmlModelBinder.h"

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
    class CItemEnduranceInfo : public CObject
    {
    protected:
        enum IMAGE_LIST
        {
            IMAGE_PETHP_FRAME = BITMAP_ITEM_ENDURANCE_INFO_BEGIN,
            IMAGE_PETHP_BAR,
            IMAGE_ITEM_DUR_BOOTS,
            IMAGE_ITEM_DUR_CAP,
            IMAGE_ITEM_DUR_GLOVES,
            IMAGE_ITEM_DUR_LOWER,
            IMAGE_ITEM_DUR_NECKLACE,
            IMAGE_ITEM_DUR_RING,
            IMAGE_ITEM_DUR_SHIELD,
            IMAGE_ITEM_DUR_UPPER,
            IMAGE_ITEM_DUR_WEAPON,
            IMAGE_ITEM_DUR_WING,
        };

        enum IMAGE_SIZE
        {
            PETHP_FRAME_WIDTH = 57,
            PETHP_FRAME_HEIGHT = 23,
            PETHP_BAR_WIDTH = 49,
            PETHP_BAR_HEIGHT = 3,
            ITEM_DUR_WIDTH = 23,
            ITEM_DUR_HEIGHT = 23,
        };

        enum ITEM_DUR_ARROW_TYPE
        {
            ARROWTYPE_NONE = -1,
            ARROWTYPE_BOW = ITEM_BOW + 15,
            ARROWTYPE_CROSSBOW = ITEM_BOW + 7,
        };

        enum
        {
            UI_INTERVAL_HEIGHT = 1,
            UI_INTERVAL_WIDTH = 2,
        };

    private:
        CManager* m_pNewUIMng;
        POINT						m_UIStartPos;
        int							m_iTextEndPosX;
        POINT						m_ItemDurUIStartPos;

        int							m_iCurArrowType;
        int							m_iItemDurImageIndex[MAX_EQUIPMENT];
        int							m_iTooltipIndex;

    public:
        CItemEnduranceInfo();
        virtual ~CItemEnduranceInfo();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        bool BtnProcess();

        float GetLayerDepth();	//. 3.5f

        void OpenningProcess();
        void ClosingProcess();

        // Hides the document outside the main scene (CSystem::SyncMainSceneHudVisibility()).
        void SyncDocVisibility(bool sceneAllowsShow);

    private:
        // Right-aligns the durability icons to the world the open windows leave uncovered.
        void FollowUncoveredWorld();
        void LoadImages();
        void UnloadImages();

        void InitImageIndex();

        void RenderLeft();
        void RenderRight();

        void RenderHPUI(int iX, int iY, wchar_t* pszName, int iLife, int iMaxLife = 255, bool bWarning = false);
        void RenderTooltip(int iX, int iY, const ITEM* pItem, const DWORD& dwTextColor);
        //void RenderItemDurIcon( int iImageIndex, int iX, int iY, int iWidth, int iHeight, DWORD dwColor, wchar_t* pszName );
        bool RenderEquipedHelperLife(int iX, int iY);
        bool RenderEquipedPetLife(int iX, int iY);
        bool RenderSummonMonsterLife(int iX, int iY);
        bool RenderNumArrow(int iX, int iY);
        bool RenderItemEndurance(int ix, int iY);

        // The HUD in RmlUi (item_endurance.rml): main context, behind its other documents (the
        // original drew it at layer depth 3.5, under the panels). Render() fills it; the native
        // drawing is the fallback when RmlUi is not available.
        void BuildRmlUi();
        void ReloadRmlTheme();
        void SyncView();
        void SyncLeftColumn();
        void SyncIcons();
        void SyncTooltip();

        RmlModelBinder<UI::ItemEndurance::ItemEnduranceRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;
        bool m_themeReloadRegistered = false;
        bool m_sceneAllowsShow = false;
    };
}

#endif // !defined(AFX_NEWUIITEMENDURANCEINFO_H__ADB04FC1_C3E3_47B5_8026_C78C5800500C__INCLUDED_)
