// w_ModelView.cpp: implementation of the ModelView class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef NEW_USER_INTERFACE_UISYSTEM
#include "ZzzOpenglUtil.h"
#include "ZzzLodTerrain.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzAI.h"
#endif //NEW_USER_INTERFACE_UISYSTEM
#ifdef SOCKET_SYSTEM
#include "SocketSystem.h"
#endif	// SOCKET_SYSTEM

#include "w_ModelView.h"

#ifdef NEW_USER_INTERFACE_UISYSTEM

extern int gix, giy;
extern void MoveCharacter(CHARACTER *c,OBJECT *o);
extern void MoveCharacterVisual(CHARACTER *c,OBJECT *o);

namespace
{
	CHARACTER			m_Character;

	BOOL CompareItemEqual(const PART_t * item1, const PART_t * item2)
	{
		return (item1->Type		== item2->Type		&&
				item1->Level	== item2->Level		&&
				item1->Option1	== item2->Option1	);
	}

	BOOL CompareItemEqual(const PART_t * item1, const ITEM * item2, int iDefaultValue)
	{
		if (item2->Type == -1)
		{
			return (item1->Type		== iDefaultValue			&&
					item1->Level	== ((item2->Level>>3)&15)	&&
					item1->Option1	== item2->Option1			);
			
		}
		else
		{
			return (item1->Type		== item2->Type+MODEL_ITEM	&&
					item1->Level	== ((item2->Level>>3)&15)	&&
					item1->Option1	== item2->Option1			);
		}
	}

	void SetItemToPhoto(PART_t * itemDest, const ITEM * itemSrc, int iDefaultValue)
	{
		if (itemSrc->Type == -1)
		{
			itemDest->Type		= iDefaultValue;
			itemDest->Level		= ((itemSrc->Level>>3)&15);
			itemDest->Option1	= itemSrc->Option1;
			
		}
		else
		{
			itemDest->Type		= itemSrc->Type+MODEL_ITEM;
			itemDest->Level		= ((itemSrc->Level>>3)&15);
			itemDest->Option1	= itemSrc->Option1;
		}
	}

