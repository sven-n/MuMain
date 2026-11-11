#pragma once

#include "Render/Models/DrawLook.h"

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
        // A new storage shows no point effect its team's points had (the
        // spawn hook IllusionTempleStorageReset): the allied or the
        // illusion one.
        void HideStoragePointEffect(bool allied);
        bool IsStoragePointEffectShown(bool allied) const;
        void ReceiveCursedTempleState(const eCursedTempleState state);

    public:
        bool CreateObject(OBJECT* o);

    public:
        bool AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b);
        void PlayBGM();
        void ResetCursedTemple();

    public:
        bool MoveObject(OBJECT* o);
        void MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b);
        bool MoveMonsterVisual(OBJECT* o, BMD* b);
        void MoveMonsterSoundVisual(OBJECT* o, BMD* b);

    public:
        bool RenderObject_AfterCharacter(const OBJECT* o, DrawLook& look, BMD* b);
        bool UpdateObjectEffects(OBJECT* o, BMD* b);
        bool UpdateMonsterEffects(CHARACTER* c, OBJECT* o, BMD* b);
        bool RenderObjectMesh(const OBJECT* o, DrawLook& look, BMD* b, bool ExtraMon = 0);
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
