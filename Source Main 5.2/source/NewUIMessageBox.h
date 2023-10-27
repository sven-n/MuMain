// NEWUIMESSAGEBOX.h: interface for the CNEWUIMESSAGEBOX class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NewUIBase.h"
#include "./ExternalObject/leaf/xstreambuf.h"
#include "ZzzTexture.h"
#include "UIBaseDef.h"

#pragma warning(disable : 4786)
#include <string>
#include <queue>

namespace SEASON3B
{
    enum CALLBACK_RESULT
    {
        CALLBACK_CONTINUE = 0,
        CALLBACK_BREAK,
        CALLBACK_EXCEPTION,
        CALLBACK_POP_ALL_EVENTS,
    };
    typedef CALLBACK_RESULT(*EVENT_CALLBACK)(class CNewUIMessageBoxBase*, const leaf::xstreambuf&);

    enum
    {
        MSGBOX_EVENT_NONE = 0,
        MSGBOX_EVENT_DESTROY,
        MSGBOX_EVENT_MOUSE_HOVER,
        MSGBOX_EVENT_MOUSE_LBUTTON_DOWN,
        MSGBOX_EVENT_MOUSE_LBUTTON_UP,
        MSGBOX_EVENT_MOUSE_RBUTTON_DOWN,
        MSGBOX_EVENT_MOUSE_RBUTTON_UP,
        MSGBOX_EVENT_PRESSKEY_ESC,
        MSGBOX_EVENT_PRESSKEY_RETURN,
        MSGBOX_EVENT_USER_DEFINE = 0x0F00,
        // common msgbox
        MSGBOX_EVENT_USER_COMMON_OK,
        MSGBOX_EVENT_USER_COMMON_CANCEL,
        // keypad
        MSGBOX_EVENT_USER_CUSTOM_KEYPAD_INPUT,
        MSGBOX_EVENT_USER_CUSTOM_KEYPAD_DELETE,
        // use fruit
        MSGBOX_EVENT_USER_CUSTOM_USE_FRUIT_ADD,
        MSGBOX_EVENT_USER_CUSTOM_USE_FRUIT_MINUS,
        // gem interation
        MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY,
        MSGBOX_EVENT_USER_CUSTOM_GEM_DISJOINT,
        MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY_BLESSING,
        MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY_SOUL,
        MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY_TEN,
        MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY_TWENTY,
        MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY_THIRTY,
        MSGBOX_EVENT_USER_CUSTOM_GEM_DISJOINT_BLESSING,
        MSGBOX_EVENT_USER_CUSTOM_GEM_DISJOINT_SOUL,
        MSGBOX_EVENT_USER_CUSTOM_GEM_DISJOINT_DISJOINT,
        // system menu
        MSGBOX_EVENT_USER_CUSTOM_SYSTEMMENU_GAMEOVER,
        MSGBOX_EVENT_USER_CUSTOM_SYSTEMMENU_CHOOSESERVER,
        MSGBOX_EVENT_USER_CUSTOM_SYSTEMMENU_CHOOSECHARACTER,
        MSGBOX_EVENT_USER_CUSTOM_SYSTEMMENU_OPTION,
        // mix menu
        MSGBOX_EVENT_USER_CUSTOM_MIXMENU_GENERALMIX,
        MSGBOX_EVENT_USER_CUSTOM_MIXMENU_CHAOSMIX,
        MSGBOX_EVENT_USER_CUSTOM_MIXMENU_MIX380,
        // trainer menu
        MSGBOX_EVENT_USER_CUSTOM_TRAINER_MENU_RECOVER,
        MSGBOX_EVENT_USER_CUSTOM_TRAINER_MENU_REVIVE,
        MSGBOX_EVENT_USER_CUSTOM_TRAINER_MENU_RECOVER_DARKSPRIT,
        MSGBOX_EVENT_USER_CUSTOM_TRAINER_MENU_RECOVER_DARKHORSE,
        // elpis menu
        MSGBOX_EVENT_USER_CUSTOM_ELPIS_ABOUT_REFINARY,
        MSGBOX_EVENT_USER_CUSTOM_ELPIS_ABOUT_JEWELOFHARMONY,
        MSGBOX_EVENT_USER_CUSTOM_ELPIS_REFINE,
        // dialog
        MSGBOX_EVENT_USER_CUSTOM_DIALOG_END,
        // progress
        MSGBOX_EVENT_USER_CUSTOM_PROGRESS_CLOSINGPROCESS,
        MSGBOX_EVENT_USER_CUSTOM_PROGRESS_COMPLETEPROCESS,
        // duel
        MSGBOX_EVENT_USER_CUSTOM_DUEL_OK,
        MSGBOX_EVENT_USER_CUSTOM_DUEL_CANCEL,
        MSGBOX_EVENT_USER_CUSTOM_CB_WHITE,
        MSGBOX_EVENT_USER_CUSTOM_CB_RED,
        MSGBOX_EVENT_USER_CUSTOM_CB_GOLD,
        // seed master menu
        MSGBOX_EVENT_USER_CUSTOM_SEED_MASTER_MENU_EXTRACT_SEED,
        MSGBOX_EVENT_USER_CUSTOM_SEED_MASTER_MENU_SEED_SPHERE,
        // seed investigator menu
        MSGBOX_EVENT_USER_CUSTOM_SEED_INVESTIGATOR_MENU_ATTACH_SOCKET,
        MSGBOX_EVENT_USER_CUSTOM_SEED_INVESTIGATOR_MENU_DETACH_SOCKET,