	bool ChangeCodeItem( ITEM* ip, BYTE* itemdata )
	{
		int Type = ConvertItemType( itemdata );

		if( Type == 0x1FFF ) 
		{
			return false;
		}
		else
		{
			ip->Type							= Type;
			ip->x								= 0;
			ip->y								= 0;
			ip->Number							= 0;
			ip->Durability						= itemdata[2];
			ip->Option1							= itemdata[3];
			ip->ExtOption						= itemdata[4];
			ip->option_380						= false;
			byte is380option					= ( ( (itemdata[5] & 0x08) << 4) >>7);
			ip->option_380						= is380option;

			ip->Jewel_Of_Harmony_Option			= (itemdata[6] & 0xf0) >> 4;
			ip->Jewel_Of_Harmony_OptionLevel	= itemdata[6] & 0x0f;

#ifdef SOCKET_SYSTEM
			// 소켓 아이템 옵션 (0x00~0xF9: 옵션고유번호, 0xFE: 빈 소켓, 0xFF: 막힌 소켓)
			BYTE bySocketOption[5] = { itemdata[7], itemdata[8], itemdata[9], itemdata[10], itemdata[11] };
			ip->SocketCount = MAX_SOCKETS;

			for (int i = 0; i < MAX_SOCKETS; ++i)
			{
				ip->bySocketOption[i] = bySocketOption[i];	// 서버에서 받은 내용 백업
			}

#ifdef _VS2008PORTING
			for (int i = 0; i < MAX_SOCKETS; ++i)
#else // _VS2008PORTING
			for (i = 0; i < MAX_SOCKETS; ++i)
#endif // _VS2008PORTING
			{
				if (bySocketOption[i] == 0xFF)		// 소켓이 막힘 (DB상에는 0x00 으로 되어있음)
				{
					ip->SocketCount = i;
					break;
				}
				else if (bySocketOption[i] == 0xFE)	// 소켓이 비어있음 (DB상에는 0xFF 으로 되어있음)
				{
					ip->SocketSeedID[i] = SOCKET_EMPTY;
				}
				else	// 0x00~0xF9 까지 소켓 고유번호로 사용, MAX_SOCKET_OPTION(50)단위로 나누어 고유번호로 표시 (DB상에는 0x01~0xFA로 되어있음)
				{
					ip->SocketSeedID[i] = bySocketOption[i] % SEASON4A::MAX_SOCKET_OPTION;
					ip->SocketSphereLv[i] = int(bySocketOption[i] / SEASON4A::MAX_SOCKET_OPTION) + 1;
				}
			}
			if (g_SocketItemMgr.IsSocketItem(ip))	// 소켓 아이템이면
			{
				ip->SocketSeedSetOption = itemdata[6];	// 조화의보석옵션값으로 시드세트옵션을 표현함
				ip->Jewel_Of_Harmony_Option = 0;
				ip->Jewel_Of_Harmony_OptionLevel = 0;
			}
			else
			{
				ip->SocketSeedSetOption = SOCKET_EMPTY;
			}
#endif	// SOCKET_SYSTEM

			ItemConvert( ip,itemdata[1],itemdata[3],itemdata[4] );
		}

		return true;
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace ui;

ModelView::ModelView( const string& name ) : UIWnd( name ), m_Zoom( 0.8f ), m_Angle( 90.0f ), m_MouseDowned( false )
{
	Initialize();
	RegisterDrawEvent( eDraw_ViewPortMesh );
}

ModelView::~ModelView()
{
	Destroy();
}

void ModelView::Initialize()
{
	//당분간 헬퍼는 안 한다..-.-;;
	//기획 내용이 언제 바뀔지 모르기때문에
	//일단 주석...-.-;;;
	//memset(&m_PhotoHelper, 0, sizeof(OBJECT));
	m_Character.Initialize();
	CreateCharacterPointer(&m_Character, MODEL_PLAYER, ( Hero->PositionX), ( Hero->PositionY), 0);
	
	CHARACTER * c = &m_Character;
	OBJECT * o = &c->Object;

	c->HideShadow = true;
	c->SafeZone = false;
	o->Live = false;

	// 이동
	Vector( 0, 0, 0, m_Character.Object.Position );
}

void ModelView::Destroy()
{
	DeleteCloth( &m_Character, &m_Character.Object);
}

void ModelView::OnMove( const Coord& prevPos )
{
	UIWnd::OnMove( prevPos );
}

void ModelView::OnResize( const Dim& prevSize )
{
	UIWnd::OnResize( prevSize );
}

void ModelView::OnMouseOver()
{
    UIWnd::OnMouseOver();
    m_PrevMousePos.x = -1;
    m_MouseDowned   = false;
}

void ModelView::OnMouseOut()
{
    UIWnd::OnMouseOut();
    m_MouseDowned   = false;
}

void ModelView::OnLButtonDown()
{
    UIWnd::OnLButtonDown();
    m_PrevMousePos.x = -1;
    m_MouseDowned   = true;
}

void ModelView::OnLButtonUp()
{
    UIWnd::OnLButtonUp();
    m_MouseDowned   = false;
}

void ModelView::OnLButtonDBClk()
{
	UIWnd::OnLButtonDBClk();
	m_Angle = 90.f;
	m_Zoom  = 0.8f;
}

void ModelView::OnWheelScroll( int delta )
{
	UIWnd::OnWheelScroll( delta );

	float zoom = static_cast<float>(delta * 0.0001f);
	
	if( m_Zoom + zoom > 0.75 && m_Zoom + zoom < 0.9 )
	{
		SetZoom( zoom );
	}
}

void ModelView::OnMouseMove( const Coord& coord )
{
    UIWnd::OnMouseMove( coord );

    if( m_MouseDowned )
    {
        if( m_PrevMousePos.x != -1 )
        {
			SetAngle( static_cast<float>(m_PrevMousePos.x - coord.x) );
        }
        m_PrevMousePos  = coord;
    }
}

void ModelView::Process( int delta )
{

}

void ModelView::SetClass(BYTE byClass)
{
	if (m_bIsInitialized == FALSE) return;
	// 백업
    CHARACTER *c = CharactersClient;
	CharactersClient = &m_Character;
	// 캐릭터 클래스 설정
	CharactersClient->Class = byClass;
	SetChangeClass(CharactersClient);
	// 복원
	CharactersClient = c;
}

const bool ModelView::SetEquipmentItem( const CASHSHOP_ITEMLIST& eqitem )
{
	ITEM        ip;
	CHARACTER * c = &m_Character;
	OBJECT*     o = &c->Object;

	ChangeCodeItem( &ip, (BYTE*)eqitem.s_btItemInfo );
	int Level     = (ip.Level>>3)&15;
	ITEM_ATTRIBUTE *p = &ItemAttribute[ip.Type];
	
	int ExClass = IsSecondClass(c->Class);
	
	// 0이면 장비를 착용 할 수 없으며, 파트가 -1이면 착용 가능한 장비가 아님
	if( p->RequireClass[GetBaseClass(c->Class)] == 0 || ip.Part == -1 ) return false;

	// 2이면 서브 클래스만 착용 가능 하다.
	if( p->RequireClass[GetBaseClass(c->Class)] == 2 && !ExClass ) return false;

	if( ip.Part == EQUIPMENT_HELM )
	{
		c->BodyPart[BODYPART_HELM].Type      = MODEL_ITEM+ip.Type;
		c->BodyPart[BODYPART_HELM].Level	 = Level;
		c->BodyPart[BODYPART_HELM].ExtOption = ip.ExtOption;
		c->BodyPart[BODYPART_HELM].Option1   = ip.Option1;
	}
	else if( ip.Part == EQUIPMENT_ARMOR)
	{
		c->BodyPart[BODYPART_ARMOR].Type      = MODEL_ITEM+ip.Type;
		c->BodyPart[BODYPART_ARMOR].Level     = Level;
		c->BodyPart[BODYPART_ARMOR].ExtOption = ip.ExtOption;
		c->BodyPart[BODYPART_ARMOR].Option1   = ip.Option1;
	}
	else if( ip.Part == EQUIPMENT_PANTS)
	{
		c->BodyPart[BODYPART_PANTS].Type      = MODEL_ITEM+ip.Type;
		c->BodyPart[BODYPART_PANTS].Level     = Level;
		c->BodyPart[BODYPART_PANTS].ExtOption = ip.ExtOption;
		c->BodyPart[BODYPART_PANTS].Option1   = ip.Option1;
	}
	else if( ip.Part == EQUIPMENT_GLOVES)
	{
		c->BodyPart[BODYPART_GLOVES].Type      = MODEL_ITEM+ip.Type;
		c->BodyPart[BODYPART_GLOVES].Level     = Level;
		c->BodyPart[BODYPART_GLOVES].ExtOption = ip.ExtOption;
		c->BodyPart[BODYPART_GLOVES].Option1   = ip.Option1;
	}
	else if( ip.Part == EQUIPMENT_BOOTS)
	{
		c->BodyPart[BODYPART_BOOTS].Type      = MODEL_ITEM+ip.Type;
		c->BodyPart[BODYPART_BOOTS].Level     = Level;
		c->BodyPart[BODYPART_BOOTS].ExtOption = ip.ExtOption;
		c->BodyPart[BODYPART_BOOTS].Option1   = ip.Option1;
	}
	else if( ip.Part == EQUIPMENT_WEAPON_RIGHT || ip.Part == EQUIPMENT_WEAPON_LEFT )
	{
		c->Weapon[ip.Part].Type        = MODEL_ITEM+ip.Type;
		c->Weapon[ip.Part].Level       = Level;
		c->BodyPart[ip.Part].ExtOption = ip.ExtOption;
		c->BodyPart[ip.Part].Option1   = ip.Option1;

		if( p->TwoHand )
		{
			c->Weapon[ip.Part+1].Type        = -1;
			c->Weapon[ip.Part+1].Level       = -1;
			c->BodyPart[ip.Part+1].ExtOption = -1;
			c->BodyPart[ip.Part+1].Option1   = -1;
		}
	}
	else if( ip.Part == EQUIPMENT_WING )
	{
		c->Wing.Type				   = MODEL_ITEM+ip.Type;
		c->Wing.Level				   = Level;
		c->BodyPart[ip.Part].ExtOption = ip.ExtOption;
		c->BodyPart[ip.Part].Option1   = ip.Option1;
	}
	else
	{
		//추후에 추가 하도록 하자..-.-;;
		return false;
	}

	c->HideShadow = true;
	c->SafeZone = false;
	o->Live = false;

	return true;
}

void ModelView::SetAnimation(int iAnimationType)
{
	m_iSettingAnimation = iAnimationType;
	SetPhotoPose(m_iSettingAnimation);
}

int ModelView::SetPhotoPose(int iCurrentAni, int iMoveDir)
{
	static const int MAX_POSE_NUM = 24;
	static int siPose[MAX_POSE_NUM] = {
		AT_STAND1, AT_GREETING1, AT_CLAP1, AT_GESTURE1, AT_DIRECTION1, AT_AWKWARD1, AT_CRY1, AT_SEE1, 
		AT_CHEER1, AT_UNKNOWN1, AT_WIN1, AT_SMILE1, AT_SLEEP1, AT_COLD1, AT_AGAIN1, AT_RESPECT1, 
		AT_SALUTE1, AT_GOODBYE1, AT_MOVE1, AT_RUSH1,AT_SIT1, AT_POSE1, AT_HEALING1, AT_ATTACK1
	};
#ifdef _VS2008PORTING
	int iCurrentAniArray = 0;
	for (iCurrentAniArray = 0; iCurrentAniArray < MAX_POSE_NUM; ++iCurrentAniArray)
#else // _VS2008PORTING
	for (int iCurrentAniArray = 0; iCurrentAniArray < MAX_POSE_NUM; ++iCurrentAniArray)
#endif // _VS2008PORTING
	{
		if (iCurrentAni == siPose[iCurrentAniArray]) break;
	}
	iCurrentAniArray += iMoveDir;
	if (iCurrentAniArray < 0) iCurrentAniArray = MAX_POSE_NUM * 100 + iCurrentAniArray;
	iCurrentAniArray %= MAX_POSE_NUM;
	iCurrentAni = siPose[iCurrentAniArray];

	CHARACTER * c = &m_Character;
	OBJECT * o = &c->Object;
	int WorldBackup = World;
	switch(iCurrentAni)
	{
	case AT_STAND1:
		World = WD_0LORENCIA;
   		SetPlayerStop(c);
		World = WorldBackup;
		break;
	case AT_ATTACK1:
		World = WD_0LORENCIA;
		SetPlayerAttack(c);
		World = WorldBackup;
      	c->AttackTime = 1;
		c->Object.AnimationFrame = 0;
		c->TargetCharacter = -1;
		break;
	case AT_MOVE1:
		World = WD_0LORENCIA;
		SetPlayerWalk(c);
		World = WorldBackup;
		break;
	case AT_SIT1:
		if(!IsFemale(c->Class))
     		SetAction(&c->Object,PLAYER_SIT1);
		else
     		SetAction(&c->Object,PLAYER_SIT_FEMALE1);
		break;
	case AT_POSE1:
		if(!IsFemale(c->Class))
    		SetAction(&c->Object,PLAYER_POSE1);
		else
    		SetAction(&c->Object,PLAYER_POSE_FEMALE1);
		break;
	case AT_HEALING1:
		if(!IsFemale(c->Class))
    		SetAction(&c->Object,PLAYER_HEALING1);
		else
    		SetAction(&c->Object,PLAYER_HEALING_FEMALE1);
		break;
	case AT_GREETING1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_GREETING1);
		else
			SetAction(o,PLAYER_GREETING_FEMALE1);
		break;
	case AT_GOODBYE1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_GOODBYE1);
		else
			SetAction(o,PLAYER_GOODBYE_FEMALE1);
		break;
	case AT_CLAP1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_CLAP1);
		else
			SetAction(o,PLAYER_CLAP_FEMALE1);
		break;
	case AT_GESTURE1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_GESTURE1);
		else
			SetAction(o,PLAYER_GESTURE_FEMALE1);
		break;
	case AT_DIRECTION1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_DIRECTION1);
		else
			SetAction(o,PLAYER_DIRECTION_FEMALE1);
		break;
	case AT_UNKNOWN1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_UNKNOWN1);
		else
			SetAction(o,PLAYER_UNKNOWN_FEMALE1);
		break;
	case AT_CRY1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_CRY1);
		else
			SetAction(o,PLAYER_CRY_FEMALE1);
		break;
	case AT_AWKWARD1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_AWKWARD1);
		else
			SetAction(o,PLAYER_AWKWARD_FEMALE1);
		break;
	case AT_SEE1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_SEE1);
		else
			SetAction(o,PLAYER_SEE_FEMALE1);
		break;
	case AT_CHEER1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_CHEER1);
		else
			SetAction(o,PLAYER_CHEER_FEMALE1);
		break;
	case AT_WIN1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_WIN1);
		else
			SetAction(o,PLAYER_WIN_FEMALE1);
		break;
	case AT_SMILE1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_SMILE1);
		else
			SetAction(o,PLAYER_SMILE_FEMALE1);
		break;
	case AT_SLEEP1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_SLEEP1);
		else
			SetAction(o,PLAYER_SLEEP_FEMALE1);
		break;
	case AT_COLD1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_COLD1);
		else
			SetAction(o,PLAYER_COLD_FEMALE1);
		break;
	case AT_AGAIN1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_AGAIN1);
		else
			SetAction(o,PLAYER_AGAIN_FEMALE1);
		break;
	case AT_RESPECT1:
		SetAction(o,PLAYER_RESPECT1);
		break;
	case AT_SALUTE1:
		SetAction(o,PLAYER_SALUTE1);
		break;
	case AT_RUSH1:
		SetAction(o,PLAYER_RUSH1);
		break;
	default:
		break;
	}

	MoveCharacter(c, o);
	MoveCharacterVisual(c, o);
	m_iCurrentAnimation = iCurrentAni;
	return iCurrentAni;
}

