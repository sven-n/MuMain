// MonkSystem.cpp: implementation of the CMonkSystem class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#ifdef PBG_ADD_NEWCHAR_MONK
#include "MonkSystem.h"
#include "ZzzOpenData.h"
#include "ZzzEffect.h"
#include "ZzzAI.h"
#include "ZzzLodTerrain.h"
#include "wsclientinline.h"
#include "CSChaosCastle.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMonkSystem g_CMonkSystem;

CItemEqualType::CItemEqualType()
{
	m_nModelType=0;
	m_nSubLeftType=0;
	m_nSubRightType=0;
}

CItemEqualType::~CItemEqualType()
{

}

void CItemEqualType::SetModelType(int _ModelIndex, int _Left, int _Right)
{
	m_nModelType = _ModelIndex;
	m_nSubLeftType = _Left;
	m_nSubRightType = _Right;
}

CMonkSystem::CMonkSystem()
{
	m_nRepeatedlyCnt =0;
	memset(m_arrRepeatedly,0,sizeof(DamageInfo)*MAX_REPEATEDLY);
	Init();
	RegistItem();
	
	m_nTotalCnt = m_mapItemEqualType.size();
	SetSwordformGlovesItemType();
	InitLower();
}

CMonkSystem::~CMonkSystem()
{
	Destroy();
}

void CMonkSystem::Init()
{
	m_mapItemEqualType.clear();
	memset(&m_cItemEqualType, 0, sizeof(CItemEqualType));
	m_listGloveformSword.clear();

	InitEffectOnce();
}

void CMonkSystem::Destroy()
{
	m_mapItemEqualType.clear();
	m_listGloveformSword.clear();
	m_DarksideIndex.clear();
	m_DarkTargetIndex.clear();
}

void CMonkSystem::RegistItem()
{
	m_cItemEqualType.SetModelType(MODEL_SWORD+32, MODEL_SWORD_32_LEFT, MODEL_SWORD_32_RIGHT);
	m_mapItemEqualType.insert(tm_ItemEqualType::value_type(m_cItemEqualType.GetModelType(), m_cItemEqualType));

	m_cItemEqualType.SetModelType(MODEL_SWORD+33, MODEL_SWORD_33_LEFT, MODEL_SWORD_33_RIGHT);
	m_mapItemEqualType.insert(tm_ItemEqualType::value_type(m_cItemEqualType.GetModelType(), m_cItemEqualType));

	m_cItemEqualType.SetModelType(MODEL_SWORD+34, MODEL_SWORD_34_LEFT, MODEL_SWORD_34_RIGHT);
	m_mapItemEqualType.insert(tm_ItemEqualType::value_type(m_cItemEqualType.GetModelType(), m_cItemEqualType));
}

void CMonkSystem::LoadModelItem()
{
	AccessModel(MODEL_SWORD+32, "Data\\Item\\", "Sword33");	
	AccessModel(MODEL_SWORD_32_LEFT, "Data\\Item\\", "SwordL33");
	AccessModel(MODEL_SWORD_32_RIGHT, "Data\\Item\\", "SwordR33");

	AccessModel(MODEL_SWORD+33, "Data\\Item\\", "Sword34");	
	AccessModel(MODEL_SWORD_33_LEFT, "Data\\Item\\", "SwordL34");
	AccessModel(MODEL_SWORD_33_RIGHT, "Data\\Item\\", "SwordR34");

	AccessModel(MODEL_SWORD+34, "Data\\Item\\", "Sword35");	
	AccessModel(MODEL_SWORD_34_LEFT, "Data\\Item\\", "SwordL35");
	AccessModel(MODEL_SWORD_34_RIGHT, "Data\\Item\\", "SwordR35");

	AccessModel(MODEL_ARMORINVEN_60, "Data\\player\\", "Armor_inventory60");
	AccessModel(MODEL_ARMORINVEN_61, "Data\\player\\", "ArmorMale61_inventory");
	AccessModel(MODEL_ARMORINVEN_62, "Data\\player\\", "ArmorMale62_inventory");
}

void CMonkSystem::LoadModelItemTexture()
{
	OpenTexture(MODEL_SWORD+32, "player\\");	
	OpenTexture(MODEL_SWORD_32_LEFT, "player\\");
	OpenTexture(MODEL_SWORD_32_RIGHT, "player\\");

	OpenTexture(MODEL_SWORD+33, "Item\\");
	OpenTexture(MODEL_SWORD_33_LEFT, "Item\\");
	OpenTexture(MODEL_SWORD_33_RIGHT, "Item\\");

	OpenTexture(MODEL_SWORD+34, "player\\");
	OpenTexture(MODEL_SWORD_34_LEFT, "player\\");
	OpenTexture(MODEL_SWORD_34_RIGHT, "player\\");
	
	OpenTexture(MODEL_ARMORINVEN_60, "player\\");
	OpenTexture(MODEL_ARMORINVEN_61, "player\\");
	OpenTexture(MODEL_ARMORINVEN_62, "player\\");
}

int CMonkSystem::GetSubItemType(int _Type, int _Left)
{	
	int iType = _Type;
	tm_ItemEqualType::iterator iter = m_mapItemEqualType.find(iType);

	CItemEqualType _SubType;
	if(iter == m_mapItemEqualType.end())
		return iType;

	_SubType = (CItemEqualType)iter->second;

	if(_Left)
		return _SubType.GetSubLeftType();
	else
		return _SubType.GetSubRightType();

	return iType;
}

int CMonkSystem::GetModelItemType(int _Type)
{
	for(tm_ItemEqualType::iterator iter = m_mapItemEqualType.begin(); iter != m_mapItemEqualType.end(); ++iter)
	{
		CItemEqualType _ItemType;
		if(iter == m_mapItemEqualType.end())
			return _Type;

		_ItemType = (CItemEqualType)iter->second;
		if(_ItemType.GetSubLeftType() == _Type ||_ItemType.GetSubRightType() == _Type)
			return _ItemType.GetModelType();
	}
	return _Type;
}

int CMonkSystem::OrginalTypeCommonItemMonk(int _ModifyType)
{
	if(_ModifyType >= MODEL_HELM_MONK && _ModifyType <= MODEL_BOOTS_MONK + MODEL_ITEM_COMMONCNT_RAGEFIGHTER)
	{
		int nItemType = (_ModifyType - MODEL_HELM_MONK)/MODEL_ITEM_COMMONCNT_RAGEFIGHTER+7;
		int nItemSubType = (_ModifyType - MODEL_HELM_MONK)%MODEL_ITEM_COMMONCNT_RAGEFIGHTER+5;
		
		int OrgItemType = (nItemType == 10) ? nItemType+1 : nItemType;
		int OrgItemSubType = (nItemSubType >= 7) ? nItemSubType+1 : nItemSubType;
		_ModifyType = OrgItemType*MAX_ITEM_INDEX+OrgItemSubType+MODEL_ITEM;
	}
	return _ModifyType;
}