        MSGBOX_EVENT_USER_CUSTOM_RESET_CHARACTER_POINT,

        MSGBOX_EVENT_USER_CUSTOM_DELGARDO_REGISTRATION_LUCKY_COIN,
        MSGBOX_EVENT_USER_CUSTOM_DELGARDO_EXCHANGE_LUCKY_COIN,

        MSGBOX_EVENT_USER_CUSTOM_INGAMESHOP_PRESENT,

        MSGBOX_EVENT_USER_CUSTOM_LUCKYITEM_TRADE,
        MSGBOX_EVENT_USER_CUSTOM_LUCKYITEM_REFINERY,

        MSGBOX_EVENT_USER_CUSTOM_GEM_SELECTMIX,
        MSGBOX_EVENT_USER_CUSTOM_GEM_SELECT,
    };

    class CNewUIManager;

    class CNewUIMessageBoxBase
    {
        typedef std::map<DWORD, EVENT_CALLBACK>	type_map_callback;

        POINT	m_Pos;
        SIZE	m_Size;
        float	m_fPriority;
        bool	m_bCanMove;

        type_map_callback	m_mapCallbacks;

        float m_fOpacityAlpha;
        vec3_t m_vColor;
    public:
        CNewUIMessageBoxBase();
        virtual ~CNewUIMessageBoxBase();

        virtual bool Create(int x, int y, int width, int height, float fPriority = 3.f);
        virtual void Release();

        virtual void SetPos(int x, int y);
        void SetSize(int width, int height);
        const POINT& GetPos();
        const SIZE& GetSize();
        void SetCanMove(bool bCanMove);
        bool CanMove();

        float GetPriority() const;

        void AddCallbackFunc(EVENT_CALLBACK pFunc, DWORD dwEvent);
        void RemoveCallbackFunc(DWORD dwEvent);
        void RemoveAllCallbackFuncs();

        EVENT_CALLBACK GetCallbackFunc(DWORD dwEvent);

        virtual bool Update() = 0;
        virtual bool Render() = 0;

        void SendEvent(CNewUIMessageBoxBase* pOwner, DWORD dwEvent);
        void SendEvent(CNewUIMessageBoxBase* pOwner, DWORD dwEvent, const leaf::xstreambuf& xParam);
        void RenderMsgBackColor(bool _bRender = false);
        void SetMsgBackOpacity(float _fAlpha = 0.5f);
        const float& GetMsgBackOpacity() const { return m_fOpacityAlpha; }
        void SetMsgBackColor(vec3_t _vColor = NULL);
        const vec3_t& GetMsgBackColor() const { return m_vColor; }
    };

    class CNewUIMessageBoxFactory
    {
        typedef std::list<CNewUIMessageBoxBase*>	type_list_msgbox;
        type_list_msgbox	m_listMsgBoxes;

    public:
        CNewUIMessageBoxFactory() {}
        ~CNewUIMessageBoxFactory() { DeleteAllMessageBoxes(); }

