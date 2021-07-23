// w_ModelView.h: interface for the ModelView class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_UISYSTEM

#if !defined(AFX_W_MODELVIEW_H__5EA1C31A_E431_4AD0_B198_5AA69BE70661__INCLUDED_)
#define AFX_W_MODELVIEW_H__5EA1C31A_E431_4AD0_B198_5AA69BE70661__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "w_ImageUnit.h"
#include "w_UIWnd.h"

/*
// 모델 뷰어의 이름이 필요하다.( id라고 생각 하자 ) 
ui::ModelViewPtr shoppreviewmodel( new ui::ModelView( "shoppreviewmodel" ) );
// 히어로( 자기 자신 )의 장비를 카피 한다.
shoppreviewmodel->CopyPlayer();
// 좌표 ( 부모의 상대 좌표 )
shoppreviewmodel->Move( Coord( 0, 0 ) );
// 그림 뷰의 크기
shoppreviewmodel->Resize( Dim( 170, 198 ) );
// 부모에 붙이기
previewPanel->AddChild( shoppreviewmodel );
*/

namespace ui
{
	BoostSmartPointer( ModelView );
	class ModelView : public UIWnd 
	{
	public:
		ModelView( const string& name );
		virtual ~ModelView();

	public:
		void SetAngle( const float angle );
		void SetZoom( const float zoom );
		void CopyPlayer();

		void SetClass( BYTE byClass );
		const bool SetEquipmentItem( const CASHSHOP_ITEMLIST& eqitem );

	protected:
		virtual void DrawViewPortMesh();
		virtual void Process( int delta );

	protected:
		virtual void OnMove( const Coord& prevPos );
		virtual void OnResize( const Dim& prevSize );
		virtual void OnMouseOver();
		virtual void OnMouseOut();
		virtual void OnLButtonDown();
		virtual void OnLButtonUp();
		virtual void OnLButtonDBClk();
		virtual void OnWheelScroll( int delta );
		virtual void OnMouseMove( const Coord& coord );

	private:
		void SetAnimation( int iAnimationType );
		int SetPhotoPose(int iCurrentAni, int iMoveDir = 0);
		void Initialize();
		void Destroy();

	private:
		bool		m_MouseDowned;
		float		m_Zoom;
		float       m_Angle;
        Coord		m_PrevMousePos;

		BOOL m_bIsInitialized;

		int m_iShowType;
		int m_iCurrentAnimation;	// 현재 행동
		int m_iSettingAnimation;	// 설정된 행동
		int m_iCurrentFrame;		// 현재 프레임
	};

	inline
	void ModelView::SetAngle( const float angle )
	{
		m_Angle -= angle;
	}

	inline
	void ModelView::SetZoom( const float zoom )
	{
		m_Zoom += zoom;
	}
};

#endif // !defined(AFX_W_MODELVIEW_H__5EA1C31A_E431_4AD0_B198_5AA69BE70661__INCLUDED_)

#endif //NEW_USER_INTERFACE_UISYSTEM