int CMonkSystem::ModifyTypeCommonItemMonk(int _OrginalType)
{
	int nItemType = (_OrginalType - MODEL_ITEM) / MAX_ITEM_INDEX;
	int nItemSubType = (_OrginalType - MODEL_ITEM) % MAX_ITEM_INDEX;
	int nCommonItem[MODEL_ITEM_COMMONCNT_RAGEFIGHTER] = {5,6,8,9};

	if (nItemType >= 7 && nItemType <= 11)
	{
		for(int i=0; i<MODEL_ITEM_COMMONCNT_RAGEFIGHTER; ++i)
		{
			if(nItemSubType == nCommonItem[i])
			{
				int _TempItemType = (nItemType == 11) ? nItemType-1 : nItemType;
				_OrginalType = MODEL_HELM_MONK + (_TempItemType - 7) * MODEL_ITEM_COMMONCNT_RAGEFIGHTER + i;
				break;
			}
		}
	}

	return _OrginalType;
}

bool CMonkSystem::IsRagefighterCommonWeapon(BYTE _Class, int _Type)
{
	if((GetBaseClass(_Class) ==  CLASS_RAGEFIGHTER) && 
	((_Type == MODEL_SWORD+0) || (_Type == MODEL_SWORD+1)
	|| (_Type == MODEL_AXE+0) || (_Type == MODEL_AXE+1) || (_Type == MODEL_AXE+3)
	|| (_Type == MODEL_MACE+0) || (_Type == MODEL_MACE+1) || (_Type == MODEL_MACE+2)
	|| (_Type == MODEL_MACE+3) || (_Type == MODEL_MACE+4)))
	{
		return true;
	}

	return false;
}

bool CMonkSystem::IsSwordformGloves(int _Type)
{
	_Type = EqualItemModelType(_Type);

	tm_ItemEqualType::iterator iter = m_mapItemEqualType.find(_Type);
	
	CItemEqualType _SubType;
	if(iter == m_mapItemEqualType.end())
		return false;

	_SubType = (CItemEqualType)iter->second;

	if(_SubType.GetModelType() == _Type)
		return true;
	
	return false;
}

void CMonkSystem::RenderSwordformGloves(CHARACTER *_pCha, int _ModelType, int _Hand, float _Alpha, bool _Translate, int _Select)
{
	PART_t *w = &_pCha->Weapon[_Hand];
	RenderPartObject(&_pCha->Object,ModifyTypeSwordformGloves(_ModelType, _Hand),w,_pCha->Light,_Alpha,w->Level<<3,w->Option1,w->ExtOption,false,false,_Translate,_Select);
}

int CMonkSystem::ModifyTypeSwordformGloves(int _ModelType, int _LeftHand)
{
	return GetSubItemType(_ModelType, _LeftHand);
}

int CMonkSystem::EqualItemModelType(int _Type)
{
	return GetModelItemType(_Type);
}

void CMonkSystem::MoveBlurEffect(CHARACTER* _pCha, OBJECT* _pObj, BMD* pModel)
{

	if(_pCha == NULL)	return;
	if(_pObj == NULL)	return;
	if(pModel == NULL)	return;

	BMD *b = &Models[_pObj->Type];
	vec3_t  Light;
	vec3_t StartPos, StartLocal, EndPos, EndLocal;
	float fDelay =10.0f;
	float fSpeedPerFrame = b->Actions[_pObj->CurrentAction].PlaySpeed/fDelay;
	float fAnimationFrame = _pObj->AnimationFrame - b->Actions[_pObj->CurrentAction].PlaySpeed;

	if(fAnimationFrame > 5.5f)
		return;

	for(int i=0; i<fDelay; i++)
	{
		b->Animation(BoneTransform, fAnimationFrame, _pObj->PriorAnimationFrame, _pObj->PriorAction, _pObj->Angle, _pObj->HeadAngle);
			
		Vector(1.0f, 1.0f, 1.0f, Light);
		int _LeftHand=0;
		if(_pObj->CurrentAction==PLAYER_ATTACK_SWORD_LEFT1 || _pObj->CurrentAction==PLAYER_ATTACK_SWORD_LEFT2)
			_LeftHand = 1;

		Vector(-30.0f, 0.0f, 0.0f, StartLocal);
		Vector(30.0f, 0.0f, 0.0f, EndLocal);

		b->TransformPosition(BoneTransform[_pCha->Weapon[_LeftHand].LinkBone], StartLocal, StartPos, false);
		b->TransformPosition(BoneTransform[_pCha->Weapon[_LeftHand].LinkBone], EndLocal, EndPos, false);
		CreateBlur(_pCha, StartPos, EndPos, Light, 5, true);

		fAnimationFrame += fSpeedPerFrame;
	}
}

void CMonkSystem::SetSwordformGlovesItemType()
{
	for(tm_ItemEqualType::iterator iter = m_mapItemEqualType.begin(); iter != m_mapItemEqualType.end(); ++iter)
	{
		CItemEqualType _ItemType;
		if(iter == m_mapItemEqualType.end())
			return;

		_ItemType = (CItemEqualType)iter->second;
		m_listGloveformSword.push_back(_ItemType.GetModelType()%MODEL_ITEM);
	}
}

bool CMonkSystem::IsSwordformGlovesItemType(int _Type)
{
	for(list_ItemType::iterator iter = m_listGloveformSword.begin(); iter != m_listGloveformSword.end(); ++iter)
	{
		if(*iter == _Type)
			return true;
	}
	return false;
}

bool CMonkSystem::RageEquipmentWeapon(int _Index, short _ItemType)
{
	int _OtherEquip = (_Index == EQUIPMENT_WEAPON_LEFT) ? EQUIPMENT_WEAPON_RIGHT : EQUIPMENT_WEAPON_LEFT;
	ITEM *pOtherHand = &CharacterMachine->Equipment[_OtherEquip];
	//글러브형 무기는 글러브형무기하고만 착용가능
	if(g_CMonkSystem.IsSwordformGlovesItemType(_ItemType))
	{	
		if(pOtherHand->Type == -1)
			return true;
		else if(!g_CMonkSystem.IsSwordformGlovesItemType(pOtherHand->Type))
			return false;
	}
	else
	{
		if(pOtherHand->Type == -1)
			return true;
		else if(g_CMonkSystem.IsSwordformGlovesItemType(pOtherHand->Type))
			return false;
	}
	return true;
}

bool CMonkSystem::SetRepeatedly(int _Damage, int _DamageType, bool _Double, bool _bEndRepeatedly)
{
	if(m_nRepeatedlyCnt < MAX_REPEATEDLY)
	{
		if(!_bEndRepeatedly)
		{
			if(m_nRepeatedlyCnt == MAX_REPEATEDLY-1)
			{
				m_nRepeatedlyCnt = MAX_REPEATEDLY-1;
				return false;
			}
		}

		m_arrRepeatedly[m_nRepeatedlyCnt].m_Damage = _Damage;
		m_arrRepeatedly[m_nRepeatedlyCnt].m_DamageType = _DamageType;
		m_arrRepeatedly[m_nRepeatedlyCnt].m_Double = _Double;
		m_nRepeatedlyCnt++;

		return true;
	}
	return false;
}

int CMonkSystem::GetRepeatedlyDamage(int _index)
{
	return m_arrRepeatedly[_index].m_Damage;
}

int CMonkSystem::GetRepeatedlyDamageType(int _index)
{
	return m_arrRepeatedly[_index].m_DamageType;
}

bool CMonkSystem::GetRepeatedlyDouble(int _index)
{
	return m_arrRepeatedly[_index].m_Double;
}

