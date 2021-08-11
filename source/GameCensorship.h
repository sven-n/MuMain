// GameCensorship.h: interface for the CGameCensorship class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GAMECENSORSHIP_H__BF1AD1A5_06C5_47AA_9F65_22EB0E88E86F__INCLUDED_)
#define AFX_GAMECENSORSHIP_H__BF1AD1A5_06C5_47AA_9F65_22EB0E88E86F__INCLUDED_

#pragma once

#include "Sprite.h"
#include "./Time/Timer.h"

namespace SEASON3A
{

	class CGameCensorship  
	{
		enum
		{
			IMAGE_COUNT = 5,
			IMAGE_12 = BITMAP_GAME_CENSORSHIP_BEGIN,
			IMAGE_15,
			IMAGE_18,
			IMAGE_FEAR,
			IMAGE_VIOLENCE,
		};

		enum
		{
			SPRITE_12 = 0,
			SPRITE_15,
			SPRITE_18,
			SPRITE_FEAR,
			SPRITE_VIOLENCE,
			SPRITE_COUNT,
		};

	public:
		enum
		{
			STATE_LOADING = 0,
			STATE_12,
			STATE_15,
			STATE_18,
		};

		virtual ~CGameCensorship();
		static CGameCensorship* GetInstance();

		void Create();
		void Release();

		void Update();
		void Render();

		bool IsVisible();
		void SetVisible(bool bVisible);
		void SetState(DWORD dwState);

	private:
		CGameCensorship();
		void LoadImage();

		bool m_bVisible;

		DWORD m_dwState;

		CTimer m_Timer;
		DWORD m_dwTime;

		CSprite m_ImageSprite[IMAGE_COUNT];

		float m_fScreenRateX;
		float m_fScreenRateY;

	};

}

#define g_GameCensorship SEASON3A::CGameCensorship::GetInstance()

#endif // !defined(AFX_GAMECENSORSHIP_H__BF1AD1A5_06C5_47AA_9F65_22EB0E88E86F__INCLUDED_)
