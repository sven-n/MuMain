// NewUIButton.h: interface for the CNewUIButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIBUTTON_H__7DC4490D_D859_4159_9EE5_FBC4ECDE209A__INCLUDED_)
#define AFX_NEWUIBUTTON_H__7DC4490D_D859_4159_9EE5_FBC4ECDE209A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace SEASON3B
{
	enum BUTTON_STATE	//. 다른거랑 이름 똑같아서 조낸 곂침: 이름 바꾸거나 맴버로 넣으셈
	{
		BUTTON_STATE_UP = 0,
		BUTTON_STATE_DOWN,
		BUTTON_STATE_OVER,
#ifdef KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
		BUTTON_STATE_LOCK,
#endif // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
	};

	enum RADIOGROUPEVENT
	{
		RADIOGROUPEVENT_NONE = -1,
	};

	struct ButtonInfo 
	{
		int s_ImgIndex;
		int s_BTstate;
		unsigned int s_imgColor;
#ifdef KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
		unsigned int s_FontColor;
#endif // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE

		
		ButtonInfo() : s_ImgIndex(0), s_BTstate(0), s_imgColor(0xffffffff) { }
	};
	
	typedef map<int, ButtonInfo>  ButtonStateMap;
	
	class CNewUIBaseButton  
	{
	public:
		CNewUIBaseButton();
		virtual ~CNewUIBaseButton();

	public:
		void SetPos( const POINT& pos );
		void SetSize( const POINT& size );
		void SetPos( int x, int y );
		void SetSize( int sx, int sy );
		
	public:
		const POINT& GetPos();
		const POINT& GetSize();
		const BUTTON_STATE GetBTState();

	public:
#ifdef KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
		virtual void Lock();
		virtual void UnLock();
#else // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
		void Lock();
		void UnLock();
#endif // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
		bool IsLock();

	public:
		bool RadioProcess();
		bool Process();

	protected:
		POINT					m_Pos;
		POINT					m_Size;
		BUTTON_STATE			m_EventState;
		bool					m_Lock;	
	};

	inline
	void CNewUIBaseButton::SetPos( const POINT& pos )
	{
		m_Pos = pos;
	}

	inline
	void CNewUIBaseButton::SetSize( const POINT& size )
	{
		m_Size = size;
	}

	inline
	const POINT& CNewUIBaseButton::GetPos()
	{
		return m_Pos;
	}

	inline
	const POINT& CNewUIBaseButton::GetSize()
	{
		return m_Size;
	}

	inline
	const BUTTON_STATE CNewUIBaseButton::GetBTState()
	{
		return m_EventState;
	}

#ifndef KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE			// #ifndef
	inline
	void CNewUIBaseButton::Lock()
	{
		m_Lock = true;
	}

	inline
	void CNewUIBaseButton::UnLock()
	{
		m_Lock = false;
	}
#endif // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE

	inline
	bool CNewUIBaseButton::IsLock()
	{
		return m_Lock;
	}

	class CNewUIButton : public CNewUIBaseButton
	{
	public:
		CNewUIButton();
		virtual ~CNewUIButton();
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#ifdef KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
		void ChangeButtonImgState( bool imgregister, int imgindex, 
								   bool overflg = false, bool bLockImage = false, bool bClickEffect = false );
#else // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
		void ChangeButtonImgState( bool imgregister, int imgindex, 
								   bool overflg = false, bool isimgwidth = false, bool bClickEffect = false );
#endif // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
		void ChangeButtonImgState( bool imgregister, int imgindex, 
								   bool overflg = false, bool isimgwidth = false );
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
		void ChangeButtonInfo( int x, int y, int sx, int sy );

	private:
		void Initialize();
		void Destroy();
		
	public:
		void RegisterButtonState( BUTTON_STATE eventstate, int imgindex, int btstate );
		void UnRegisterButtonState();

	public:
		void ChangeImgColor( BUTTON_STATE eventstate, unsigned int color );
		void ChangeText( unicode::t_string btname );
		void SetFont( HFONT hFont );					// 추가 : Pruarin(07.09.03)

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
		void ChangeButtonState( BUTTON_STATE eventstate, int iButtonState );
		void MoveTextPos(int iX, int iY);		// 중앙기준으로 X,Y로 얼마나 옮길지를 계산
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

		void ChangeTextBackColor( const DWORD bcolor );
		void ChangeTextColor( const DWORD color );

		void ChangeToolTipText( unicode::t_string tooltiptext, bool istoppos = false );
		void ChangeToolTipTextColor( const DWORD color );
		void SetToolTipFont( HFONT hFont );

#ifndef KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE			// #ifndef
		void ChangeImgWidth( bool isimgwidth );
#endif // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
		void ChangeImgIndex( int imgindex, int curimgstate = 0 );
		void ChangeAlpha( unsigned char fAlpha, bool isfontalph = true ); // 0 ~ 255
		void ChangeAlpha( float fAlpha, bool isfontalph = true ); // 0.0f ~ 1.0f

#ifdef KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
		void Lock();
		void UnLock();
#endif // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
		
	public:
		bool UpdateMouseEvent();

	public:
		
#ifdef PJH_ADD_MINIMAP
		bool Render( bool RendOption = false );
#else // PJH_ADD_MINIMAP
		bool Render( );
#endif // PJH_ADD_MINIMAP

	private:
		void ChangeFrame();

	private:
		ButtonStateMap           m_ButtonInfo;

	private:
		unicode::t_string		m_Name;
		unicode::t_string		m_TooltipText;

		HFONT					m_hTextFont;			// 추가 : Pruarin(07.08.03)
		HFONT					m_hToolTipFont;			// 추가 : Pruarin(07.08.03)
		DWORD					m_NameColor;
		DWORD					m_NameBackColor;
		DWORD					m_TooltipTextColor;

		int						m_CurImgIndex;
		int						m_CurImgState;

		WORD					m_ImgWidth;
		WORD					m_ImgHeight;

		unsigned int			m_CurImgColor;
		bool					m_IsTopPos;
#ifndef KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE				// #ifndef
		bool                    m_IsImgWidth;
#endif // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
		
		unsigned char			m_fAlpha;

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
		bool					m_bClickEffect;				// 버튼 눌렸을때 버튼 이미지에따라 글자도 움직이는 효과
		int						m_iMoveTextPosX;
		int						m_iMoveTextPosY;
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
	};

#ifndef KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE				// #ifndef
	inline
	void CNewUIButton::ChangeImgWidth( bool isimgwidth )
	{
		m_IsImgWidth = isimgwidth;
	}
#endif // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE

	inline
	void CNewUIButton::ChangeText( unicode::t_string btname )
	{
		m_Name = btname;
	}

	// 추가 : Pruarin(07.09.03)
	inline
	void CNewUIButton::SetFont( HFONT hFont )
	{
		m_hTextFont = hFont;	
	}

	inline
	void CNewUIButton::ChangeTextBackColor( const DWORD bcolor )
	{
		m_NameBackColor = bcolor;
	}
	
	inline
	void CNewUIButton::ChangeTextColor( const DWORD color )
	{
		m_NameColor = color;
	}

	inline
	void CNewUIButton::ChangeToolTipText( unicode::t_string tooltiptext, bool istoppos )
	{
		m_TooltipText = tooltiptext; 
		m_IsTopPos = istoppos;
		//m_hToolTipFont = g_hFont;
	}

	inline
		void CNewUIButton::SetToolTipFont( HFONT hFont )	
	{
		m_hToolTipFont = hFont;
	}

	inline
	void CNewUIButton::ChangeToolTipTextColor( const DWORD color )
	{
		m_TooltipTextColor = color;
	}
	
	class CNewUIRadioButton : public CNewUIBaseButton
	{
	public:
		CNewUIRadioButton();
		virtual ~CNewUIRadioButton ();

	public:
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#ifdef KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
		void ChangeRadioButtonImgState(int imgindex, bool bMouseOnImage, bool bLockImage, bool bClickEffect);
#else // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
		void ChangeRadioButtonImgState(int imgindex, bool isDown = false, bool bClickEffect = false);
#endif // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
		void ChangeRadioButtonImgState(int imgindex, bool isDown = false);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
		void ChangeRadioButtonInfo(int x, int y, int sx, int sy);
		void ChangeFrame( BUTTON_STATE eventstate );

	public:
		void ChangeImgColor( BUTTON_STATE eventstate, unsigned int color );
		void ChangeText(unicode::t_string btname);
		void ChangeTextBackColor(const DWORD bcolor);
		void ChangeTextColor(const DWORD color);
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
		void ChangeButtonState( BUTTON_STATE eventstate, int iButtonState );
		void ChangeButtonState( int iImgIndex, BUTTON_STATE eventstate, int iButtonState );
		void SetFont(HFONT hFont);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
#ifdef KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
		void Lock();
		void UnLock();
#endif // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE

	public:
		void RegisterButtonState(BUTTON_STATE eventstate, int imgindex, int btstate);
		void UnRegisterButtonState();

	public:
		bool UpdateMouseEvent( bool isGroupevent = false );

	public:
		bool Render();

	private:
		void ChangeImgIndex( int imgindex, int curimgstate = 0 );
		void ChangeFrame();
		void Initialize();
		void Destroy();

	private:
		ButtonStateMap           m_RadioButtonInfo;
		unicode::t_string		 m_Name;

		DWORD					m_NameColor;
		DWORD					m_NameBackColor;
		DWORD					m_CurImgIndex; 
		DWORD					m_CurImgState;
		DWORD					m_ImgWidth;
		DWORD					m_ImgHeight;
		DWORD					m_CurImgColor;
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
		HFONT					m_hTextFont;
		bool					m_bClickEffect;
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
#ifdef KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
		bool					m_bLockImage;
#endif // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
	};

	inline
	void CNewUIRadioButton::ChangeText( unicode::t_string btname )
	{
		m_Name = btname;
	}
	
	inline
	void CNewUIRadioButton::ChangeTextBackColor( const DWORD bcolor )
	{
		m_NameBackColor = bcolor;
	}
	
	inline
	void CNewUIRadioButton::ChangeTextColor( const DWORD color )
	{
		m_NameColor = color;
	}
	
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
	inline
		void CNewUIRadioButton::SetFont(HFONT hFont)
	{
		m_hTextFont = hFont;
	}
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

	class CNewUIRadioGroupButton
	{
	public:
		CNewUIRadioGroupButton();
		virtual ~CNewUIRadioGroupButton();

	public:
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#ifdef KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
		void CreateRadioGroup( int radiocount, int imgindex, bool bFirstIndexBtnDown = true, 
								bool bMouseOnImage = false, bool bLockImage = false, bool bClickEffect = false );
#else // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
		void CreateRadioGroup( int radiocount, int imgindex, bool bClickEffect = false );
#endif // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
		void ChangeRadioButtonInfo( bool iswidth, int x, int y, int sx, int sy, int iDistance = 1);
		void MoveTextPos(int iX, int iY);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
		void CreateRadioGroup( int radiocount, int imgindex );
		void ChangeRadioButtonInfo( bool iswidth, int x, int y, int sx, int sy );
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
		void ChangeRadioText( list<unicode::t_string>& textlist );
		void ChangeFrame( int buttonIndex );
		void LockButtonindex( int buttonIndex );					// 추가 : Pruarin(07.09.03)
#ifdef KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
		void UnLockButtonIndex( int iButtonIndex );
#endif // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
		void SetFont( HFONT hFont, int iButtonIndex );
		void SetFont( HFONT hFont );
		void ChangeButtonState( BUTTON_STATE eventstate, int iButtonState );			// EventState에 따른 Radio버튼의 State를 모두 변경한다.
		void ChangeButtonState( int iBtnIndex, int iImgIndex, 
								BUTTON_STATE eventstate, int iButtonState);				// EventState에 따른 해당Index인 Radio버튼의 State를 변경한다. (이미지도 변경가능)
		POINT GetPos(int iButtonIndex);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
		void SetFontIndex( int buttonIndex, HFONT hFont );			// 추가 : Pruarin(07.09.03)
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

	public:
		void RegisterRadioButton( CNewUIRadioButton* button );
		void UnRegisterRadioButton();

	public:
		const int GetCurButtonIndex();

	public:
		int UpdateMouseEvent();

	public:
		bool Render();
 
	private:
		void SetCurButtonIndex( int index );
		void Initialize();
		void Destroy();

	private:
		typedef list<CNewUIRadioButton*>      RadioButtonList;

	private:
		RadioButtonList				m_RadioList;
		DWORD						m_CurButtonIndex;
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
		int							m_iButtonDistance;			// 버튼과 버튼사이의 간격
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
	};

	inline
	void CNewUIRadioGroupButton::SetCurButtonIndex( int index )
	{
		m_CurButtonIndex = index;
	}

	inline
	const int CNewUIRadioGroupButton::GetCurButtonIndex()
	{
		return m_CurButtonIndex;
	}
};

#endif // !defined(AFX_NEWUIBUTTON_H__7DC4490D_D859_4159_9EE5_FBC4ECDE209A__INCLUDED_)





