int CMonkSystem::GetRepeatedlyCnt()
{
	return m_nRepeatedlyCnt;
}

bool CMonkSystem::SetRageSkillAni(int _nSkill, OBJECT* _pObj)
{
	switch(_nSkill)
	{
		case AT_SKILL_THRUST:
			SetAction(_pObj, PLAYER_SKILL_THRUST);
			return true;
		case AT_SKILL_STAMP:
			SetAction(_pObj, PLAYER_SKILL_STAMP);
			return true;
		case AT_SKILL_GIANTSWING:
			SetAction(_pObj, PLAYER_SKILL_GIANTSWING);
			return true;
		case AT_SKILL_DARKSIDE:
			SetAction(_pObj, PLAYER_SKILL_DARKSIDE_READY);
			return true;
		case AT_SKILL_DRAGON_LOWER:
			SetAction(_pObj, PLAYER_SKILL_DRAGONLORE);
			return true;
		case AT_SKILL_DRAGON_KICK:
			SetAction(_pObj, PLAYER_SKILL_DRAGONKICK);
			return true;
		case AT_SKILL_ATT_UP_OURFORCES:
		case AT_SKILL_HP_UP_OURFORCES:
		case AT_SKILL_DEF_UP_OURFORCES:
			return false;
	}
	return false;
}

bool CMonkSystem::IsRageHalfwaySkillAni(int _nSkill)
{
	switch(_nSkill)
	{
		case AT_SKILL_STAMP:
		case AT_SKILL_GIANTSWING:
		case AT_SKILL_DRAGON_KICK:
			return true;
		default:
			return false;
	}
	return false;
}

bool CMonkSystem::SendAttackPacket(CHARACTER* _pCha, int _nMoveTarget, int _nSkill)
{
	OBJECT *pObj = &_pCha->Object;
	
	BYTE TargetPosX = (BYTE)(_pCha->TargetPosition[0]/TERRAIN_SCALE);
	BYTE TargetPosY = (BYTE)(_pCha->TargetPosition[1]/TERRAIN_SCALE);

	vec3_t vDis;
	Vector(0.0f, 0.0f, 0.0f, vDis);
	VectorSubtract(_pCha->TargetPosition, _pCha->Object.Position, vDis);
	VectorNormalize(vDis);
	VectorScale(vDis, TERRAIN_SCALE, vDis);
	VectorSubtract(_pCha->TargetPosition, vDis, vDis);
	BYTE CharPosX = (BYTE)(vDis[0]/TERRAIN_SCALE);
	BYTE CharPosY = (BYTE)(vDis[1]/TERRAIN_SCALE);

	if(_nMoveTarget <= -1)
	{
		return false;
	}

	int TargetIndex = TERRAIN_INDEX(TargetPosX, TargetPosY);

	if((TerrainWall[TargetIndex]&TW_NOMOVE) != TW_NOMOVE && (TerrainWall[TargetIndex]&TW_NOGROUND) != TW_NOGROUND)
	{
//		if(!InChaosCastle())
			SendPosition(CharPosX, CharPosY);

		VectorCopy(CharactersClient[_nMoveTarget].Object.Position, _pCha->TargetPosition);
		//몬스터의 넉백효과의 의해 이펙트가 혼란스러움 방지
		if(!(pObj->CurrentAction == PLAYER_SKILL_GIANTSWING && m_btAttState == FRAME_SECONDATT))
			pObj->Angle[2] = CreateAngle(pObj->Position[0], pObj->Position[1], _pCha->TargetPosition[0], _pCha->TargetPosition[1]);

		SendRequestMagic(_nSkill, CharactersClient[_nMoveTarget].Key);
		return true;
	}

	return false;
}

bool CMonkSystem::RageFighterEffect(OBJECT* _pObj, int _Type)
{
	BMD *b = &Models[_Type];
	if(_pObj->CurrentAction == PLAYER_SKILL_DARKSIDE_READY)
	{
		Vector(1.f, 1.f, 1.f, b->BodyLight);
		float fAlpha=1.0f;
		int _nAniKey = Models[_pObj->Type].Actions[PLAYER_SKILL_DARKSIDE_READY].NumAnimationKeys;
		float _nRoop = (float)(180.0f/_nAniKey)*_pObj->AnimationFrame+180.0f;
		fAlpha = sinf(_nRoop*Q_PI/180)*0.7f+1.0f;
		_pObj->Angle[2] = 45.0f;
		b->RenderBody(RENDER_TEXTURE, fAlpha, _pObj->BlendMesh, _pObj->BlendMeshLight, _pObj->BlendMeshTexCoordU, _pObj->BlendMeshTexCoordV);
		return true;
	}
	else if(_pObj->CurrentAction == PLAYER_SKILL_DARKSIDE_ATTACK)
	{
		Vector(1.0f, 1.0f, 1.0f, b->BodyLight);
		float fAlpha = _pObj->Alpha;
		VectorScale(b->BodyLight, fAlpha, b->BodyLight);
		b->RenderBody(RENDER_TEXTURE|RENDER_BRIGHT, fAlpha, _pObj->BlendMesh, _pObj->BlendMeshLight, _pObj->BlendMeshTexCoordU, _pObj->BlendMeshTexCoordV);
		return true;
	}
	else if(_pObj->CurrentAction == PLAYER_SKILL_ATT_UP_OURFORCES
		|| _pObj->CurrentAction == PLAYER_SKILL_HP_UP_OURFORCES)
	{
		float fAlpha=1.0f;
		if(_pObj->AnimationFrame < 4)
		{
			fAlpha = fAlpha - _pObj->AnimationFrame*0.1f;
		}
		else if(_pObj->AnimationFrame > 8)
		{
			fAlpha = (_pObj->AnimationFrame-6)*0.1f + 0.4f;
		}
		else
		{
			fAlpha = 0.0f;
		}
		Vector(1.0f, 1.0f, 1.0f, b->BodyLight);
		b->RenderBody(RENDER_BRIGHT|RENDER_COLOR, _pObj->Alpha, _pObj->BlendMesh, _pObj->BlendMeshLight, _pObj->BlendMeshTexCoordU, _pObj->BlendMeshTexCoordV);
		b->RenderBody(RENDER_TEXTURE, fAlpha, _pObj->BlendMesh, _pObj->BlendMeshLight, _pObj->BlendMeshTexCoordU, _pObj->BlendMeshTexCoordV);
		
		return true;
	}
	return false;
}

void CMonkSystem::SetDarksideTargetIndex(WORD* _pTargetIndex)
{
	for(int i=0; i<DARKSIDE_TARGET_MAX; ++i)
	{
		m_DarksideIndex.push_back(_pTargetIndex[i]);
		if(_pTargetIndex[i] != DS_TARGET_NONE)
		{
			m_DarkTargetIndex.push_back(FindCharacterIndex(_pTargetIndex[i]));
			m_nDarksideEffectTotal++;
			m_fDistanceFrame = 0;
			m_fDistanceNextFrame = 0;
			m_bDarkSideEffOnce = false;
			m_bDarkSideEffOnce2 =false;
		}
	}

	if(!m_nDarksideEffectTotal)
	{
		SendRequestMagic(AT_SKILL_DARKSIDE, HeroKey);
	}
	else
	{
		PlayBuffer(SOUND_RAGESKILL_DARKSIDE_ATTACK);
	}
}