void ModelView::CopyPlayer()
{
	if (m_Character.Class != Hero->Class)
	{
		m_Character.Class = Hero->Class;
		SetChangeClass( &m_Character );
	}

	int i;
    int maxClass = MAX_CLASS;

	// 장비 바뀌었나 체크
	BOOL bChangeArmor = FALSE;
	BOOL bChangeWeapon = FALSE;
	BOOL bChangeWing = FALSE;
	BOOL bChangeHelper = FALSE;

	if (Hero->Change == FALSE)
	{
		for (i = 0; i < MAX_BODYPART; ++i)
		{
			if (CompareItemEqual(&m_Character.BodyPart[i], &Hero->BodyPart[i]) == FALSE)
			{
				bChangeArmor = TRUE;
				break;
			}
		}
		for (i = 0; i < 2; ++i)
		{
			if (CompareItemEqual(&m_Character.Weapon[i], &Hero->Weapon[i]) == FALSE)
			{
				bChangeWeapon = TRUE;
				break;
			}
		}
		if (CompareItemEqual(&m_Character.Wing, &Hero->Wing) == FALSE)
			bChangeWing = TRUE;
		if (CompareItemEqual(&m_Character.Helper, &Hero->Helper) == FALSE)
			bChangeHelper = TRUE;
	}
	else	// 변신 상태
	{

		if (CompareItemEqual(&m_Character.BodyPart[BODYPART_HELM], &CharacterMachine->Equipment[EQUIPMENT_HELM],
			MODEL_BODY_HELM+GetSkinModelIndex(Hero->Class)) == FALSE) bChangeArmor = TRUE;
		else if (CompareItemEqual(&m_Character.BodyPart[BODYPART_ARMOR], &CharacterMachine->Equipment[EQUIPMENT_ARMOR],
			MODEL_BODY_ARMOR+GetSkinModelIndex(Hero->Class)) == FALSE) bChangeArmor = TRUE;
		else if (CompareItemEqual(&m_Character.BodyPart[BODYPART_PANTS], &CharacterMachine->Equipment[EQUIPMENT_PANTS],
			MODEL_BODY_PANTS+GetSkinModelIndex(Hero->Class)) == FALSE) bChangeArmor = TRUE;
		else if (CompareItemEqual(&m_Character.BodyPart[BODYPART_GLOVES], &CharacterMachine->Equipment[EQUIPMENT_GLOVES],
			MODEL_BODY_GLOVES+GetSkinModelIndex(Hero->Class)) == FALSE) bChangeArmor = TRUE;
		else if (CompareItemEqual(&m_Character.BodyPart[BODYPART_BOOTS], &CharacterMachine->Equipment[EQUIPMENT_BOOTS],
			MODEL_BODY_BOOTS+GetSkinModelIndex(Hero->Class)) == FALSE) bChangeArmor = TRUE;

		for (i = 0; i < 2; ++i)
		{
			if (CompareItemEqual(&m_Character.Weapon[i], &CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT+i], -1) == FALSE)
			{
				bChangeWeapon = TRUE;
				break;
			}
		}
		if (CompareItemEqual(&m_Character.Wing, &CharacterMachine->Equipment[EQUIPMENT_WING], -1) == FALSE)
			bChangeWing = TRUE;
		if (CompareItemEqual(&m_Character.Helper, &CharacterMachine->Equipment[EQUIPMENT_HELPER], -1) == FALSE)
			bChangeHelper = TRUE;
	}

	if (bChangeArmor == FALSE && bChangeWeapon == FALSE && bChangeWing == FALSE && bChangeHelper == FALSE)
		return;

	if (Hero->Change == FALSE)
	{
		if (bChangeArmor == TRUE)
		{
			DeleteCloth(&m_Character, NULL, NULL);
			memcpy(&m_Character.BodyPart, &Hero->BodyPart, sizeof(PART_t) * MAX_BODYPART);
			for (i = 0; i < MAX_BODYPART; ++i)
			{
				m_Character.BodyPart[i].m_pCloth[0] = NULL;
				m_Character.BodyPart[i].m_pCloth[1] = NULL;
				m_Character.BodyPart[i].m_byNumCloth = 0;
			}
		}
		if (bChangeWeapon == TRUE)
		{
			memcpy(&m_Character.Weapon, &Hero->Weapon, sizeof(PART_t) * 2);
		}
		if (bChangeWing == TRUE)
		{
			memcpy(&m_Character.Wing, &Hero->Wing, sizeof(PART_t));
		}
		if (bChangeHelper == TRUE)
		{
			memcpy(&m_Character.Helper, &Hero->Helper, sizeof(PART_t));
		}
	}
	else	// 변신 상태
	{
		if (bChangeArmor == TRUE)
		{
			DeleteCloth(&m_Character, NULL, NULL);

			m_Character.BodyPart[BODYPART_HEAD].Type = MODEL_BODY_HELM + GetSkinModelIndex(Hero->Class);
			SetItemToPhoto(&m_Character.BodyPart[BODYPART_HELM], &CharacterMachine->Equipment[EQUIPMENT_HELM],
				MODEL_BODY_HELM+GetSkinModelIndex(Hero->Class));
			SetItemToPhoto(&m_Character.BodyPart[BODYPART_ARMOR], &CharacterMachine->Equipment[EQUIPMENT_ARMOR],
				MODEL_BODY_ARMOR+GetSkinModelIndex(Hero->Class));
			SetItemToPhoto(&m_Character.BodyPart[BODYPART_PANTS], &CharacterMachine->Equipment[EQUIPMENT_PANTS],
				MODEL_BODY_PANTS+GetSkinModelIndex(Hero->Class));
			SetItemToPhoto(&m_Character.BodyPart[BODYPART_GLOVES], &CharacterMachine->Equipment[EQUIPMENT_GLOVES],
				MODEL_BODY_GLOVES+GetSkinModelIndex(Hero->Class));
			SetItemToPhoto(&m_Character.BodyPart[BODYPART_BOOTS], &CharacterMachine->Equipment[EQUIPMENT_BOOTS],
				MODEL_BODY_BOOTS+GetSkinModelIndex(Hero->Class));

			for (i = 0; i < MAX_BODYPART; ++i)
			{
				m_Character.BodyPart[i].m_pCloth[0] = NULL;
				m_Character.BodyPart[i].m_pCloth[1] = NULL;
				m_Character.BodyPart[i].m_byNumCloth = 0;
			}
		}
		if (bChangeWeapon == TRUE)
		{
			for (i = 0; i < 2; ++i)
			{
				SetItemToPhoto(&m_Character.Weapon[i], &CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT+i], -1);
			}
		}
		if (bChangeWing == TRUE)
		{
			SetItemToPhoto(&m_Character.Wing, &CharacterMachine->Equipment[EQUIPMENT_WING], -1);
		}
		if (bChangeHelper == TRUE)
		{
			SetItemToPhoto(&m_Character.Helper, &CharacterMachine->Equipment[EQUIPMENT_HELPER], -1);
		}
	}
	
	if (bChangeHelper == TRUE)
	{
		SetPhotoPose(AT_ATTACK1);
		m_iSettingAnimation = AT_ATTACK1;
	}
	else
	{
		SetPhotoPose(m_iCurrentAnimation);
	}

	SetAnimation( AT_ATTACK2 );
}

void ModelView::DrawViewPortMesh()
{
	CHARACTER * c = &m_Character;
	OBJECT * o = &c->Object;

	c->Helper.Type = 0;

	int WorldBackup = World;
	World = WD_0LORENCIA;
	MoveCharacter(c, o);
	MoveCharacterVisual(c, o);
	World = WorldBackup;

	Coord pos  = GetScreenPosition();
	Dim	  size = GetSize();

	BeginDrawViewPortMesh( pos, size, o->Position );

	Vector( 0.0f, 0.0f, m_Angle, o->Angle);
	o->Scale = 0.7f * m_Zoom;
	Vector( 1, 1, 1, o->Light );

	RenderCharacter( c, o );

	EndDrawViewPortMesh();
}
	
#endif //NEW_USER_INTERFACE_UISYSTEM