        enum INSTANCE_STATE { INSTANCE_NEW = 1, INSTANCE_REFERENCE };
        template <class T>
        class TContainer
        {
            T* m_pObj;
            INSTANCE_STATE	m_InstState;
        public:

            TContainer() : m_pObj(NULL), m_InstState(INSTANCE_NEW) { m_pObj = new T; }		//. create instance
            TContainer(TContainer& _container) : m_pObj(NULL), m_InstState(INSTANCE_REFERENCE)
            {
                if (_container.GetInstance())
                    m_pObj = _container.GetInstance();	//. copy instance
            }
            virtual ~TContainer()
            {
                if (m_InstState == INSTANCE_NEW)
                    delete m_pObj;
            }

            T* GetInstance() { return m_pObj; }
            void SetInstState(INSTANCE_STATE InstState) { m_InstState = InstState; }

            TContainer<T>& operator = (const TContainer<T>& _container)
            {
                if (_container.GetInstance())
                {
                    if (m_InstState == INSTANCE_NEW)
                        delete m_pObj;
                    m_pObj = _container.GetInstance();
                    m_InstState = INSTANCE_REFERENCE;
                }
            }
        };
        template <class T>

        T* NewMessageBox(TContainer<T>& _container)
        {
            T* pObj = _container.GetInstance();
            m_listMsgBoxes.push_back(pObj);
            _container.SetInstState(INSTANCE_REFERENCE);
            return pObj;
        }

        void DeleteMessageBox(const CNewUIMessageBoxBase* pObj)
        {
            auto li =
                std::find(m_listMsgBoxes.begin(), m_listMsgBoxes.end(), pObj);
            if (li != m_listMsgBoxes.end())
            {
                delete (*li);
                m_listMsgBoxes.erase(li);
            }
        }
        void DeleteAllMessageBoxes()
        {
            auto li = m_listMsgBoxes.begin();
            for (; li != m_listMsgBoxes.end(); li++)
                delete (*li);	//. delete instance
            m_listMsgBoxes.clear();
        }
    };

    class CNewUIMessageBoxMng : public CNewUIObj
    {
        class CNewUIEvent
        {
            CNewUIMessageBoxBase* m_pOwner;
            DWORD	m_dwEvent;
            leaf::xstreambuf	m_xParam;

        public:
            CNewUIEvent(CNewUIMessageBoxBase* pOwner, DWORD dwEvent) : m_pOwner(pOwner), m_dwEvent(dwEvent) {}
            CNewUIEvent(CNewUIMessageBoxBase* pOwner, DWORD dwEvent, const leaf::xstreambuf& param)
            {
                m_pOwner = pOwner;
                m_dwEvent = dwEvent;
                m_xParam = param;
            }

            CNewUIMessageBoxBase* GetOwner() const { return m_pOwner; }
            DWORD GetEvent() const { return m_dwEvent; }
            const leaf::xstreambuf& GetParam() const { return m_xParam; }
        };

        enum EVENT_STATE
        {
            EVENT_NONE = 0,
            EVENT_WND_MOUSE_HOVER,
            EVENT_WND_MOUSE_LBUTTON_DOWN,
            EVENT_WND_MOUSE_RBUTTON_DOWN,
        };

        typedef std::vector<CNewUIMessageBoxBase*> type_vector_msgbox;
        typedef std::queue<CNewUIEvent*>	type_queue_event;

        CNewUIManager* m_pNewUIMng;
        CNewUIMessageBoxFactory* m_pMsgBoxFactory;
        type_vector_msgbox			m_vecMsgBoxes;
        type_queue_event			m_queueEvents;
        EVENT_STATE					m_EventState;

    public:
        enum IMAGE_LIST
        {
            IMAGE_MSGBOX_TOP = BITMAP_INTERFACE_NEW_MESSAGEBOX_BEGIN,
            IMAGE_MSGBOX_MIDDLE,
            IMAGE_MSGBOX_BOTTOM,
            IMAGE_MSGBOX_BACK,
            IMAGE_MSGBOX_BTN_OK,
            IMAGE_MSGBOX_BTN_CANCEL,
            IMAGE_MSGBOX_BTN_CLOSE,
            IMAGE_MSGBOX_BTN_EMPTY,
            IMAGE_MSGBOX_BTN_EMPTY_SMALL,
            IMAGE_MSGBOX_BTN_EMPTY_BIG,
            IMAGE_MSGBOX_BTN_EMPTY_VERY_SMALL,
            IMAGE_MSGBOX_LINE,
            IMAGE_MSGBOX_TOP_TITLEBAR,
            IMAGE_MSGBOX_SEPARATE_LINE,
            IMAGE_MSGBOX_PROGRESS_BG,
            IMAGE_MSGBOX_PROGRESS_BAR,
            IMAGE_MSGBOX_DUEL_BACK,
        };