WORD CMonkSystem::GetDarksideTargetIndex(int _nIndex)
{
	if(_nIndex >= (int)m_DarkTargetIndex.size())
	{
		return 0;
	}

	vec_DarkTargetIndex::iterator iter = m_DarkTargetIndex.begin();
	if(iter == m_DarkTargetIndex.end())
	{
		return 0;
	}

	return m_DarkTargetIndex[_nIndex];
}

int CMonkSystem::GetDarksideCnt()
{
	return m_nDarksideCnt;
}

void CMonkSystem::SetDarksideCnt()
{
	m_nDarksideCnt++;
}

bool CMonkSystem::SendDarksideAtt(OBJECT* _pObj)
{

	if(_pObj == NULL)	return false;

	if(m_nDarksideCnt >= DARKSIDE_TARGET_MAX)
	{
		InitDarksideTarget();
		return false;
	}
	
	vec_DarkIndex::iterator iter = m_DarksideIndex.begin();
	if(iter == m_DarksideIndex.end())
	{
		return false;
	}

	int _DarksideTargetIndex = m_DarksideIndex[m_nDarksideCnt];
	
	if(_DarksideTargetIndex == DS_TARGET_NONE)
	{
		return false;
	}

	SendRequestMagic(AT_SKILL_DARKSIDE, _DarksideTargetIndex);
	m_nDarksideCnt++;
	return true;
}

void CMonkSystem::InitDarksideTarget()
{
	m_nDarksideCnt =0;
	m_nDarksideEffectTotal = 0;
	m_nDarksideEffectAttCnt = 0;
	m_fOtherAniFrame = 0.1f;
	m_fDistanceFrame = 0;
	m_fDistanceNextFrame = 0;
	m_bDarkSideEffOnce = false;
	m_bDarkSideEffOnce2 = false;

	vec_DarkIndex::iterator iter = m_DarksideIndex.begin();
	for(; iter != m_DarksideIndex.end(); ++iter)
	{
		*iter = DS_TARGET_NONE;
	}
	m_DarksideIndex.clear();

	vec_DarkTargetIndex::iterator itertarget = m_DarkTargetIndex.begin();
	for(; itertarget != m_DarkTargetIndex.end(); ++itertarget)
	{
		*itertarget = -1;
	}
	m_DarkTargetIndex.clear();
}

int CMonkSystem::GetDarksideEffectTotal()
{
	return m_nDarksideEffectTotal;
}

bool CMonkSystem::CalculateDarksideTrans(OBJECT* _pObj, vec3_t _vPos, float _fAni, float _fNextAni)
{
	vec3_t _StartPos, _TargetPos;
	vec3_t _DirectionVec;
	float _Distance = _fAni*_fAni*0.05f;
	float _NextDistance = _fNextAni*_fNextAni*0.05f;

	VectorCopy(_pObj->Position, _StartPos);
	VectorCopy(_vPos, _TargetPos);
	VectorSubtract(_TargetPos, _StartPos, _DirectionVec);
	VectorNormalize(_DirectionVec);
	VectorScale(_DirectionVec, _Distance, _DirectionVec);
	VectorAdd(_pObj->Position, _DirectionVec, _pObj->Position);
   	
	vec3_t vAngle;
	VectorCopy(_pObj->Angle, vAngle);
	vAngle[2] = CreateAngle(_StartPos[0], _StartPos[1], _TargetPos[0], _TargetPos[1]);
	VectorCopy(vAngle, _pObj->Angle);

	if(_fNextAni)
	{
		vec3_t _NextStartPos;
		VectorCopy(_StartPos, _NextStartPos);
		VectorCopy(_vPos, _TargetPos);
		VectorSubtract(_TargetPos, _NextStartPos, _DirectionVec);
		VectorNormalize(_DirectionVec);
		VectorScale(_DirectionVec, _NextDistance, _DirectionVec);
		VectorAdd(_NextStartPos, _DirectionVec, _NextStartPos);

		VectorSubtract(_StartPos, _TargetPos, _TargetPos);
		float fTargetLen = VectorLength(_TargetPos);
		
		VectorSubtract(_StartPos, _NextStartPos, _NextStartPos);
		float fNextLen = VectorLength(_NextStartPos);

		VectorCopy(_pObj->Position, _NextStartPos);
		VectorSubtract(_StartPos, _NextStartPos, _NextStartPos);
		float fObjLen = VectorLength(_NextStartPos);

		if(fTargetLen > fObjLen)
		{
			if(fTargetLen < fNextLen)
			{
				m_bDarkSideEffOnce = true;
				return true;
			}
		}
	}
	return false;
}

