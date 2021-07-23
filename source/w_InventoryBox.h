// w_InventoryBox.h: interface for the InventoryBox class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_UISYSTEM

#if !defined(AFX_W_INVENTORYBOX_H__F7181555_C9DF_4294_AA1C_4ED89FD9273B__INCLUDED_)
#define AFX_W_INVENTORYBOX_H__F7181555_C9DF_4294_AA1C_4ED89FD9273B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//OTL....ㅠ..ㅠ
#include "w_ImageUnit.h"
#include "w_UIWnd.h"

/*
InventoryBox는 현재 자기 자신의 인벤토리 박스 밖에 만들수 없다...-=-;;;

g_pMyInventory->UpdateShop( GetScreenPosition() );

g_pMyInventory->RenderShop( GetScreenPosition() );

*/

namespace ui
{
	BoostSmartPointer( InventoryBox );
	class InventoryBox : public UIWnd 
	{
	public:
		enum InventoryBoxState
		{
			eInventoryBox_Up,
			eInventoryBox_Down,

			eInventoryBox_StateCount
		};

	public:
		InventoryBox( const string& name );
		virtual ~InventoryBox();

	public:
		void MakeImage( const int bitmapindex, const Dim& count );
		void SetImageIndex( const int bitmapindex );
		void SetItem( const int inventorytype );
		
	protected:
		virtual void Process( int delta );
		virtual void DrawImage();
		virtual void DrawMesh();
		virtual void DrawRect();

	protected:
		virtual void OnMove( const Coord& prevPos );
		virtual void OnResize( const Dim& prevSize );

	private:
		const Color& QueryInventoryBoxColor( const ITEM *p );
		void CalculateInventoryBoxCoord();
		void CalculateInventoryBoxDim();
		void SetInventoryBoxCount( const Dim& count );
		void Initialize();
		void Destroy();

	private:
		struct InventoryBoxImage
		{
			array< image::ImageUnitPtr, eInventoryBox_StateCount >				m_Image;
			InventoryBoxState													m_ImageState;

			InventoryBoxImage() : m_ImageState( eInventoryBox_Up )
			{
				for( int i = 0; i < eInventoryBox_StateCount; ++i )
				{
					m_Image[i] = image::ImageUnit::Make();
				}
			}

			void ChangeFrame( InventoryBoxState state )
			{
				m_ImageState = state;
			}

			void Draw( const Color& color )
			{
				m_Image[m_ImageState]->SetColor( color );
				m_Image[m_ImageState]->Draw();
			}

			void SetImageIndex( const int bitmapindex )
			{
				if( m_Image[eInventoryBox_Up] ) m_Image[eInventoryBox_Up]->SetImageIndex( bitmapindex );
				if( m_Image[eInventoryBox_Down] ) m_Image[eInventoryBox_Down]->SetImageIndex( bitmapindex+1 );
			}

			void SetPosition( const Coord& pos )
			{
				if( m_Image[eInventoryBox_Up] ) m_Image[eInventoryBox_Up]->SetPosition( pos );
				if( m_Image[eInventoryBox_Down] ) m_Image[eInventoryBox_Down]->SetPosition( pos );
			}

			void SetSize( const Dim& size )
			{
				if( m_Image[eInventoryBox_Up] ) m_Image[eInventoryBox_Up]->SetSize( size );
				if( m_Image[eInventoryBox_Down] ) m_Image[eInventoryBox_Down]->SetSize( size );
			}

			const Coord& GetPosition()
			{
				assert( m_Image[eInventoryBox_Up] && m_Image[eInventoryBox_Down] );
				return m_Image[eInventoryBox_Up]->GetPosition();
			}

			const Dim& GetSize()
			{
				assert( m_Image[eInventoryBox_Up] && m_Image[eInventoryBox_Down] );
				return m_Image[eInventoryBox_Up]->GetSize();
			}
		};

	private:
		typedef vector< BoostSmart_Ptr(InventoryBoxImage) >       Image_Vector;

	private:
		Image_Vector		m_Image;
		Dim					m_InventoryBoxCount;
		ITEM*				m_InventoryItem;
		Color				m_Color;
		bool				m_IsToolTip;
		//ToolTipPanelPtr		m_ToolTipPanel;
	};

	inline
	void InventoryBox::SetInventoryBoxCount( const Dim& count )
	{
		m_InventoryBoxCount = count;
	}

};

#endif // !defined(AFX_W_INVENTORYBOX_H__F7181555_C9DF_4294_AA1C_4ED89FD9273B__INCLUDED_)

#endif //NEW_USER_INTERFACE_UISYSTEM