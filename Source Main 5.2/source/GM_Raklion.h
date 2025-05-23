// GM_Raklion.h: interface for the CGM_Raklion class.
//////////////////////////////////////////////////////////////////////

#pragma once

class BMD;

#include "w_BaseMap.h"
#include "./Time/Timer.h"

namespace SEASON4A
{
    SmartPointer(CGM_Raklion);
    class CGM_Raklion : public BaseMap
    {
    public:
        enum RAKLION_STATE
        {
            RAKLION_STATE_IDLE = 0,
            RAKLION_STATE_NOTIFY_1 = 1,
            RAKLION_STATE_STANDBY = 2,
            RAKLION_STATE_NOTIFY_2 = 3,
            RAKLION_STATE_READY = 4,
            RAKLION_STATE_START_BATTLE = 5,
            RAKLION_STATE_NOTIFY_3 = 6,
            RAKLION_STATE_CLOSE_DOOR = 7,
            RAKLION_STATE_ALL_USER_DIE = 8,
            RAKLION_STATE_NOTIFY_4 = 9,
            RAKLION_STATE_END = 10,
            RAKLION_STATE_DETAIL_STATE = 11,
            RAKLION_STATE_MAX = 12,
        };

        enum RAKLION_BATTLE_OF_SELUPAN_PATTERN
        {
            BATTLE_OF_SELUPAN_NONE = 0,
            BATTLE_OF_SELUPAN_STANDBY = 1,
            BATTLE_OF_SELUPAN_PATTERN_1 = 2,
            BATTLE_OF_SELUPAN_PATTERN_2 = 3,
            BATTLE_OF_SELUPAN_PATTERN_3 = 4,
            BATTLE_OF_SELUPAN_PATTERN_4 = 5,
            BATTLE_OF_SELUPAN_PATTERN_5 = 6,
            BATTLE_OF_SELUPAN_PATTERN_6 = 7,
            BATTLE_OF_SELUPAN_PATTERN_7 = 8,
            BATTLE_OF_SELUPAN_DIE = 9,
            BATTLE_OF_SELUPAN_MAX = 10,
        };

    public:
        static CGM_RaklionPtr Make();
        virtual ~CGM_Raklion();

    public:	// Object
        virtual bool CreateObject(OBJECT* o);
        virtual bool MoveObject(OBJECT* o);
        virtual bool RenderObjectVisual(OBJECT* o, BMD* b);
        virtual bool RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);
        virtual void RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);

    public:	// Character
        virtual CHARACTER* CreateMonster(int iType, int PosX, int PosY, int Key);
        virtual bool MoveMonsterVisual(OBJECT* o, BMD* b);
        virtual void MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b);
        virtual bool RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);
        virtual bool AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b);
        virtual bool SetCurrentActionMonster(CHARACTER* c, OBJECT* o);

    public: // Sound
        virtual bool PlayMonsterSound(OBJECT* o);
        virtual void PlayObjectSound(OBJECT* o);
        void PlayBGM();

    public:
        void Init();
        void Destroy();

        //-----------------------------------------------------------------------------------------------

    private:
        CGM_Raklion();
        bool RenderMonster(OBJECT* o, BMD* b, bool ExtraMon = 0);
        void SetBossMonsterAction(CHARACTER* c, OBJECT* o);

    public:
        bool CreateSnow(PARTICLE* o);
        void RenderBaseSmoke();
        void SetState(BYTE byState, BYTE byDetailState);
        bool CanGoBossMap();
        void SetCanGoBossMap();
        void SetEffect();
        void MoveEffect();
        void CreateMapEffect();

    private:
        CTimer2 m_Timer;
        BYTE m_byState;
        BYTE m_byDetailState;
        bool m_bCanGoBossMap;
        bool m_bVisualEffect;
        bool m_bMusicBossMap;
        bool m_bBossHeightMove;
    };
}

extern bool IsIceCity();