void CMonkSystem::DarksideRendering(CHARACTER* pCha, PART_t *pPart, bool Translate, int Select)
{
	OBJECT* pObj = &pCha->Object;
	float fAnimationFrame = pObj->AnimationFrame;
	int Type = pPart->Type;

	vec3_t vPos, vStartPos, vOrgPos, vOrgAngle;
	unsigned short OrgCurrentAction;
	OrgCurrentAction = pObj->CurrentAction;
	VectorCopy(pObj->Position, vOrgPos);
	VectorCopy(pObj->Position, vPos);
	VectorCopy(pObj->StartPosition, vStartPos);
	VectorCopy(pObj->Angle, vOrgAngle);
	
	if(GetDarksideEffectTotal() > m_nDarksideEffectAttCnt)
	{
		int _TargetIndex = GetDarksideTargetIndex(m_nDarksideEffectAttCnt);

		m_fOtherAniFrame = (float)(pCha->AttackTime+1)*0.1f*2.0f;
		m_fDistanceFrame += 1.0f;
		m_fDistanceNextFrame = m_fDistanceFrame+1.0f;
		if(m_fOtherAniFrame >= 2)
		{
			m_fOtherAniFrame = 1.9f;
		}

		if(_TargetIndex >= 0)
		{
			VectorCopy(CharactersClient[_TargetIndex].Object.Position, vPos);
		}
		VectorCopy(vOrgPos, pObj->StartPosition);
		CalculateDarksideTrans(pObj, vPos, m_fDistanceFrame, m_fDistanceNextFrame);

		vec3_t vDisLen;
		VectorSubtract(pObj->Position, pObj->StartPosition, vDisLen);
		int nLen = VectorLength(vDisLen);

		vec3_t vLight, vAngle;
		Vector(1.0f, 1.0f, 1.0f, vLight);
		if(nLen > 30 && !m_bDarkSideEffOnce2)
		{
			VectorCopy(pObj->Angle, vAngle);
			Vector(90.0f,0.0f,vAngle[2]-180.0f, vAngle);
			CreateEffect(MODEL_SHOCKWAVE01, pObj->Position, vAngle, vLight, 3, pObj, -1, 0, 0, 0, 0.8f);
			m_bDarkSideEffOnce2 = true;
		}
		if(m_bDarkSideEffOnce)
		{
			CreateParticle(BITMAP_DAMAGE2, vPos, pObj->Angle, vLight, 0, 1.0f);
			m_bDarkSideEffOnce = false;
		}

		if(GetDarksideCnt() > m_nDarksideEffectAttCnt)
		{
			m_nDarksideEffectAttCnt++;
			m_fOtherAniFrame = 0.1f;
			m_fDistanceFrame = 0;
			m_fDistanceNextFrame = 0;
			m_bDarkSideEffOnce = false;
			m_bDarkSideEffOnce2 = false;
		}

		pObj->AnimationFrame = m_fOtherAniFrame;
		pObj->CurrentAction = PLAYER_SKILL_DARKSIDE_ATTACK;
		Calc_ObjectAnimation(pObj, Translate, Select);
		RenderPartObject(pObj,Type,pPart,pCha->Light,pObj->Alpha,pPart->Level<<3,pPart->Option1,pPart->ExtOption,false,false,Translate,Select);
	}
	
	if(m_nDarksideCnt > 0)
	{
		if(fAnimationFrame > (m_nDummyIndex*2) && m_nDummyIndex < 5)
		{
			if(pObj->m_sTargetIndex>=0)
			{
				VectorCopy(CharactersClient[pObj->m_sTargetIndex].Object.Position, vPos);
			}
			if(m_nDarksideEffectAttCnt < (int)m_DarkTargetIndex.size())
			{
				VectorCopy(CharactersClient[m_DarkTargetIndex[m_nDarksideEffectAttCnt]].Object.Position, vPos);
			}

			VectorCopy(vPos, pObj->Position);
			SetDummy(pObj->Position, vPos, m_nDummyIndex);
			m_fDummyAniFrame =0.0f;
			m_nDummyIndex++;
		}
		
		for(int index=0; index<m_nDummyIndex; ++index)
		{
#ifdef PBG_MOD_RAGEFIGHTERSOUND
			bool bChange=false;
			if(pObj->Kind == KIND_PLAYER && pObj->Type == MODEL_PLAYER && 
				(pObj->SubType == MODEL_SKELETON_CHANGED || pObj->SubType == MODEL_PANDA
				|| pObj->SubType ==MODEL_SKELETON_PCBANG || pObj->SubType == MODEL_HALLOWEEN
				|| pObj->SubType ==MODEL_XMAS_EVENT_CHANGE_GIRL || pObj->SubType ==MODEL_SKELETON1))
			{
				bChange = true;
			}

			if(IsDummyRender(pObj->Position, vPos, m_fDummyAniFrame, index, bChange))
#endif //PBG_MOD_RAGEFIGHTERSOUND
			{
				tm_DummyUnit::iterator iter = m_tmDummyUnit.find(index);
				if(iter == m_tmDummyUnit.end())
					break;
				
				CDummyUnit* pDummy = &iter->second;
				VectorCopy(pDummy->GetStartPosition(), pObj->StartPosition);
				
				pObj->Alpha = pDummy->GetAlpha();
				
				vec3_t vAngle;
				VectorCopy(pObj->Angle, vAngle);
				vAngle[2] = CreateAngle(pObj->StartPosition[0], pObj->StartPosition[1], pObj->Position[0], pObj->Position[1]);
				VectorCopy(vAngle, pObj->Angle);
				
				vec3_t vLight;
				int DamageEff = (int)(m_fDummyAniFrame*10.0f);
				if(DamageEff==15)
				{
					Vector(1.0f, 1.0f, 1.0f, vLight);
					VectorScale(vLight, 0.2f, vLight);
					Vector(90.0f,0.0f,vAngle[2]-180.0f, vAngle);
					CreateEffect(MODEL_SHOCKWAVE01, pObj->Position, vAngle, vLight, 4, pObj, -1, 0, 0, 0, 0.8f);
				}
				
				DamageEff = (int)(m_fDummyAniFrame*10.0f);
				if(DamageEff == 10 && (GetDarksideEffectTotal() > m_nDarksideEffectAttCnt))
				{
					Vector(1.0f, 1.0f, 1.0f, vLight);
					CreateParticle(BITMAP_DAMAGE2, vPos, pObj->Angle, vLight, 0, 1.0f);
				}
				
				pObj->AnimationFrame = m_fDummyAniFrame;
				pObj->CurrentAction = PLAYER_SKILL_DARKSIDE_ATTACK;
				Calc_ObjectAnimation(pObj, Translate, Select);
				RenderPartObject(pObj,Type,pPart,pCha->Light,pObj->Alpha,pPart->Level<<3,pPart->Option1,pPart->ExtOption,false,false,Translate,Select);
			}
		}
	}
	
	pObj->Alpha =1.0f;
	pObj->CurrentAction = OrgCurrentAction;
	pObj->AnimationFrame = fAnimationFrame;
	VectorCopy(vOrgPos, pObj->Position);
	VectorCopy(vStartPos, pObj->StartPosition);
	VectorCopy(vOrgAngle, pObj->Angle);
	Calc_ObjectAnimation(pObj, Translate, Select);
	RenderPartObject(pObj,Type,pPart,pCha->Light,pObj->Alpha,pPart->Level<<3,pPart->Option1,pPart->ExtOption,false,false,Translate,Select);
}

void CMonkSystem::InitDummyCal()
{
	m_tmDummyUnit.clear();
	memset(&m_tmDummyUnit, 0, sizeof(CDummyUnit)*m_tmDummyUnit.size());
	m_nDummyIndex=0;
}

void CMonkSystem::DestroyDummy()
{
	m_tmDummyUnit.clear();
}

void CMonkSystem::SetDummy(vec3_t pos, vec3_t target, int dummyindex)
{
	float Matrix[3][4];
	vec3_t vAngle, tempPos, outPos, vRangePos;
	VectorCopy(pos, tempPos);
	Vector(150.0f, 150.0f, 0.0f, vRangePos);
	Vector(0.0f,0.0f,60.0f*dummyindex,vAngle);
	AngleMatrix(vAngle,Matrix);
	VectorRotate(vRangePos, Matrix, outPos);
	VectorAdd(pos, outPos, pos);
	pos[2] = tempPos[2];

	m_CDummyUnit.Init(pos, target);
	m_tmDummyUnit.insert(tm_DummyUnit::value_type(dummyindex, m_CDummyUnit));
}
#ifdef PBG_MOD_RAGEFIGHTERSOUND
bool CMonkSystem::IsDummyRender(vec3_t vOutPos, vec3_t Target, float& pAni, int DummyIndex, bool Change)
#else //PBG_MOD_RAGEFIGHTERSOUND
bool CMonkSystem::IsDummyRender(vec3_t vOutPos, vec3_t Target, float& pAni, int DummyIndex)
#endif //PBG_MOD_RAGEFIGHTERSOUND
{
	tm_DummyUnit::iterator iter = m_tmDummyUnit.find(DummyIndex);

	if(iter == m_tmDummyUnit.end())
		return false;

	CDummyUnit* pDummy = &iter->second;

	if(pDummy->IsDistance())
	{
#ifdef PBG_MOD_RAGEFIGHTERSOUND
		pDummy->CalDummyPosition(vOutPos, pAni, Change);
#else //PBG_MOD_RAGEFIGHTERSOUND
		pDummy->CalDummyPosition(vOutPos, pAni);
#endif //PBG_MOD_RAGEFIGHTERSOUND
		return true;
	}
	else
	{
		return false;
	}

	return true;
}

