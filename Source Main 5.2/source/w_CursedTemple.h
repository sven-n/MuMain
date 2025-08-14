#pragma once

class BMD;
namespace SEASON3A
{
    class CursedTemple
    {
    public:
        virtual ~CursedTemple();

    private:
        CursedTemple();
        void Initialize();
        void Destroy();

    public:
        void Process();
        void Draw();

    public:
        static CursedTemple* GetInstance();
        bool GetInterfaceState(int type, int subtype = -1);
        void SetInterfaceState(bool state, int subtype = -1);

    public:
        bool IsHolyItemPickState();
        bool IsPartyMember(DWORD selectcharacterindex);
        void ReceiveCursedTempleInfo(const BYTE* ReceiveBuffer);
        void ReceiveCursedTempleState(const eCursedTempleState state);

    public:
        bool CreateObject(OBJECT* o);
        CHARACTER* CreateCharacters(EMonsterType iType, int iPosX, int iPosY, int iKey);

    public:
        bool SetCurrentActionMonster(CHARACTER* c, OBJECT* o);
        bool AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b);
        void PlayBGM();
        void ResetCursedTemple();

    public:
        bool MoveObject(OBJECT* o);
        void MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b);
        bool MoveMonsterVisual(OBJECT* o, BMD* b);
        void MoveMonsterSoundVisual(OBJECT* o, BMD* b);

    public:
        bool RenderObject_AfterCharacter(OBJECT* o, BMD* b);
        bool RenderObjectVisual(OBJECT* o, BMD* b);
        bool RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);
        bool RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);
        void UpdateTempleSystemMsg(int _Value);
        void SetGaugebarEnabled(bool bFlag);
        void SetGaugebarCloseTimer();
        bool IsGaugebarEnabled();

    private:
        bool			m_IsTalkEnterNpc;
        bool			m_InterfaceState;
        WORD			m_HolyItemPlayerIndex;
        eCursedTempleState	m_CursedTempleState;
        std::list<int>			m_TerrainWaterIndex;
        WORD				m_AlliedPoint;
        WORD				m_IllusionPoint;
        bool				m_ShowAlliedPointEffect;
        bool				m_ShowIllusionPointEffect;
        bool				m_bGaugebarEnabled;
        float				m_fGaugebarCloseTimer;
    };
};

#define g_TimeController SEASON3A::TimeController::GetInstance()
#define g_CursedTemple SEASON3A::CursedTemple::GetInstance()
