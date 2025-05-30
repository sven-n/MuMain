// NewUILuckyItemWnd.h: interface for the CNewUILuckyItemWnd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUILUCKYITEMWND_H__F57DF84F_0A44_444A_838D_78CBC35544EB__INCLUDED_)
#define AFX_NEWUILUCKYITEMWND_H__F57DF84F_0A44_444A_838D_78CBC35544EB__INCLUDED_

#pragma once

#include "NewUIBase.h"
#include "NewUIInventoryCtrl.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"

namespace SEASON3B
{
#define	LUCKYITEMMAXLINE	20
    enum eNEWUIFRAME { eFrame_BG, eFrame_T, eFrame_L, eFrame_R, eFrame_B, eFrame_END };
    enum eIMGLIST { eImgList_MixBtn = eFrame_END, eImgList_END };
    enum eLUCKYITEMTYPE { eLuckyItemType_None = 0, eLuckyItemType_Trade, eLuckyItemType_Refinery, eLuckyItemAct_End };
    enum eLUCKYITEM { eLuckyItem_None = 0, eLuckyItem_Move, eLuckyItem_Act, eLuckyITem_Result, eLuckyItem_End };

    struct sImgList
    {
        float	s_fWid;
        float	s_fHgt;
        int		s_nImgIndex;
        void sImgList::Set(int _nIndex, float _fWid, float _fHgt)
        {
            s_nImgIndex = _nIndex;
            s_fWid = _fWid;
            s_fHgt = _fHgt;
        }
    };
    struct sImgFrame
    {
        sImgList	s_Img;
        POINT		s_ptPos;
    };
    struct sText
    {
        int		s_nTextIndex;	// 글로벌 텍스트 인덱스
        DWORD	s_dwColor;		// 텍스트 색깔
        int		s_nLine;		// 텍스트 정렬
    };

    class CNewUILuckyItemWnd : public CNewUIObj
    {
    private:
        CNewUIManager* m_pNewUIMng;
        CNewUIInventoryCtrl* m_pNewInventoryCtrl;
        CNewUIButton			m_BtnMix;
        float					m_fInvenClr[3];
        float					m_fInvenClrWarning[3];
        sImgList				m_sImgList[eImgList_END];
        sImgFrame				m_sFrame[eFrame_END];
        wchar_t			m_szSubject[255];
        sText					m_sText[LUCKYITEMMAXLINE];
        int						m_nTextMaxLine;
        POINT					m_ptPos;
        float					m_fSizeX;
        float					m_fSizeY;
        int						m_nResult;
        int						m_nMixEffectTimer;
        eLUCKYITEMTYPE			m_eType;
        eLUCKYITEM				m_eWndAction;
        eLUCKYITEM				m_eEnd;

    private:
        void	LoadImg(void);
        void	SetFrame(void);
        void	SetFrame_Text(eLUCKYITEM _eType);
        bool	Process_InventoryCtrl(void);

        int		GetLuckyItemRate(int _nType);
        void	Render_Frame(void);
        void	RenderMixEffect(void);
        void	Reset(void);
        void	AddText(int _nGlobalTextIndex, DWORD _dwColor = 0xFFFFFFFF, int _bLine = RT3_SORT_CENTER);

        bool	Check_LuckyItem_Trade(ITEM* _pItem);
        bool	Check_LuckyItem_Refinery(ITEM* _pItem);

        static void UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB);
    public:
        CNewUIInventoryCtrl* GetInventoryCtrl() const;

        int		SetActAction();
        STORAGE_TYPE SetMoveAction();
        void	GetResult(BYTE _byResult, int _nIndex, std::span<const BYTE> pbyItemPacket);
        bool	Process_BTN_Action(void);
        bool	Process_InventoryCtrl_InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket);
        void	Process_InventoryCtrl_DeleteItem(int iIndex);
        bool	Check_LuckyItem(ITEM* _pItem);
        bool	Check_LuckyItem_InWnd(void);

        //-Virtual Function [lem_2010.9.1]
        bool	Create(CNewUIManager* pNewUIMng, int x, int y);
        void	Release(void);
        void	OpeningProcess(void);
        bool	ClosingProcess(void);
        bool	UpdateMouseEvent();
        bool	UpdateKeyEvent();
        bool	Update();
        bool	Render();
        float	GetLayerDepth();	//. 3.4f
        //- Virtual Function End

        __inline void	SetAct(eLUCKYITEMTYPE _eAct) { m_eType = _eAct; }
        __inline void	SetPos(int _nX, int _nY) { m_ptPos.x = _nX, m_ptPos.y = _nY; }
        __inline void	SetSize(float _fX, float _fY) { m_fSizeX = _fX, m_fSizeY = _fY; }

        __inline eLUCKYITEMTYPE	GetAct(void) { return m_eType; }
        CNewUILuckyItemWnd();
        virtual ~CNewUILuckyItemWnd();
    };
}
#endif // !defined(AFX_NEWUILUCKYITEMWND_H__F57DF84F_0A44_444A_838D_78CBC35544EB__INCLUDED_)