void CMonkSystem::InitConsecutiveState(float _fFirstFrame, float _fSecondFrame, BYTE _btAttState)
{
	m_fFirstFrame = _fFirstFrame;
	
	if(_fFirstFrame < _fSecondFrame)
		m_fSecondFrame = _fSecondFrame;
	else
		m_fSecondFrame = 9999.f;

	m_btAttState = _btAttState;
}

bool CMonkSystem::IsConsecutiveAtt(float _fAttFrame)
{
	if(_fAttFrame >= m_fFirstFrame && _fAttFrame < m_fSecondFrame)
	{
		if(m_btAttState == FRAME_FIRSTATT)
			return false;
		else
		{
			m_btAttState = FRAME_FIRSTATT;
			return true;
		}
	}
	else if(_fAttFrame >= m_fSecondFrame)
	{
		if(m_btAttState == FRAME_SECONDATT)
			return false;
		else
		{
			m_btAttState = FRAME_SECONDATT;
			return true;
		}
	}
	return false;
}

void CMonkSystem::RenderRepeatedly(int _Key, OBJECT *pObj)
{
	vec3_t Position, Light, Light2;
	VectorCopy(pObj->Position, Position);
	WORD Damage;
	for(int _index=0; _index<m_nRepeatedlyCnt; _index++)
	{
		Damage = m_arrRepeatedly[_index].m_Damage;
		float scale = 15.0f;

		switch(m_arrRepeatedly[_index].m_DamageType)
		{	//데미지타입에 따른컬러
		case 0:
			if(_Key == HeroKey)
			{
				Vector(1.f,0.f,0.f,Light);
			}
			else
			{
				Vector(1.f,0.6f,0.f,Light);
			}
			break;
		case 1:	scale = 50.f;	Vector(0.0f,1.f,1.f,Light);	break;
		case 2:	scale = 50.f;	Vector(0.f,1.f,0.6f,Light);	break;
		case 3:	scale = 50.f;	Vector(0.f,0.6f,1.f,Light);	break;
		case 4:	Vector(1.f,0.f,1.f,Light);break;
		case 5:	Vector(0.f,1.f,0.f,Light);break;
		case 6:	Vector(0.7f,0.4f,1.0f,Light);break;
		default:Vector(1.f, 1.f, 1.f, Light);break;
		}

		if(!Damage)
		{
			if(_Key == HeroKey)
			{
				Vector(1.f,1.f,1.f,Light);
			}
			else
			{
				Vector(0.5f,0.5f,0.5f,Light);
			}
			CreatePoint(Position,-1,Light, 15.0f, true, true);
		}
		else
		{
			CreatePoint(Position, Damage, Light, scale, true, true);
		}
	
		if(m_arrRepeatedly[_index].m_Double)
		{	// 더블데미지
			Position[2] += 10.f;
			Vector(Light[0]-0.2f,Light[1]-0.2f,Light[2]-0.2f,Light2);
			CreatePoint(Position,Damage,Light2, scale+5.f, true);
		}

		Position[1] += (_index%2 == 0) ? -40.0f : 40.0f;
		Position[2] -= (rand()%10 + 15.0f);
	}
	m_nRepeatedlyCnt = 0;
}

bool CMonkSystem::IsRideNotUseSkill(int _nSkill, short _Type)
{
	if(_Type != MODEL_HELPER+37 && _Type != MODEL_HELPER+2 && _Type != MODEL_HELPER+3)
		return false;

	// 탈것타고 있을 경우 사용 불가능한 스킬
	switch(_nSkill)
	{
	case AT_SKILL_THRUST:
	case AT_SKILL_STAMP:
	case AT_SKILL_GIANTSWING:
	case AT_SKILL_DRAGON_KICK:
	case AT_SKILL_OCCUPY:
		return true;
	default:
		return false;
	}
	return false;
}

bool CMonkSystem::IsSwordformGlovesUseSkill(int _nSkill)
{
	switch(_nSkill)
	{
	case AT_SKILL_GIANTSWING:	//여기 스킬들은 장갑형 무기를 착용시에만 사용가능
	case AT_SKILL_DRAGON_LOWER:
	case AT_SKILL_DRAGON_KICK:
		{
			ITEM *pOtherHand = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];
			if(g_CMonkSystem.IsSwordformGlovesItemType(pOtherHand->Type))
			{
				return true;
			}
			else
			{
				pOtherHand = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT];
				if(g_CMonkSystem.IsSwordformGlovesItemType(pOtherHand->Type))
				{
					return true;
				}
			}		
		}
		return false;
	default:
		return true;
	}
	return true;
}

bool CMonkSystem::IsChangeringNotUseSkill(short _LType, short _RType, int _LLevel, int _RLevel)
{
	if(_LType == ITEM_HELPER+68 || _RType == ITEM_HELPER+68)
	{
		return true;
	}
	
	if(((_LType == ITEM_HELPER+10)
		&& (_LLevel == 0 || _LLevel == 8 || _LLevel == 24 || _LLevel == 32 || _LLevel == 40)) ||
		((_RType == ITEM_HELPER+10)
		&& (_RLevel == 0 || _RLevel == 8 || _RLevel == 24 || _RLevel == 32 || _RLevel == 40))
		)
	{
		return true;
	}

	return false;
}

void CMonkSystem::InitEffectOnce()
{
	m_bUseEffectOnce = false;
}

bool CMonkSystem::GetSkillUseState()
{
	return m_bUseEffectOnce;
}