        virtual ~CNewUIMessageBoxMng();

        bool Create(CNewUIManager* pNewUIMng);
        void Release();

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();		//. 10.7f
        float GetKeyEventOrder();	//. 10.f

        static CNewUIMessageBoxMng* GetInstance();
        static bool ComparePriority(CNewUIMessageBoxBase* pObj1, CNewUIMessageBoxBase* pObj2);

        template <class T>

        T* NewMessageBox(CNewUIMessageBoxFactory::TContainer<T>& container)
        {
            T* pMsgBox = NULL;
            if (m_pMsgBoxFactory)
            {
                pMsgBox = m_pMsgBoxFactory->NewMessageBox(container);
                m_vecMsgBoxes.push_back(pMsgBox);
            }
            return pMsgBox;
        }

        void DeleteMessageBox(const CNewUIMessageBoxBase* pObj);
        void PopMessageBox();
        void PopAllMessageBoxes();

        bool IsEmpty();

        void SendEvent(CNewUIMessageBoxBase* pOwner, DWORD dwEvent);
        void SendEvent(CNewUIMessageBoxBase* pOwner, DWORD dwEvent, const leaf::xstreambuf& xParam);

    protected:
        CNewUIMessageBoxMng();

        void PopEvent();
        void PopAllEvents();

    private:
        void LoadImages();
        void UnloadImages();
    };

    class IMsgBoxLayout
    {
    public:
        virtual bool SetLayout() = 0;
    };

    template <class _M>
    class TMsgBoxLayout : public IMsgBoxLayout
    {
        static _M* ms_pMsgBox;
    public:
        TMsgBoxLayout()
        {
            CNewUIMessageBoxMng* pNewUIMsgBoxMng = CNewUIMessageBoxMng::GetInstance();
            ms_pMsgBox = pNewUIMsgBoxMng->NewMessageBox(CNewUIMessageBoxFactory::TContainer<_M>());
        }
        virtual ~TMsgBoxLayout() {}

        static _M* GetMsgBox() { return ms_pMsgBox; }
    };
    template<class _M> _M* TMsgBoxLayout<_M>::ms_pMsgBox = NULL;

    template <class _L>
    class TMsgBoxLayoutContainer
    {
        _L* m_pMsgBoxLayout;
    public:
        TMsgBoxLayoutContainer() : m_pMsgBoxLayout(NULL) {}
        ~TMsgBoxLayoutContainer() { Release(); }

        bool Create()
        {
            if (m_pMsgBoxLayout)
                return false;
            m_pMsgBoxLayout = new _L;
            return true;
        }
        void Release()
        {
            delete m_pMsgBoxLayout;
            m_pMsgBoxLayout = NULL;
        }
        bool SetLayout()
        {
            if (m_pMsgBoxLayout)
                return m_pMsgBoxLayout->SetLayout();
            return false;
        }
    };

    template <class _L>
    bool CreateMessageBox(TMsgBoxLayoutContainer<_L>& container)
    {
        if (false == container.Create()) //. MessageBox Layout
            return false;

        return container.SetLayout();
    }
    template <class _L, class _M>
    bool CreateMessageBox(TMsgBoxLayoutContainer<_L>& container, _M** ppMsgBox)
    {
        if (false == container.Create()) //. MessageBox Layout
            return false;
        if (ppMsgBox)
            *ppMsgBox = TMsgBoxLayout<_M>::GetMsgBox();

        return container.SetLayout();
    }
}

#define MSGBOX_LAYOUT_CLASS(x) SEASON3B::TMsgBoxLayoutContainer<x>()
#define MSGBOX_CLASS(x) SEASON3B::CNewUIMessageBoxFactory::TContainer<x>()
#define g_MessageBox SEASON3B::CNewUIMessageBoxMng::GetInstance()