bool CMonkSystem::RageCreateEffect(OBJECT *_pObj, int _nSkill)
{
	switch(_nSkill)
	{
	case AT_SKILL_THRUST:
		{
			if(m_bUseEffectOnce)
				return false;

			m_bUseEffectOnce = true;
			vec3_t vPosition;
			VectorCopy(CharactersClient[_pObj->m_sTargetIndex].Object.Position, vPosition);
			_pObj->Angle[2] = CreateAngle(_pObj->Position[0],_pObj->Position[1],vPosition[0],vPosition[1]);
			CreateEffect(MODEL_WOLF_HEAD_EFFECT, _pObj->Position, _pObj->Angle, _pObj->Light, 0, _pObj);
			CreateEffect(BITMAP_SBUMB, vPosition, _pObj->Angle, _pObj->Light, 0, _pObj, -1, 0, 0, 0, 2.1f);

			PlayBuffer(SOUND_RAGESKILL_THRUST);
		}
		return true;
	case AT_SKILL_STAMP:
		{
			if(m_bUseEffectOnce)
				return false;

			m_bUseEffectOnce = true;
			CreateEffect(MODEL_DOWN_ATTACK_DUMMY_R, _pObj->Position, _pObj->Angle, _pObj->Light, 0, _pObj);

			PlayBuffer(SOUND_RAGESKILL_STAMP);
		}
		return true;
	case AT_SKILL_GIANTSWING:
		{
			CreateEffect(BITMAP_SWORDEFF, _pObj->Position, _pObj->Angle, _pObj->Light, 0, _pObj);

			PlayBuffer(SOUND_RAGESKILL_GIANTSWING);
		}
		return true;
	case AT_SKILL_ATT_UP_OURFORCES:
		{
			if(m_bUseEffectOnce)
				return false;

			m_bUseEffectOnce = true;

			vec3_t vLight;
			Vector(0.2f, 0.4f,1.0f,vLight);

			vec3_t _vAngle;
			VectorCopy(_pObj->Angle, _vAngle);
			CreateEffect(BITMAP_SWORD_EFFECT_MONO, _pObj->Position, _pObj->Angle, vLight, 0, _pObj, -1, 0, 0, 0, 1.0f);

			for(int i=0; i<3; ++i)
			{
				Vector(0.6f, 0.65f,1.0f,vLight);
				CreateEffect(MODEL_SHOCKWAVE_GROUND01, _pObj->Position, _pObj->Angle, vLight, 0, _pObj, -1, 0, 0, 0, 1.0f - (i*0.15f));
				
				Vector(0.65f, 0.8f,1.0f,vLight);
				CreateEffect(BITMAP_EVENT_CLOUD, _pObj->Position, _pObj->Angle, vLight, 0, _pObj, -1, 0, 0, 0, 1.0f);
			}
			Vector(0.5f, 0.55f, 1.0f, vLight);
			CreateEffect(MODEL_WINDFOCE, _pObj->Position, _pObj->Angle, vLight, 0, _pObj, -1, 0, 0, 0, 1.0f);

			Vector(0.5f, 0.55f, 0.9f,vLight);
			CreateEffect(BITMAP_MAGIC, _pObj->Position, _pObj->Angle, vLight, 13);
		}
		return true;
	case AT_SKILL_HP_UP_OURFORCES:
		{
			if(m_bUseEffectOnce)
				return false;

			m_bUseEffectOnce = true;

			vec3_t vLight;
			Vector(0.5f, 0.28f, 1.0f, vLight);
			vec3_t _vAngle;
			VectorCopy(_pObj->Angle, _vAngle);
			CreateEffect(BITMAP_SWORD_EFFECT_MONO, _pObj->Position, _pObj->Angle, vLight, 1, _pObj, -1, 0, 0, 0, 1.0f);
			for(int i=0; i<3; ++i)
			{
				Vector(1.0f, 0.35f,1.0f,vLight);
				CreateEffect(MODEL_SHOCKWAVE_GROUND01, _pObj->Position, _pObj->Angle, vLight, 0, _pObj, -1, 0, 0, 0, 1.0f - (i*0.15f));
	
				Vector(0.8f, 0.4f,1.0f,vLight);
				CreateEffect(BITMAP_EVENT_CLOUD, _pObj->Position, _pObj->Angle, vLight, 0, _pObj, -1, 0, 0, 0, 1.0f);
			}
			Vector(0.85f, 0.2f,1.0f,vLight);
			CreateEffect(MODEL_WINDFOCE, _pObj->Position, _pObj->Angle, vLight, 4, _pObj, -1, 0, 0, 0, 1.0f);

			Vector(0.78f, 0.2f,1.0f,vLight);
			CreateEffect(BITMAP_MAGIC, _pObj->Position, _pObj->Angle, vLight, 13);
		}
		return true;
	case AT_SKILL_DEF_UP_OURFORCES:
		{
			if(m_bUseEffectOnce)
				return false;

			m_bUseEffectOnce = true;

			vec3_t vLight;
			Vector(1.0f, 0.15f,0.0f,vLight);
			vec3_t _vAngle;
			VectorCopy(_pObj->Angle, _vAngle);
			CreateEffect(BITMAP_SWORD_EFFECT_MONO, _pObj->Position, _pObj->Angle, vLight, 2, _pObj, -1, 0, 0, 0, 1.0f);

			for(int i=0; i<3; ++i)
			{
				Vector(1.0f, 0.1f, 0.0f,vLight);
				CreateEffect(MODEL_SHOCKWAVE_GROUND01, _pObj->Position, _pObj->Angle, vLight, 0, _pObj, -1, 0, 0, 0, 1.0f - (i*0.15f));
				
				Vector(1.0f, 0.3f,0.2f,vLight);
				CreateEffect(BITMAP_EVENT_CLOUD, _pObj->Position, _pObj->Angle, vLight, 0, _pObj, -1, 0, 0, 0, 1.0f);
			}
			Vector(1.0f, 0.2f,0.0f,vLight);
			CreateEffect(MODEL_WINDFOCE, _pObj->Position, _pObj->Angle, vLight, 5, _pObj, -1, 0, 0, 0, 1.0f);

			Vector(1.0f, 0.15f,0.0f,vLight);
			CreateEffect(BITMAP_MAGIC, _pObj->Position, _pObj->Angle, vLight, 13);
		}
		return true;
	case AT_SKILL_DRAGON_KICK:
		{
			if(m_bUseEffectOnce)
				return false;

			m_bUseEffectOnce = true;
			
			vec3_t vLight;
			Vector(0.7f, 0.7f, 1.0f, vLight);
			CreateEffect(MODEL_DRAGON_KICK_DUMMY, _pObj->Position, _pObj->Angle, vLight, 0, _pObj, -1, 0 ,0, 0, 1.0f);

			vec3_t Light,Position,P,dp, vAngle;
#ifdef PBG_MOD_RAGEFIGHTERSOUND
			if(_pObj->m_sTargetIndex<0)
				return true;
#endif //PBG_MOD_RAGEFIGHTERSOUND
			VectorCopy(CharactersClient[_pObj->m_sTargetIndex].Object.Position, Position);
			VectorCopy(_pObj->Angle, vAngle);
			vAngle[2] = CreateAngle(_pObj->Position[0],_pObj->Position[1],Position[0],Position[1]);
			
			float Matrix[3][4];
			Vector(0.f,20.f,0.f,P);
			AngleMatrix(vAngle,Matrix);
			VectorRotate(P,Matrix,dp);
			VectorAdd(dp,_pObj->Position,Position);
			Vector(0.8f, 0.9f, 1.6f, Light);
			CreateEffect ( MODEL_MULTI_SHOT3, Position, vAngle, Light, 0);
			CreateEffect ( MODEL_MULTI_SHOT3, Position, vAngle, Light, 0);
			
			Vector(0.f,0.f,0.f,P);
			AngleMatrix(vAngle,Matrix);
			VectorRotate(P,Matrix,dp);
			VectorAdd(dp,_pObj->Position,Position);
			
			CreateEffect ( MODEL_MULTI_SHOT1, Position, vAngle, Light, 0);
			CreateEffect ( MODEL_MULTI_SHOT1, Position, vAngle, Light, 0);
			CreateEffect ( MODEL_MULTI_SHOT1, Position, vAngle, Light, 0);
			
			Vector(0.f,20.f,0.f,P);
			AngleMatrix(vAngle,Matrix);
			VectorRotate(P,Matrix,dp);
			VectorAdd(dp,_pObj->Position,Position);
			CreateEffect ( MODEL_MULTI_SHOT2, Position, vAngle, Light, 0);
			CreateEffect ( MODEL_MULTI_SHOT2, Position, vAngle, Light, 0);

			PlayBuffer(SOUND_RAGESKILL_DRAGONKICK);
		}
		return true;
	case AT_SKILL_DRAGON_LOWER:
		{
			if(m_bUseEffectOnce)
				return false;

			m_bUseEffectOnce = true;

			vec3_t vLight;
			Vector(1.0f, 1.0f, 1.0f, vLight);
			CreateEffect(BITMAP_LAVA, _pObj->Position, _pObj->Angle, vLight, 0, _pObj, -1, 0, 0, 0, 1.0f);

			CreateEffect(MODEL_BLOW_OF_DESTRUCTION, _pObj->Position, _pObj->Angle, vLight, 2, _pObj);

			InitLower();

			PlayBuffer(SOUND_RAGESKILL_DRAGONLOWER);
		}
		break;
	case AT_SKILL_DARKSIDE:
		{
			if(m_bUseEffectOnce)
				return false;

			m_bUseEffectOnce = true;

			vec3_t vLight;
			Vector(1.0f, 0.6f, 0.6f, vLight);
			CreateEffect(BITMAP_EVENT_CLOUD, _pObj->Position, _pObj->Angle, vLight, 1, _pObj, -1, 0, 0, 0, 1.0f);

			Vector(1.0f, 0.4f, 0.0f, vLight);
			CreateEffect(BITMAP_MAGIC, _pObj->Position, _pObj->Angle, vLight, 14);

			Vector(1.0f, 0.5f, 0.2f, vLight);
			CreateEffect(MODEL_WINDFOCE, _pObj->Position, _pObj->Angle, vLight, 2, _pObj, -1, 0, 0, 0, 1.0f);

			InitDummyCal();

			PlayBuffer(SOUND_RAGESKILL_DARKSIDE);
		}
		break;
	case AT_SKILL_OCCUPY:
		{
			vec3_t Light,Position,P,dp;
			
			float Matrix[3][4];
			Vector(0.f,20.f,0.f,P);
			AngleMatrix(_pObj->Angle,Matrix);
			VectorRotate(P,Matrix,dp);
			VectorAdd(dp,_pObj->Position,Position);
			Vector(0.8f, 0.9f, 1.6f, Light);
			CreateEffect ( MODEL_MULTI_SHOT3, Position, _pObj->Angle, Light, 0);
			CreateEffect ( MODEL_MULTI_SHOT3, Position, _pObj->Angle, Light, 0);
			
			Vector(0.f,0.f,0.f,P);
			AngleMatrix(_pObj->Angle,Matrix);
			VectorRotate(P,Matrix,dp);
			VectorAdd(dp,_pObj->Position,Position);
			
			CreateEffect ( MODEL_MULTI_SHOT1, Position, _pObj->Angle, Light, 0);
			CreateEffect ( MODEL_MULTI_SHOT1, Position, _pObj->Angle, Light, 0);
			CreateEffect ( MODEL_MULTI_SHOT1, Position, _pObj->Angle, Light, 0);
			
			Vector(0.f,20.f,0.f,P);
			AngleMatrix(_pObj->Angle,Matrix);
			VectorRotate(P,Matrix,dp);
			VectorAdd(dp,_pObj->Position,Position);
			CreateEffect ( MODEL_MULTI_SHOT2, Position, _pObj->Angle, Light, 0);
			CreateEffect ( MODEL_MULTI_SHOT2, Position, _pObj->Angle, Light, 0);
		}
		break;
	default:
		break;
	}
	return false;
}

void CMonkSystem::InitLower()
{
	m_nLowerEffCnt = 0;
}

int CMonkSystem::GetLowerEffCnt()
{
	return m_nLowerEffCnt;
}

bool CMonkSystem::SetLowerEffEct()
{
	if(m_nLowerEffCnt > 3)
	{
		return false;
	}
	m_nLowerEffCnt++;

	return true;
}

CDummyUnit::CDummyUnit()
{
	Init();
}

CDummyUnit::~CDummyUnit()
{
	Destroy();
}

void CDummyUnit::Init(vec3_t Pos, vec3_t Target)
{
	Vector(0.0f, 0.0f, 0.0f, m_vPosition);
	Vector(0.0f, 0.0f, 0.0f, m_vStartPosition);
	Vector(0.0f, 0.0f, 0.0f, m_vTargetPosition);
	Vector(0.0f, 0.0f, 0.0f, m_vDirection);

	if(Pos!=NULL)
	{
		VectorCopy(Pos, m_vPosition);
		VectorCopy(m_vPosition, m_vStartPosition);
	}
	if(Target!=NULL)
	{
		VectorCopy(Target, m_vTargetPosition);
	}

	VectorSubtract(m_vTargetPosition, m_vStartPosition, m_vDirection);
	VectorNormalize(m_vDirection);

	m_fAniFrame= 0.0f;
	m_fDisFrame = 0.0f;
	m_fAlpha =0.2f;

#ifdef PBG_MOD_RAGEFIGHTERSOUND
	m_fAniFrameSpeed = 30.0f;
#endif //PBG_MOD_RAGEFIGHTERSOUND
}

void CDummyUnit::Destroy()
{
	m_fAniFrame = 0.0f;
	m_fDisFrame = 0.0f;
	m_fAlpha =0.2f;
}

bool CDummyUnit::IsDistance()
{
	vec3_t vDisPos;
	float fDis;
	VectorSubtract(m_vStartPosition, m_vPosition, vDisPos);
	fDis = VectorLength(vDisPos);
	
	if(fDis > 300)
	{
		if(m_fAlpha < 0)
		{
			m_fAlpha = 0.0f;
		}
		else
		{
			m_fAlpha -= 0.07f;
		}
	}
	else
	{
		if(m_fAlpha > 0.6f)
		{
			m_fAlpha = 0.6f;
		}
		else
		{
			m_fAlpha += 0.02f;
		}
	}

	if(fDis > MAX_DUMMYDISTANCE)
		return false;

	return true;
}

vec_t* CDummyUnit::GetPosition()
{
	return m_vPosition;
}

vec_t* CDummyUnit::GetStartPosition()
{
	return m_vStartPosition;
}

float CDummyUnit::GetAniFrame()
{
	return m_fAniFrame;
}

float CDummyUnit::GetAlpha()
{
	return m_fAlpha;
}
#ifdef PBG_MOD_RAGEFIGHTERSOUND
void CDummyUnit::CalDummyPosition(vec3_t vOutPos, float& fAni,bool bChange)
#else //PBG_MOD_RAGEFIGHTERSOUND
void CDummyUnit::CalDummyPosition(vec3_t vOutPos, float& fAni)
#endif //PBG_MOD_RAGEFIGHTERSOUND
{
	m_fDisFrame += 0.7f;
	m_fAniFrame += 0.05f;
	if(m_fAniFrame>=2.0f)
	{
		m_fAniFrame = 1.9f;
	}
	fAni = m_fAniFrame;
	float _fDisFrame = m_fDisFrame*0.7f;
	if(InChaosCastle()
#ifdef PBG_MOD_RAGEFIGHTERSOUND
		|| bChange
#endif //PBG_MOD_RAGEFIGHTERSOUND
		)
	{
#ifdef PBG_MOD_RAGEFIGHTERSOUND
		_fDisFrame*=m_fAniFrameSpeed;
		fAni *= m_fAniFrameSpeed;
#else //PBG_MOD_RAGEFIGHTERSOUND
		_fDisFrame*=30.0f;
		fAni *= 30.0f;
#endif //PBG_MOD_RAGEFIGHTERSOUND
	}
	VectorNormalize(m_vDirection);
	VectorScale(m_vDirection, _fDisFrame , m_vDirection);
	VectorAdd(m_vPosition, m_vDirection, m_vPosition);
	VectorCopy(m_vPosition, vOutPos);
}
#endif //PBG_ADD_NEWCHAR_MONK