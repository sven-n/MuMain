///////////////////////////////////////////////////////////////////////////////
// 게임에 필요한 텍스트로 저장된 여러가지 정보 데이타를 읽어들임
// 아이템, 스킬, 게이트, 욕 필터링, 몬스터, 케릭터 정보 읽기
// 공격력, 방어력, 마력 등등 계산 함수
//
// *** 함수 레벨: 2
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzInfomation.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzScene.h"
#include "ZzzInterface.h"
#include "ReadScript.h"
#include "zzzInventory.h"
#include "zzzopenglutil.h"
#include "./Utilities/Log/ErrorReport.h"
#include "zzzLodTerrain.h"

#ifdef ANTIHACKING_ENABLE
#include "proc.h"
#endif
#include "CSItemOption.h"
#ifdef PET_SYSTEM
#include "GIPetManager.h"
#endif// PET_SYSTEM

#include "GMHellas.h"
#include "CComGem.h"
#ifndef KJH_DEL_PC_ROOM_SYSTEM		// #ifndef
#ifdef ADD_PCROOM_POINT_SYSTEM
#include "PCRoomPoint.h" 
#endif	// ADD_PCROOM_POINT_SYSTEM
#endif // KJH_DEL_PC_ROOM_SYSTEM
#include "NewUIInventoryCtrl.h"
#ifdef SOCKET_SYSTEM
#include "SocketSystem.h"
#endif	// SOCKET_SYSTEM
#include "NewUISystem.h"

CLASS_ATTRIBUTE     ClassAttribute[MAX_CLASS];
MONSTER_SCRIPT      MonsterScript [MAX_MONSTER];
GATE_ATTRIBUTE      *GateAttribute = NULL;
SKILL_ATTRIBUTE     *SkillAttribute = NULL;
ITEM_ATTRIBUTE      *ItemAttribute = NULL;
CHARACTER_MACHINE   *CharacterMachine = NULL;
CHARACTER_ATTRIBUTE *CharacterAttribute = NULL;


#ifdef STATE_LIMIT_TIME
static  int     g_iWorldStateTime = 0;
static  DWORD   g_dwWorldStateBack= 0;
#endif// STATE_LIMIT_TIME


static BYTE bBuxCode[3] = {0xfc,0xcf,0xab};
static void BuxConvert(BYTE *Buffer,int Size)
{
	for(int i=0;i<Size;i++)
		Buffer[i] ^= bBuxCode[i%3];
}

extern int GetMoveReqZenFromMCB(const char * pszTargetName);

///////////////////////////////////////////////////////////////////////////////
// text
///////////////////////////////////////////////////////////////////////////////

CGlobalText GlobalText;

void SaveTextFile(char *FileName)
{
	FILE *fp = fopen(FileName,"wb");
	
	int Size = MAX_GLOBAL_TEXT_STRING;
	BYTE *Buffer = new BYTE [Size];
	for(int i=0;i<MAX_TEXTS;i++)
	{
		memcpy(Buffer,GlobalText[i],Size);
		BuxConvert(Buffer,Size);
		fwrite(Buffer,Size,1,fp);
	}
	delete [] Buffer;
	fclose(fp);
}

///////////////////////////////////////////////////////////////////////////////
// server list
///////////////////////////////////////////////////////////////////////////////

#ifndef KJH_ADD_SERVER_LIST_SYSTEM				// #ifndef
void OpenServerListScript(char *FileName)
{
	if((SMDFile=fopen(FileName,"rb")) == NULL)	return;
	SMDToken Token;

	Token = (*GetToken)();ServerNumber = (int)TokenNumber;
	for(int i=0;i<ServerNumber;i++)
	{
		Token = (*GetToken)();strcpy(ServerList[i].Name,TokenString);
		Token = (*GetToken)();ServerList[i].Number = (int)TokenNumber;
	}
	
	int NumberHi = 0;
	int NumberLow = 0;
	while(true)
	{
		SERVER_t *s = &ServerList[NumberHi].Server[NumberLow];
		Token = (*GetToken)();
		if(Token == END) break;
		if(Token == NAME && strcmp("end",TokenString)==NULL) break;
		strcpy(s->IP,TokenString);
		Token = (*GetToken)();s->Port = (int)TokenNumber;
		NumberLow++;
		if(NumberLow >= ServerList[NumberHi].Number)
		{
			NumberHi ++;
			NumberLow = 0;
		}
	}
	fclose(SMDFile);
}

void SaveServerListFile(char *FileName)
{
	FILE *fp = fopen(FileName,"wb");
	int Size = sizeof(SERVER_LIST_t);
	BYTE *Buffer = new BYTE [Size];
	memcpy(Buffer,&ServerNumber,1);
	BuxConvert(Buffer,1);
	fwrite(Buffer,1,1,fp);
	for(int i=0;i<ServerNumber;i++)
	{
		memcpy(Buffer,&ServerList[i],Size);
		BuxConvert(Buffer,Size);
		fwrite(Buffer,Size,1,fp);
	}
	delete [] Buffer;
	fclose(fp);
}
void OpenServerListFile(char *FileName)
{
	FILE *fp = fopen(FileName,"rb");
	if(fp == NULL)
	{
		char Text[256];
		sprintf(Text,"%s - File not exist.\r\n",FileName);
		g_ErrorReport.Write( Text);
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
		return;
	}
	
	int Size = sizeof(SERVER_LIST_t);
	BYTE *Buffer = new BYTE [Size];
	fread(Buffer,1,1,fp);
	BuxConvert(Buffer,1);
	//memcpy(&ServerNumber,Buffer,1);
	for(int i=0;i<ServerNumber;i++)
	{
		fread(Buffer,Size,1,fp);
		BuxConvert(Buffer,Size);
		memcpy(&ServerList[i],Buffer,Size);
	}
	delete [] Buffer;
	fclose(fp);
}
#endif // KJH_ADD_SERVER_LIST_SYSTEM



///////////////////////////////////////////////////////////////////////////////
// filter
// 현재(10.06.21) 아래 두가지 함수만 사용하고 있음
//		void OpenFilterFile(char *FileName)
//		void OpenNameFilterFile(char *FileName)
///////////////////////////////////////////////////////////////////////////////

char AbuseFilter[MAX_FILTERS][20];
char AbuseNameFilter[MAX_NAMEFILTERS][20];
int  AbuseFilterNumber = 0;
int  AbuseNameFilterNumber = 0;

void OpenFilterFileText(char *FileName)
{
	if((SMDFile=fopen(FileName,"rb")) == NULL)	return;
	SMDToken Token;
	while(true)
	{
		Token = (*GetToken)();
		if(Token == END) break;
		strcpy(AbuseFilter[AbuseFilterNumber],TokenString);
		AbuseFilterNumber++;
	}
	fclose(SMDFile);
}

void SaveFilterFile(char *FileName)
{
	FILE *fp = fopen(FileName,"wb");
	int Size = 20;
	BYTE *Buffer = new BYTE [Size*MAX_FILTERS];
	BYTE *pSeek = Buffer;
	for(int i=0;i<MAX_FILTERS;i++)
	{
		memcpy(pSeek,AbuseFilter[i],Size);
		BuxConvert(pSeek,Size);
		pSeek += Size;
	}
	DWORD dwCheckSum = GenerateCheckSum2( Buffer, Size*MAX_FILTERS, 0x3E7D);
	fwrite(Buffer,Size*MAX_FILTERS,1,fp);
	fwrite(&dwCheckSum,sizeof ( DWORD),1,fp);
	delete [] Buffer;
	fclose(fp);
}

void OpenFilterFile(char *FileName)
{
	FILE *fp = fopen(FileName,"rb");
	if(fp == NULL)
	{
		char Text[256];
    	sprintf(Text,"%s - File not exist.",FileName);
		g_ErrorReport.Write( Text);
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
		return;
	}
	/*int Size = 20;
	BYTE *Buffer = new BYTE [Size];
	for(int i=0;i<MAX_FILTERS;i++)
	{
		fread(Buffer,Size,1,fp);
		BuxConvert(Buffer,Size);
		memcpy(AbuseFilter[i],Buffer,Size);
        if(AbuseFilter[i][0] == NULL)
		{
			AbuseFilterNumber = i;
			break;
		}
	}*/

	int Size = 20;
	BYTE *Buffer = new BYTE [Size*MAX_FILTERS];
	fread(Buffer,Size*MAX_FILTERS,1,fp);
	DWORD dwCheckSum;
	fread(&dwCheckSum,sizeof ( DWORD),1,fp);
	fclose(fp);
	if ( dwCheckSum != GenerateCheckSum2( Buffer, Size*MAX_FILTERS, 0x3E7D))
	{
		char Text[256];
    	sprintf(Text,"%s - File corrupted.",FileName);
		g_ErrorReport.Write( Text);
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
	}
	else
	{
		BYTE *pSeek = Buffer;
		for(int i=0;i<MAX_FILTERS;i++)
		{
			BuxConvert(pSeek,Size);
			memcpy(AbuseFilter[i],pSeek,Size);
			if(AbuseFilter[i][0] == NULL)
			{
				AbuseFilterNumber = i;
				break;
			}
			pSeek += Size;
		}
	}

	delete [] Buffer;
	//fclose(fp);
}

void OpenNameFilterFileText(char *FileName)
{
	if((SMDFile=fopen(FileName,"rb")) == NULL)	return;
	SMDToken Token;
	while(true)
	{
		Token = (*GetToken)();
		if(Token == END) break;
		strcpy(AbuseNameFilter[AbuseNameFilterNumber],TokenString);
		AbuseNameFilterNumber++;
	}
	fclose(SMDFile);
}

void SaveNameFilterFile(char *FileName)
{
	FILE *fp = fopen(FileName,"wb");
	int Size = 20;
	BYTE *Buffer = new BYTE [Size*MAX_FILTERS];
	BYTE *pSeek = Buffer;
	for(int i=0;i<MAX_FILTERS;i++)
	{
		memcpy(pSeek,AbuseNameFilter[i],Size);
		BuxConvert(pSeek,Size);
		pSeek += Size;
	}
	DWORD dwCheckSum = GenerateCheckSum2( Buffer, Size*MAX_FILTERS, 0x2BC1);
	fwrite(Buffer,Size*MAX_FILTERS,1,fp);
	fwrite(&dwCheckSum,sizeof ( DWORD),1,fp);
	delete [] Buffer;
	fclose(fp);
}

void OpenNameFilterFile(char *FileName)
{
	FILE *fp = fopen(FileName,"rb");
	if(fp == NULL)
	{
		char Text[256];
    	sprintf(Text,"%s - File not exist.",FileName);
		g_ErrorReport.Write( Text);
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
		return;
	}
	int Size = 20;
	BYTE *Buffer = new BYTE [Size*MAX_NAMEFILTERS];
	
	fread(Buffer,Size*MAX_NAMEFILTERS,1,fp);
	
	DWORD dwCheckSum;
	fread(&dwCheckSum,sizeof ( DWORD),1,fp);
	fclose(fp);
	if ( dwCheckSum != GenerateCheckSum2( Buffer, Size*MAX_NAMEFILTERS, 0x2BC1))
	{
		char Text[256];
    	sprintf(Text,"%s - File corrupted.",FileName);
		g_ErrorReport.Write( Text);
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
	}
	else
	{
		BYTE *pSeek = Buffer;
		for(int i=0;i<MAX_NAMEFILTERS;i++)
		{
			BuxConvert(pSeek,Size);
			memcpy(AbuseNameFilter[i],pSeek,Size);
			if(AbuseNameFilter[i][0] == NULL)
			{
				AbuseNameFilterNumber = i;
				break;
			}
			pSeek += Size;
		}
	}
	delete [] Buffer;
}


///////////////////////////////////////////////////////////////////////////////
// gate
///////////////////////////////////////////////////////////////////////////////

void OpenGateScriptText(char *FileName)
{
	if((SMDFile=fopen(FileName,"rb")) == NULL)	return;
	SMDToken Token;
	while(true)
	{
		Token = (*GetToken)();
		if(Token == END) break;
		if(Token == NAME && strcmp("end",TokenString)==NULL) break;
		if(Token == NUMBER)
		{
			int Index = (int)TokenNumber;
			GATE_ATTRIBUTE *p = &GateAttribute[Index];
			
			Token = (*GetToken)();p->Flag   = (int)TokenNumber;
			Token = (*GetToken)();p->Map    = (int)TokenNumber;
			Token = (*GetToken)();p->x1     = (int)TokenNumber;
			Token = (*GetToken)();p->y1     = (int)TokenNumber;
			Token = (*GetToken)();p->x2     = (int)TokenNumber;
			Token = (*GetToken)();p->y2     = (int)TokenNumber;
			Token = (*GetToken)();p->Target = (int)TokenNumber;
			Token = (*GetToken)();p->Angle  = (int)TokenNumber;
			Token = (*GetToken)();p->Level  = (int)TokenNumber;
		}
	}
	fclose(SMDFile);
}

void SaveGateScript(char *FileName)
{
	FILE *fp = fopen(FileName,"wb");
	int Size = sizeof(GATE_ATTRIBUTE);
	BYTE *Buffer = new BYTE [Size];
	for(int i=0;i<MAX_GATES;i++)
	{
		memcpy(Buffer,&GateAttribute[i],Size);
        BuxConvert(Buffer,Size);
    	fwrite(Buffer,Size,1,fp);
	}
	delete [] Buffer;
	fclose(fp);
}

void OpenGateScript(char *FileName)
{
	FILE *fp = fopen(FileName,"rb");
	if(fp != NULL)
	{
		int Size = sizeof(GATE_ATTRIBUTE);
		BYTE *Buffer = new BYTE [Size];
		for(int i=0;i<MAX_GATES;i++)
		{
			fread(Buffer,Size,1,fp);
            BuxConvert(Buffer,Size);
			memcpy(&GateAttribute[i],Buffer,Size);
		}
		delete [] Buffer;
		fclose(fp);
	}
	else
	{
		char Text[256];
    	sprintf(Text,"%s - File not exist.",FileName);
		g_ErrorReport.Write( Text);
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
	}
}

//#ifdef ADD_MONSTER_SKILL
void OpenMonsterSkillScript(char *FileName)
{
	// -1값으로 모든값을 초기화
	memset(MonsterSkill, -1, sizeof(Script_Skill));

	FILE *fp = fopen(FileName, "rb");
	if(fp != NULL)
	{
		int Size = (sizeof(Script_Skill) + sizeof(int));
		BYTE *Buffer = new BYTE [Size];
		int FileCount = 0;
		fread(&FileCount,sizeof(int),1,fp);
		for(int i=0;i<FileCount;i++)
		{
			fread(Buffer,Size,1,fp);
            BuxConvert(Buffer,Size);
			int dummy = -1;
			int Seek = 0;
			memcpy(&dummy,Buffer + Seek,sizeof(int));
			Seek += sizeof(int);
#ifdef CSK_FIX_MONSTERSKILL
			memcpy(MonsterSkill[dummy].Skill_Num,Buffer + Seek,sizeof(int)*MAX_MONSTERSKILL_NUM);
			Seek += (sizeof(int)*MAX_MONSTERSKILL_NUM);
#else // CSK_FIX_MONSTERSKILL
			memcpy(MonsterSkill[dummy].Skill_Num,Buffer + Seek,sizeof(int)*5);
			Seek += (sizeof(int)*5);
#endif // CSK_FIX_MONSTERSKILL
			memcpy(&MonsterSkill[dummy].Slot,Buffer + Seek,sizeof(int));
		}
		delete [] Buffer;
		fclose(fp);
	}
	else
	{
		char Text[256];
    	sprintf(Text,"%s - File not exist.",FileName);
		g_ErrorReport.Write( Text);
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
	}
}
//#endif
///////////////////////////////////////////////////////////////////////////////
// skill
///////////////////////////////////////////////////////////////////////////////

void OpenNpcScript(char *FileName)
{
	if((SMDFile=fopen(FileName,"rb")) == NULL)
	{
		char Text[256];
    	sprintf(Text,"%s - File not exist.",FileName);
		g_ErrorReport.Write( Text);
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
		return;
	}
	SMDToken Token;
	while(true)
	{
		Token = (*GetToken)();
		if(Token == END) break;
		if(Token == NAME && strcmp("end",TokenString)==NULL) break;
		if(Token == NUMBER)
		{
			int Type,x,y,Dir;
			Type = (int)TokenNumber;
			Token = (*GetToken)();x   = (int)TokenNumber;
			Token = (*GetToken)();y   = (int)TokenNumber;
			Token = (*GetToken)();Dir = (int)TokenNumber;
		}
	}
	fclose(SMDFile);
}

void OpenSkillScriptText(char *FileName)
{
	if((SMDFile=fopen(FileName,"rb")) == NULL)	return;
	SMDToken Token;
	while(true)
	{
		Token = (*GetToken)();
		if(Token == END) break;
		if(Token == NAME && strcmp("end",TokenString)==NULL) break;
		if(Token == NUMBER)
		{
			int Index = (int)TokenNumber;
			SKILL_ATTRIBUTE *p = &SkillAttribute[Index];
			
			Token = (*GetToken)();strcpy(p->Name,TokenString);
			Token = (*GetToken)();p->Level    = (int)TokenNumber;
			Token = (*GetToken)();p->Damage   = (int)TokenNumber;
			Token = (*GetToken)();p->Mana     = (int)TokenNumber;
			Token = (*GetToken)();p->AbilityGuage = (int)TokenNumber;
			Token = (*GetToken)();p->Distance = (int)TokenNumber;
			Token = (*GetToken)();p->Energy = (int)TokenNumber;
			Token = (*GetToken)();
			Token = (*GetToken)();
			Token = (*GetToken)();
			Token = (*GetToken)();
		}
	}
	fclose(SMDFile);
}

void SaveSkillScript(char *FileName)
{
	FILE *fp = fopen(FileName,"wb");
	int Size = sizeof(SKILL_ATTRIBUTE);
	BYTE *Buffer = new BYTE [Size*MAX_SKILLS];
	BYTE *pSeek = Buffer;
	for(int i=0;i<MAX_SKILLS;i++)
	{
		memcpy(pSeek,&SkillAttribute[i],Size);
        BuxConvert(pSeek,Size);

		pSeek += Size;
	}
   	fwrite(Buffer,Size*MAX_SKILLS,1,fp);
	DWORD dwCheckSum = GenerateCheckSum2( Buffer, Size*MAX_SKILLS, 0x5A18);
   	fwrite(&dwCheckSum, sizeof( DWORD),1,fp);
	delete [] Buffer;
	fclose(fp);
}

void OpenSkillScript(char *FileName)
{
	FILE *fp = fopen(FileName,"rb");
	if(fp != NULL)
	{
		int Size = sizeof(SKILL_ATTRIBUTE);
		// 읽기
		BYTE *Buffer = new BYTE [Size*MAX_SKILLS];
		fread(Buffer,Size*MAX_SKILLS,1,fp);
		// crc 체크
		DWORD dwCheckSum;
		fread(&dwCheckSum,sizeof ( DWORD),1,fp);
		fclose(fp);
		if ( dwCheckSum != GenerateCheckSum2( Buffer, Size*MAX_SKILLS, 0x5A18))
		{
			char Text[256];
    		sprintf(Text,"%s - File corrupted.",FileName);
			g_ErrorReport.Write( Text);
			MessageBox(g_hWnd,Text,NULL,MB_OK);
			SendMessage(g_hWnd,WM_DESTROY,0,0);
		}
		else
		{
			BYTE *pSeek = Buffer;
			for(int i=0;i<MAX_SKILLS;i++)
			{

				BuxConvert(pSeek,Size);
				memcpy(&SkillAttribute[i],pSeek,Size);

				pSeek += Size;
			}
		}
		delete [] Buffer;
	}
	else
	{
		char Text[256];
    	sprintf(Text,"%s - File not exist.",FileName);
		g_ErrorReport.Write( Text);
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
	}
}

#ifdef LDS_FIX_APPLYSKILLTYPE_AND_CURSEDTEMPLEWRONGPARTYMEMBER	// LDS_FIX_APPLYSKILLTYPE_AND_CURSEDTEMPLEWRONGPARTYMEMBER
BOOL IsValidateSkillIdx( INT iSkillIdx )
{
	if( iSkillIdx >= MAX_SKILLS || iSkillIdx < 0 )
	{
		return FALSE;
	}
	
	return TRUE;
}

BOOL IsCorrectSkillType( INT iSkillSeq, eTypeSkill iSkillTypeIdx )
{
	if ( IsValidateSkillIdx( iSkillSeq ) == FALSE )
	{
		return FALSE;
	}
	
	SKILL_ATTRIBUTE &CurSkillAttribute = SkillAttribute[iSkillSeq];
	
	if( CurSkillAttribute.TypeSkill == (BYTE)iSkillTypeIdx )
	{
		return TRUE;
	}
	
	return FALSE;
}

BOOL IsCorrectSkillType_FrendlySkill( INT iSkillSeq )
{
	return IsCorrectSkillType( iSkillSeq, eTypeSkill_FrendlySkill );
}

BOOL IsCorrectSkillType_Buff( INT iSkillSeq )
{
	return IsCorrectSkillType( iSkillSeq, eTypeSkill_Buff );
}

BOOL IsCorrectSkillType_DeBuff( INT iSkillSeq )
{
	return IsCorrectSkillType( iSkillSeq, eTypeSkill_DeBuff );	
}

BOOL IsCorrectSkillType_CommonAttack( INT iSkillSeq )
{
	return IsCorrectSkillType( iSkillSeq, eTypeSkill_CommonAttack );
}

#endif // LDS_FIX_APPLYSKILLTYPE_AND_CURSEDTEMPLEWRONGPARTYMEMBER

///////////////////////////////////////////////////////////////////////////////
// dialog
///////////////////////////////////////////////////////////////////////////////


int g_iCurrentDialog = -1;
DIALOG_SCRIPT g_DialogScript[MAX_DIALOG];

void OpenDialogFileText(char *FileName)
{
	ZeroMemory( g_DialogScript, MAX_DIALOG * sizeof ( DIALOG_SCRIPT));

	if((SMDFile=fopen(FileName,"rb")) == NULL)	return;
	SMDToken Token;
	while(true)
	{
		Token = (*GetToken)();
		if(Token == END) break;
		if(Token == NUMBER)
		{
			int Index = (int)TokenNumber;
			Token = (*GetToken)();
			strcpy( g_DialogScript[Index].m_lpszText, TokenString);

			g_DialogScript[Index].m_iNumAnswer = 0;
			while(true)
			{
				Token = (*GetToken)();
				int Link = (int)TokenNumber;
				if(Token==NAME && strcmp("end",TokenString)==NULL) break;
				g_DialogScript[Index].m_iLinkForAnswer[g_DialogScript[Index].m_iNumAnswer] = Link;
				Token = (*GetToken)();
				int Return = (int)TokenNumber;
				g_DialogScript[Index].m_iReturnForAnswer[g_DialogScript[Index].m_iNumAnswer] = Return;
				Token = (*GetToken)();
				strcpy( g_DialogScript[Index].m_lpszAnswer[g_DialogScript[Index].m_iNumAnswer], TokenString);

				g_DialogScript[Index].m_iNumAnswer++;
			}
			g_DialogScript[Index].m_iLinkForAnswer[g_DialogScript[Index].m_iNumAnswer] = -1;
		}
	}
	fclose(SMDFile);
}

void SaveDialogFile(char *FileName)
{
	FILE *fp = fopen(FileName,"wb");
	int Size = sizeof ( DIALOG_SCRIPT);
	BYTE *Buffer = new BYTE [Size];
	for(int i=0;i<MAX_DIALOG;i++)
	{
		memcpy(Buffer,&g_DialogScript[i],Size);
		BuxConvert(Buffer,Size);
		fwrite(Buffer,Size,1,fp);
	}
	delete [] Buffer;
	fclose(fp);
}

void OpenDialogFile(char *FileName)
{
	FILE *fp = fopen(FileName,"rb");
	if(fp == NULL)
	{
		char Text[256];
    	sprintf(Text,"%s - File not exist.",FileName);
		g_ErrorReport.Write( Text);
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
		return;
	}
	int Size = sizeof ( DIALOG_SCRIPT);
	BYTE *Buffer = new BYTE [Size];
	for(int i=0;i<MAX_DIALOG;i++)
	{
		fread(Buffer,Size,1,fp);
		BuxConvert(Buffer,Size);
		memcpy(&g_DialogScript[i],Buffer,Size);
	}
	delete [] Buffer;
	fclose(fp);
}

///////////////////////////////////////////////////////////////////////////////
// item
///////////////////////////////////////////////////////////////////////////////

int ConvertItemType(BYTE *Item)
{
	int ret = 0;
	ret = Item[0]+(Item[3]&128)*2+ (Item[5]&240)*32;
	return ret;
}

void OpenItemScriptText(char *FileName)	// 0.1 ( 2003.01.08)
{
	if((SMDFile=fopen(FileName,"rb")) == NULL)	return;
	SMDToken Token;
	while(true)
	{
		Token = (*GetToken)();
		if(Token == END) break;
		if(Token == NUMBER)
		{
			int Type = (int)TokenNumber;
			while(true)
			{
				Token = (*GetToken)();
				int Index = (int)TokenNumber;
				if(Token==NAME && strcmp("end",TokenString)==NULL) break;
				int iItemIndex = Type*MAX_ITEM_INDEX+Index;
               	ITEM_ATTRIBUTE *p = &ItemAttribute[iItemIndex];
				memset(p,0,sizeof(ITEM_ATTRIBUTE));

				Token = (*GetToken)();p->Width  = (int)TokenNumber;
				Token = (*GetToken)();p->Height = (int)TokenNumber;
				Token = (*GetToken)();//시리얼
				Token = (*GetToken)();//옵션
				Token = (*GetToken)();//몬스터
				Token = (*GetToken)();strcpy(p->Name,TokenString);
				if(Type>=0 && Type<=5)
				{
					Token = (*GetToken)();p->Level	      	  = (int)TokenNumber;
					Token = (*GetToken)();p->DamageMin		  = (int)TokenNumber;
					Token = (*GetToken)();p->DamageMax		  = (int)TokenNumber;
					Token = (*GetToken)();p->WeaponSpeed	  = (int)TokenNumber;
					Token = (*GetToken)();p->Durability       = (int)TokenNumber;
					Token = (*GetToken)();p->MagicDur        = (int)TokenNumber;
					Token = (*GetToken)();p->RequireStrength  = (int)TokenNumber;
					Token = (*GetToken)();p->RequireDexterity = (int)TokenNumber;
					if(p->Width >= 2) p->TwoHand = true;
				}
				if(Type>=6 && Type<=11)
				{
      				Token = (*GetToken)();p->Level            = (int)TokenNumber;
					if(Type==6)
					{
						Token = (*GetToken)();p->Defense      = (int)TokenNumber;
						Token = (*GetToken)();p->SuccessfulBlocking = (int)TokenNumber;
					}
					else if(Type>=7 && Type<=9)
					{
						Token = (*GetToken)();p->Defense      = (int)TokenNumber;
						Token = (*GetToken)();p->MagicDefense = (int)TokenNumber;
					}
					else if(Type==10)
					{
						Token = (*GetToken)();p->Defense      = (int)TokenNumber;
						Token = (*GetToken)();p->WeaponSpeed  = (int)TokenNumber;
					}
					else if(Type==11)
					{
						Token = (*GetToken)();p->Defense      = (int)TokenNumber;
						Token = (*GetToken)();p->WalkSpeed    = (int)TokenNumber;
					}
					Token = (*GetToken)();p->Durability		  = (int)TokenNumber;
					Token = (*GetToken)();p->RequireLevel		  = (int)TokenNumber;
					Token = (*GetToken)();p->RequireStrength  = (int)TokenNumber;
					Token = (*GetToken)();p->RequireDexterity = (int)TokenNumber;
				}
				if(Type==12)
				{
      				Token = (*GetToken)();p->Level         = (int)TokenNumber;
					Token = (*GetToken)();p->Defense       = (int)TokenNumber;
					Token = (*GetToken)();p->Durability	   = (int)TokenNumber;
      				Token = (*GetToken)();p->RequireLevel  = (int)TokenNumber;
      				Token = (*GetToken)();p->RequireEnergy = (int)TokenNumber;
					Token = (*GetToken)();p->RequireStrength = (int)TokenNumber;
					Token = (*GetToken)();p->RequireDexterity = (int)TokenNumber;
				}
				if(Type==13)
				{
      				Token = (*GetToken)();p->Level          = (int)TokenNumber;
      				Token = (*GetToken)();p->Durability     = (int)TokenNumber;
					Token = (*GetToken)();p->Resistance[0]  = (int)TokenNumber;
					Token = (*GetToken)();p->Resistance[1]  = (int)TokenNumber;
					Token = (*GetToken)();p->Resistance[2]  = (int)TokenNumber;
					Token = (*GetToken)();p->Resistance[3]  = (int)TokenNumber;
					p->RequireLevel = p->Level;
				}
				if(Type==14)
				{
					Token = (*GetToken)();p->Value = (int)TokenNumber;
      				Token = (*GetToken)();p->Level = (int)TokenNumber;
				}
				if(Type==15)
				{
      				Token = (*GetToken)();p->Level         = (int)TokenNumber;
					Token = (*GetToken)();p->RequireLevel = (int)TokenNumber;
      				Token = (*GetToken)();p->RequireEnergy = (int)TokenNumber;
                    p->DamageMin = p->Level;
                    p->DamageMax = p->Level+p->Level/2;
				}
				//사용 클래스
				if(Type<=13 || Type==15)
				{
					Token = (*GetToken)();p->RequireClass[0]  = (int)TokenNumber;
					Token = (*GetToken)();p->RequireClass[1]  = (int)TokenNumber;
					Token = (*GetToken)();p->RequireClass[2]  = (int)TokenNumber;
					Token = (*GetToken)();p->RequireClass[3]  = (int)TokenNumber;
				}
			}
		}
	}
	fclose(SMDFile);
	/*for(int i=0;i<MAX_ITEM_INDEX/2;i++)
	{
		memcpy(&ItemAttribute[ITEM_ARMOR+i+MAX_ITEM_INDEX/2],&ItemAttribute[ITEM_ARMOR+i],sizeof(ITEM_ATTRIBUTE));
		memcpy(&ItemAttribute[ITEM_PANTS+i+MAX_ITEM_INDEX/2],&ItemAttribute[ITEM_PANTS+i],sizeof(ITEM_ATTRIBUTE));
		memcpy(&ItemAttribute[ITEM_GLOVE+i+MAX_ITEM_INDEX/2],&ItemAttribute[ITEM_GLOVE+i],sizeof(ITEM_ATTRIBUTE));
		memcpy(&ItemAttribute[ITEM_BOOTS +i+MAX_ITEM_INDEX/2],&ItemAttribute[ITEM_BOOTS +i],sizeof(ITEM_ATTRIBUTE));
	}*/
}

void SaveItemScript(char *FileName)
{
	FILE *fp = fopen(FileName,"wb");
	int Size = sizeof(ITEM_ATTRIBUTE);
	BYTE *Buffer = new BYTE [Size*MAX_ITEM];
	BYTE *pSeek = Buffer;
	for(int i=0;i<MAX_ITEM;i++)
	{
   		memcpy(pSeek,&ItemAttribute[i],Size);
        BuxConvert(pSeek,Size);

		pSeek += Size;
	}
   	fwrite(Buffer,Size*MAX_ITEM,1,fp);
	DWORD dwCheckSum = GenerateCheckSum2( Buffer, Size*MAX_ITEM, 0xE2F1);
   	fwrite(&dwCheckSum, sizeof( DWORD),1,fp);
	delete [] Buffer;
	fclose(fp);
}

void OpenItemScript(char *FileName)
{
	FILE *fp = fopen(FileName,"rb");
	if(fp != NULL)
	{
		int Size = sizeof(ITEM_ATTRIBUTE);
		// 읽기
		BYTE *Buffer = new BYTE [Size*MAX_ITEM];
		fread(Buffer,Size*MAX_ITEM,1,fp);
		// crc 체크
		DWORD dwCheckSum;
		fread(&dwCheckSum,sizeof ( DWORD),1,fp);
		fclose(fp);
		if ( dwCheckSum != GenerateCheckSum2( Buffer, Size*MAX_ITEM, 0xE2F1))
		{
			char Text[256];
    		sprintf(Text,"%s - File corrupted.",FileName);
			g_ErrorReport.Write( Text);
			MessageBox(g_hWnd,Text,NULL,MB_OK);
			SendMessage(g_hWnd,WM_DESTROY,0,0);
		}
		else
		{
			BYTE *pSeek = Buffer;
			for(int i=0;i<MAX_ITEM;i++)
			{
				BuxConvert(pSeek,Size);
				memcpy(&ItemAttribute[i],pSeek,Size);
				pSeek += Size;
			}
		}
		delete [] Buffer;
	}
	else
	{
		char Text[256];
    	sprintf(Text,"%s - File not exist.",FileName);
		g_ErrorReport.Write( Text);
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
	}
}

void PrintItem(char *FileName)
{
	FILE *fp = fopen(FileName,"wt");
    fprintf(fp,"                이름  최소공격력 최대공격력 방어력 방어율 필요힘 필요민첩 필요에너지\n");
	//fprintf(fp,"                이름    카오스성공확률\n");
	bool Excellent = true;
	for(int i=0;i<16*MAX_ITEM_INDEX;i++)
	{
		if ( ( i & 0x1FF) == 0)
		{
			fprintf(fp, "------------------------------------------------------------------------------------------------------\n");
		}
        ITEM_ATTRIBUTE *p = &ItemAttribute[i];
		if(p->Name[0] != NULL)
		{
			int Plus;
			if(i >= 12*MAX_ITEM_INDEX)
				Plus = 1;
			else
				Plus = 10;
			for(int j=0;j<Plus;j++)
			{
				int Level = j;
				int RequireStrength = 0;
				int RequireDexterity = 0;
				int RequireEnergy = 0;
				int DamageMin = p->DamageMin;
				int DamageMax = p->DamageMax;
				int Defense   = p->Defense;
				int SuccessfulBlocking = p->SuccessfulBlocking;
				if(DamageMin > 0)
				{
					if(Excellent)
					{
						if(p->Level)
							DamageMin += p->DamageMin*25/p->Level+5;
					}
					DamageMin += Level*3;
				}
				if(DamageMax > 0)
				{
					if(Excellent)
					{
						if(p->Level)
							DamageMax += p->DamageMin*25/p->Level+5;
					}
					DamageMax += Level*3;
				}
				if(Defense   > 0)
				{
					if(i>=ITEM_SHIELD && i<ITEM_SHIELD+MAX_ITEM_INDEX)
					{
						Defense += Level;
					}
					else
					{
       					if(Excellent)
						{
							if(p->Level)
								Defense    += p->Defense*12/p->Level+4+p->Level/5;
						}
						Defense += Level*3;
					}
				}
				if(SuccessfulBlocking > 0)
				{
     				if(Excellent)
					{
						if(p->Level)
							SuccessfulBlocking += p->SuccessfulBlocking*25/p->Level+5;
					}
					SuccessfulBlocking += Level*3;
				}
				int ItemLevel = p->Level;
     	 		if(Excellent)
              		ItemLevel = p->Level + 25;
				if(p->RequireStrength)
					RequireStrength  = 20+p->RequireStrength *(ItemLevel+Level*3)*3/100;
				else
					RequireStrength  = 0;
				if(p->RequireDexterity)
					RequireDexterity = 20+p->RequireDexterity*(ItemLevel+Level*3)*3/100;
				else
					RequireDexterity = 0;
				if(p->RequireEnergy)
				{
					RequireEnergy    = 20+p->RequireEnergy   *(ItemLevel+Level*3)*4/10;
				}
				else
				{
					RequireEnergy    = 0;
				}
				if(i>=ITEM_STAFF && i<ITEM_STAFF*MAX_ITEM_INDEX)
				{
             		DamageMin = DamageMin/2+Level*2;
             		DamageMax = 0;
				}
				ITEM ip;
				ip.Type       = i;
       			ItemConvert(&ip,j<<3,Excellent,0);

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
				ItemValue( &ip, 0);
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
				int iValue = ItemValue( &ip, 0);
				int iRate = min( iValue / 20000, 100);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
				if(j==0)
     				fprintf(fp,"%20s %8d %8d %8d %8d %8d %8d %8d %8d %8d\n",p->Name,DamageMin,DamageMax,Defense, SuccessfulBlocking,RequireStrength,RequireDexterity,RequireEnergy,p->WeaponSpeed,ItemValue(&ip));
     				//fprintf(fp,"%20s %4d%%",p->Name, iRate);
				else
     				fprintf(fp,"%17s +%d %8d %8d %8d %8d %8d %8d %8d %8d %8d\n","",Level,DamageMin,DamageMax,Defense,SuccessfulBlocking,RequireStrength,RequireDexterity,RequireEnergy,p->WeaponSpeed,ItemValue(&ip));
					//fprintf(fp,"%4d%%<+%d>",iRate,Level);
			}
			fprintf(fp, "\n");
		}
	}
	fclose(fp);
}


//////////////////////////////////////////////////////////////////////////
//	법서에 해당되는 스킬 인덱스를 리턴한다.
//////////////////////////////////////////////////////////////////////////
BYTE getSkillIndexByBook ( int Type )
{
	int SkillIndex = Type-ITEM_ETC+1;
	switch ( SkillIndex )
	{
	case 17: SkillIndex = AT_SKILL_BLAST_POISON; break;
	case 18: SkillIndex = AT_SKILL_BLAST_FREEZE; break;
	case 19: SkillIndex = AT_SKILL_BLAST_HELL;   break;
	}

	return SkillIndex;
}

#ifdef CSK_FIX_EPSOLUTESEPTER
// 셉터 아이템인가?
bool IsCepterItem(int iType)
{
	if( (iType >= ITEM_MACE+8 && iType <= ITEM_MACE+15)
		|| iType == ITEM_MACE+17
#ifdef LDK_ADD_GAMBLERS_WEAPONS
		|| iType == ITEM_MACE+18
#endif //LDK_ADD_GAMBLERS_WEAPONS
		)
	{
		return true;
	}

	return false;
}

#endif // CSK_FIX_EPSOLUTESEPTER

//////////////////////////////////////////////////////////////////////////
//	아이템 정보를 입력한다.
//////////////////////////////////////////////////////////////////////////
void ItemConvert(ITEM *ip,BYTE Attribute1,BYTE Attribute2, BYTE Attribute3 )
{
	// Attribute1 - [스킬][레벨][레벨][레벨][레벨][행운][옵션][옵션]
	// Attribute2 - [----][옵션][엑설][엑설][엑설][엑설][엑설][엑설]
	// Attribute3 - [이름][----][----][----][세트][세트][세트][세트]
	ip->Level = Attribute1;
	int     Level = (Attribute1>>3)&15;
    int     excel = Attribute2&63;
    int     excelWing = excel;
	int     excelAddValue = 0;
    bool    bExtOption = false;

    if ( ( ip->Type>=ITEM_WING+3 && ip->Type<=ITEM_WING+6 )	// 정령의 날개 ~ 암흑의 날개
		|| ( ip->Type >= ITEM_WING+36 && ip->Type <= ITEM_WING+40 )	// 3차 날개들
#ifdef ADD_ALICE_WINGS_1
		|| (ip->Type >= ITEM_WING+42 && ip->Type <= ITEM_WING+43)	// 소환술사 2,3차 날개.
#endif	// ADD_ALICE_WINGS_1
		|| ip->Type==ITEM_SWORD+19	// 대천사의 절대검
		|| ip->Type==ITEM_BOW+18	// 대천사의 절대석궁	
		|| ip->Type==ITEM_STAFF+10	// 대천사의 절대지팡이
		|| ip->Type==ITEM_MACE+13	// 대천사의 절대셉터
		|| ip->Type == ITEM_HELPER+30 // 군주의 망토
#ifdef LDK_ADD_INGAMESHOP_SMALL_WING			// 기간제 날개 작은(군망, 재날, 요날, 천날, 사날)
		|| ( ITEM_WING+130 <= ip->Type && ip->Type <= ITEM_WING+134 )
#endif //LDK_ADD_INGAMESHOP_SMALL_WING
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		|| (ip->Type >= ITEM_WING+49 && ip->Type <= ITEM_WING+50)	// 레이지파이터날개
		|| (ip->Type == ITEM_WING+135)	// 작은무인의망토
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		) 
    {
        excel = 0;
    }
	
    if ( (Attribute3%0x4)==EXT_A_SET_OPTION || (Attribute3%0x4)==EXT_B_SET_OPTION )    //  세트 아이템.
    {
        excel = 1;
        bExtOption = true;
    }

	ITEM_ATTRIBUTE *p = &ItemAttribute[ip->Type];
	ip->TwoHand            = p->TwoHand;
	ip->WeaponSpeed        = p->WeaponSpeed;
    ip->DamageMin          = p->DamageMin;
	ip->DamageMax		   = p->DamageMax;
	ip->SuccessfulBlocking = p->SuccessfulBlocking;
	ip->Defense            = p->Defense;
	ip->MagicDefense       = p->MagicDefense;
	ip->WalkSpeed          = p->WalkSpeed;
	ip->MagicPower		   = p->MagicPower;

    int setItemDropLevel = p->Level+30;

	//	액설런트 카오스 아이템의 증가 수치.
	if ( ip->Type==ITEM_MACE+6 )	// 카오스의 드라곤 도끼
	{
		excelAddValue = 15;
	}
	else if ( ip->Type==ITEM_BOW+6 )	// 카오스네이쳐보우
	{
		excelAddValue = 30;
	}
	else if ( ip->Type==ITEM_STAFF+7 )	// 카오스 번개지팡이
	{
		excelAddValue = 25;
	}

	if ( p->DamageMin>0 )
	{
        //  액설런트 아이템일 경우.
		if ( excel>0 )
		{
			if ( p->Level )
            {
                if ( excelAddValue )
					ip->DamageMin += excelAddValue;
				else
					ip->DamageMin += p->DamageMin*25/p->Level+5;
            }
		}
        if ( bExtOption )    //  세트 아이템.
        {
            ip->DamageMin += 5+(setItemDropLevel/40);
        }
        ip->DamageMin     += (min(9,Level)*3);	// ~ +9아이템
		switch(Level - 9)
		{
#ifdef LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
		case 6: ip->DamageMin += 9;	// +15 아이템
		case 5: ip->DamageMin += 8;	// +14 아이템
#endif //LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
		case 4: ip->DamageMin += 7;	// +13 아이템
		case 3: ip->DamageMin += 6;	// +12 아이템
		case 2: ip->DamageMin += 5;	// +11 아이템
		case 1: ip->DamageMin += 4;	// +10 아이템
		default: break;
		};
	}
	if ( p->DamageMax>0 )
	{
        //  액설런트 아이템일 경우.
		if ( excel>0 )
		{
			if ( p->Level )
            {
                if ( excelAddValue )
					ip->DamageMax += excelAddValue;
				else
					ip->DamageMax += p->DamageMin*25/p->Level+5;
            }
		}
        if ( bExtOption )    //  세트 아이템.
        {
            ip->DamageMax += 5+(setItemDropLevel/40);
        }
        ip->DamageMax     += (min(9,Level)*3);	// ~ +9아이템
		switch ( Level-9 )
		{
#ifdef LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
		case 6: ip->DamageMax += 9;	// +15 아이템
		case 5: ip->DamageMax += 8;	// +14 아이템
#endif //LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
		case 4: ip->DamageMax += 7;	// +13 아이템
		case 3: ip->DamageMax += 6;	// +12 아이템
		case 2: ip->DamageMax += 5;	// +11 아이템
		case 1: ip->DamageMax += 4;	// +10 아이템
		default: break;
		};
	}
	if ( p->MagicPower>0 )
	{
        //  액설런트 아이템일 경우.
		if ( excel>0 )
		{
			if ( p->Level )
            {
				if ( excelAddValue )
					ip->MagicPower += excelAddValue;
				else
					ip->MagicPower += p->MagicPower*25/p->Level+5;
            }
		}
        if ( bExtOption )    //  세트 아이템.
        {
            ip->MagicPower += 2+(setItemDropLevel/60);
        }
        ip->MagicPower += (min(9,Level)*3);	// ~ +9아이템
		switch ( Level-9 )
		{
#ifdef LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
		case 6: ip->MagicPower += 9;	// +15 아이템
		case 5: ip->MagicPower += 8;	// +14 아이템
#endif //LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
		case 4: ip->MagicPower += 7;	// +13 아이템
		case 3: ip->MagicPower += 6;	// +12 아이템
		case 2: ip->MagicPower += 5;	// +11 아이템
		case 1: ip->MagicPower += 4;	// +10 아이템
		default: break;
		};
		
		ip->MagicPower /= 2;
		
		//  셉터가 아닐 경우에만.
#ifdef CSK_FIX_EPSOLUTESEPTER
		if(IsCepterItem(ip->Type) == false)
#else // CSK_FIX_EPSOLUTESEPTER
		if ( ip->Type<ITEM_MACE+8 || ip->Type>ITEM_MACE+15 )
#endif // CSK_FIX_EPSOLUTESEPTER
		{
            ip->MagicPower += Level*2;
		}
	}

	if ( p->SuccessfulBlocking>0 )
	{
        //  액설런트 아이템일 경우.
		if(excel> 0)
		{
			if(p->Level)
				ip->SuccessfulBlocking += p->SuccessfulBlocking*25/p->Level+5;
		}
        ip->SuccessfulBlocking += (min(9,Level)*3);	// ~ +9아이템
		switch(Level - 9)
		{
#ifdef LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
		case 6: ip->SuccessfulBlocking += 9;	// +15 아이템
		case 5: ip->SuccessfulBlocking += 8;	// +14 아이템
#endif //LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
		case 4: ip->SuccessfulBlocking += 7;	// +13 아이템
		case 3: ip->SuccessfulBlocking += 6;	// +12 아이템
		case 2: ip->SuccessfulBlocking += 5;	// +11 아이템
		case 1: ip->SuccessfulBlocking += 4;	// +10 아이템
		default: break;
		};
	}
#ifdef PBG_MOD_NEWCHAR_MONK_WING_2
	if(ip->Type==ITEM_HELPER+30)
	{
		////////////////////////////////////////////////////////////////////
		// 군망옵션기획변경 (인덱스 변경 불가능)
		// 13인덱스 아이템은 defense 가 없다 군망하나로 확장하기엔 ㅎㅎ
		// 방어력 : 15 + (망토레벨 * 2) 기타 레벨옵션은 하단 소스확인 요망
		////////////////////////////////////////////////////////////////////
		p->Defense = 15;
		ip->Defense = 15;
	}
#endif //PBG_MOD_NEWCHAR_MONK_WING_2
	if ( p->Defense>0 )
	{
		if(ip->Type>=ITEM_SHIELD && ip->Type<ITEM_SHIELD+MAX_ITEM_INDEX)
		{
     		ip->Defense            += Level;
            if ( bExtOption )    //  세트 아이템.
            {
                ip->Defense = ip->Defense+(ip->Defense*20/setItemDropLevel+2);
            }
		}
		else
		{
            //  액설런트 아이템일 경우.
    		if(excel> 0)
			{
				if(p->Level)
      				ip->Defense    += p->Defense*12/p->Level+4+p->Level/5;
			}
            if ( bExtOption )    //  세트 아이템.
            {
                ip->Defense = ip->Defense+(ip->Defense*3/setItemDropLevel+2+setItemDropLevel/30);
            }

            //  천공 추가 날개.
#ifdef ADD_ALICE_WINGS_1
			if ((ip->Type>=ITEM_WING+3 && ip->Type<=ITEM_WING+6) || ip->Type == ITEM_WING+42)
#else	// ADD_ALICE_WINGS_1
            if ( ip->Type>=ITEM_WING+3 && ip->Type<=ITEM_WING+6 )
#endif	// ADD_ALICE_WINGS_1
            {
                ip->Defense     += (min(9,Level)*2);	// ~ +9아이템
            }
            else if ( ip->Type==ITEM_HELPER+30 
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
				|| ip->Type==ITEM_WING+49				// 무인의 망토
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
				)        //  군주의 망토.
            {
                ip->Defense     += ( min( 9, Level )*2 );	// ~ +9아이템
            }
#ifdef ADD_ALICE_WINGS_1
			else if ((ip->Type >= ITEM_WING+36 && ip->Type <= ITEM_WING+40) || ip->Type == ITEM_WING+43
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
				|| (ip->Type == ITEM_WING+50)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
				)	// 3차 날개들
#else	// ADD_ALICE_WINGS_1
			else if ( ip->Type >= ITEM_WING+36 && ip->Type <= ITEM_WING+40 )	// 3차 날개들
#endif	// ADD_ALICE_WINGS_1
			{
                ip->Defense     += (min(9,Level)*4);	// ~ +9아이템
			}
            else
            {
                ip->Defense     += (min(9,Level)*3);	// ~ +9아이템
            }
#ifdef ADD_ALICE_WINGS_1
			if ((ip->Type >= ITEM_WING+36 && ip->Type <= ITEM_WING+40) || ip->Type == ITEM_WING+43
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
				|| ip->Type == ITEM_WING+50	//군림의망토
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
				)	// 3차 날개들
#else	// ADD_ALICE_WINGS_1
			if ( ip->Type >= ITEM_WING+36 && ip->Type <= ITEM_WING+40 )	// 3차 날개들
#endif	// ADD_ALICE_WINGS_1
			{
				switch(Level - 9)
				{
#ifdef LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
				case 6: ip->Defense += 10;	// +15 아이템
				case 5: ip->Defense += 9;	// +14 아이템
#endif //LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
				case 4: ip->Defense += 8;	// +13 아이템
				case 3: ip->Defense += 7;	// +12 아이템
				case 2: ip->Defense += 6;	// +11 아이템
				case 1: ip->Defense += 5;	// +10 아이템
				default: break;
				};
			}
			else
			{
				switch(Level - 9)
				{
#ifdef LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
				case 6: ip->Defense += 9;	// +15 아이템
				case 5: ip->Defense += 8;	// +14 아이템
#endif //LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
				case 4: ip->Defense += 7;	// +13 아이템
				case 3: ip->Defense += 6;	// +12 아이템
				case 2: ip->Defense += 5;	// +11 아이템
				case 1: ip->Defense += 4;	// +10 아이템
				default: break;
				};
			}
		}
	}
	if ( p->MagicDefense>0 )
    {
        ip->MagicDefense += (min(9,Level)*3);	// ~ +9아이템
		switch(Level - 9)
		{
#ifdef LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
		case 6: ip->MagicDefense += 9;	// +15 아이템
		case 5: ip->MagicDefense += 8;	// +14 아이템
#endif //LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
		case 4: ip->MagicDefense += 7;	// +13 아이템
		case 3: ip->MagicDefense += 6;	// +12 아이템
		case 2: ip->MagicDefense += 5;	// +11 아이템
		case 1: ip->MagicDefense += 4;	// +10 아이템
		default: break;
		};
    }

	////////필요 능력치/////////////////////////////////////////////////////////////////////////////
	
	//  일반 아이템일 경우
	int ItemLevel = p->Level;
    //  액설런트 아이템일 경우.
    if( excel )	ItemLevel = p->Level + 25;
	//  세트 아이템일 경우
	else if( bExtOption ) ItemLevel = p->Level + 30;

	//  요구 레벨.
    int addValue = 4;
#ifdef ADD_ALICE_WINGS_1
	if ((ip->Type>=ITEM_WING+3 && ip->Type<=ITEM_WING+6) || ip->Type == ITEM_WING+42)
#else	// ADD_ALICE_WINGS_1
	if ( ip->Type>=ITEM_WING+3 && ip->Type<=ITEM_WING+6 )
#endif	// ADD_ALICE_WINGS_1
    {
        addValue = 5;
    }

	if(p->RequireLevel && ((ip->Type >= ITEM_SWORD && ip->Type < ITEM_WING)
		|| (ip->Type == ITEM_HELPER+37)	//^ 펜릴 아이템 속성 관련
#ifdef ADD_ALICE_WINGS_1
		|| (ip->Type >= ITEM_WING+7 && ip->Type <= ITEM_WING+40)
		|| (ip->Type >= ITEM_WING+43 && ip->Type < ITEM_HELPER)
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		&& (ip->Type != ITEM_WING+49)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		))
#else	// ADD_ALICE_WINGS_1
		|| (ip->Type >= ITEM_WING+7 && ip->Type <= ITEM_HELPER)))     //레벨
#endif	// ADD_ALICE_WINGS_1
		ip->RequireLevel = p->RequireLevel;
	else if (p->RequireLevel && ((ip->Type >= ITEM_WING && ip->Type <= ITEM_WING+7)
#ifdef ADD_ALICE_WINGS_1
		|| (ip->Type >= ITEM_WING+41 && ip->Type <= ITEM_WING+42)
#endif	// ADD_ALICE_WINGS_1
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		|| (ip->Type == ITEM_WING+49)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		|| ip->Type >= ITEM_HELPER))
     	ip->RequireLevel = p->RequireLevel+Level*addValue;//반지,목걸이;
	else
		ip->RequireLevel = 0;


    if(p->RequireStrength)   //힘
		ip->RequireStrength  = 20+(p->RequireStrength)*(ItemLevel+Level*3)*3/100;
	else	ip->RequireStrength  = 0;

	if(p->RequireDexterity)  //민첩
		ip->RequireDexterity = 20+(p->RequireDexterity)*(ItemLevel+Level*3)*3/100;
	else	ip->RequireDexterity = 0;

	if(p->RequireVitality)  //체력
		ip->RequireVitality = 20+(p->RequireVitality)*(ItemLevel+Level*3)*3/100;
	else	ip->RequireVitality = 0;

	if(p->RequireEnergy)  //에너지
	{
		if (ip->Type >= ITEM_STAFF+21 && ip->Type <= ITEM_STAFF+29)	// 소환술사 서는 요구에너지가 낮다
		{
			ip->RequireEnergy = 20+(p->RequireEnergy)*(ItemLevel+Level*1)*3/100;
		}
		else
	
// p->RequireLevel : 아이템 레벨 요구치
// p->RequireEnergy : 아이템 에너지 요구치
// ip->RequireEnergy : 실제 에너지 요구치 (계산된값)
#ifdef KJH_FIX_LEARN_SKILL_ITEM_REQUIRE_STAT_CALC
		// 레벨 요구치가 0이면 원래 계산대로 간다.
		// skill.txt 에서 기존 아이템 레벨요구치를 변경할 수가 없어서, 예외처리 함.
		// 요구치 계산이 코드에 왜 있어야 하는지 의문.. 고치자!!!!
		if((p->RequireLevel > 0) && (ip->Type >= ITEM_ETC && ip->Type < ITEM_ETC+MAX_ITEM_INDEX) )	// 법서
		{
			ip->RequireEnergy = 20+(p->RequireEnergy)*(p->RequireLevel)*4/100;
		}
		else
#endif // KJH_FIX_LEARN_SKILL_ITEM_REQUIRE_STAT_CALC
 
		{
			ip->RequireEnergy = 20+(p->RequireEnergy)*(ItemLevel+Level*3)*4/100;
		}
	}
	else
	{
		ip->RequireEnergy = 0;
	}

	if(p->RequireCharisma)  //통솔
		ip->RequireCharisma = 20+(p->RequireCharisma)*(ItemLevel+Level*3)*3/100;
	else	ip->RequireCharisma = 0;

    //  소환 구슬 요구 에너지값.
    if(ip->Type==ITEM_WING+11)
    {
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		WORD Energy = 0;
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
        WORD Energy;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
        switch(Level)
        {
		case 0:Energy = 30;break;
		case 1:Energy = 60;break;
		case 2:Energy = 90;break;
		case 3:Energy = 130;break;
		case 4:Energy = 170;break;
		case 5:Energy = 210;break;
		case 6:Energy = 300;break;
#ifdef ADD_ELF_SUMMON
		case 7:Energy = 500;break;
#endif // ADD_ELF_SUMMON
        }
        ip->RequireEnergy = Energy;
    }

    if ( p->RequireCharisma )
    {
        //  다크스피릿의 요구 통솔.
        if ( ip->Type==MODEL_HELPER+5 )
            ip->RequireCharisma = (185+(p->RequireCharisma*15));
        else
            ip->RequireCharisma = p->RequireCharisma;
    }

	if ( ip->Type==ITEM_HELPER+10 )//   변신반지
	{
		if ( Level<=2 )
			ip->RequireLevel = 20;
		else
			ip->RequireLevel = 50;
	}

    //  액셀런트.
	if( (ip->Type >= ITEM_HELM+29 && ip->Type <= ITEM_HELM+33) ||
		(ip->Type >= ITEM_ARMOR+29 && ip->Type <= ITEM_ARMOR+33) ||
		(ip->Type >= ITEM_PANTS+29 && ip->Type <= ITEM_PANTS+33) ||
		(ip->Type >= ITEM_GLOVES+29 && ip->Type <= ITEM_GLOVES+33) ||
		(ip->Type >= ITEM_BOOTS+29 && ip->Type <= ITEM_BOOTS+33)  ||
		ip->Type == ITEM_SWORD+22 ||
		ip->Type == ITEM_SWORD+23 ||
		ip->Type == ITEM_STAFF+12 ||
		ip->Type == ITEM_BOW+21 ||
		ip->Type == ITEM_MACE+14
#ifdef ASG_ADD_STORMBLITZ_380ITEM
		|| ITEM_STAFF+19 == ip->Type
		|| ITEM_HELM+43 == ip->Type
		|| ITEM_ARMOR+43 == ip->Type
		|| ITEM_PANTS+43 == ip->Type
		|| ITEM_GLOVES+43 == ip->Type
		|| ITEM_BOOTS+43 == ip->Type
#endif	// ASG_ADD_STORMBLITZ_380ITEM
#ifdef LEM_ADD_LUCKYITEM	// 럭키아이템 요구레벨 예외처리 excel
		|| Check_LuckyItem( ip->Type )
#endif // LEM_ADD_LUCKYITEM
		)
	{
		excel = 0;
	}

	if ( excel>0 )
	{
		if(ip->RequireLevel > 0 && ip->Type != ITEM_HELPER+37)	//^ 펜릴 아이템 속성 관련
      		ip->RequireLevel += 20;
	}

	ip->SpecialNum = 0;
    //  추가 날개.
#ifdef ADD_ALICE_WINGS_1
	if ((ip->Type >= ITEM_WING+3 && ip->Type <= ITEM_WING+6) || ip->Type == ITEM_WING+42)
#else	// ADD_ALICE_WINGS_1
    if ( ip->Type>=ITEM_WING+3 && ip->Type<=ITEM_WING+6 )
#endif	// ADD_ALICE_WINGS_1
    {
        if ( excelWing&0x01 )           //  최대 HP+50증가.
        {
			ip->SpecialValue[ip->SpecialNum] = 50+Level*5;
			ip->Special[ip->SpecialNum] = AT_IMPROVE_HP_MAX;ip->SpecialNum++;
        }
        if ( (excelWing>>1)&0x01 )      //  최대 MP+50증가.
        {
			ip->SpecialValue[ip->SpecialNum] = 50+Level*5;
			ip->Special[ip->SpecialNum] = AT_IMPROVE_MP_MAX;ip->SpecialNum++;
        }
        if ( (excelWing>>2)&0x01 )      //  3% 확률로 적에 방어력 무시 공격.     ???  데미지 HP -> MP로 손실.
        {
			ip->SpecialValue[ip->SpecialNum] = 3;
			ip->Special[ip->SpecialNum] = AT_ONE_PERCENT_DAMAGE;ip->SpecialNum++;
        }
        //  적용되지 않는 코드.
        if ( (excelWing>>3)&0x01 )      //  AG 증가.
        {
			ip->SpecialValue[ip->SpecialNum] = 50;
			ip->Special[ip->SpecialNum] = AT_IMPROVE_AG_MAX;ip->SpecialNum++;
        }
        if ( (excelWing>>4)&0x01 )      //  공격속도 증가.
        {
			ip->SpecialValue[ip->SpecialNum] = 5;
			ip->Special[ip->SpecialNum] = AT_IMPROVE_ATTACK_SPEED;ip->SpecialNum++;
        }
    }
    else if ( ip->Type==ITEM_HELPER+30 
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
			|| ip->Type==ITEM_WING+49
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
			)        //  군주의 망토.
    {
#ifndef PBG_MOD_NEWCHAR_MONK_WING_2			//정리할시에 제거할 소스
		/////////////////////////////////////////////////////////////////////
		// 망토 관련 군주의망토/무인의 망토를 다른 날개 처럼 옵션을 제거 하고
		// 아이템의 수치로 적용한다- 기획의도
		/////////////////////////////////////////////////////////////////////
        //  방어력 증가.
		int Cal = 0;
		if(Level <= 9)
			Cal = Level;
		else
			Cal = 9;
		ip->SpecialValue[ip->SpecialNum] = 15+Cal*2;
		switch(Level - 9)
		{
#ifdef LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
		case 6: ip->SpecialValue[ip->SpecialNum] += 9;	// +15 아이템
		case 5: ip->SpecialValue[ip->SpecialNum] += 8;	// +14 아이템
#endif //LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
		case 4: ip->SpecialValue[ip->SpecialNum] += 7;	// +13 아이템
		case 3: ip->SpecialValue[ip->SpecialNum] += 6;	// +12 아이템
		case 2: ip->SpecialValue[ip->SpecialNum] += 5;	// +11 아이템
		case 1: ip->SpecialValue[ip->SpecialNum] += 4;	// +10 아이템
		default: break;
		};
#ifdef PBG_MOD_NEWCHAR_MONK_WING
		if(ip->Type!=ITEM_WING+49)
		{
#endif //PBG_MOD_NEWCHAR_MONK_WING
		ip->Special[ip->SpecialNum] = AT_SET_OPTION_IMPROVE_DEFENCE; ip->SpecialNum++;
#ifdef PBG_MOD_NEWCHAR_MONK_WING
		}
#endif //PBG_MOD_NEWCHAR_MONK_WING

        //  데미지 증가.
		ip->SpecialValue[ip->SpecialNum] = 20+Level*2;
		ip->Special[ip->SpecialNum] = AT_SET_OPTION_IMPROVE_DAMAGE; ip->SpecialNum++;
#endif //PBG_MOD_NEWCHAR_MONK_WING_2			//정리할시에 제거할 소스
        if ( excelWing&0x01 )           //  최대 HP+50증가.
        {
			ip->SpecialValue[ip->SpecialNum] = 50+Level*5;
			ip->Special[ip->SpecialNum] = AT_IMPROVE_HP_MAX;ip->SpecialNum++;
        }
        
        if ( (excelWing>>1)&0x01 )      //  최대 MP+50증가.
        {
			ip->SpecialValue[ip->SpecialNum] = 50+Level*5;
			ip->Special[ip->SpecialNum] = AT_IMPROVE_MP_MAX;ip->SpecialNum++;
        }
        
        if ( (excelWing>>2)&0x01 )      //  1% 확률로 적에 방어력 무시 공격.     ???  데미지 HP -> MP로 손실.
        {
			ip->SpecialValue[ip->SpecialNum] = 3;
			ip->Special[ip->SpecialNum] = AT_ONE_PERCENT_DAMAGE;ip->SpecialNum++;
        }
        
        if ( (excelWing>>3)&0x01 
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
			&& (ip->Type != ITEM_WING+49)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
			)      //  통솔 증가.
        {
		    ip->SpecialValue[ip->SpecialNum] = 10+Level*5;
		    ip->Special[ip->SpecialNum] = AT_SET_OPTION_IMPROVE_CHARISMA; ip->SpecialNum++;
        }
    }
#ifdef ADD_ALICE_WINGS_1
	else if ((ip->Type>=ITEM_WING+36 && ip->Type<=ITEM_WING+40) || ip->Type == ITEM_WING+43
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
			|| (ip->Type == ITEM_WING+50)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		)	// 3차 날개
#else	// ADD_ALICE_WINGS_1
	else if ( ip->Type>=ITEM_WING+36 && ip->Type<=ITEM_WING+40 )	// 3차 날개
#endif	// ADD_ALICE_WINGS_1
    {
        if ( excelWing&0x01 )           //  %5 확률로 적 방어력 무시
        {
			ip->SpecialValue[ip->SpecialNum] = 5;
			ip->Special[ip->SpecialNum] = AT_ONE_PERCENT_DAMAGE;ip->SpecialNum++;
        }
        if ( (excelWing>>1)&0x01 )      //  5% 확률로 적 공격력 50% 돌려줌
        {
			ip->SpecialValue[ip->SpecialNum] = 5;
			ip->Special[ip->SpecialNum] = AT_DAMAGE_REFLECTION;ip->SpecialNum++;
        }
        if ( (excelWing>>2)&0x01 )      //  5% 확률로 유저 생명 100% 순간 회복
        {
			ip->SpecialValue[ip->SpecialNum] = 5;
			ip->Special[ip->SpecialNum] = AT_RECOVER_FULL_LIFE;ip->SpecialNum++;
        }
        if ( (excelWing>>3)&0x01 )      //  5% 확률로 유저 마력 100% 순간 회복
        {
			ip->SpecialValue[ip->SpecialNum] = 5;
			ip->Special[ip->SpecialNum] = AT_RECOVER_FULL_MANA;ip->SpecialNum++;
        }
    }
	//옵션1(스킬) 1/6
	if((Attribute1>>7)&1)
	{        
        //  스킬 연결.
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
		if ( p->m_wSkillIndex!=0 )
#else //PBG_ADD_NEWCHAR_MONK_SKILL
        if ( p->m_bySkillIndex!=0 )
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
        {
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
			ip->Special[ip->SpecialNum] = p->m_wSkillIndex; ip->SpecialNum++;
#else //PBG_ADD_NEWCHAR_MONK_SKILL
            ip->Special[ip->SpecialNum] = p->m_bySkillIndex; ip->SpecialNum++;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
        }
    }
	//옵션2(행운) 1/10
	if((Attribute1>>2)&1)
	{
		if(ip->Type>=ITEM_SWORD && ip->Type<ITEM_BOOTS+MAX_ITEM_INDEX)
		{
			if(ip->Type!=ITEM_BOW+7 && ip->Type!=ITEM_BOW+15)
			{
     			ip->Special[ip->SpecialNum] = AT_LUCK;ip->SpecialNum++;
			}
		}
#ifdef ADD_ALICE_WINGS_1
		if ((ip->Type>=ITEM_WING && ip->Type<=ITEM_WING+6) || (ip->Type>=ITEM_WING+41 && ip->Type<=ITEM_WING+42))
#else	// ADD_ALICE_WINGS_1
		if(ip->Type>=ITEM_WING && ip->Type<=ITEM_WING+6)
#endif	// ADD_ALICE_WINGS_1
		{
   			ip->Special[ip->SpecialNum] = AT_LUCK;ip->SpecialNum++;
		}
        if ( ip->Type==ITEM_HELPER+30 
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
			|| ip->Type==ITEM_WING+49
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
			) //  군주의 망토.
        {
   			ip->Special[ip->SpecialNum] = AT_LUCK;ip->SpecialNum++;
        }
#ifdef ADD_ALICE_WINGS_1
		if (( ip->Type>=ITEM_WING+36 && ip->Type<=ITEM_WING+40) || ip->Type==ITEM_WING+43
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
			|| (ip->Type==ITEM_WING+50)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
			)	// 3차 날개
#else	// ADD_ALICE_WINGS_1
		if ( ip->Type>=ITEM_WING+36 && ip->Type<=ITEM_WING+40 )	// 3차 날개
#endif	// ADD_ALICE_WINGS_1
        {
   			ip->Special[ip->SpecialNum] = AT_LUCK;ip->SpecialNum++;
        }
	}
	//옵션3(추가포인트) 1/8 1/10 1/14
	int Option3 = ((Attribute1)&3) + ((Attribute2)&64)/64*4;
	if(Option3)
	{
        //  디노란트.
        if ( ip->Type==ITEM_HELPER+3 )
        {
            if ( Option3&0x01 )         //  추가 데미지 흡수.
            {
				ip->SpecialValue[ip->SpecialNum] = 5;
				ip->Special[ip->SpecialNum] = AT_DAMAGE_ABSORB;ip->SpecialNum++;
            }
            if ( (Option3>>1)&0x01 )    //  최대 AG +50 증가.
            {
				ip->SpecialValue[ip->SpecialNum] = 50;
				ip->Special[ip->SpecialNum] = AT_IMPROVE_AG_MAX;ip->SpecialNum++;
            }
            if ( (Option3>>2)&0x01 )    //  공격 속도 5 증가.
            {
   			    ip->SpecialValue[ip->SpecialNum] = 5;
			    ip->Special[ip->SpecialNum] = AT_IMPROVE_ATTACK_SPEED;ip->SpecialNum++;
            }
        }
        else
        {
	        if(ip->Type>=ITEM_SWORD && ip->Type<ITEM_BOW+MAX_ITEM_INDEX)
		    {
			    if(ip->Type!=ITEM_BOW+7 && ip->Type!=ITEM_BOW+15)
			    {
				    ip->SpecialValue[ip->SpecialNum] = Option3*4;
				    ip->Special[ip->SpecialNum] = AT_IMPROVE_DAMAGE;ip->SpecialNum++;
				    ip->RequireStrength += Option3*5;
			    }
		    }
			if(ip->Type>=ITEM_STAFF && ip->Type<ITEM_STAFF+MAX_ITEM_INDEX)
		    {
			    ip->SpecialValue[ip->SpecialNum] = Option3*4;
				if (ip->Type>=ITEM_STAFF+21 && ip->Type<=ITEM_STAFF+29)	// 소환술사 소환수책.
					ip->Special[ip->SpecialNum] = AT_IMPROVE_CURSE;
				else
					ip->Special[ip->SpecialNum] = AT_IMPROVE_MAGIC;
				ip->SpecialNum++;
                ip->RequireStrength += Option3*5;
		    }
		    if(ip->Type>=ITEM_SHIELD && ip->Type<ITEM_SHIELD+MAX_ITEM_INDEX)
		    {
			    ip->SpecialValue[ip->SpecialNum] = Option3*5;
			    ip->Special[ip->SpecialNum] = AT_IMPROVE_BLOCKING;ip->SpecialNum++;
			    ip->RequireStrength += Option3*5;
		    }
		    if(ip->Type>=ITEM_HELM && ip->Type<ITEM_BOOTS+MAX_ITEM_INDEX)
		    {
			    ip->SpecialValue[ip->SpecialNum] = Option3*4;
			    ip->Special[ip->SpecialNum] = AT_IMPROVE_DEFENSE;ip->SpecialNum++;
			    ip->RequireStrength += Option3*5;
		    }
		    if(ip->Type>=ITEM_HELPER+8 && ip->Type<ITEM_HELPER+MAX_ITEM_INDEX && ip->Type!=ITEM_HELPER+30 )//반지,목걸이
		    {
                if ( ip->Type==ITEM_HELPER+24 )    //  마법사의 반지.
                {
                    ip->SpecialValue[ip->SpecialNum] = Option3;
			        ip->Special[ip->SpecialNum] = AT_IMPROVE_MAX_MANA;ip->SpecialNum++;
                }
                else if ( ip->Type==ITEM_HELPER+28 )    //  기술의 목걸이.
                {
                    ip->SpecialValue[ip->SpecialNum] = Option3;
			        ip->Special[ip->SpecialNum] = AT_IMPROVE_MAX_AG;ip->SpecialNum++;
                }
                else
                {
                    ip->SpecialValue[ip->SpecialNum] = Option3;
			        ip->Special[ip->SpecialNum] = AT_LIFE_REGENERATION;ip->SpecialNum++;
                }
            }
	        if(ip->Type==ITEM_WING)//요정
		    {
			    ip->SpecialValue[ip->SpecialNum] = Option3;
			    ip->Special[ip->SpecialNum] = AT_LIFE_REGENERATION;ip->SpecialNum++;
		    }
#ifdef ADD_ALICE_WINGS_1
			else if (ip->Type==ITEM_WING+1 || ip->Type==ITEM_WING+41)	//천공, 재앙의날개
#else	// ADD_ALICE_WINGS_1
			else if(ip->Type==ITEM_WING+1)//천공
#endif	// ADD_ALICE_WINGS_1
		    {
			    ip->SpecialValue[ip->SpecialNum] = Option3*4;
			    ip->Special[ip->SpecialNum] = AT_IMPROVE_MAGIC;ip->SpecialNum++;
		    }
            else if(ip->Type==ITEM_WING+2)//사탄
		    {
			    ip->SpecialValue[ip->SpecialNum] = Option3*4;
			    ip->Special[ip->SpecialNum] = AT_IMPROVE_DAMAGE;ip->SpecialNum++;
		    }
            else if ( ip->Type==ITEM_WING+3 )   //  정령의 날개.
            {
                if ( (excelWing>>5)&0x01 )    //  액설런트의 ?번째 bit가 참일때.
                {
			        ip->SpecialValue[ip->SpecialNum] = Option3;
			        ip->Special[ip->SpecialNum] = AT_LIFE_REGENERATION;ip->SpecialNum++;
                }
                else
                {
			        ip->SpecialValue[ip->SpecialNum] = Option3*4;
			        ip->Special[ip->SpecialNum] = AT_IMPROVE_DAMAGE;ip->SpecialNum++;
                }
            }
            else if ( ip->Type==ITEM_WING+4 )   //  전설의 날개.
            {
                if ( (excelWing>>5)&0x01 )    //  액설런트의 ?번째 bit가 참일때.
                {
			        ip->SpecialValue[ip->SpecialNum] = Option3*4;
			        ip->Special[ip->SpecialNum] = AT_IMPROVE_MAGIC;ip->SpecialNum++;
                }
                else
                {
			        ip->SpecialValue[ip->SpecialNum] = Option3;
			        ip->Special[ip->SpecialNum] = AT_LIFE_REGENERATION;ip->SpecialNum++;
                }
            }
            else if ( ip->Type==ITEM_WING+5 )   //  드라곤의 날개.
            {
                if ( (excelWing>>5)&0x01 )    //  액설런트의 ?번째 bit가 참일때.
                {
			        ip->SpecialValue[ip->SpecialNum] = Option3*4;
			        ip->Special[ip->SpecialNum] = AT_IMPROVE_DAMAGE;ip->SpecialNum++;
                }
                else
                {
			        ip->SpecialValue[ip->SpecialNum] = Option3;
			        ip->Special[ip->SpecialNum] = AT_LIFE_REGENERATION;ip->SpecialNum++;
                }
            }
            else if ( ip->Type==ITEM_WING+6 )   //  암흑의 날개.
            {
                if ( (excelWing>>5)&0x01 )    //  액설런트의 ?번째 bit가 참일때.
                {
			        ip->SpecialValue[ip->SpecialNum] = Option3*4;
			        ip->Special[ip->SpecialNum] = AT_IMPROVE_DAMAGE;ip->SpecialNum++;
                }
                else
                {
			        ip->SpecialValue[ip->SpecialNum] = Option3*4;
			        ip->Special[ip->SpecialNum] = AT_IMPROVE_MAGIC;ip->SpecialNum++;
                }
            }
            else if ( ip->Type==ITEM_HELPER+30 ) //  군주의 망토.
			{
				ip->SpecialValue[ip->SpecialNum] = Option3*4;
				ip->Special[ip->SpecialNum] = AT_IMPROVE_DAMAGE;ip->SpecialNum++;
			}
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
			else if(ip->Type==ITEM_WING+49)
			{
				if((excelWing>>5)&0x01)
				{
					ip->SpecialValue[ip->SpecialNum] = Option3*4;
					ip->Special[ip->SpecialNum] = AT_IMPROVE_DAMAGE;ip->SpecialNum++;
				}
				else
				{
					ip->SpecialValue[ip->SpecialNum] = Option3;
					ip->Special[ip->SpecialNum] = AT_LIFE_REGENERATION;ip->SpecialNum++;
				}
			}
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
#ifdef ADD_ALICE_WINGS_1
			else if (ip->Type == ITEM_WING+42)	// 절망의날개.
			{
				ip->SpecialValue[ip->SpecialNum] = Option3*4;
				if ((excelWing>>5)&0x01)    //  액설런트의 ?번째 bit가 참일때.
			        ip->Special[ip->SpecialNum] = AT_IMPROVE_MAGIC;
                else
			        ip->Special[ip->SpecialNum] = AT_IMPROVE_CURSE;
				ip->SpecialNum++;
			}
#endif	// ADD_ALICE_WINGS_1
			else if ( ip->Type==ITEM_WING+36 )   //  폭풍의 날개
			{
				if ( (excelWing>>4)&0x01 )    //  액설런트의 ?번째 bit가 참일때.
				{
					ip->SpecialValue[ip->SpecialNum] = Option3*4;
					ip->Special[ip->SpecialNum] = AT_IMPROVE_DAMAGE;ip->SpecialNum++;
				}
				else if ( (excelWing>>5)&0x01 )    //  액설런트의 ?번째 bit가 참일때.
				{
					ip->SpecialValue[ip->SpecialNum] = Option3*4;
					ip->Special[ip->SpecialNum] = AT_IMPROVE_DEFENSE;ip->SpecialNum++;
				}
				else
				{
			        ip->SpecialValue[ip->SpecialNum] = Option3;
			        ip->Special[ip->SpecialNum] = AT_LIFE_REGENERATION;ip->SpecialNum++;
				}
			}
			else if ( ip->Type==ITEM_WING+37 )   //  시공의 날개
			{
				if ( (excelWing>>4)&0x01 )    //  액설런트의 ?번째 bit가 참일때.
				{
					ip->SpecialValue[ip->SpecialNum] = Option3*4;
					ip->Special[ip->SpecialNum] = AT_IMPROVE_MAGIC;ip->SpecialNum++;
				}
				else if ( (excelWing>>5)&0x01 )    //  액설런트의 ?번째 bit가 참일때.
				{
					ip->SpecialValue[ip->SpecialNum] = Option3*4;
					ip->Special[ip->SpecialNum] = AT_IMPROVE_DEFENSE;ip->SpecialNum++;
				}
				else
				{
			        ip->SpecialValue[ip->SpecialNum] = Option3;
			        ip->Special[ip->SpecialNum] = AT_LIFE_REGENERATION;ip->SpecialNum++;
				}
			}
			else if ( ip->Type==ITEM_WING+38 )   //  환영의 날개
			{
				if ( (excelWing>>4)&0x01 )    //  액설런트의 ?번째 bit가 참일때.
				{
					ip->SpecialValue[ip->SpecialNum] = Option3*4;
					ip->Special[ip->SpecialNum] = AT_IMPROVE_DAMAGE;ip->SpecialNum++;
				}
				else if ( (excelWing>>5)&0x01 )    //  액설런트의 ?번째 bit가 참일때.
				{
					ip->SpecialValue[ip->SpecialNum] = Option3*4;
					ip->Special[ip->SpecialNum] = AT_IMPROVE_DEFENSE;ip->SpecialNum++;
				}
				else
				{
			        ip->SpecialValue[ip->SpecialNum] = Option3;
			        ip->Special[ip->SpecialNum] = AT_LIFE_REGENERATION;ip->SpecialNum++;
				}
			}
			else if ( ip->Type==ITEM_WING+39 )   //  파멸의 날개
			{
				if ( (excelWing>>4)&0x01 )    //  액설런트의 ?번째 bit가 참일때.
				{
					ip->SpecialValue[ip->SpecialNum] = Option3*4;
					ip->Special[ip->SpecialNum] = AT_IMPROVE_DAMAGE;ip->SpecialNum++;
				}
				else if ( (excelWing>>5)&0x01 )    //  액설런트의 ?번째 bit가 참일때.
				{
					ip->SpecialValue[ip->SpecialNum] = Option3*4;
					ip->Special[ip->SpecialNum] = AT_IMPROVE_MAGIC;ip->SpecialNum++;
				}
				else
				{
			        ip->SpecialValue[ip->SpecialNum] = Option3;
			        ip->Special[ip->SpecialNum] = AT_LIFE_REGENERATION;ip->SpecialNum++;
				}
			}
			else if ( ip->Type==ITEM_WING+40 
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
				|| (ip->Type==ITEM_WING+50)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
				) //  제왕의 망토
			{
 				if ( (excelWing>>4)&0x01 )    //  액설런트의 ?번째 bit가 참일때.
				{
					ip->SpecialValue[ip->SpecialNum] = Option3*4;
					ip->Special[ip->SpecialNum] = AT_IMPROVE_DAMAGE;ip->SpecialNum++;
				}
				else if ( (excelWing>>5)&0x01 )    //  액설런트의 ?번째 bit가 참일때.
				{
					ip->SpecialValue[ip->SpecialNum] = Option3*4;
					ip->Special[ip->SpecialNum] = AT_IMPROVE_DEFENSE;ip->SpecialNum++;
				}
				else
				{
			        ip->SpecialValue[ip->SpecialNum] = Option3;
			        ip->Special[ip->SpecialNum] = AT_LIFE_REGENERATION;ip->SpecialNum++;
				}
			}
#ifdef ADD_ALICE_WINGS_2
			else if (ip->Type == ITEM_WING+43)	// 차원의 날개
			{
 				if ( (excelWing>>4)&0x01 )    //  액설런트의 ?번째 bit가 참일때.
				{
					ip->SpecialValue[ip->SpecialNum] = Option3*4;
					ip->Special[ip->SpecialNum] = AT_IMPROVE_MAGIC;ip->SpecialNum++;
				}
				else if ( (excelWing>>5)&0x01 )    //  액설런트의 ?번째 bit가 참일때.
				{
					ip->SpecialValue[ip->SpecialNum] = Option3*4;
					ip->Special[ip->SpecialNum] = AT_IMPROVE_CURSE;ip->SpecialNum++;
				}
				else
				{
			        ip->SpecialValue[ip->SpecialNum] = Option3;
			        ip->Special[ip->SpecialNum] = AT_LIFE_REGENERATION;ip->SpecialNum++;
				}
			}
#endif	// ADD_ALICE_WINGS_2
        }
    }
    if ( ip->Type==ITEM_HELPER+4 ) //  다크호스.
    {
        //  다크호스 옵션 설정.
#ifdef KJH_FIX_DARKLOAD_PET_SYSTEM
        giPetManager::SetPetItemConvert( ip, giPetManager::GetPetInfo(ip) );
#else // KJH_FIX_DARKLOAD_PET_SYSTEM
        giPetManager::ItemConvert ( ip );
#endif // KJH_FIX_DARKLOAD_PET_SYSTEM
    }

    //  방어 관련.
	if((ip->Type>=ITEM_SHIELD && ip->Type<ITEM_BOOTS+MAX_ITEM_INDEX) ||//방어구
	    (ip->Type>=ITEM_HELPER+8 && ip->Type<=ITEM_HELPER+9)
       || (ip->Type>=ITEM_HELPER+21 && ip->Type<=ITEM_HELPER+24))       //  추가 반지.
	{
      	//생명증가
		if((Attribute2>>5)&1)
		{
			ip->Special[ip->SpecialNum] = AT_IMPROVE_LIFE;ip->SpecialNum++;
		}	
		//마나증가
		if((Attribute2>>4)&1)
		{
			ip->Special[ip->SpecialNum] = AT_IMPROVE_MANA;ip->SpecialNum++;
		}	
		//데미지감소
		if((Attribute2>>3)&1)
		{
			ip->Special[ip->SpecialNum] = AT_DECREASE_DAMAGE;ip->SpecialNum++;
		}	
		//데미지반사
		if((Attribute2>>2)&1)
		{
			ip->Special[ip->SpecialNum] = AT_REFLECTION_DAMAGE;ip->SpecialNum++;
		}	
		//방어성공율
		if((Attribute2>>1)&1)
		{
			ip->Special[ip->SpecialNum] = AT_IMPROVE_BLOCKING_PERCENT;ip->SpecialNum++;
		}	
		//획득젠증가
		if((Attribute2)&1)
		{
			ip->Special[ip->SpecialNum] = AT_IMPROVE_GAIN_GOLD;ip->SpecialNum++;
		}	
	}
    //  공격력 관련.
	if((ip->Type>=ITEM_SWORD && ip->Type<ITEM_STAFF+MAX_ITEM_INDEX) ||  //  무기.
       (ip->Type>=ITEM_HELPER+12 && ip->Type<=ITEM_HELPER+13)           //  목걸이.
       || (ip->Type>=ITEM_HELPER+25 && ip->Type<=ITEM_HELPER+28))       //  추가 목걸이
	{
      	//마나사용량감소
		if((Attribute2>>5)&1)
		{
			ip->Special[ip->SpecialNum] = AT_EXCELLENT_DAMAGE;ip->SpecialNum++;
		}	
		if ( (ip->Type>=ITEM_STAFF && ip->Type<ITEM_STAFF+MAX_ITEM_INDEX) ||//지팡이
			 (ip->Type==ITEM_HELPER+12)
           || ( ip->Type==ITEM_HELPER+25 || ip->Type==ITEM_HELPER+27 ))     //  추가 목걸이.
		{
			//마력증가(레벨)
			if((Attribute2>>4)&1)
			{
     			ip->SpecialValue[ip->SpecialNum] = CharacterAttribute->Level/20;
				ip->Special[ip->SpecialNum] = AT_IMPROVE_MAGIC_LEVEL;ip->SpecialNum++;
			}	
			//마력증가(퍼센트)
			if((Attribute2>>3)&1)
			{
		   		ip->SpecialValue[ip->SpecialNum] = 2;
				ip->Special[ip->SpecialNum] = AT_IMPROVE_MAGIC_PERCENT;ip->SpecialNum++;
			}	
		}
		else
		{
			//공격력증가(레벨)
			if((Attribute2>>4)&1)
			{
     			ip->SpecialValue[ip->SpecialNum] = CharacterAttribute->Level/20;
				ip->Special[ip->SpecialNum] = AT_IMPROVE_DAMAGE_LEVEL;ip->SpecialNum++;
			}	
			//공격력증가(퍼센트)
			if((Attribute2>>3)&1)
			{
		   		ip->SpecialValue[ip->SpecialNum] = 2;
				ip->Special[ip->SpecialNum] = AT_IMPROVE_DAMAGE_PERCENT;ip->SpecialNum++;
			}	
		}
		//공격속도
		if((Attribute2>>2)&1)
		{
   			ip->SpecialValue[ip->SpecialNum] = 7;
			ip->Special[ip->SpecialNum] = AT_IMPROVE_ATTACK_SPEED;ip->SpecialNum++;
		}	
		//획득생명증가
		if((Attribute2>>1)&1)
		{
			ip->Special[ip->SpecialNum] = AT_IMPROVE_GAIN_LIFE;ip->SpecialNum++;
		}	
		//획득마나증가
		if((Attribute2)&1)
		{
			ip->Special[ip->SpecialNum] = AT_IMPROVE_GAIN_MANA;ip->SpecialNum++;
		}	
	}
 	if (ip->Type == ITEM_HELPER+20 )
	{
        switch ( Level )
        {
        case 0:
            // 마법사의 반지 옵션
            ip->SpecialValue[ip->SpecialNum] = 10;
            ip->Special[ip->SpecialNum] = AT_IMPROVE_MAGIC_PERCENT;ip->SpecialNum++;
            ip->SpecialValue[ip->SpecialNum] = 10;
            ip->Special[ip->SpecialNum] = AT_IMPROVE_DAMAGE_PERCENT;ip->SpecialNum++;
            ip->SpecialValue[ip->SpecialNum] = 10;
            ip->Special[ip->SpecialNum] = AT_IMPROVE_ATTACK_SPEED;ip->SpecialNum++;
            break;

        case 3:
            // 마법사의 반지 옵션
            ip->SpecialValue[ip->SpecialNum] = 10;
            ip->Special[ip->SpecialNum] = AT_IMPROVE_MAGIC_PERCENT;ip->SpecialNum++;
            ip->SpecialValue[ip->SpecialNum] = 10;
            ip->Special[ip->SpecialNum] = AT_IMPROVE_DAMAGE_PERCENT;ip->SpecialNum++;
            ip->SpecialValue[ip->SpecialNum] = 10;
            ip->Special[ip->SpecialNum] = AT_IMPROVE_ATTACK_SPEED;ip->SpecialNum++;
            break;
        }
	}
#ifdef YDG_ADD_CS7_CRITICAL_MAGIC_RING
	if(ip->Type == ITEM_HELPER+107)		// 치명마법반지
	{
        ip->SpecialValue[ip->SpecialNum] = 15;
        ip->Special[ip->SpecialNum] = AT_IMPROVE_MAGIC_PERCENT;ip->SpecialNum++;
        ip->SpecialValue[ip->SpecialNum] = 15;
        ip->Special[ip->SpecialNum] = AT_IMPROVE_DAMAGE_PERCENT;ip->SpecialNum++;
        ip->SpecialValue[ip->SpecialNum] = 10;
        ip->Special[ip->SpecialNum] = AT_IMPROVE_ATTACK_SPEED;ip->SpecialNum++;
	}
#endif	// YDG_ADD_CS7_CRITICAL_MAGIC_RING

	//part
	if(ip->Type>=ITEM_BOW && ip->Type<ITEM_BOW+8 || ip->Type==ITEM_BOW+17 )
		ip->Part = EQUIPMENT_WEAPON_LEFT;
	if(ip->Type>=ITEM_STAFF+21 && ip->Type<=ITEM_STAFF+29)
		ip->Part = EQUIPMENT_WEAPON_LEFT;
	else if(ip->Type>=ITEM_SWORD && ip->Type<ITEM_STAFF+MAX_ITEM_INDEX)
		ip->Part = EQUIPMENT_WEAPON_RIGHT;
	else if(ip->Type>=ITEM_SHIELD && ip->Type<ITEM_SHIELD+MAX_ITEM_INDEX)
		ip->Part = EQUIPMENT_WEAPON_LEFT;
	else if(ip->Type>=ITEM_HELM && ip->Type<ITEM_HELM+MAX_ITEM_INDEX)
		ip->Part = EQUIPMENT_HELM;
	else if(ip->Type>=ITEM_ARMOR && ip->Type<ITEM_ARMOR+MAX_ITEM_INDEX)
		ip->Part = EQUIPMENT_ARMOR;
	else if(ip->Type>=ITEM_PANTS && ip->Type<ITEM_PANTS+MAX_ITEM_INDEX)
		ip->Part = EQUIPMENT_PANTS;
	else if(ip->Type>=ITEM_GLOVES && ip->Type<ITEM_GLOVES+MAX_ITEM_INDEX)
		ip->Part = EQUIPMENT_GLOVES;
	else if(ip->Type>=ITEM_BOOTS && ip->Type<ITEM_BOOTS+MAX_ITEM_INDEX)
		ip->Part = EQUIPMENT_BOOTS;
	else if(ip->Type>=ITEM_WING && ip->Type<ITEM_WING+7)
		ip->Part = EQUIPMENT_WING;
#ifdef ADD_ALICE_WINGS_1
	else if(ip->Type>=ITEM_WING+36 && ip->Type<=ITEM_WING+43)
#else	// ADD_ALICE_WINGS_1
	else if(ip->Type>=ITEM_WING+36 && ip->Type<=ITEM_WING+40)
#endif	// ADD_ALICE_WINGS_1
		ip->Part = EQUIPMENT_WING;
#ifdef PET_SYSTEM
	else if(ip->Type==ITEM_HELPER+5)
		ip->Part = EQUIPMENT_WEAPON_LEFT;
#endif// PET_SYSTEM
#ifdef DARK_WOLF
    else if ( ip->Type==ITEM_HELPER+6 )
		ip->Part = EQUIPMENT_WEAPON_LEFT;
#endif// DARK_WOLF
	else if(ip->Type>=ITEM_HELPER && ip->Type<ITEM_HELPER+8)
		ip->Part = EQUIPMENT_HELPER;
    else if((ip->Type>=ITEM_HELPER+8 && ip->Type<ITEM_HELPER+12) || (ip->Type == ITEM_HELPER+20 && Level == 0)
		|| (ip->Type == ITEM_HELPER+20 && Level == 3)
		)
		ip->Part = EQUIPMENT_RING_RIGHT;
    else if( ip->Type>=ITEM_HELPER+21 && ip->Type<=ITEM_HELPER+24 )
		ip->Part = EQUIPMENT_RING_RIGHT;
    else if( ip->Type>=ITEM_HELPER+25 && ip->Type<=ITEM_HELPER+28 )
		ip->Part = EQUIPMENT_AMULET;
	else if(ip->Type>=ITEM_HELPER+12 && ip->Type<ITEM_HELPER+MAX_ITEM_INDEX)
		ip->Part = EQUIPMENT_AMULET;
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
	else if(ip->Type>=ITEM_WING+49 && ip->Type<=ITEM_WING+50)
		ip->Part = EQUIPMENT_WING;
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
	else 
		ip->Part = -1;
}

//  아이템 팔기/사기 가격.
int ItemValue(ITEM *ip,int goldType)
{
	if(ip->Type == -1) return 0;

	ITEM_ATTRIBUTE *p = &ItemAttribute[ip->Type];

	__int64 Gold = 0;

    //  법서, 구슬 가격 수정.
	if ( p->iZen!=0 )
	{
		Gold = p->iZen;
		
		if( goldType )  //  팔기. 수리.
		{
			Gold = Gold/3;
		}

		if(Gold >= 1000)
			Gold = Gold/100*100;
		else if(Gold >= 100)
			Gold = Gold/10*10;

		return (int)Gold;
	}
	
	int Type = ip->Type / MAX_ITEM_INDEX;
	int Level = (ip->Level>>3)&15;
	bool Excellent = false;
	for(int i=0;i<ip->SpecialNum;i++)
	{
		switch(ip->Special[i])
		{
		case AT_IMPROVE_LIFE:          
		case AT_IMPROVE_MANA:          
		case AT_DECREASE_DAMAGE:       
		case AT_REFLECTION_DAMAGE:   
		case AT_IMPROVE_BLOCKING_PERCENT:         
		case AT_IMPROVE_GAIN_GOLD:     
		case AT_EXCELLENT_DAMAGE:   
		case AT_IMPROVE_DAMAGE_LEVEL:  
		case AT_IMPROVE_DAMAGE_PERCENT:
		case AT_IMPROVE_MAGIC_LEVEL:   
		case AT_IMPROVE_MAGIC_PERCENT: 
		case AT_IMPROVE_ATTACK_SPEED:  
		case AT_IMPROVE_GAIN_LIFE:     
		case AT_IMPROVE_GAIN_MANA:     
			Excellent = true;
			break;
		}
	}
	int Level2 = p->Level+Level*3;
	if(Excellent)
		Level2 += 25;

	if(ip->Type==ITEM_BOW+7)        //  석궁 화살.
	{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		int sellMoney = 0;
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
        int sellMoney;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

        switch ( Level )
        {
        case    0 :
            sellMoney = 100;
            break;
        case    1 :
            sellMoney = 1400;
            break;
        case    2 :
            sellMoney = 2200;
            break;
#ifdef ASG_ADD_ELF_ARROW_PLUS3
		case    3 :
            sellMoney = 3000;
            break;
#endif	// ASG_ADD_ELF_ARROW_PLUS3
        }
		if(p->Durability > 0)
     		Gold = (sellMoney*ip->Durability/p->Durability);//+(170*(Level*2));
	}
	else if(ip->Type==ITEM_BOW+15)  //  화살.
	{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		int sellMoney = 0;
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
        int sellMoney;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

        switch ( Level )
        {
        case    0 :
            sellMoney = 70;
            break;
        case    1 :
            sellMoney = 1200;
            break;
        case    2 :
            sellMoney = 2000;
            break;
#ifdef ASG_ADD_ELF_ARROW_PLUS3
		case    3 :
            sellMoney = 2800;
            break;
#endif	// ASG_ADD_ELF_ARROW_PLUS3
        }
		if(p->Durability > 0)
     		Gold = (sellMoney*ip->Durability/p->Durability);//+(80*(Level*2));
	}
	else if(COMGEM::isCompiledGem(ip))
	{
		Gold = COMGEM::CalcItemValue(ip);
	}
	else if(ip->Type==ITEM_POTION+13)//축복의 보석
	{
    	Gold = 9000000;
	}
	else if(ip->Type==ITEM_POTION+14)//영원의 보석
	{
    	Gold = 6000000;
	}
	else if(ip->Type==ITEM_WING+15)//혼돈의 보석
	{
    	Gold = 810000;
	}
    else if(ip->Type==ITEM_POTION+16)// 생명의 보석.
    {
        Gold = 45000000;
    }
    else if(ip->Type==ITEM_POTION+22)// 창조의 보석.
    {
        Gold = 36000000;
    }
#ifdef PBG_ADD_GENSRANKING
	else if(ip->Type == ITEM_POTION+141)	//빛나는보석함
	{
		Gold = 224000*3;
	}
	else if(ip->Type == ITEM_POTION+142)	//세련된보석함
	{
		Gold = 182000*3;
	}
	else if(ip->Type == ITEM_POTION+143)	//철제보석함
	{
		Gold = 157000*3;
	}
	else if(ip->Type == ITEM_POTION+144)	//낡은보석함
	{
		Gold = 121000*3;
	}
#endif //PBG_ADD_GENSRANKING

    else if(ip->Type==ITEM_HELPER+14)   //  로크의 깃털.
    {
        switch ( Level )
        {
        case 0: Gold = 180000;  break;  //  로크의 깃털.
        case 1: Gold = 7500000; break;  //  군주의 문장.
        }
    }
	else if(ip->Type==ITEM_HELPER+3)	// 디노란트
	{
        Gold = 960000;
        for(int i=0;i<ip->SpecialNum;i++)
		{
			switch(ip->Special[i])
            {
			case AT_DAMAGE_ABSORB:  
            case AT_IMPROVE_AG_MAX:
            case AT_IMPROVE_ATTACK_SPEED:
				Gold += 300000;
				break;
            }
        }
	}
    else if(ip->Type==ITEM_HELPER+15)   //  스테이트 증/감 열매.
    {
        Gold = 33000000;
    }
#ifdef KJH_FIX_WOPS_K26602_NPCSHOP_ITEM_PRICE_AS_MASTERLEVEL
    else if ( ip->Type==ITEM_HELPER+16 || ip->Type==ITEM_HELPER+17 )    //  대천사의 서, 블러드 본.
    {
        switch ( Level )
        {
        case 1: Gold = 10000; break;
        case 2: Gold = 50000; break;
        case 3: Gold = 100000; break;
        case 4: Gold = 300000; break;
        case 5: Gold = 500000; break;
        case 6: Gold = 800000; break;
		case 7: Gold = 1000000; break;
		case 8: Gold = 1200000; break;
		default:
			Gold = 0;
        }
	}
#else // KJH_FIX_WOPS_K26602_NPCSHOP_ITEM_PRICE_AS_MASTERLEVEL
    else if ( ip->Type==ITEM_HELPER+16 || ip->Type==ITEM_HELPER+17 )    //  대천사의 서, 블러드 본.
    {
        switch ( Level )
        {
        case 1: Gold = 5000; break;
        case 2: Gold = 7000; break;
        case 3: Gold = 10000; break;
        case 4: Gold = 13000; break;
        case 5: Gold = 16000; break;
        case 6: Gold = 20000; break;
		case 7: Gold = 25000; break;
		case 8: Gold = 30000; break;
		default:
			Gold = 0;
        }
		Gold *= 3;   
	}
#endif // KJH_FIX_WOPS_K26602_NPCSHOP_ITEM_PRICE_AS_MASTERLEVEL
    else if ( ip->Type==ITEM_HELPER+18 )    //  투명망토.
    {
		Gold = 200000 + 20000 * ( Level - 1);
		if ( Level == 1)
		{
			Gold = 50000;
		}
        Gold *= 3;
    }
	else if ( ip->Type==ITEM_POTION+28 )    //  읽어버린 지도.
	{
		Gold = 200000*3;
	}
    else if ( ip->Type==ITEM_POTION+29 )    //  쿤둔의 표식.
    {
        Gold = ( ip->Durability*10000 )*3;
    }
#ifdef YDG_ADD_DOPPELGANGER_ITEM
	else if ( ip->Type==ITEM_POTION+111 )    //  차원의마경
	{
		Gold = 200000*3;
	}
    else if ( ip->Type==ITEM_POTION+110 )    //  차원의 표식
    {
        Gold = ( ip->Durability*10000 )*3;
    }
#endif	// YDG_ADD_DOPPELGANGER_ITEM
#ifdef LDK_ADD_EMPIREGUARDIAN_ITEM
	else if( ITEM_POTION+102 == ip->Type )
	{
		Gold = 10000*3;
	}
	else if( ITEM_POTION+109 == ip->Type )	//가이온의명령서, 세크로미콘 
	{
		Gold = 10000*3;
	}
	else if( ITEM_POTION+101 == ip->Type || ITEM_POTION+103 <= ip->Type && ip->Type <= ITEM_POTION+108 )	//의문의쪽지, 세크로미콘 조각
	{
        Gold = ( ip->Durability*10000 )*3;
	}
#endif //LDK_ADD_EMPIREGUARDIAN_ITEM
#ifdef MYSTERY_BEAD
	else if ( ip->Type==ITEM_WING+26 )    //  신비의구슬 판매가능
	{
		if(Level == 0)
			Gold = 20000*3;
	}
#endif // MYSTERY_BEAD
    else if ( ip->Type==ITEM_HELPER+29 )    //  근위병세트 ( 카오스 캐슬 입장 아이템 ).
    {
        Gold = 5000;
    }
#ifdef LENA_EXCHANGE_ZEN
    else if(ip->Type==ITEM_POTION+21)
	{	//. 0 : 레나, 1 : 스톤, 2 : 우정의돌, 3 : 성주의표식
		if(Level == 0)
			Gold = 9000;
    }
#endif
#ifdef BLOODCASTLE_2ND_PATCH
	else if(ip->Type==ITEM_POTION+21)
	{	//. 0 : 레나, 1 : 스톤, 2 : 우정의돌, 3 : 성주의표식
		if(Level == 0)
			Gold = 9000;
		else if(Level == 1)
			Gold = 9000;
#ifdef FRIENDLYSTONE_EXCHANGE_ZEN
		else if(Level == 2)
			Gold = 3000*3;
#endif // FRIENDLYSTONE_EXCHANGE_ZEN
		else if(Level == 3)
		{
			Gold = ip->Durability * 3900;
		}
	}
#endif // BLOODCASTLE_2ND_PATCH
#ifdef KJH_FIX_WOPS_K26602_NPCSHOP_ITEM_PRICE_AS_MASTERLEVEL
	else if(ip->Type==ITEM_POTION+17)// 악마의 눈
	{
		int iValue[8] = {30000, 10000, 50000, 100000, 300000, 500000, 800000, 1000000};
		Gold = iValue[min( max( 0, Level), 7)];
	}
	else if(ip->Type==ITEM_POTION+18)// 악마의 열쇠
	{
		int iValue[8] = {30000, 15000, 75000, 150000, 450000, 750000, 1200000, 1500000};
		Gold = iValue[min( max( 0, Level), 7)];
	}
	else if(ip->Type==ITEM_POTION+19)// 데블스퀘어 초대권
	{
		int iValue[8] = {120000, 60000, 84000, 120000, 180000, 240000, 300000, 180000};
		Gold = iValue[min( max( 0, Level), 7)];
	}
#else // KJH_FIX_WOPS_K26602_NPCSHOP_ITEM_PRICE_AS_MASTERLEVEL
	else if(ip->Type==ITEM_POTION+17)// 악마의 눈
	{
		int iValue[7] = {30000, 15000, 21000, 30000, 45000, 60000, 75000};
		Gold = iValue[min( max( 0, Level), 6)];
	}
	else if(ip->Type==ITEM_POTION+18)// 악마의 열쇠
	{
		int iValue[7] = {30000, 15000, 21000, 30000, 45000, 60000, 75000};
		Gold = iValue[min( max( 0, Level), 6)];
	}
	else if(ip->Type==ITEM_POTION+19)// 데블스퀘어 초대권
	{
		int iValue[5] = { 120000, 60000, 84000, 120000, 180000};
		Gold = iValue[min( max( 0, Level), 4)];
	}
#endif // KJH_FIX_WOPS_K26602_NPCSHOP_ITEM_PRICE_AS_MASTERLEVEL
#ifdef KJH_FIX_20080910_NPCSHOP_PRICE
    else if (ip->Type==ITEM_HELPER+49 || ip->Type==ITEM_HELPER+50 || ip->Type==ITEM_HELPER+51)	// 낡은두루마리, 환영교단의서약, 피의두루마리
    {
        switch ( Level )
        {
        case 1: Gold = 500000; break;
        case 2: Gold = 600000; break;
        case 3: Gold = 800000; break;
        case 4: Gold = 1000000; break;
        case 5: Gold = 1200000; break;
        case 6: Gold = 1400000; break;
		default:
			Gold = 3000*3;
        }
	}
#endif // KJH_FIX_20080910_NPCSHOP_PRICE
	else if(ip->Type==ITEM_POTION+23 || ip->Type==ITEM_POTION+24 || ip->Type==ITEM_POTION+25 || ip->Type==ITEM_POTION+26
		|| ip->Type==ITEM_POTION+65 || ip->Type==ITEM_POTION+66 || ip->Type==ITEM_POTION+67 || ip->Type==ITEM_POTION+68
		)	// 전직 퀘스트 아이템
	{
		Gold = 9000;
	}
	else if(ip->Type==ITEM_POTION+9 && Level == 1)// 사랑의 올리브
	{
		Gold = 1000;
	}
	else if(ip->Type==ITEM_POTION+20)	// 사랑의 묘약
	{
		Gold = 900;
	}
	else if(ip->Type == ITEM_POTION+51)	// 크리스마스의 별
	{
		Gold = 200000;
	}
	else if(ip->Type == ITEM_POTION+63)	// 폭죽
	{
		Gold = 200000;
	}
	else if(ip->Type == ITEM_POTION+52)	// GM 선물상자
	{
		Gold = 33*3;
	}
	else if(ip->Type==ITEM_POTION+10)	// 이동문서
	{
#ifdef _PVP_ADD_MOVE_SCROLL
		switch (Level)
		{
		case 0: Gold = 750; break;
		case 1: Gold = GetMoveReqZenFromMCB("Lorencia") * 0.9f; break;		//  로랜시아
		case 2: Gold = GetMoveReqZenFromMCB("Noria") * 0.9f; break;		//  노리아
		case 3: Gold = GetMoveReqZenFromMCB("Devias") * 0.9f; break;		//  데비아스
		case 4: Gold = GetMoveReqZenFromMCB("Dungeon") * 0.9f; break;		//  던전
		case 5: Gold = GetMoveReqZenFromMCB("Atlans") * 0.9f; break;		//  아틀란스
		case 6: Gold = GetMoveReqZenFromMCB("LostTower") * 0.9f; break;	//  로스트타워
		case 7: Gold = GetMoveReqZenFromMCB("Tarkan") * 0.9f; break;		//  타르칸
		case 8: Gold = GetMoveReqZenFromMCB("Icarus") * 0.9f; break;		//  이카루스
		default: Gold = 750; break;
		};
#else	// _PVP_ADD_MOVE_SCROLL
		Gold = 750;
#endif	// _PVP_ADD_MOVE_SCROLL
	}
#ifdef _PVP_MURDERER_HERO_ITEM
	else if(ip->Type==ITEM_POTION+30)	// 영웅살인마징표
	{
		Gold = 3000;
	}
#endif	// _PVP_MURDERER_HERO_ITEM
    else if ( ip->Type==ITEM_POTION+31 )// 수호의보석
    {
        Gold = 60000000;
    }
    else if ( ip->Type==ITEM_POTION+7 ) //  스패셜 물약.//종훈물약
    {
        switch ( Level )
        {
			//2100000
			//1500000
        case 0: Gold = 900000*ip->Durability; break;
        case 1: Gold = 450000*ip->Durability; break;
        }
    }
    else if ( ip->Type==ITEM_HELPER+7 ) //  용병.
    {
        switch ( Level )
        {
		case 0: Gold =  1500000; break;
        case 1: Gold =  1200000; break;		        
        }
    }
    else if ( ip->Type==ITEM_HELPER+11 )
    {
        switch ( Level )
        {
        case 0: Gold = 100000; break;
        case 1: Gold = 2400000; break;
        }
    }
	else if(ip->Type >= ITEM_POTION+35 && ip->Type <= ITEM_POTION+37)
	{
		switch(ip->Type)
		{
			case ITEM_POTION+35:
				Gold = 2000;
				break;
			case ITEM_POTION+36:
				Gold = 4000;
				break;
			case ITEM_POTION+37:
				Gold = 6000;
				break;
		}
		Gold *= ip->Durability;
	}
	else if(ip->Type >= ITEM_POTION+38 && ip->Type <= ITEM_POTION+40)
	{
		switch(ip->Type)
		{
			case ITEM_POTION+38:
				Gold = 2500;
				break;
			case ITEM_POTION+39:
				Gold = 5000;
				break;
			case ITEM_POTION+40:
				Gold = 7500;
				break;
		}
		Gold *= ip->Durability;
	}
#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
	else if( ip->Type == ITEM_POTION+100 )
	{
		Gold = 100*3;
		Gold *= ip->Durability;
	}
#endif // KJH_PBG_ADD_SEVEN_EVENT_2008
#ifdef PBG_FIX_GOLD_CHERRYBLOSSOMTREE
	else if( ip->Type == ITEM_POTION+90  || ip->Type == ITEM_POTION+85 || ip->Type == ITEM_POTION+86 || ip->Type == ITEM_POTION+87)
	{
		Gold = 300;
		Gold *= ip->Durability;
	}
#endif //PBG_FIX_GOLD_CHERRYBLOSSOMTREE

	else if(p->Value > 0)
	{
		Gold = p->Value*p->Value*10/12;

#ifdef CSK_FIX_POTION_VALUE
		if(ip->Type == ITEM_POTION+3 || ip->Type == ITEM_POTION+6)	
		{
			// 큰치료물약, 큰마나물약 가격 1500으로 하드코딩으로 바꿔달라고 함
			// 스크립트로 value 값을 바꾸면 되지만 1500 값이 나올려면 소수점이 들어가서 하드코딩으로 감
			Gold = 1500;	
		}
#endif // CSK_FIX_POTION_VALUE

		if(ip->Type>=ITEM_POTION && ip->Type<=ITEM_POTION+8)	//물약
		{
			if(Level > 0)
#ifdef KJW_FIX_CHINESE_POTION_PRICE
				Gold *= (__int64)pow((double)2,(double)Level);
#else // KJW_FIX_CHINESE_POTION_PRICE
	#ifdef CHINESE_PRICERISE
				Gold *= (__int64)pow((double)3,(double)Level);
	#else
				Gold *= (__int64)pow((double)2,(double)Level);
	#endif	// CHINESE_PRICERISE
#endif // KJW_FIX_CHINESE_POTION_PRICE

#ifdef KJH_FIX_POTION_PRICE
			Gold = Gold/10*10;
			Gold *= (__int64)ip->Durability;

			if( goldType )  //  팔기. 수리.
            {
				Gold = Gold/3;
				Gold = Gold/10*10;
            }
			return (int)Gold;
#else // KJH_FIX_POTION_PRICE
			Gold *= ip->Durability;
            if( goldType )  //  팔기. 수리.
            {
		        Gold = Gold/3;
            }
            Gold = Gold/10*10;
			return (int)Gold;
#endif // KJH_FIX_POTION_PRICE
		}
	}
	else if (ip->Type==ITEM_HELPER+20)	// 마법사의 반지
	{
		if (Level == 0)
#ifdef LDK_MOD_INGAMESHOP_ITEM_CHANGE_VALUE
			Gold = 3000;
#else //LDK_MOD_INGAMESHOP_ITEM_CHANGE_VALUE
			Gold = 30000;
#endif //LDK_MOD_INGAMESHOP_ITEM_CHANGE_VALUE
		else if (Level == 1)
			Gold = 0;
	}
	else if (ip->Type==ITEM_HELPER+52 || ip->Type==ITEM_HELPER+53)	// 콘도르의 깃털,불꽃
	{
		Gold = 3000000;
	}
    else if ( ip->Type==ITEM_HELPER+31 )    //  영혼.
    {
        switch ( Level )
        {
        case 0 : Gold = 10000000*3; break;    //  다크호스.
        case 1 : Gold = 5000000*3;  break;    //  다크스피릿.
        }
    }
	else if( ( ( Type==12 && (ip->Type>ITEM_WING+6
#ifdef ADD_ALICE_WINGS_1
		&& !(ip->Type>=ITEM_WING+36 && ip->Type<=ITEM_WING+43)
#else	// ADD_ALICE_WINGS_1
		&& !( ip->Type>=ITEM_WING+36 && ip->Type<=ITEM_WING+40)
#endif	// ADD_ALICE_WINGS_1
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		&& (ip->Type!=ITEM_WING+50)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		) ) || Type==13 || Type==15 ) )//12(날개,구슬) 13(사탄,반지) 15(법서), 디노란트 제외.
	{
		Gold = 100+Level2*Level2*Level2;
#ifdef _VS2008PORTING
		for(int i=0;i<ip->SpecialNum;i++)
#else // _VS2008PORTING
		for(i=0;i<ip->SpecialNum;i++)
#endif // _VS2008PORTING
		{
			switch(ip->Special[i])
			{
			case AT_LIFE_REGENERATION:
				Gold += Gold*ip->SpecialValue[i];
				break;
			}
		}
	}
#ifdef YDG_FIX_LUCKY_CHARM_VALUE_TO_0
	else if (ip->Type == ITEM_POTION+53)	// 행운의부적
	{
		Gold = 0;
	}
#endif	// YDG_FIX_LUCKY_CHARM_VALUE_TO_0
	else
	{
        switch(Level)     //+무기일수록 가중치 붙음
        {
        case 5:Level2 += 4;break;
        case 6:Level2 += 10;break;
        case 7:Level2 += 25;break;
        case 8:Level2 += 45;break;
        case 9:Level2 += 65;break;
        case 10:Level2 += 95;break;	// +10
        case 11:Level2 += 135;break;
        case 12:Level2 += 185;break;
        case 13:Level2 += 245;break;
#ifdef LDK_ADD_14_15_GRADE_ITEM_VALUE
		// 아이템 14 15 판매금액 추가
        case 14:Level2 += 305;break;
        case 15:Level2 += 365;break; // 한계 가중치 더이상 못올림
#endif //LDK_ADD_14_15_GRADE_ITEM_VALUE
        }
        //  날개, 망토.
	    if( ( Type==12 && ip->Type<=ITEM_WING+6) || ip->Type==ITEM_HELPER+30 
#ifdef ADD_ALICE_WINGS_1
			|| (ip->Type>=ITEM_WING+36 && ip->Type<=ITEM_WING+43)	// 3차날개, 소환술사 날개.
#else	// ADD_ALICE_WINGS_1
			|| ( ip->Type>=ITEM_WING+36 && ip->Type<=ITEM_WING+40 )	// 3차날개
#endif	// ADD_ALICE_WINGS_1
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
			|| (ip->Type==ITEM_WING+50)	// 레이지 파이터 날개
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
			)
        {
            Gold = 40000000+((40+Level2)*Level2*Level2*11);
        }
        else
        {
		    Gold = 100+(40+Level2)*Level2*Level2/8;
        }
		if(Type>=0 && Type<=6) //무기일때
		{
			if(!p->TwoHand) //한손무기일때 80%
				Gold = Gold*80/100;
		}
		for(int i=0;i<ip->SpecialNum;i++)
		{
			switch(ip->Special[i])
			{
			case AT_SKILL_BLOCKING:
			case AT_SKILL_SWORD1:
			case AT_SKILL_SWORD2:
			case AT_SKILL_SWORD3:
			case AT_SKILL_SWORD4:
			case AT_SKILL_SWORD5:
			case AT_SKILL_MANY_ARROW_UP:
			case AT_SKILL_MANY_ARROW_UP+1:
			case AT_SKILL_MANY_ARROW_UP+2:
			case AT_SKILL_MANY_ARROW_UP+3:
			case AT_SKILL_MANY_ARROW_UP+4:
			case AT_SKILL_CROSSBOW:
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
			case AT_SKILL_RECOVER:
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
			case AT_SKILL_GAOTIC:
#endif //PJH_SEASON4_DARK_NEW_SKILL_CAOTIC			case AT_SKILL_BLAST_POISON:
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
			case AT_SKILL_MULTI_SHOT:
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
#ifdef PJH_SEASON4_MASTER_RANK4
			case AT_SKILL_POWER_SLASH_UP:
			case AT_SKILL_POWER_SLASH_UP+1:
			case AT_SKILL_POWER_SLASH_UP+2:
			case AT_SKILL_POWER_SLASH_UP+3:
			case AT_SKILL_POWER_SLASH_UP+4:
#endif //PJH_SEASON4_MASTER_RANK4
			case AT_SKILL_ICE_BLADE:
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
			case AT_SKILL_THRUST:
			case AT_SKILL_STAMP:
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
				Gold += ( DWORD)( ( float)Gold*1.5f);
				break;
			case AT_IMPROVE_DAMAGE:
			case AT_IMPROVE_MAGIC:
			case AT_IMPROVE_CURSE:
			case AT_IMPROVE_DEFENSE:
			case AT_LIFE_REGENERATION:
                if ( ( Type==12 && ip->Type<=ITEM_WING+6 )
#ifdef ADD_ALICE_WINGS_1
					|| (ip->Type>=ITEM_WING+36 && ip->Type<=ITEM_WING+43)	// 3차날개, 소환술사 날개.
#else	// ADD_ALICE_WINGS_1
					|| ( ip->Type>=ITEM_WING+36 && ip->Type<=ITEM_WING+40 )	// 3차날개
#endif	// ADD_ALICE_WINGS_1
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
					|| (ip->Type>=ITEM_WING+49 && ip->Type<=ITEM_WING+50)	// 레이지파이터날개
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
					)    //  날개.
                {
					int iOption = ip->SpecialValue[i];
					if ( AT_LIFE_REGENERATION == ip->Special[i])
					{
						iOption *= 4;
					}
				    switch(iOption)
				    {
				    case 4 :Gold += ( __int64)( ( double)Gold* 6/10);break;
				    case 8 :Gold += ( __int64)( ( double)Gold*14/10);break;
				    case 12:Gold += ( __int64)( ( double)Gold*28/10);break;
				    case 16:Gold += ( __int64)( ( double)Gold*56/10);break;
				    }
                }
                else
                {
				    switch(ip->SpecialValue[i])
				    {
				    case 4 :Gold += ( __int64)( ( double)Gold* 6/10);break;
				    case 8 :Gold += ( __int64)( ( double)Gold*14/10);break;
				    case 12:Gold += ( __int64)( ( double)Gold*28/10);break;
				    case 16:Gold += ( __int64)( ( double)Gold*56/10);break;
				    }
                }
				break;
			case AT_IMPROVE_BLOCKING:
				switch(ip->SpecialValue[i])
				{
				case 5 :Gold += ( __int64)( ( double)Gold* 6/10);break;
				case 10:Gold += ( __int64)( ( double)Gold*14/10);break;
				case 15:Gold += ( __int64)( ( double)Gold*28/10);break;
				case 20:Gold += ( __int64)( ( double)Gold*56/10);break;
				}
				break;
			case AT_LUCK:
				Gold += ( __int64)( ( double)Gold*25/100);
				break;
			case AT_IMPROVE_LIFE:          
			case AT_IMPROVE_MANA:          
			case AT_DECREASE_DAMAGE:       
			case AT_REFLECTION_DAMAGE:     
			case AT_IMPROVE_BLOCKING_PERCENT:         
			case AT_IMPROVE_GAIN_GOLD:     
			case AT_EXCELLENT_DAMAGE:   
			case AT_IMPROVE_DAMAGE_LEVEL:  
			case AT_IMPROVE_DAMAGE_PERCENT:
			case AT_IMPROVE_MAGIC_LEVEL:   
			case AT_IMPROVE_MAGIC_PERCENT: 
			case AT_IMPROVE_GAIN_LIFE:     
			case AT_IMPROVE_GAIN_MANA:     
			case AT_IMPROVE_ATTACK_SPEED:  
				Gold += Gold;
				break;

                //  디노란트.
            case AT_IMPROVE_EVADE:
				Gold += Gold;
				break;

                //  추가 날개.
            case AT_IMPROVE_HP_MAX :
            case AT_IMPROVE_MP_MAX :
            case AT_ONE_PERCENT_DAMAGE :  
            case AT_IMPROVE_AG_MAX :
            case AT_DAMAGE_ABSORB : 
			case AT_DAMAGE_REFLECTION:	// [대상의 공격이 유저에게 명중한 경우] 5% 확률로 적 공격력 50% 돌려줌
			case AT_RECOVER_FULL_LIFE:	// [대상의 공격이 유저에게 명중한 경우] 5% 확률로 유저 생명 100% 순간 회복
			case AT_RECOVER_FULL_MANA:	// [대상의 공격이 유저에게 명중한 경우] 5% 확률로 유저 마나 100% 순간 회복
				Gold += ( __int64)( ( double)Gold*25/100);
                break;
			}
		}
#ifdef SOCKET_SYSTEM
		// 소켓 아이템 가격 보너스
		Gold += g_SocketItemMgr.CalcSocketBonusItemValue(ip, Gold);
#endif	// SOCKET_SYSTEM
	}
	Gold = min(Gold,3000000000);

#ifdef YDG_FIX_REPAIR_COST_ADJUST_TO_SERVER_SETTING
	if (goldType == 2)	// 서버 공식과 맞게 수리시에는 끝자리 버린후 계산
	{
		if(Gold >= 1000)
		{
			Gold = (Gold / 100) * 100;
		}
		else if(Gold >= 100)
		{
			Gold = (Gold / 10) * 10;
		}
	}
#endif	// YDG_FIX_REPAIR_COST_ADJUST_TO_SERVER_SETTING

	if( goldType )  //  팔기. 수리.
	{
		Gold = Gold/3;
	}

	//^ 펜릴 아이템 상점 판매 가격(/3 안하기 위해서 이곳에 위치)
	if(ip->Type == ITEM_HELPER+32)	// 갑옷 파편(겹쳐진 갯수*가격)
		Gold = ip->Durability*50;
	else if(ip->Type == ITEM_HELPER+33)	// 여신의 가호
		Gold = ip->Durability*100;
	else if(ip->Type == ITEM_HELPER+34)	// 맹수의 발톱
		Gold = ip->Durability*1000;
	else if(ip->Type == ITEM_HELPER+35)	// 뿔피리 조각
		Gold = ip->Durability*10000;
	else if(ip->Type == ITEM_HELPER+36)	// 부러진 뿔피리
		Gold = 30000;
	else if(ip->Type == ITEM_HELPER+37)	// 펜릴의 뿔피리
		Gold = 50000;
#ifndef KJH_FIX_20080910_NPCSHOP_PRICE		// #ifndef		정리할 때 지워야 하는 소스
#ifdef KJH_FIX_WOPS_K26602_NPCSHOP_ITEM_PRICE_AS_MASTERLEVEL
    else if (ip->Type==ITEM_HELPER+49 || ip->Type==ITEM_HELPER+50 || ip->Type==ITEM_HELPER+51)	// 낡은두루마리, 환영교단의서약, 피의두루마리
    {
        switch ( Level )
        {
        case 1: Gold = 500000; break;
        case 2: Gold = 600000; break;
        case 3: Gold = 800000; break;
        case 4: Gold = 1000000; break;
        case 5: Gold = 1200000; break;
        case 6: Gold = 1400000; break;
		default:
			Gold = 3000*3;
        }
	}
#else // KJH_FIX_WOPS_K26602_NPCSHOP_ITEM_PRICE_AS_MASTERLEVEL
	else if (ip->Type==ITEM_HELPER+49 || ip->Type==ITEM_HELPER+50 || ip->Type==ITEM_HELPER+51)	// 낡은두루마리,환영교단의서약,피의두루마리
	{
		Gold = 3000;
	}
#endif // KJH_FIX_WOPS_K26602_NPCSHOP_ITEM_PRICE_AS_MASTERLEVEL
#endif // KJH_FIX_20080910_NPCSHOP_PRICE		// #ifndef		정리할 때 지워야 하는 소스

#ifndef KJH_DEL_PC_ROOM_SYSTEM		// #ifndef
#ifdef ADD_PCROOM_POINT_SYSTEM
	// PC방 포인트 상점인가?
	if (CPCRoomPtSys::Instance().IsPCRoomPointShopMode())
	{
		switch(ip->Type)
		{
#ifdef ADD_EVENT_PCROOM_POINT_HALF
			
	#ifdef CSK_LUCKY_SEAL	// 해외와 틀림.
		case ITEM_HELPER+43:
			Gold = 12;
			break;
		case ITEM_HELPER+44:
			Gold = 18;
			break;
		case ITEM_HELPER+45:
			Gold = 12;
			break;
	#endif	//  CSK_LUCKY_SEAL
	#ifdef CSK_PCROOM_ITEM
		case ITEM_POTION+55:
			Gold = 12;
			break;
		case ITEM_POTION+56:
			Gold = 18;
			break;
		case ITEM_POTION+57:
			Gold = 24;
			break;
	#endif // CSK_PCROOM_ITEM
	#ifdef LDS_ADD_PCROOM_ITEM_JPN_6TH
		case ITEM_HELPER+96:		// 강함의 인장
			Gold = 16;
			break;
	#endif // LDS_ADD_PCROOM_ITEM_JPN_6TH
			
#else	// ADD_EVENT_PCROOM_POINT_HALF
			
	#ifdef ASG_PCROOM_POINT_SYSTEM_MODIFY
			
	  #ifdef CSK_LUCKY_SEAL	// 해외와 틀림.
		case ITEM_HELPER+43:
			Gold = 6;
			break;
		case ITEM_HELPER+44:
			Gold = 9;
			break;
		case ITEM_HELPER+45:
			Gold = 15;
			break;
	  #endif	//  CSK_LUCKY_SEAL
	  #ifdef CSK_PCROOM_ITEM
		case ITEM_POTION+55:
			Gold = 9;
			break;
		case ITEM_POTION+56:
			Gold = 15;
			break;
		case ITEM_POTION+57:
			Gold = 30;
			break;
	  #endif // CSK_PCROOM_ITEM
	  #ifdef LDS_ADD_PCROOM_ITEM_JPN_6TH
		case ITEM_HELPER+96:		// 강함의 인장
			Gold = 32;
			break;
	  #endif // LDS_ADD_PCROOM_ITEM_JPN_6TH
			
	#else	// ASG_PCROOM_POINT_SYSTEM_MODIFY	
	  #ifdef CSK_LUCKY_SEAL	// 해외와 틀림.
		case ITEM_HELPER+43:
			Gold = 24;
			break;
		case ITEM_HELPER+44:
			Gold = 36;
			break;
		case ITEM_HELPER+45:
			Gold = 24;
			break;
	  #endif	//  CSK_LUCKY_SEAL
	  #ifdef CSK_PCROOM_ITEM
		case ITEM_POTION+55:
			Gold = 24;
			break;
		case ITEM_POTION+56:
			Gold = 36;
			break;
		case ITEM_POTION+57:
			Gold = 48;
			break;
	  #endif // CSK_PCROOM_ITEM
	  #ifdef LDS_ADD_PCROOM_ITEM_JPN_6TH
		case ITEM_HELPER+96:		// 강함의 인장
			Gold = 32;
			break;
	  #endif // LDS_ADD_PCROOM_ITEM_JPN_6TH
	
	#endif	// ASG_PCROOM_POINT_SYSTEM_MODIFY
#endif	// ADD_EVENT_PCROOM_POINT_HALF
		}
	}
	else	// PC방 포인트 상점이 아님.
	{
		switch(ip->Type)
		{
#ifdef CSK_PCROOM_ITEM
		case ITEM_POTION+55:
		case ITEM_POTION+56:
		case ITEM_POTION+57:
			Gold = 3000;	// 일반 상점에서 팔 때의 가격.
			break;
#endif // CSK_PCROOM_ITEM
		}
	}
#endif	// ADD_PCROOM_POINT_SYSTEM
#endif // KJH_DEL_PC_ROOM_SYSTEM

#ifdef HELLOWIN_EVENT
	if(ip->Type >= ITEM_POTION+45 && ip->Type <= ITEM_POTION+50)
	{
		Gold = ip->Durability*50;
	}
#endif // HELLOWIN_EVENT

#ifdef ASG_ADD_TIME_LIMIT_QUEST_ITEM
	switch (ip->Type)
	{
	case ITEM_POTION+153:
	case ITEM_POTION+154:
	case ITEM_POTION+155:
	case ITEM_POTION+156:
		Gold = ip->Durability*1000;
		break;
	case ITEM_POTION+157:
	case ITEM_POTION+158:
	case ITEM_POTION+159:
		Gold = 10000;
		break;
	}
#endif	// ASG_ADD_TIME_LIMIT_QUEST_ITEM 
	
#ifdef LDK_ADD_GAMBLE_RANDOM_ICON
	if( ip->Type == ITEM_HELPER+71 || ip->Type == ITEM_HELPER+72 || ip->Type == ITEM_HELPER+73 || ip->Type == ITEM_HELPER+74 ||ip->Type == ITEM_HELPER+75 )	// 겜블러 아이콘
	{
#if SELECTED_LANGUAGE == LANGUAGE_ENGLISH
		Gold = 2000000;
#else	// SELECTED_LANGUAGE == LANGUAGE_ENGLISH
#ifdef LDK_MOD_GAMBLE_SUMMER_SALE_EVENT
		Gold = 500000; // 여름 이벤트로 50% 세일
#else //LDK_MOD_GAMBLE_SUMMER_SALE_EVENT
		Gold = 1000000; //npc 상인에 따라 가격이 변함
#endif //LDK_MOD_GAMBLE_SUMMER_SALE_EVENT
#endif	// SELECTED_LANGUAGE == LANGUAGE_ENGLISH
	}
#endif //LDK_ADD_GAMBLE_RANDOM_ICON
	
    //  다크호스, 다크스피릿은 /3을 하지 않는다.
#ifdef KJH_FIX_DARKLOAD_PET_SYSTEM
	if( (ip->Type == ITEM_HELPER+4) || (ip->Type == ITEM_HELPER+5) )
	{		
		PET_INFO* pPetInfo = giPetManager::GetPetInfo( ip );

		if( pPetInfo->m_dwPetType == PET_TYPE_NONE )
			return -1;

		Gold = giPetManager::GetPetItemValue( pPetInfo );
	}
#else // KJH_FIX_DARKLOAD_PET_SYSTEM
    switch ( ip->Type )
    {
    case ITEM_HELPER+4 :    //  다크호스.
        Gold = giPetManager::ItemValue ( PET_TYPE_DARK_HORSE );
        break;

    case ITEM_HELPER+5 :    //  다크스피릿.
        Gold = giPetManager::ItemValue ( PET_TYPE_DARK_SPIRIT );
        break;

#ifdef DARK_WOLF
    case ITEM_HELPER+6 :    //  다크울프.
        Gold = giPetManager::ItemValue ( PET_TYPE_DARK_WOLF );
        break;  
#endif// DARK_WOLF
	}
#endif // KJH_FIX_DARKLOAD_PET_SYSTEM
 
#ifdef LDK_MOD_PREMIUMITEM_SELL
	switch(ip->Type)
	{
#ifdef LDK_MOD_INGAMESHOP_ITEM_CHANGE_VALUE
		//가격 변경 후
#ifdef LDS_ADD_INGAMESHOP_ITEM_KEYSILVER			// 인게임샾 아이템 // 신규 키(실버)				// MODEL_POTION+112
	case ITEM_POTION+112:
#endif // LDS_ADD_INGAMESHOP_ITEM_KEYSILVER
#ifdef LDS_ADD_INGAMESHOP_ITEM_KEYGOLD				// 인게임샾 아이템 // 신규 키(골드)				// MODEL_POTION+113
	case ITEM_POTION+113:
#endif // LDS_ADD_INGAMESHOP_ITEM_KEYGOLD
#ifdef LDK_ADD_INGAMESHOP_LOCKED_GOLD_CHEST		// 봉인된 금색상자
	case ITEM_POTION+121:
#endif //LDK_ADD_INGAMESHOP_LOCKED_GOLD_CHEST
#ifdef LDK_ADD_INGAMESHOP_LOCKED_SILVER_CHEST	// 봉인된 은색상자
	case ITEM_POTION+122:
#endif //LDK_ADD_INGAMESHOP_LOCKED_SILVER_CHEST
#ifdef LDK_ADD_INGAMESHOP_GOLD_CHEST			// 금색상자
	case ITEM_POTION+123:
#endif //LDK_ADD_INGAMESHOP_GOLD_CHEST
#ifdef LDK_ADD_INGAMESHOP_SILVER_CHEST			// 은색상자
	case ITEM_POTION+124:
#endif //LDK_ADD_INGAMESHOP_SILVER_CHEST
#ifdef PJH_ADD_PANDA_PET
	case ITEM_HELPER+80:  //펜더펫
#endif //PJH_ADD_PANDA_PET
#ifdef PJH_ADD_PANDA_CHANGERING
	case ITEM_HELPER+76:  //펜더변신반지
#endif //PJH_ADD_PANDA_CHANGERING
#ifdef LDK_ADD_PC4_GUARDIAN
	case ITEM_HELPER+64:  //유료 데몬
	case ITEM_HELPER+65:  //유료 수호정령
#endif //LDK_ADD_PC4_GUARDIAN
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGSAPPHIRE	// 신규 사파이어(푸른색)링	// MODEL_HELPER+109
	case ITEM_HELPER+109:
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGSAPPHIRE
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGRUBY		// 신규 루비(붉은색)링		// MODEL_HELPER+110
	case ITEM_HELPER+110:
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGRUBY
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGTOPAZ	// 신규 토파즈(주황)링		// MODEL_HELPER+111
	case ITEM_HELPER+111:
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGTOPAZ
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGAMETHYST	// 신규 자수정(보라색)링	// MODEL_HELPER+112
	case ITEM_HELPER+112:
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGAMETHYST
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETRUBY	// 신규 루비(붉은색) 목걸이	// MODEL_HELPER+113
	case ITEM_HELPER+113:
#endif // LDS_ADD_INGAMESHOP_ITEM_AMULETRUBY			// 인게임샾 아이템 // 신규 루비(붉은색) 목걸이	// MODEL_HELPER+113
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETEMERALD		// 인게임샾 아이템 // 신규 에메랄드(푸른) 목걸이// MODEL_HELPER+114
	case ITEM_HELPER+114:
#endif // LDS_ADD_INGAMESHOP_ITEM_AMULETEMERALD		// 인게임샾 아이템 // 신규 에메랄드(푸른) 목걸이// MODEL_HELPER+114
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETSAPPHIRE		// 인게임샾 아이템 // 신규 사파이어(녹색) 목걸이// MODEL_HELPER+115
	case ITEM_HELPER+115:
		Gold = 1000;
		break;
#endif // LDS_ADD_INGAMESHOP_ITEM_AMULETSAPPHIRE		// 인게임샾 아이템 // 신규 사파이어(녹색) 목걸이// MODEL_HELPER+115		

#ifdef YDG_ADD_SKELETON_CHANGE_RING
	case ITEM_HELPER+122:	// 스켈레톤 변신반지
#endif //YDG_ADD_SKELETON_CHANGE_RING
#ifdef YDG_ADD_SKELETON_PET
	case ITEM_HELPER+123:	// 스켈레톤 펫
		Gold = 2000;
		break;
#endif //YDG_ADD_SKELETON_PET

#ifdef LDK_ADD_INGAMESHOP_SMALL_WING			// 기간제 날개 작은(군망, 재날, 요날, 천날, 사날)
	case ITEM_WING+130:
	case ITEM_WING+131:
	case ITEM_WING+132:
	case ITEM_WING+133:
	case ITEM_WING+134:
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
	case ITEM_WING+135:
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		Gold = 80;
		break;
#endif //LDK_ADD_INGAMESHOP_SMALL_WING

#else //LDK_MOD_INGAMESHOP_ITEM_CHANGE_VALUE
		//가격 변경 전
#ifdef LDS_ADD_INGAMESHOP_ITEM_KEYSILVER			// 인게임샾 아이템 // 신규 키(실버)				// MODEL_POTION+112
	case ITEM_POTION+112: Gold = 10000; break;
#endif // LDS_ADD_INGAMESHOP_ITEM_KEYSILVER
#ifdef LDS_ADD_INGAMESHOP_ITEM_KEYGOLD				// 인게임샾 아이템 // 신규 키(골드)				// MODEL_POTION+113
	case ITEM_POTION+113: Gold = 10000; break;
#endif // LDS_ADD_INGAMESHOP_ITEM_KEYGOLD
#ifdef LDK_ADD_INGAMESHOP_LOCKED_GOLD_CHEST		// 봉인된 금색상자
	case ITEM_POTION+121:	Gold = 10000; break;
#endif //LDK_ADD_INGAMESHOP_LOCKED_GOLD_CHEST
#ifdef LDK_ADD_INGAMESHOP_LOCKED_SILVER_CHEST	// 봉인된 은색상자
	case ITEM_POTION+122:	Gold = 10000; break;
#endif //LDK_ADD_INGAMESHOP_LOCKED_SILVER_CHEST
#ifdef LDK_ADD_INGAMESHOP_GOLD_CHEST			// 금색상자
	case ITEM_POTION+123:	Gold = 10000; break;
#endif //LDK_ADD_INGAMESHOP_GOLD_CHEST
#ifdef LDK_ADD_INGAMESHOP_SILVER_CHEST			// 은색상자
	case ITEM_POTION+124:	Gold = 10000; break;
#endif //LDK_ADD_INGAMESHOP_SILVER_CHEST
#ifdef LDK_ADD_INGAMESHOP_SMALL_WING			// 기간제 날개 작은(군망, 재날, 요날, 천날, 사날)
	case ITEM_WING+130:
	case ITEM_WING+131:
	case ITEM_WING+132:
	case ITEM_WING+133:
	case ITEM_WING+134:
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
	case ITEM_WING+135:
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		Gold = 10000;
		break;
#endif //LDK_ADD_INGAMESHOP_SMALL_WING
#ifdef PJH_ADD_PANDA_PET
	case ITEM_HELPER+80:  //펜더펫
		Gold = 10000;
#endif //PJH_ADD_PANDA_PET
#ifdef PJH_ADD_PANDA_CHANGERING
	case ITEM_HELPER+76:  //펜더변신반지
		Gold = 10000;
#endif //PJH_ADD_PANDA_CHANGERING
#ifdef YDG_ADD_SKELETON_CHANGE_RING
	case ITEM_HELPER+122:	// 스켈레톤 변신반지
#endif //YDG_ADD_SKELETON_CHANGE_RING
#ifdef YDG_ADD_SKELETON_PET
	case ITEM_HELPER+123:	// 스켈레톤 펫
#endif //YDG_ADD_SKELETON_PET
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
	case ITEM_HELPER+128:	// 매조각상
	case ITEM_HELPER+129:	// 양조각상
	case ITEM_HELPER+134:	// 편자
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
	case ITEM_HELPER+130:	// 신규 오크참
	case ITEM_HELPER+131:	// 신규 메이플참
	case ITEM_HELPER+132:	// 신규 골든오크참
	case ITEM_HELPER+133:	// 골든메이플참
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2

#ifdef LDK_ADD_PC4_GUARDIAN
	case ITEM_HELPER+64:  //유료 데몬
	case ITEM_HELPER+65:  //유료 수호정령
		Gold = 10000;
#endif //LDK_ADD_PC4_GUARDIAN
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGSAPPHIRE		// 인게임샾 아이템 // 신규 사파이어(푸른색)링	// MODEL_HELPER+109
	case ITEM_HELPER+109:
		{
			Gold = 10000;
		}
		break;
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGSAPPHIRE		// 인게임샾 아이템 // 신규 사파이어(푸른색)링	// MODEL_HELPER+109
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGRUBY			// 인게임샾 아이템 // 신규 루비(붉은색)링		// MODEL_HELPER+110
	case ITEM_HELPER+110:
		{
			Gold = 10000;
		}
		break;
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGRUBY			// 인게임샾 아이템 // 신규 루비(붉은색)링		// MODEL_HELPER+110
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGTOPAZ			// 인게임샾 아이템 // 신규 토파즈(주황)링		// MODEL_HELPER+111
	case ITEM_HELPER+111:
		{
			Gold = 10000;
		}
		break;
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGTOPAZ			// 인게임샾 아이템 // 신규 토파즈(주황)링		// MODEL_HELPER+111
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGAMETHYST		// 인게임샾 아이템 // 신규 자수정(보라색)링		// MODEL_HELPER+112
	case ITEM_HELPER+112:
		{
			Gold = 10000;
		}
		break;
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGAMETHYST		// 인게임샾 아이템 // 신규 자수정(보라색)링		// MODEL_HELPER+112
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETRUBY			// 인게임샾 아이템 // 신규 루비(붉은색) 목걸이	// MODEL_HELPER+113
	case ITEM_HELPER+113:
		{
			Gold = 10000;
		}
		break;
#endif // LDS_ADD_INGAMESHOP_ITEM_AMULETRUBY			// 인게임샾 아이템 // 신규 루비(붉은색) 목걸이	// MODEL_HELPER+113
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETEMERALD		// 인게임샾 아이템 // 신규 에메랄드(푸른) 목걸이// MODEL_HELPER+114
	case ITEM_HELPER+114:
		{
			Gold = 10000;
		}
		break;
#endif // LDS_ADD_INGAMESHOP_ITEM_AMULETEMERALD		// 인게임샾 아이템 // 신규 에메랄드(푸른) 목걸이// MODEL_HELPER+114
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETSAPPHIRE		// 인게임샾 아이템 // 신규 사파이어(녹색) 목걸이// MODEL_HELPER+115
	case ITEM_HELPER+115:
		{
			Gold = 10000;
		}
		break;
#endif // LDS_ADD_INGAMESHOP_ITEM_AMULETSAPPHIRE		// 인게임샾 아이템 // 신규 사파이어(녹색) 목걸이// MODEL_HELPER+115		
#endif //LDK_MOD_INGAMESHOP_ITEM_CHANGE_VALUE
	}
#endif //LDK_MOD_PREMIUMITEM_SELL

	// CSK수리금지
	// 수리가 안되는 물건들은 여기에 예외처리 함. (goto EXIT_CALCULATE; 로 가야함)
    //  수리가 필요한 아이템은 그 비용을 뺀 나머지 값이 팔리는 가격이다.
	//. item filtering
#ifdef CSK_PCROOM_ITEM
	if(ip->Type >= ITEM_POTION+55 && ip->Type <= ITEM_POTION+57)
	{
		goto EXIT_CALCULATE;
	}
#endif // CSK_PCROOM_ITEM
	
#ifdef LDS_ADD_PCROOM_ITEM_JPN_6TH
	if(ip->Type == ITEM_HELPER+96)
	{
		goto EXIT_CALCULATE;
	}
#endif // LDS_ADD_PCROOM_ITEM_JPN_6TH	

	if( ip->Type == ITEM_HELPER+10 || ip->Type == ITEM_HELPER+20 || ip->Type == ITEM_HELPER+29
#ifdef MYSTERY_BEAD
		|| ip->Type == ITEM_WING+26
#endif // MYSTERAY_BEAD
		)
		goto EXIT_CALCULATE;
	if( ip->Type == ITEM_BOW+7 || ip->Type == ITEM_BOW+15 || ip->Type >= ITEM_POTION )
		goto EXIT_CALCULATE;
	if( ip->Type >= ITEM_WING+7 && ip->Type <= ITEM_WING+19 )	//. 구슬 필터링
		goto EXIT_CALCULATE;
	if( (ip->Type >= ITEM_HELPER+14 && ip->Type <= ITEM_HELPER+19) || ip->Type==ITEM_POTION+21 )
		goto EXIT_CALCULATE;
	if( ip->Type==ITEM_POTION+7 || ip->Type==ITEM_HELPER+7 || ip->Type==ITEM_HELPER+11 )//종훈물약
		goto EXIT_CALCULATE;
#ifdef KJH_FIX_20080910_NPCSHOP_PRICE
	if( (ip->Type>=ITEM_HELPER+49) && (ip->Type<=ITEM_HELPER+51))		// 낡은두루마리, 환영교단의서약, 피의두루마리
		goto EXIT_CALCULATE;
#endif // KJH_FIX_20080910_NPCSHOP_PRICE

#ifdef LDK_MOD_PREMIUMITEM_SELL
	switch(ip->Type)
	{
#ifdef LDS_ADD_INGAMESHOP_ITEM_KEYSILVER			// 인게임샾 아이템 // 신규 키(실버)				// MODEL_POTION+112
	case ITEM_POTION+112:	goto EXIT_CALCULATE;
#endif // LDS_ADD_INGAMESHOP_ITEM_KEYSILVER
#ifdef LDS_ADD_INGAMESHOP_ITEM_KEYGOLD				// 인게임샾 아이템 // 신규 키(골드)				// MODEL_POTION+113
	case ITEM_POTION+113:	goto EXIT_CALCULATE;
#endif // LDS_ADD_INGAMESHOP_ITEM_KEYGOLD
#ifdef LDK_ADD_INGAMESHOP_LOCKED_GOLD_CHEST		// 봉인된 금색상자
	case ITEM_POTION+121:	goto EXIT_CALCULATE;
#endif //LDK_ADD_INGAMESHOP_LOCKED_GOLD_CHEST
#ifdef LDK_ADD_INGAMESHOP_LOCKED_SILVER_CHEST	// 봉인된 은색상자
	case ITEM_POTION+122:	goto EXIT_CALCULATE;
#endif //LDK_ADD_INGAMESHOP_LOCKED_SILVER_CHEST
#ifdef LDK_ADD_INGAMESHOP_GOLD_CHEST			// 금색상자
	case ITEM_POTION+123:	goto EXIT_CALCULATE;
#endif //LDK_ADD_INGAMESHOP_GOLD_CHEST
#ifdef LDK_ADD_INGAMESHOP_SILVER_CHEST			// 은색상자
	case ITEM_POTION+124:	goto EXIT_CALCULATE;
#endif //LDK_ADD_INGAMESHOP_SILVER_CHEST
#ifdef LDK_ADD_INGAMESHOP_SMALL_WING			// 기간제 날개 작은(군망, 재날, 요날, 천날, 사날)
	case ITEM_WING+130:
	case ITEM_WING+131:
	case ITEM_WING+132:
	case ITEM_WING+133:
	case ITEM_WING+134:
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
	case ITEM_WING+135:
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		goto EXIT_CALCULATE;
#endif //LDK_ADD_INGAMESHOP_SMALL_WING
#ifdef YDG_ADD_SKELETON_CHANGE_RING
	case ITEM_HELPER+122:	// 스켈레톤 변신반지
		goto EXIT_CALCULATE;
#endif //YDG_ADD_SKELETON_CHANGE_RING
#ifdef YDG_ADD_SKELETON_PET
	case ITEM_HELPER+123:	// 스켈레톤 펫
		goto EXIT_CALCULATE;
#endif //YDG_ADD_SKELETON_PET
#ifdef PJH_ADD_PANDA_PET
	case ITEM_HELPER+80:  //펜더펫
		goto EXIT_CALCULATE;
#endif //PJH_ADD_PANDA_PET
#ifdef PJH_ADD_PANDA_CHANGERING
	case ITEM_HELPER+76:  //펜더변신반지
		goto EXIT_CALCULATE;
#endif //PJH_ADD_PANDA_CHANGERING
#ifdef LDK_ADD_PC4_GUARDIAN
	case ITEM_HELPER+64:  //유료 데몬
	case ITEM_HELPER+65:  //유료 수호정령
		goto EXIT_CALCULATE;
#endif //LDK_ADD_PC4_GUARDIAN
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGSAPPHIRE		// 인게임샾 아이템 // 신규 사파이어(푸른색)링	// MODEL_HELPER+109
	case ITEM_HELPER+109:
		goto EXIT_CALCULATE;
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGSAPPHIRE		// 인게임샾 아이템 // 신규 사파이어(푸른색)링	// MODEL_HELPER+109
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGRUBY			// 인게임샾 아이템 // 신규 루비(붉은색)링		// MODEL_HELPER+110
	case ITEM_HELPER+110:
		goto EXIT_CALCULATE;
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGRUBY			// 인게임샾 아이템 // 신규 루비(붉은색)링		// MODEL_HELPER+110
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGTOPAZ			// 인게임샾 아이템 // 신규 토파즈(주황)링		// MODEL_HELPER+111
	case ITEM_HELPER+111:
		goto EXIT_CALCULATE;
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGTOPAZ			// 인게임샾 아이템 // 신규 토파즈(주황)링		// MODEL_HELPER+111
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGAMETHYST		// 인게임샾 아이템 // 신규 자수정(보라색)링		// MODEL_HELPER+112
	case ITEM_HELPER+112:
		goto EXIT_CALCULATE;
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGAMETHYST		// 인게임샾 아이템 // 신규 자수정(보라색)링		// MODEL_HELPER+112
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETRUBY			// 인게임샾 아이템 // 신규 루비(붉은색) 목걸이	// MODEL_HELPER+113
	case ITEM_HELPER+113:
		goto EXIT_CALCULATE;
#endif // LDS_ADD_INGAMESHOP_ITEM_AMULETRUBY			// 인게임샾 아이템 // 신규 루비(붉은색) 목걸이	// MODEL_HELPER+113
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETEMERALD		// 인게임샾 아이템 // 신규 에메랄드(푸른) 목걸이// MODEL_HELPER+114
	case ITEM_HELPER+114:
		goto EXIT_CALCULATE;
#endif // LDS_ADD_INGAMESHOP_ITEM_AMULETEMERALD		// 인게임샾 아이템 // 신규 에메랄드(푸른) 목걸이// MODEL_HELPER+114
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETSAPPHIRE		// 인게임샾 아이템 // 신규 사파이어(녹색) 목걸이// MODEL_HELPER+115
	case ITEM_HELPER+115:
		goto EXIT_CALCULATE;
#endif // LDS_ADD_INGAMESHOP_ITEM_AMULETSAPPHIRE		// 인게임샾 아이템 // 신규 사파이어(녹색) 목걸이// MODEL_HELPER+115
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
	case ITEM_HELPER+128:
	case ITEM_HELPER+129:
	case ITEM_HELPER+134:
		goto EXIT_CALCULATE;
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
	case ITEM_HELPER+130:	// 신규 오크참
	case ITEM_HELPER+131:	// 신규 메이플참
	case ITEM_HELPER+132:	// 신규 골든오크참
	case ITEM_HELPER+133:	// 골든메이플참
		goto EXIT_CALCULATE;
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
	}
#endif //LDK_MOD_PREMIUMITEM_SELL

	if(goldType==1 && !(ip->Type >= ITEM_HELPER+32 && ip->Type <= ITEM_HELPER+37))	//^ 펜릴 아이템 관련
    {
        //  수리비용을 계산한다.
//        char Text[100];
        //int repairGold = ConvertRepairGold(Gold,ip->Durability, p->Durability, ip->Type, Text);
		DWORD maxDurability = calcMaxDurability ( ip, p, Level );
		float persent = 1.f - ( (float)ip->Durability / (float)maxDurability);
        DWORD repairGold = (DWORD)(Gold * 0.6f * persent);

		if(ip->Type == ITEM_HELPER+31)
			repairGold = 0;
	
        Gold -= repairGold;
    }
EXIT_CALCULATE:

	if(Gold >= 1000)
	{
		Gold = (Gold / 100) * 100;
	}
	else if(Gold >= 100)
	{
		Gold = (Gold / 10) * 10;
	}
#ifdef LEM_ADD_LUCKYITEM	// 럭키아이템 아이템 하나 //  수리비용을 계산한다. ( 제외시키기! )
	if( Check_LuckyItem( ip->Type ) )	Gold = 0;
#endif // LEM_ADD_LUCKYITEM
	
	return (int)Gold;
}

#ifndef KJH_DEL_PC_ROOM_SYSTEM			// #ifndef
#ifndef ASG_PCROOM_POINT_SYSTEM_MODIFY		// 정리시 삭제.
#ifdef ADD_PCROOM_POINT_SYSTEM
// 아이템 구입 조건 포인트 얻음.
int GetItemBuyingTermsPoint(ITEM* pItem)
{
	int nPoint = 0;

	return nPoint;
}
#endif	// ADD_PCROOM_POINT_SYSTEM
#endif	// ASG_PCROOM_POINT_SYSTEM_MODIFY	// 정리시 삭제.
#endif // KJH_DEL_PC_ROOM_SYSTEM

/*	미사용 함수라 주석처리 (양대근)
int ItemValueOld(ITEM *ip,int goldType)
{
	if(ip->Type == -1) return 0;

	ITEM_ATTRIBUTE *p = &ItemAttribute[ip->Type];

	int Gold = 0;
	int Type = ip->Type / MAX_ITEM_INDEX;
	int Level = (ip->Level>>3)&15;
	bool Excellent = false;
	for(int i=0;i<ip->SpecialNum;i++)
	{
		switch(ip->Special[i])
		{
		case AT_IMPROVE_LIFE:          
		case AT_IMPROVE_MANA:          
		case AT_DECREASE_DAMAGE:       
		case AT_REFLECTION_DAMAGE:     
		case AT_IMPROVE_BLOCKING_PERCENT:         
		case AT_IMPROVE_GAIN_GOLD:     
		case AT_EXCELLENT_DAMAGE:   
		case AT_IMPROVE_DAMAGE_LEVEL:  
		case AT_IMPROVE_DAMAGE_PERCENT:
		case AT_IMPROVE_MAGIC_LEVEL:   
		case AT_IMPROVE_MAGIC_PERCENT: 
		case AT_IMPROVE_ATTACK_SPEED:  
		case AT_IMPROVE_GAIN_LIFE:     
		case AT_IMPROVE_GAIN_MANA:     
			Excellent = true;
			break;
		}
	}
	int Level2 = p->Level+Level*3;
	if(Excellent)
		Level2 += 25;

	if(ip->Type==ITEM_BOW+7)        //  석궁 화살.
	{
        int sellMoney;

        switch ( Level )
        {
        case    0 :
            sellMoney = 100;
            break;
        case    1 :
            sellMoney = 1400;
            break;
        case    2 :
            sellMoney = 2200;
            break;
        }
		if(p->Durability > 0)
     		Gold = (sellMoney*ip->Durability/p->Durability);//+(170*(Level*2));
	}
	else if(ip->Type==ITEM_BOW+15)  //  화살.
	{
        int sellMoney;

        switch ( Level )
        {
        case    0 :
            sellMoney = 70;
            break;
        case    1 :
            sellMoney = 1200;
            break;
        case    2 :
            sellMoney = 2000;
            break;
        }
		if(p->Durability > 0)
     		Gold = (sellMoney*ip->Durability/p->Durability);//+(80*(Level*2));
	}
	else if(ip->Type==ITEM_POTION+13)//석
	{
    	Gold = 100000;
	}
	else if(ip->Type==ITEM_POTION+14)//영
	{
    	Gold = 70000;
	}
	else if(ip->Type==ITEM_WING+15)//혼돈
	{
    	Gold = 40000;
	}
    else if(ip->Type==ITEM_POTION+16)// 생명의 보석.
    {
        Gold = 210000;
    }
    else if(ip->Type==ITEM_POTION+22)// 창조의 보석.
    {
        Gold = 18000000;
    }
    else if(ip->Type==ITEM_HELPER+14)   //  로크의 깃털.
    {
        Gold = 180000;
    }
	else if(ip->Type==ITEM_HELPER+3)	// 디노란트
	{
        Gold = 960000;
        for(int i=0;i<ip->SpecialNum;i++)
		{
			switch(ip->Special[i])
            {
			case AT_DAMAGE_ABSORB:  
            case AT_IMPROVE_AG_MAX:
            case AT_IMPROVE_ATTACK_SPEED:
				Gold += 300000;
				break;
            }
        }
	}
    else if ( ip->Type==ITEM_HELPER+4 ) //  다크호스
    {
        Gold = 33000000;
    }
    else if(ip->Type==ITEM_HELPER+15)   //  스테이트 증/감 열매.
    {
        Gold = 33000000;
    }
#ifdef LENA_EXCHANGE_ZEN
    else if(ip->Type==ITEM_POTION+21)
    {
        Gold = 9000;
    }
#endif
	else if(ip->Type==ITEM_POTION+17)// 악마의 눈
	{
		int iValue[5] = { 30000, 15000, 21000, 30000, 45000};
		Gold = iValue[min( max( 0, Level), 4)];
	}
	else if(ip->Type==ITEM_POTION+18)// 악마의 열쇠
	{
		int iValue[5] = { 30000, 15000, 21000, 30000, 45000};
		Gold = iValue[min( max( 0, Level), 4)];
	}
	else if(ip->Type==ITEM_POTION+19)// 데블스퀘어 초대권
	{
		int iValue[5] = { 120000, 60000, 84000, 120000, 180000};
		Gold = iValue[min( max( 0, Level), 4)];
	}
	else if(ip->Type==ITEM_POTION+9 && Level == 1)// 사랑의 올리브
	{
		Gold = 1000;
	}
	else if(ip->Type==ITEM_POTION+20)	// 사랑의 묘약
	{
		Gold = 900;
	}
	else if(p->Value > 0)//물약
	{
		Gold = p->Value*p->Value*10/12;
		if(ip->Type>=ITEM_POTION && ip->Type<=ITEM_POTION+8)
          	Gold *= ip->Durability;
	}
	else if( ( ( Type==12 && ip->Type>ITEM_WING+6) || Type==13 || Type==15 ) )//12(날개,구슬) 13(사탄,반지) 15(법서), 디노란트 제외.
	{
		Gold = 100+Level2*Level2*Level2;
        for(i=0;i<ip->SpecialNum;i++)
		{
			switch(ip->Special[i])
			{
			case AT_LIFE_REGENERATION:
				Gold += Gold*ip->SpecialValue[i];
				break;
			}
		}
	}
	else
	{
		switch(Level)     //+무기일수록 가중치 붙음
		{
		case 5:Level2 += 4;break;
		case 6:Level2 += 10;break;
		case 7:Level2 += 25;break;
		case 8:Level2 += 45;break;
		case 9:Level2 += 65;break;
		case 10:Level2 += 95;break;	// +10
		case 11:Level2 += 135;break;
		}
        //  날개.
	    if( ( Type==12 && ip->Type<=ITEM_WING+6) )
        {
            Gold = 40000000+((40+Level2)*Level2*Level2*11);
        }
        else
        {
		    Gold = 100+(40+Level2)*Level2*Level2/8;
        }
		if(Type>=0 && Type<=6) //무기일때
		{
			if(!p->TwoHand) //한손무기일때 80%
				Gold = Gold*80/100;
		}
        for(int i=0;i<ip->SpecialNum;i++)
		{
			switch(ip->Special[i])
			{
			case AT_SKILL_BLOCKING:
			case AT_SKILL_SWORD1:
			case AT_SKILL_SWORD2:
			case AT_SKILL_SWORD3:
			case AT_SKILL_SWORD4:
			case AT_SKILL_SWORD5:
			case AT_SKILL_CROSSBOW:
			case AT_SKILL_ICE_BLADE:
				Gold += ( int)( ( float)Gold*1.5f);
				break;
			case AT_IMPROVE_DAMAGE:
			case AT_IMPROVE_MAGIC:
			case AT_IMPROVE_DEFENSE:
                if ( Type==12 && ip->Type<=ITEM_WING+6 )    //  날개.
                {
				    switch(ip->SpecialValue[i])
				    {
				    case 4 :Gold += ( int)( ( float)Gold* 3/10);break;
				    case 8 :Gold += ( int)( ( float)Gold* 6/10);break;
				    case 12:Gold += ( int)( ( float)Gold*10/10);break;
				    case 16:Gold += ( int)( ( float)Gold*20/10);break;
				    }
                }
                else
                {
				    switch(ip->SpecialValue[i])
				    {
				    case 4 :Gold += ( int)( ( float)Gold* 6/10);break;
				    case 8 :Gold += ( int)( ( float)Gold*14/10);break;
				    case 12:Gold += ( int)( ( float)Gold*28/10);break;
				    case 16:Gold += ( int)( ( float)Gold*56/10);break;
				    }
                }
				break;
			case AT_IMPROVE_BLOCKING:
				switch(ip->SpecialValue[i])
				{
				case 5 :Gold += ( int)( ( float)Gold* 6/10);break;
				case 10:Gold += ( int)( ( float)Gold*14/10);break;
				case 15:Gold += ( int)( ( float)Gold*28/10);break;
				case 20:Gold += ( int)( ( float)Gold*56/10);break;
				}
				break;
			case AT_LUCK:
				Gold += ( int)( ( float)Gold*25/100);
				break;
			case AT_IMPROVE_LIFE:          
			case AT_IMPROVE_MANA:          
			case AT_DECREASE_DAMAGE:       
			case AT_REFLECTION_DAMAGE:     
			case AT_IMPROVE_BLOCKING_PERCENT:         
			case AT_IMPROVE_GAIN_GOLD:     
			case AT_EXCELLENT_DAMAGE:   
			case AT_IMPROVE_DAMAGE_LEVEL:  
			case AT_IMPROVE_DAMAGE_PERCENT:
			case AT_IMPROVE_MAGIC_LEVEL:   
			case AT_IMPROVE_MAGIC_PERCENT: 
			case AT_IMPROVE_GAIN_LIFE:     
			case AT_IMPROVE_GAIN_MANA:     
            case AT_IMPROVE_HP_MAX :
            case AT_IMPROVE_MP_MAX :
            case AT_ONE_PERCENT_DAMAGE :  
			case AT_IMPROVE_ATTACK_SPEED:  
            case AT_IMPROVE_AG_MAX :
            case AT_IMPROVE_EVADE:
				Gold += Gold;
				break;
			}
		}
	}
    if( goldType )  //  팔기. 수리.
    {
	    Gold = Gold/3;
    }

    //  수리가 필요한 아이템은 그 비용을 뺀 나머지 값이 팔리는 가격이다.
    if(goldType==1 && 
     ((ip->Type<ITEM_POTION  && ip->Type!=ITEM_HELPER+10) &&
       ip->Type!=ITEM_BOW+7  && ip->Type!=ITEM_BOW+15      &&
      (ip->Type<ITEM_WING || ip->Type>ITEM_WING+6) &&   //  날개.
		ip->Type != ITEM_HELPER+29 &&
      (ip->Type!=ITEM_HELPER && ip->Type!=ITEM_HELPER+1 && ip->Type!=ITEM_HELPER+2 && ip->Type!=ITEM_HELPER+3 && ip->Type!=ITEM_HELPER+15) ) )
    {
        //  수리비용을 계산한다.
		int maxDurability = calcMaxDurability ( ip, p, Level );
		float   persent = 1.f - ( (float)ip->Durability / (float)maxDurability);
        int repairGold = (int)(Gold * 0.6f * persent);

        Gold -= repairGold;
    }

	if(Gold >= 1000)
		Gold = Gold/100*100;
	else if(Gold >= 100)
		Gold = Gold/10*10;
	return Gold;
}
*/

#ifdef PSW_BUGFIX_REQUIREEQUIPITEM

bool IsRequireEquipItem(ITEM* pItem)
{
	if(pItem == NULL){
		return false;
	}
	
	ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
	
	bool bEquipable = false;
	
	///////////////////////////////////장착 가능한 클래스 체크///////////////////////////////////
	
	//. 착용 可
	if( pItemAttr->RequireClass[GetBaseClass(Hero->Class)] ){
		bEquipable = true; 
	}
	//. 마검일 경우 법사, 기사것을 착용 可
	else if (GetBaseClass(Hero->Class) == CLASS_DARK && pItemAttr->RequireClass[CLASS_WIZARD] 
		&& pItemAttr->RequireClass[CLASS_KNIGHT]) {
		bEquipable = true; 
	}
	
	// 전직단계를 검사해서 
	BYTE byFirstClass = GetBaseClass(Hero->Class);
	BYTE byStepClass = GetStepClass(Hero->Class);
	if( pItemAttr->RequireClass[byFirstClass] > byStepClass)
	{
		return false;
	}
	
	if(bEquipable == false)
		return false;
	
	///////////////////////////////////요구 능력치 체크///////////////////////////////////////
	
	WORD wStrength = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
	WORD wDexterity = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
	WORD wEnergy = CharacterAttribute->Energy + CharacterAttribute->AddEnergy;
	WORD wVitality = CharacterAttribute->Vitality + CharacterAttribute->AddVitality;
	WORD wCharisma = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;
	WORD wLevel = CharacterAttribute->Level;
	
	int iItemLevel = (pItem->Level>>3)&15;
	
	int iDecNeedStrength = 0, iDecNeedDex = 0;

	extern JewelHarmonyInfo*   g_pUIJewelHarmonyinfo;
	if( iItemLevel >= pItem->Jewel_Of_Harmony_OptionLevel )
	{
		StrengthenCapability SC;
		g_pUIJewelHarmonyinfo->GetStrengthenCapability( &SC, pItem, 0 );
		
		if( SC.SI_isNB ) 
		{
			iDecNeedStrength = SC.SI_NB.SI_force;
			iDecNeedDex      = SC.SI_NB.SI_activity;
		}
	}
	
	if ( pItem->RequireStrength - iDecNeedStrength > wStrength )
		return false;    //  요구힘보다 작으면 실패.
	if ( pItem->RequireDexterity - iDecNeedDex > wDexterity )
		return false;    //  요구민첩보다 작으면 실패.
	if ( pItem->RequireEnergy > wEnergy )
		return false;    //  요구에너지보다 작으면 실패.
	if ( pItem->RequireVitality > wVitality )
		return false;    //  요구체력보다 작으면 실패.
	if( pItem->RequireCharisma > wCharisma )
		return false;    //  요구통솔보다 작으면 실패.
	if( pItem->RequireLevel > wLevel )
		return false;    //  요구레벨보다 작으면 실패.
	
	/////////////////////////////////////////통솔 예외//////////////////////////////////////////
	
#ifdef KJH_FIX_DARKLOAD_PET_SYSTEM
	if(pItem->Type == ITEM_HELPER+5 )			// 다크스피릿		   
	{
		PET_INFO* pPetInfo = giPetManager::GetPetInfo( pItem );
		WORD wRequireCharisma = (185+(pPetInfo->m_wLevel*15));
		if( wRequireCharisma > wCharisma ) {
			return false;	//  요구통솔보다 작으면 실패.
		}
	}
#else // KJH_FIX_DARKLOAD_PET_SYSTEM
	PET_INFO petInfo;
	giPetManager::GetPetInfo(petInfo, pItem->Type);
	WORD wRequireCharisma = (185+(petInfo.m_wLevel*15));		// 이혁재 - 다크호스, 스피릿 요구 통솔적용

	// 이혁재 - 다크 스피릿 일때 요구통솔이 작으면 실패
	if(pItem->Type == 421 )		   
	{
		if( wRequireCharisma > wCharisma ) {
			return false;	//  요구통솔보다 작으면 실패.
		}
	}
#endif // KJH_FIX_DARKLOAD_PET_SYSTEM
	
	return bEquipable;
}

#endif //PSW_BUGFIX_REQUIREEQUIPITEM

// 추가데미지
void PlusSpecial(WORD *Value,int Special,ITEM *Item)
{
	if(Item->Type == -1) {
		return;
	}

#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_SPECIALOPTION
	if( IsRequireEquipItem(Item) )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_SPECIALOPTION
	{
		for(int i=0;i<Item->SpecialNum;i++)
		{
			//  내구력이 0이면은 아이템의 능력을 받지 못한다.
			if(Item->Special[i] == Special && Item->Durability!=0)
				*Value += Item->SpecialValue[i];
		}
	}
}

// 추가 +% 데미지
void PlusSpecialPercent(WORD *Value,int Special,ITEM *Item,WORD Percent)
{
	if(Item->Type == -1) return;

#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_SPECIALOPTION
	if( IsRequireEquipItem(Item) )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_SPECIALOPTION
	{
		for(int i=0;i<Item->SpecialNum;i++)
		{
			if(Item->Special[i] == Special && Item->Durability!=0 )
				*Value += *Value*Percent/100;
		}
	}
}

void PlusSpecialPercent2(WORD *Value,int Special,ITEM *Item)
{
	if(Item->Type == -1) return;

#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_SPECIALOPTION
	if( IsRequireEquipItem(Item) )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_SPECIALOPTION
	{
		for(int i=0;i<Item->SpecialNum;i++)
		{
			if(Item->Special[i] == Special && Item->Durability!=0 )
				*Value += (unsigned short)(*Value*(Item->SpecialValue[i]/100.f));
		}
	}
}

WORD ItemDefense(ITEM *Item)
{
	if(Item->Type == -1) return 0;
	WORD Defense = Item->Defense;
	PlusSpecial(&Defense,AT_IMPROVE_DEFENSE,Item);
	return Defense;
}

WORD ItemMagicDefense(ITEM *Item)
{
	if(Item->Type == -1) return 0;
	WORD MagicDefense = Item->MagicDefense;
	//PlusSpecial(&MagicDefense,PLUS_MAGIC_DEFENSE,Item);
	return MagicDefense;
}

WORD ItemWalkSpeed(ITEM *Item)
{
	if(Item->Type == -1) return 0;
	WORD WalkSpeed = Item->WalkSpeed;
	//PlusSpecial(&WalkSpeed,PLUS_WALK_SPEED,Item);
	return WalkSpeed;
}


///////////////////////////////////////////////////////////////////////////////
// monster
///////////////////////////////////////////////////////////////////////////////

extern int HeroKey;
int EditMonsterNumber = 0;
int MonsterKey = 0;

void OpenMonsters(char *FileName)
{
	if((SMDFile=fopen(FileName,"rb")) == NULL)	return;
	SMDToken Token;
	Token = (*GetToken)();
	while(true)
	{
		Token = (*GetToken)();int Monster = (int)TokenNumber;
		if(Token == END) break;
		if(Token == NAME && strcmp("end",TokenString)==NULL) break;
		Token = (*GetToken)();
		Token = (*GetToken)();
		Token = (*GetToken)();int x = (int)TokenNumber;
		Token = (*GetToken)();int y = (int)TokenNumber;
		Token = (*GetToken)();
		if(MonsterKey == HeroKey) MonsterKey++;
        CHARACTER *c = CreateMonster(Monster,x,y,MonsterKey++);
		if(c != NULL)
			c->Object.Kind = KIND_EDIT;
	}
	fclose(SMDFile);
}

void OpenMonsterScript(char *FileName)
{
	if((SMDFile=fopen(FileName,"rb")) == NULL)	return;
	SMDToken Token;
	while(true)
	{
		Token = (*GetToken)();//번호
		if(Token == END) break;
		if(Token == NAME && strcmp("end",TokenString)==NULL) break;
        MONSTER_SCRIPT *m = &MonsterScript[EditMonsterNumber++];
		m->Type = (int)TokenNumber;
		Token = (*GetToken)();
		Token = (*GetToken)();strcpy(m->Name,TokenString);
		//Token = (*GetToken)();m->Level = (int)TokenNumber;
		//for(int i=0;i<23;i++) Token = (*GetToken)();
	}
	fclose(SMDFile);
}

//////////////////////////////////////////////////////////////////////////
//  몬스터의 이름을 알아낸다.
//////////////////////////////////////////////////////////////////////////
char*   getMonsterName ( int type )
{
    for ( int i=0; i<MAX_MONSTER; ++i )
    {
        if ( MonsterScript[i].Type==type )
        {
            return MonsterScript[i].Name;
        }
    }
    return NULL;
}

void MonsterConvert(MONSTER *m,int Level)
{
	MONSTER_SCRIPT    *p            = &MonsterScript[m->Type];
	MONSTER_ATTRIBUTE *c            = &p->Attribute;
	m->Level                        = Level;
	m->Attribute.AttackSpeed        = c->AttackSpeed;
    m->Attribute.AttackDamageMin    = c->AttackDamageMin   /2+c->AttackDamageMin   /2*(m->Level)/9;
	m->Attribute.AttackDamageMax	= c->AttackDamageMax   /2+c->AttackDamageMax   /2*(m->Level)/9;
	m->Attribute.Defense            = c->Defense           /2+c->Defense           /2*(m->Level)/9;
	m->Attribute.AttackRating       = c->AttackRating      /2+c->AttackRating      /2*(m->Level)/9;
	m->Attribute.SuccessfulBlocking = c->SuccessfulBlocking/2+c->SuccessfulBlocking/2*(m->Level)/9;
}

///////////////////////////////////////////////////////////////////////////////
// class
///////////////////////////////////////////////////////////////////////////////

void CreateClassAttribute(int Class,int Strength,int Dexterity,int Vitality,int Energy,int Life,int Mana,int LevelLife,int LevelMana,int VitalityToLife,int EnergyToMana)
{
	CLASS_ATTRIBUTE *c = &ClassAttribute[Class];
	c->Strength		  = Strength;
	c->Dexterity	  = Dexterity;
	c->Vitality		  = Vitality;
	c->Energy		  = Energy;
	c->Life           = Life;
	c->Mana           = Mana;
	c->LevelLife	  = LevelLife;
	c->LevelMana	  = LevelMana;
	c->VitalityToLife = VitalityToLife;
	c->EnergyToMana   = EnergyToMana;
	c->Shield		  =  0;
}

void CreateClassAttributes()
{
	CreateClassAttribute( 0, 18, 18, 15, 30,	  80, 60,	  1, 2, 1, 2 );	//마법사
	CreateClassAttribute( 1, 28, 20, 25, 10,	 110, 20,	  2, 1, 2, 1 );	//흑기사
	CreateClassAttribute( 2, 50, 50, 50, 30,	 110, 30,	 110, 30, 6, 3 );	//요정
	CreateClassAttribute( 3, 30, 30, 30, 30,	 120, 80,	  1, 1, 2, 2 );	//마기사
	CreateClassAttribute( 4, 30, 30, 30, 30,	 120, 80,	  1, 1, 2, 2 );	//다크로드
	CreateClassAttribute( 5, 50, 50, 50, 30,	 110, 30,	 110, 30, 6, 3 );	// 엘리스
#ifdef PBG_ADD_NEWCHAR_MONK
	CreateClassAttribute( 6, 32, 27, 25, 20,	 100, 40,	 /*사용안함 => */1, 3, 1, 1);	// 몽크
#endif //PBG_ADD_NEWCHAR_MONK
}

///////////////////////////////////////////////////////////////////////////////
// Durability Percent Calc.
///////////////////////////////////////////////////////////////////////////////
float CalcDurabilityPercent ( BYTE dur, BYTE maxDur, int Level, int option, int ExtOption )
{
    int maxDurability = maxDur;
    for( int i=0; i<((Level>>3)&15); i++ )
    {
        if(i>=4)
        {
            maxDurability+=2;
        }
        else
        {
            maxDurability++;
        }
    }

    if ( (ExtOption%0x04)==EXT_A_SET_OPTION || (ExtOption%0x04)==EXT_B_SET_OPTION )    //  세트 아이템.
    {
        maxDurability+=20;
    }
    else if((option&63) > 0 )
    {
        maxDurability+=15;
    }

    float   durP = 1.f - (dur/(float)maxDurability);

    if( durP>0.8f )
    {
        return  0.5f;
    }
    else
    if( durP>0.7f )
    {
        return  0.3f;
    }
    else
    if( durP>0.5f )
    {
        return  0.2f;
    }
    else
    {
        return  0.f;
    }
    return 0.f;
}


///////////////////////////////////////////////////////////////////////////////
// character machine
///////////////////////////////////////////////////////////////////////////////

void CHARACTER_MACHINE::Init()
{
    CreateClassAttributes();
	for(int i=0;i<MAX_EQUIPMENT;i++)
	{
		Equipment[i].Type = -1;
	}
	Gold = 0;
	StorageGold = 0;
	PacketSerial = 0;
	InfinityArrowAdditionalMana = 0;
}

void CHARACTER_MACHINE::InitAddValue ()
{
	Character.AddStrength	= 0;
	Character.AddDexterity  = 0;
	Character.AddVitality	= 0;
	Character.AddEnergy		= 0;
    Character.AddCharisma   = 0;
	Character.AddLifeMax	= 0;
	Character.AddManaMax	= 0;
}

void CHARACTER_MACHINE::SetCharacter(BYTE Class)
{
	CLASS_ATTRIBUTE *c  = &ClassAttribute[Class];
	Character.Class	    = Class;
	Character.Level	    = 1;
	Character.Strength	= c->Strength;
	Character.Dexterity = c->Dexterity;
	Character.Vitality	= c->Vitality;
	Character.Energy	= c->Energy;
	Character.Life      = c->Life;
	Character.Mana      = c->Mana;
	Character.LifeMax   = c->Life;
	Character.ManaMax   = c->Mana;
	Character.Shield	= c->Shield;
	Character.ShieldMax = c->Shield;

	InitAddValue ();
	hanguo_check5();

	for(int j=0;j<MAX_SKILLS;j++)
	{
		Character.Skill[j] = 0;
	}
}

void CHARACTER_MACHINE::InputEnemyAttribute(MONSTER *e)
{
	memcpy(&Enemy,e,sizeof(MONSTER));
}

void CHARACTER_MACHINE::CalculateDamage()
{
    WORD    DamageMin, DamageMax;

	ITEM *Right  = &Equipment[EQUIPMENT_WEAPON_RIGHT];
	ITEM *Left   = &Equipment[EQUIPMENT_WEAPON_LEFT];
	ITEM *Amulet = &Equipment[EQUIPMENT_AMULET];
	ITEM *RRing  = &Equipment[EQUIPMENT_RING_RIGHT];
	ITEM *LRing  = &Equipment[EQUIPMENT_RING_LEFT];

	WORD Strength, Dexterity, Energy;

	Strength = Character.Strength + Character.AddStrength;
	Dexterity= Character.Dexterity+ Character.AddDexterity;
	Energy	 = Character.Energy   + Character.AddEnergy;
#ifdef PBG_ADD_NEWCHAR_MONK
	WORD Vitality = Character.Vitality + Character.AddVitality;
#endif //PBG_ADD_NEWCHAR_MONK

#ifndef ASG_FIX_PICK_ITEM_FROM_INVEN	// 정리시 삭제.
	// 아이템을 든 경우
	SEASON3B::CNewUIPickedItem* pPickedItem = SEASON3B::CNewUIInventoryCtrl::GetPickedItem();
	// 인벤토리에서 온 것인가?
	if ( pPickedItem && pPickedItem->GetOwnerInventory() == g_pMyInventory->GetInventoryCtrl())
	{	
		ITEM* pItemObj = pPickedItem->GetItem();

		switch ( pPickedItem->GetSourceLinealPos() ) {
		case EQUIPMENT_WEAPON_RIGHT:
			Right = pItemObj;
			break;
		case EQUIPMENT_WEAPON_LEFT:
			Left = pItemObj;
			break;
		case EQUIPMENT_AMULET:
			Amulet = pItemObj;
			break;
		case EQUIPMENT_RING_RIGHT:
			RRing = pItemObj;
			break;
		case EQUIPMENT_RING_LEFT:
			LRing = pItemObj;
			break;
		}
	}
#endif	// ASG_FIX_PICK_ITEM_FROM_INVEN

    int CharacterClass = GetBaseClass ( Character.Class );

    //  내구력이 0이면은 그 장비를 장착했다고 인식하지 않는다.
 #ifdef ADD_SOCKET_ITEM
     if( ((GetEquipedBowType(Left) == BOWTYPE_BOW) && (Left->Durability != 0))
		|| ((GetEquipedBowType(Right) == BOWTYPE_CROSSBOW) && (Right->Durability != 0))
		)
 #else // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
    if( (Right->Type>=ITEM_BOW+8  && Right->Type<ITEM_BOW+15 && Right->Durability!=0 ) ||
		(((Left->Type>=ITEM_BOW   && Left->Type<ITEM_BOW+7 ) || Left->Type==ITEM_BOW+17
		|| Left->Type==ITEM_BOW+20 || Left->Type==ITEM_BOW+21 || Left->Type==ITEM_BOW+22
		) && Left->Durability!=0 ) || 
        (Right->Type>=ITEM_BOW+16 && Right->Type<ITEM_BOW+MAX_ITEM_INDEX && Right->Durability!=0 ) )
#endif // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
	{	
		Character.AttackDamageMinRight = Dexterity/7 + Strength/14;
		Character.AttackDamageMaxRight = Dexterity/4 + Strength/8;
		Character.AttackDamageMinLeft  = Dexterity/7 + Strength/14;
		Character.AttackDamageMaxLeft  = Dexterity/4 + Strength/8;
	}
	else
	{
        switch ( CharacterClass )
        {
        case CLASS_ELF:
			Character.AttackDamageMinRight = (Strength+Dexterity)/7;
			Character.AttackDamageMaxRight = (Strength+Dexterity)/4;
			Character.AttackDamageMinLeft  = (Strength+Dexterity)/7;
			Character.AttackDamageMaxLeft  = (Strength+Dexterity)/4;
            break;

        case CLASS_KNIGHT:
            {
                int minValue = 7;
                int maxValue = 4;
                
                minValue = 6; maxValue = 4;

                Character.AttackDamageMinRight = Strength/minValue;
                Character.AttackDamageMaxRight = Strength/maxValue;
                Character.AttackDamageMinLeft  = Strength/minValue;
                Character.AttackDamageMaxLeft  = Strength/maxValue;
            }
            break;

        case CLASS_DARK:
            {
                int minValue[2] = { 7, 10 };
                int maxValue[2] = { 4, 5 };
                
                minValue[0] = 6; minValue[1] = 12;
				maxValue[0] = 4; maxValue[1] = 9;

                Character.AttackDamageMinRight = (Strength/minValue[0])+Energy/minValue[1];
                Character.AttackDamageMaxRight = (Strength/maxValue[0])+Energy/maxValue[1];
                Character.AttackDamageMinLeft  = (Strength/minValue[0])+Energy/minValue[1];
                Character.AttackDamageMaxLeft  = (Strength/maxValue[0])+Energy/maxValue[1];
            }
            break;
        case CLASS_DARK_LORD:
			Character.AttackDamageMinRight = Strength/7+Energy/14;
			Character.AttackDamageMaxRight = Strength/5+Energy/10;
			Character.AttackDamageMinLeft  = Strength/7+Energy/14;
			Character.AttackDamageMaxLeft  = Strength/5+Energy/10;
            break;
			
		case CLASS_SUMMONER:
			Character.AttackDamageMinRight = (Strength+Dexterity)/7;
			Character.AttackDamageMaxRight = (Strength+Dexterity)/4;
			Character.AttackDamageMinLeft  = (Strength+Dexterity)/7;
			Character.AttackDamageMaxLeft  = (Strength+Dexterity)/4;
			break;
#ifdef PBG_ADD_NEWCHAR_MONK
		case CLASS_RAGEFIGHTER:
			{
			Character.AttackDamageMinRight = Strength/7 + Vitality/15;
			Character.AttackDamageMaxRight = Strength/5 + Vitality/12;
			Character.AttackDamageMinLeft  = Strength/7 + Vitality/15;
			Character.AttackDamageMaxLeft  = Strength/5 + Vitality/12;
			}
			break;
#endif //PBG_ADD_NEWCHAR_MONK
        default :
			Character.AttackDamageMinRight = Strength/8;
			Character.AttackDamageMaxRight = Strength/4;
			Character.AttackDamageMinLeft  = Strength/8;
			Character.AttackDamageMaxLeft  = Strength/4;
            break;
        }
	}

    //  날개.
    if ( Equipment[EQUIPMENT_WING].Type!=-1 )
    {
        //  내구력에 따른 공격력 감소.
        ITEM_ATTRIBUTE *p = &ItemAttribute[Equipment[EQUIPMENT_WING].Type];
        if ( p->Durability!=0 
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_DAMAGE 
			&& IsRequireEquipItem(&Equipment[EQUIPMENT_WING])
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_DAMAGE
			)
        {
            float   percent = CalcDurabilityPercent(Equipment[EQUIPMENT_WING].Durability,p->Durability,Equipment[EQUIPMENT_WING].Level,0,0);//Equipment[EQUIPMENT_WING].Option1);
            
            DamageMin = 0; DamageMax = 0;
            PlusSpecial(&DamageMin,AT_IMPROVE_DAMAGE,&Equipment[EQUIPMENT_WING]);
            PlusSpecial(&DamageMax,AT_IMPROVE_DAMAGE,&Equipment[EQUIPMENT_WING]);
            
            DamageMin = DamageMin - (WORD)(DamageMin*percent);
            DamageMax = DamageMax - (WORD)(DamageMax*percent);
            
            Character.AttackDamageMinRight += DamageMin;
            Character.AttackDamageMaxRight += DamageMax;
            Character.AttackDamageMinLeft  += DamageMin;
            Character.AttackDamageMaxLeft  += DamageMax;
        }
    }

	// 오른손
    //  내구력이 0가 아닐때만 적용.
	if ( Right->Type!=-1 && Right->Durability!=0
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_DAMAGE 
		&& IsRequireEquipItem( Right )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_DAMAGE
		)
	{
        //  내구력에 따른 공격력 감소.
        ITEM_ATTRIBUTE *p = &ItemAttribute[Right->Type];
        float   percent = CalcDurabilityPercent(Right->Durability,p->Durability,Right->Level,Right->Option1,Right->ExtOption);

        //  아이템 공격력.
        DamageMin = Right->DamageMin;
        DamageMax = Right->DamageMax;

		// 추가공격력 옵션
		PlusSpecial(&DamageMin,AT_IMPROVE_DAMAGE,Right);
		PlusSpecial(&DamageMax,AT_IMPROVE_DAMAGE,Right);

        DamageMin = DamageMin - (WORD)(DamageMin*percent);
        DamageMax = DamageMax - (WORD)(DamageMax*percent);

		//	지팡이의 공격력 감소.
		if ( Right->Type>=ITEM_STAFF && Right->Type<=ITEM_STAFF+MAX_ITEM_INDEX )
		{
			Character.AttackDamageMinLeft += (WORD)(DamageMin);
			Character.AttackDamageMaxLeft += (WORD)(DamageMax);
		}
		else
		{
			Character.AttackDamageMinRight += DamageMin;
			Character.AttackDamageMaxRight += DamageMax;
		}

        //  액설런트.
		PlusSpecial(&Character.AttackDamageMinRight,AT_IMPROVE_DAMAGE_LEVEL,Right);
		PlusSpecial(&Character.AttackDamageMaxRight,AT_IMPROVE_DAMAGE_LEVEL,Right);
		PlusSpecialPercent(&Character.AttackDamageMinRight,AT_IMPROVE_DAMAGE_PERCENT,Right,2);
		PlusSpecialPercent(&Character.AttackDamageMaxRight,AT_IMPROVE_DAMAGE_PERCENT,Right,2);
	}

	// 왼손
    if ( Left->Type!=-1 && Left->Durability!=0
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_DAMAGE 
		&& IsRequireEquipItem( Left )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_DAMAGE	
		)
	{
        //  내구력에 따른 공격력 감소.
        ITEM_ATTRIBUTE *p = &ItemAttribute[Left->Type];
        float   percent = CalcDurabilityPercent(Left->Durability,p->Durability,Left->Level,Left->Option1,Left->ExtOption);

        //  아이템 공격력.
        DamageMin = Left->DamageMin;
        DamageMax = Left->DamageMax;

		PlusSpecial(&DamageMin,AT_IMPROVE_DAMAGE,Left);
		PlusSpecial(&DamageMax,AT_IMPROVE_DAMAGE,Left);

        DamageMin = DamageMin - (WORD)(DamageMin*percent);
        DamageMax = DamageMax - (WORD)(DamageMax*percent);

		//	지팡이의 공격력 감소.
		if ( Left->Type>=ITEM_STAFF && Left->Type<=ITEM_STAFF+MAX_ITEM_INDEX )
		{
			Character.AttackDamageMinLeft += (WORD)(DamageMin);
			Character.AttackDamageMaxLeft += (WORD)(DamageMax);
		}
		else
		{
			Character.AttackDamageMinLeft += DamageMin;
			Character.AttackDamageMaxLeft += DamageMax;
		}

        //  액설런트.
		PlusSpecial(&Character.AttackDamageMinLeft,AT_IMPROVE_DAMAGE_LEVEL,Left);
		PlusSpecial(&Character.AttackDamageMaxLeft,AT_IMPROVE_DAMAGE_LEVEL,Left);
		PlusSpecialPercent(&Character.AttackDamageMinLeft,AT_IMPROVE_DAMAGE_PERCENT,Left,2);
		PlusSpecialPercent(&Character.AttackDamageMaxLeft,AT_IMPROVE_DAMAGE_PERCENT,Left,2);
	}

    //  불의 목걸이 액설런트 적용.
    if ( Amulet->Type!=-1 && Amulet->Durability!=0 
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_DAMAGE 
		&& IsRequireEquipItem( Amulet )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_DAMAGE	
		)
	{
        //  오른손.
		PlusSpecial(&Character.AttackDamageMinRight,AT_IMPROVE_DAMAGE_LEVEL,Amulet);
		PlusSpecial(&Character.AttackDamageMaxRight,AT_IMPROVE_DAMAGE_LEVEL,Amulet);
		PlusSpecialPercent(&Character.AttackDamageMinRight,AT_IMPROVE_DAMAGE_PERCENT,Amulet,2);
		PlusSpecialPercent(&Character.AttackDamageMaxRight,AT_IMPROVE_DAMAGE_PERCENT,Amulet,2);

        //  왼손.
		PlusSpecial(&Character.AttackDamageMinLeft,AT_IMPROVE_DAMAGE_LEVEL,Amulet);
		PlusSpecial(&Character.AttackDamageMaxLeft,AT_IMPROVE_DAMAGE_LEVEL,Amulet);
		PlusSpecialPercent(&Character.AttackDamageMinLeft,AT_IMPROVE_DAMAGE_PERCENT,Amulet,2);
		PlusSpecialPercent(&Character.AttackDamageMaxLeft,AT_IMPROVE_DAMAGE_PERCENT,Amulet,2);
	}
	// 마법사의 반지
    if ( RRing->Type!=-1 && RRing->Durability!=0
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_DAMAGE 
		&& IsRequireEquipItem( RRing )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_DAMAGE	
		)
	{
#ifdef YDG_ADD_CS7_CRITICAL_MAGIC_RING
        //  오른손.
		PlusSpecialPercent(&Character.AttackDamageMinRight,AT_IMPROVE_DAMAGE_PERCENT,RRing,RRing->SpecialValue[1]);
		PlusSpecialPercent(&Character.AttackDamageMaxRight,AT_IMPROVE_DAMAGE_PERCENT,RRing,RRing->SpecialValue[1]);

        //  왼손.
		PlusSpecialPercent(&Character.AttackDamageMinLeft,AT_IMPROVE_DAMAGE_PERCENT,RRing,RRing->SpecialValue[1]);
		PlusSpecialPercent(&Character.AttackDamageMaxLeft,AT_IMPROVE_DAMAGE_PERCENT,RRing,RRing->SpecialValue[1]);
#else	// YDG_ADD_CS7_CRITICAL_MAGIC_RING
        //  오른손.
		PlusSpecialPercent(&Character.AttackDamageMinRight,AT_IMPROVE_DAMAGE_PERCENT,RRing,10);
		PlusSpecialPercent(&Character.AttackDamageMaxRight,AT_IMPROVE_DAMAGE_PERCENT,RRing,10);

        //  왼손.
		PlusSpecialPercent(&Character.AttackDamageMinLeft,AT_IMPROVE_DAMAGE_PERCENT,RRing,10);
		PlusSpecialPercent(&Character.AttackDamageMaxLeft,AT_IMPROVE_DAMAGE_PERCENT,RRing,10);
#endif	// YDG_ADD_CS7_CRITICAL_MAGIC_RING
	}
    if ( LRing->Type!=-1 && LRing->Durability!=0 
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_DAMAGE 
		&& IsRequireEquipItem( LRing )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_DAMAGE	
		)
	{
#ifdef YDG_ADD_CS7_CRITICAL_MAGIC_RING
        //  오른손.
		PlusSpecialPercent(&Character.AttackDamageMinRight,AT_IMPROVE_DAMAGE_PERCENT,LRing,LRing->SpecialValue[1]);
		PlusSpecialPercent(&Character.AttackDamageMaxRight,AT_IMPROVE_DAMAGE_PERCENT,LRing,LRing->SpecialValue[1]);

        //  왼손.
		PlusSpecialPercent(&Character.AttackDamageMinLeft,AT_IMPROVE_DAMAGE_PERCENT,LRing,LRing->SpecialValue[1]);
		PlusSpecialPercent(&Character.AttackDamageMaxLeft,AT_IMPROVE_DAMAGE_PERCENT,LRing,LRing->SpecialValue[1]);
#else	// YDG_ADD_CS7_CRITICAL_MAGIC_RING
        //  오른손.
		PlusSpecialPercent(&Character.AttackDamageMinRight,AT_IMPROVE_DAMAGE_PERCENT,LRing,10);
		PlusSpecialPercent(&Character.AttackDamageMaxRight,AT_IMPROVE_DAMAGE_PERCENT,LRing,10);

        //  왼손.
		PlusSpecialPercent(&Character.AttackDamageMinLeft,AT_IMPROVE_DAMAGE_PERCENT,LRing,10);
		PlusSpecialPercent(&Character.AttackDamageMaxLeft,AT_IMPROVE_DAMAGE_PERCENT,LRing,10);
#endif	// YDG_ADD_CS7_CRITICAL_MAGIC_RING
	}

	//	세트 옵션을 적용한다.
    WORD    Damage = 0;
	
	
#ifdef LDS_FIX_SETITEM_WRONG_CALCULATION_IMPROVEATTACKVALUE		//  세트 옵션 적용 전 적용여부 flag table 을 초기화 한다.
	memset ( g_csItemOption.m_bySetOptionListOnOff, 0, sizeof( BYTE )* 16 );
#endif // LDS_FIX_SETITEM_WRONG_CALCULATION_IMPROVEATTACKVALUE

	g_csItemOption.PlusSpecial ( &Damage,  AT_SET_OPTION_IMPROVE_ATTACK_MIN );	//	최소 공격력 증가.
    Character.AttackDamageMinLeft += Damage;
    Character.AttackDamageMinRight+= Damage;

    Damage = 0;
    g_csItemOption.PlusSpecial ( &Damage,  AT_SET_OPTION_IMPROVE_ATTACK_MAX );	//	최대 공격력 증가.
    Character.AttackDamageMaxLeft += Damage;
    Character.AttackDamageMaxRight+= Damage;

    Damage = 0;
	g_csItemOption.PlusSpecialLevel ( &Damage, Dexterity, AT_SET_OPTION_IMPROVE_ATTACK_1 );	//	특정 캐릭터 공격력 증가 민첩성/60		
    Character.AttackDamageMinRight += Damage;
    Character.AttackDamageMaxRight += Damage;
    Character.AttackDamageMinLeft  += Damage;
    Character.AttackDamageMaxLeft  += Damage;
	
    Damage = 0;
	g_csItemOption.PlusSpecialLevel ( &Damage, Strength, AT_SET_OPTION_IMPROVE_ATTACK_2 );	//	특정 캐릭터 공격력 증가 힘/50		    
    Character.AttackDamageMinRight += Damage;
    Character.AttackDamageMaxRight += Damage;
    Character.AttackDamageMinLeft  += Damage;
    Character.AttackDamageMaxLeft  += Damage;

    //  화살에 붙은 추가 공격력.
    if ( ( Right->Type>=ITEM_BOW && Right->Type<ITEM_BOW+MAX_ITEM_INDEX ) &&
         ( Left->Type>=ITEM_BOW && Left->Type<ITEM_BOW+MAX_ITEM_INDEX ) 
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_DAMAGE 
		 && IsRequireEquipItem( Right ) && IsRequireEquipItem( Left ) 
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_DAMAGE	
		 )
    {
        int LLevel = ((Left->Level>>3)&15);
        int RLevel = ((Right->Level>>3)&15);

        //  ARROW의 LEVEL이 1이면은 공격력 증가. 공격력3% + 1 증가.
        if ( Left->Type==ITEM_BOW+7 && LLevel>=1 )
        {
            Character.AttackDamageMinRight += (WORD)(Character.AttackDamageMinRight*((LLevel*2+1)*0.01f)+1);
            Character.AttackDamageMaxRight += (WORD)(Character.AttackDamageMaxRight*((LLevel*2+1)*0.01f)+1);
        }
        else
        if ( Right->Type==ITEM_BOW+15 && RLevel>=1 )
        {
            Character.AttackDamageMinLeft += (WORD)(Character.AttackDamageMinLeft*((RLevel*2+1)*0.01f)+1);
            Character.AttackDamageMaxLeft += (WORD)(Character.AttackDamageMaxLeft*((RLevel*2+1)*0.01f)+1);
        }
    }
#ifdef PSW_SCROLL_ITEM
	if(g_isCharacterBuff((&Hero->Object), eBuff_EliteScroll3))  // 분노의 스크롤
	{
			ITEM_ADD_OPTION Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ITEM_POTION + 74);
			Character.AttackDamageMinRight += Item_data.m_byValue1;
			Character.AttackDamageMaxRight += Item_data.m_byValue1;
			Character.AttackDamageMinLeft  += Item_data.m_byValue1;
			Character.AttackDamageMaxLeft  += Item_data.m_byValue1;
	}
#endif// PSW_SCROLL_ITEM
#ifdef HELLOWIN_EVENT
	if(g_isCharacterBuff((&Hero->Object), eBuff_Hellowin2)) // 잭오랜턴의 분노
	{
		ITEM_ADD_OPTION Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ITEM_POTION + 47);
		Character.AttackDamageMinRight += Item_data.m_byValue1;
		Character.AttackDamageMaxRight += Item_data.m_byValue1;
		Character.AttackDamageMinLeft  += Item_data.m_byValue1;
		Character.AttackDamageMaxLeft  += Item_data.m_byValue1;
	}
#endif //HELLOWIN_EVENT
#ifdef CSK_EVENT_CHERRYBLOSSOM
	if(g_isCharacterBuff((&Hero->Object), eBuff_CherryBlossom_Petal))  // 벚꽃꽃잎
	{
		const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ITEM_POTION + 87);
		Character.AttackDamageMinRight += Item_data.m_byValue1;
		Character.AttackDamageMaxRight += Item_data.m_byValue1;
		Character.AttackDamageMinLeft  += Item_data.m_byValue1;
		Character.AttackDamageMaxLeft  += Item_data.m_byValue1;
	}
#endif //CSK_EVENT_CHERRYBLOSSOM
#ifdef ADD_SOCKET_STATUS_BONUS
	Character.AttackDamageMinRight += g_SocketItemMgr.m_StatusBonus.m_iAttackDamageMinBonus;
	Character.AttackDamageMaxRight += g_SocketItemMgr.m_StatusBonus.m_iAttackDamageMaxBonus;
	Character.AttackDamageMinLeft  += g_SocketItemMgr.m_StatusBonus.m_iAttackDamageMinBonus;
	Character.AttackDamageMaxLeft  += g_SocketItemMgr.m_StatusBonus.m_iAttackDamageMaxBonus;
#endif	// ADD_SOCKET_STATUS_BONUS
#ifdef PBG_ADD_SANTABUFF
	if(g_isCharacterBuff((&Hero->Object), eBuff_BlessingOfXmax))	//크리스마스의 축복
	{
		int _Temp = 0;
		_Temp = Character.Level / 3 +45;

		Character.AttackDamageMinRight += _Temp;
		Character.AttackDamageMaxRight += _Temp;
		Character.AttackDamageMinLeft += _Temp;
		Character.AttackDamageMaxLeft += _Temp;
	}

	if(g_isCharacterBuff((&Hero->Object), eBuff_StrengthOfSanta))	//산타의 강화
	{
		int _Temp = 30;
		
		Character.AttackDamageMinRight += _Temp;
		Character.AttackDamageMaxRight += _Temp;
		Character.AttackDamageMinLeft += _Temp;
		Character.AttackDamageMaxLeft += _Temp;

	}
#endif //PBG_ADD_SANTABUFF

#ifdef LDS_ADD_PCROOM_ITEM_JPN_6TH
	if(g_isCharacterBuff((&Hero->Object), eBuff_StrongCharm))	// 강함의 인장 (PC방 아이템)	// 강함의 인장은 공격력 + 50 
	{
		int _Temp = 50;
		
		Character.AttackDamageMinRight += _Temp;
		Character.AttackDamageMaxRight += _Temp;
		Character.AttackDamageMinLeft += _Temp;
		Character.AttackDamageMaxLeft += _Temp;
	}
#endif // LDS_ADD_PCROOM_ITEM_JPN_6TH
}

void CHARACTER_MACHINE::CalculateCriticalDamage()
{
	Character.CriticalDamage = 0;
	PlusSpecial(&Character.CriticalDamage,AT_LUCK,&Equipment[EQUIPMENT_WEAPON_RIGHT]);
	PlusSpecial(&Character.CriticalDamage,AT_LUCK,&Equipment[EQUIPMENT_WEAPON_LEFT]);
	PlusSpecial(&Character.CriticalDamage,AT_LUCK,&Equipment[EQUIPMENT_HELM  ]);
	PlusSpecial(&Character.CriticalDamage,AT_LUCK,&Equipment[EQUIPMENT_ARMOR ]);
	PlusSpecial(&Character.CriticalDamage,AT_LUCK,&Equipment[EQUIPMENT_PANTS ]);
    PlusSpecial(&Character.CriticalDamage,AT_LUCK,&Equipment[EQUIPMENT_GLOVES]);
	PlusSpecial(&Character.CriticalDamage,AT_LUCK,&Equipment[EQUIPMENT_BOOTS ]);
	PlusSpecial(&Character.CriticalDamage,AT_LUCK,&Equipment[EQUIPMENT_WING  ]);
}

void CHARACTER_MACHINE::CalculateMagicDamage()
{
	WORD Energy;
	Energy	 = Character.Energy   + Character.AddEnergy;  
	Character.MagicDamageMin = Energy/9;
	Character.MagicDamageMax = Energy/4;

	ITEM* Right  = &Equipment[EQUIPMENT_WEAPON_RIGHT];
#ifdef KJH_FIX_WOPS_K27580_EQUIPED_SUMMONING_ITEM
	ITEM* Left	= &Equipment[EQUIPMENT_WEAPON_LEFT];
#endif // KJH_FIX_WOPS_K27580_EQUIPED_SUMMONING_ITEM
	ITEM* Amulet = &Equipment[EQUIPMENT_AMULET];
	ITEM* RRing  = &Equipment[EQUIPMENT_RING_RIGHT];
	ITEM* LRing  = &Equipment[EQUIPMENT_RING_LEFT];

#ifndef ASG_FIX_PICK_ITEM_FROM_INVEN	// 정리시 삭제.
	// 아이템을 든 경우
	SEASON3B::CNewUIPickedItem* pPickedItem = SEASON3B::CNewUIInventoryCtrl::GetPickedItem();
	// 인벤토리에서 온 것인가?
	if ( pPickedItem && pPickedItem->GetOwnerInventory() == g_pMyInventory->GetInventoryCtrl())
	{	
		ITEM* pItemObj = pPickedItem->GetItem();
		
		switch ( pPickedItem->GetSourceLinealPos() ) {
		case EQUIPMENT_WEAPON_RIGHT:
			Right = pItemObj;
			break;
#ifdef KJH_FIX_WOPS_K27580_EQUIPED_SUMMONING_ITEM
		case EQUIPMENT_WEAPON_LEFT:
			Left = pItemObj;
			break;
#endif // KJH_FIX_WOPS_K27580_EQUIPED_SUMMONING_ITEM
		case EQUIPMENT_AMULET:
			Amulet = pItemObj;
			break;
		case EQUIPMENT_RING_RIGHT:
			RRing = pItemObj;
			break;
		case EQUIPMENT_RING_LEFT:
			LRing = pItemObj;
			break;
		}
	}
#endif	// ASG_FIX_PICK_ITEM_FROM_INVEN

    float   percent;
    WORD    DamageMin = 0;
    WORD    DamageMax = 0;

	// 날개
    if ( Equipment[EQUIPMENT_WING].Type!=-1 
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_MAGICDAMAGE
		&& Equipment[EQUIPMENT_WING].Durability!=0 && IsRequireEquipItem( &Equipment[EQUIPMENT_WING] )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_MAGICDAMAGE
		)
    {
        //  내구력에 따른 공격력 감소.
        ITEM_ATTRIBUTE *p = &ItemAttribute[Equipment[EQUIPMENT_WING].Type];
        ITEM *ipWing = &Equipment[EQUIPMENT_WING];
        percent = CalcDurabilityPercent(ipWing->Durability,p->Durability,ipWing->Level,0);//ipWing->Option1);

        DamageMin = 0;
        DamageMax = 0;
	    PlusSpecial(&DamageMin,AT_IMPROVE_MAGIC,&Equipment[EQUIPMENT_WING]);
	    PlusSpecial(&DamageMax,AT_IMPROVE_MAGIC,&Equipment[EQUIPMENT_WING]);

        DamageMin = DamageMin - (WORD)(DamageMin*percent);
        DamageMax = DamageMax - (WORD)(DamageMax*percent);

        Character.MagicDamageMin += DamageMin;
        Character.MagicDamageMax += DamageMax;
    }

	// 오른손 옵션 검사
    if ( Right->Type!=-1 && Right->Durability!=0
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_MAGICDAMAGE
		&& IsRequireEquipItem( Right )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_MAGICDAMAGE	
		)
	{
        ITEM_ATTRIBUTE *p = &ItemAttribute[Right->Type];
        percent = CalcDurabilityPercent(Right->Durability,p->Durability,Right->Level,Right->Option1,Right->ExtOption);
        DamageMin = 0; DamageMax = 0;
		//	마검사 마법검
		// 추가공격력 옵션 (마검사용검은 추가공격력옵션이 추가마력도 올려준다.)
		if ( Right->Type==ITEM_SWORD+21 
			|| Right->Type==ITEM_SWORD+31 
			|| Right->Type==ITEM_SWORD+23
			|| Right->Type==ITEM_SWORD+25
#ifdef KJH_FIX_RUNE_BASTARD_SWORD_TOOLTIP
 			|| Right->Type==ITEM_SWORD+28
#endif // KJH_FIX_RUNE_BASTARD_SWORD_TOOLTIP
			) // 이혁재 - 2005.9.5 ITEM_SWORD+23 추가
		{
			PlusSpecial(&DamageMin,AT_IMPROVE_DAMAGE,Right);
			PlusSpecial(&DamageMax,AT_IMPROVE_DAMAGE,Right);
		}
		else
		{
			PlusSpecial(&DamageMin,AT_IMPROVE_MAGIC,Right);
			PlusSpecial(&DamageMax,AT_IMPROVE_MAGIC,Right);
		}

		Character.MagicDamageMin += DamageMin - (WORD)(DamageMin*percent);
		Character.MagicDamageMax += DamageMax - (WORD)(DamageMax*percent);

		// 액설런트 옵션
        PlusSpecial(&Character.MagicDamageMin,AT_IMPROVE_MAGIC_LEVEL,Right);
		PlusSpecial(&Character.MagicDamageMax,AT_IMPROVE_MAGIC_LEVEL,Right);
		PlusSpecialPercent(&Character.MagicDamageMin,AT_IMPROVE_MAGIC_PERCENT,Right,2);
		PlusSpecialPercent(&Character.MagicDamageMax,AT_IMPROVE_MAGIC_PERCENT,Right,2);
	}

#ifdef PBG_FIX_SHIELD_MAGICDAMAGE
	// 왼손 옵션 검사
    if ( Left->Type!=-1 && Left->Durability!=0
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_MAGICDAMAGE
		&& IsRequireEquipItem( Left )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_MAGICDAMAGE	
		)
	{
		// 소환술사의 왼손은 CalculateCurseDamage()에서 계산 
		if (CLASS_SUMMONER != GetBaseClass(Character.Class))	
		{
			ITEM_ATTRIBUTE *p = &ItemAttribute[Left->Type];
			percent = CalcDurabilityPercent(Left->Durability,p->Durability,Left->Level,Left->Option1,Left->ExtOption);
			DamageMin = 0; DamageMax = 0;
			
			if( Left->Type >= ITEM_SWORD && Left->Type < ITEM_SHIELD )
			{
				//	마검사 마법검
				// 추가공격력 옵션 (마검사용검은 추가공격력옵션이 추가마력도 올려준다.)
				// (다른마검사용검은 두손검이라 검사할 팰요가 없다)
				if ( Left->Type==ITEM_SWORD+28 ) 
				{
					PlusSpecial(&DamageMin,AT_IMPROVE_DAMAGE,Left);
					PlusSpecial(&DamageMax,AT_IMPROVE_DAMAGE,Left);
				}
				else
				{
					PlusSpecial(&DamageMin,AT_IMPROVE_MAGIC,Left);
					PlusSpecial(&DamageMax,AT_IMPROVE_MAGIC,Left);
				}
				
				if (GetBaseClass(Character.Class) != CLASS_DARK	// 마검사의 왼손 무기는 마력에 영향을 주지 못한다 (엑옵제외)
					)
				{
					Character.MagicDamageMin += DamageMin - (WORD)(DamageMin*percent);
					Character.MagicDamageMax += DamageMax - (WORD)(DamageMax*percent);
				}
				
				// 액설런트 옵션
				PlusSpecial(&Character.MagicDamageMin,AT_IMPROVE_MAGIC_LEVEL,Left);
				PlusSpecial(&Character.MagicDamageMax,AT_IMPROVE_MAGIC_LEVEL,Left);
				PlusSpecialPercent(&Character.MagicDamageMin,AT_IMPROVE_MAGIC_PERCENT,Left,2);
				PlusSpecialPercent(&Character.MagicDamageMax,AT_IMPROVE_MAGIC_PERCENT,Left,2);
			}
		}
	}
#else // PBG_FIX_SHIELD_MAGICDAMAGE
#ifdef KJH_FIX_WOPS_K27580_EQUIPED_SUMMONING_ITEM
	// 왼손 옵션 검사
    if ( Left->Type!=-1 && Left->Durability!=0
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_MAGICDAMAGE
		&& IsRequireEquipItem( Left )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_MAGICDAMAGE	
		)
	{
		// 소환술사의 왼손은 CalculateCurseDamage()에서 계산 
		if (CLASS_SUMMONER != GetBaseClass(Character.Class))	
		{
			ITEM_ATTRIBUTE *p = &ItemAttribute[Left->Type];
			percent = CalcDurabilityPercent(Left->Durability,p->Durability,Left->Level,Left->Option1,Left->ExtOption);
			DamageMin = 0; DamageMax = 0;
			
			//	마검사 마법검
			// 추가공격력 옵션 (마검사용검은 추가공격력옵션이 추가마력도 올려준다.)
			// (다른마검사용검은 두손검이라 검사할 팰요가 없다)
			if ( Right->Type==ITEM_SWORD+28
				) 
			{
				PlusSpecial(&DamageMin,AT_IMPROVE_DAMAGE,Right);
				PlusSpecial(&DamageMax,AT_IMPROVE_DAMAGE,Right);
			}
			else
			{
				PlusSpecial(&DamageMin,AT_IMPROVE_MAGIC,Right);
				PlusSpecial(&DamageMax,AT_IMPROVE_MAGIC,Right);
			}
			
#ifdef YDG_FIX_LEFTHAND_MAGICDAMAGE
			if (GetBaseClass(Character.Class) != CLASS_DARK	// 마검사의 왼손 무기는 마력에 영향을 주지 못한다 (엑옵제외)
			)
#endif	// YDG_FIX_LEFTHAND_MAGICDAMAGE
			{
				Character.MagicDamageMin += DamageMin - (WORD)(DamageMin*percent);
				Character.MagicDamageMax += DamageMax - (WORD)(DamageMax*percent);
			}
			
			// 액설런트 옵션
			PlusSpecial(&Character.MagicDamageMin,AT_IMPROVE_MAGIC_LEVEL,Left);
			PlusSpecial(&Character.MagicDamageMax,AT_IMPROVE_MAGIC_LEVEL,Left);
			PlusSpecialPercent(&Character.MagicDamageMin,AT_IMPROVE_MAGIC_PERCENT,Left,2);
			PlusSpecialPercent(&Character.MagicDamageMax,AT_IMPROVE_MAGIC_PERCENT,Left,2);
		}
	}
#endif // KJH_FIX_WOPS_K27580_EQUIPED_SUMMONING_ITEM
#endif // PBG_FIX_SHIELD_MAGICDAMAGE

	// 목걸이
    if ( Amulet->Type!=-1 && Amulet->Durability!=0 
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_MAGICDAMAGE
		&& IsRequireEquipItem( Amulet )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_MAGICDAMAGE		
		)
	{
		PlusSpecial(&Character.MagicDamageMin,AT_IMPROVE_MAGIC_LEVEL,Amulet);
		PlusSpecial(&Character.MagicDamageMax,AT_IMPROVE_MAGIC_LEVEL,Amulet);
		PlusSpecialPercent(&Character.MagicDamageMin,AT_IMPROVE_MAGIC_PERCENT,Amulet,2);
		PlusSpecialPercent(&Character.MagicDamageMax,AT_IMPROVE_MAGIC_PERCENT,Amulet,2);
	}
	// 마법사의 반지
    if ( RRing->Type!=-1 && RRing->Durability!=0 
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_MAGICDAMAGE
		&& IsRequireEquipItem( RRing )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_MAGICDAMAGE
		)
	{
#ifdef YDG_ADD_CS7_CRITICAL_MAGIC_RING
		PlusSpecialPercent(&Character.MagicDamageMin,AT_IMPROVE_MAGIC_PERCENT,RRing,RRing->SpecialValue[0]);
		PlusSpecialPercent(&Character.MagicDamageMax,AT_IMPROVE_MAGIC_PERCENT,RRing,RRing->SpecialValue[0]);
#else	// YDG_ADD_CS7_CRITICAL_MAGIC_RING
		PlusSpecialPercent(&Character.MagicDamageMin,AT_IMPROVE_MAGIC_PERCENT,RRing,10);
		PlusSpecialPercent(&Character.MagicDamageMax,AT_IMPROVE_MAGIC_PERCENT,RRing,10);
#endif	// YDG_ADD_CS7_CRITICAL_MAGIC_RING
	}
    if ( LRing->Type!=-1 && LRing->Durability!=0 
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_MAGICDAMAGE
		&& IsRequireEquipItem( LRing )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_MAGICDAMAGE
		)
	{
#ifdef YDG_ADD_CS7_CRITICAL_MAGIC_RING
		PlusSpecialPercent(&Character.MagicDamageMin,AT_IMPROVE_MAGIC_PERCENT,LRing,LRing->SpecialValue[0]);
		PlusSpecialPercent(&Character.MagicDamageMax,AT_IMPROVE_MAGIC_PERCENT,LRing,LRing->SpecialValue[0]);
#else	// YDG_ADD_CS7_CRITICAL_MAGIC_RING
		PlusSpecialPercent(&Character.MagicDamageMin,AT_IMPROVE_MAGIC_PERCENT,LRing,10);
		PlusSpecialPercent(&Character.MagicDamageMax,AT_IMPROVE_MAGIC_PERCENT,LRing,10);
#endif	// YDG_ADD_CS7_CRITICAL_MAGIC_RING
	}

	//	세트 옵션을 적용한다.
	WORD MagicDamage = 0;

    MagicDamage = 0;
    g_csItemOption.PlusSpecialLevel ( &MagicDamage, Energy, AT_SET_OPTION_IMPROVE_MAGIC );	//	특정 캐릭터 마력 증가 에너지/70
	Character.MagicDamageMin += MagicDamage;
	Character.MagicDamageMax += MagicDamage;

#ifdef PSW_SCROLL_ITEM
	if( g_isCharacterBuff((&Hero->Object), eBuff_EliteScroll4) )
	{
		ITEM_ADD_OPTION Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ITEM_POTION + 75);
		Character.MagicDamageMin += Item_data.m_byValue1;
		Character.MagicDamageMax += Item_data.m_byValue1;
	}
#endif //PSW_SCROLL_ITEM
	if(g_isCharacterBuff((&Hero->Object), eBuff_Hellowin2)) // 잭오랜턴의 분노
	{
		ITEM_ADD_OPTION Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ITEM_POTION + 47);
		Character.MagicDamageMin += Item_data.m_byValue1;
		Character.MagicDamageMax += Item_data.m_byValue1;
	}
#ifdef CSK_EVENT_CHERRYBLOSSOM
	if(g_isCharacterBuff((&Hero->Object), eBuff_CherryBlossom_Petal))  // 벚꽃꽃잎
	{
		const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ITEM_POTION + 87);
		Character.MagicDamageMin += Item_data.m_byValue1;
		Character.MagicDamageMax += Item_data.m_byValue1;
	}
#endif //CSK_EVENT_CHERRYBLOSSOM
#ifdef KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
	if(g_isCharacterBuff((&Hero->Object), eBuff_SwellOfMagicPower))  // 마력증대
	{
		int iAdd_MP = (CharacterAttribute->Energy/9)*0.2f;
		Character.MagicDamageMin += iAdd_MP;
	}
#endif // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
#ifdef ADD_SOCKET_STATUS_BONUS
	Character.MagicDamageMin += g_SocketItemMgr.m_StatusBonus.m_iSkillAttackDamageBonus;
	Character.MagicDamageMax += g_SocketItemMgr.m_StatusBonus.m_iSkillAttackDamageBonus;
#ifdef YDG_FIX_SOCKET_MISSING_MAGIC_POWER_BONUS
	Character.MagicDamageMin += g_SocketItemMgr.m_StatusBonus.m_iMagicPowerMinBonus;
	Character.MagicDamageMax += g_SocketItemMgr.m_StatusBonus.m_iMagicPowerMaxBonus;
#else	// YDG_FIX_SOCKET_MISSING_MAGIC_POWER_BONUS
	Character.MagicDamageMin += g_SocketItemMgr.m_StatusBonus.m_iMagicPowerBonus;
	Character.MagicDamageMax += g_SocketItemMgr.m_StatusBonus.m_iMagicPowerBonus;
#endif	// YDG_FIX_SOCKET_MISSING_MAGIC_POWER_BONUS
#endif	// ADD_SOCKET_STATUS_BONUS
#ifdef YDG_ADD_SANTABUFF_MAGIC_DAMAGE
	if(g_isCharacterBuff((&Hero->Object), eBuff_StrengthOfSanta))	//산타의 강화
	{
		int _Temp = 30;

		Character.MagicDamageMin += _Temp;
		Character.MagicDamageMax += _Temp;
	}
#endif //YDG_ADD_SANTABUFF_MAGIC_DAMAGE
}

// 저주력 데미지 계산.
void CHARACTER_MACHINE::CalculateCurseDamage()
{
	if (CLASS_SUMMONER != GetBaseClass(Character.Class))	// 소환술사만 저주력이 필요하다.
		return;

	WORD wEnergy = Character.Energy + Character.AddEnergy;  
	Character.CurseDamageMin = wEnergy / 9;
	Character.CurseDamageMax = wEnergy / 4;

	// 날개
	ITEM* pEquipWing = &Equipment[EQUIPMENT_WING];
    if (pEquipWing->Type != -1
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_CURSEDAMAGE
		&& IsRequireEquipItem( pEquipWing )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_CURSEDAMAGE		
		)
    {
        ITEM_ATTRIBUTE* pAttribute = &ItemAttribute[pEquipWing->Type];
		WORD wDamageMin = 0;
		WORD wDamageMax = 0;

		// 추가 저주력 계산.
		::PlusSpecial(&wDamageMin, AT_IMPROVE_CURSE, pEquipWing);
		::PlusSpecial(&wDamageMax, AT_IMPROVE_CURSE, pEquipWing);

		// 내구력에 따른 공격력 감소.
        float fPercent = ::CalcDurabilityPercent(pEquipWing->Durability,
			pAttribute->Durability, pEquipWing->Level, 0);
        wDamageMin -= WORD(wDamageMin * fPercent);
        wDamageMax -= WORD(wDamageMax * fPercent);

        Character.CurseDamageMin += wDamageMin;
        Character.CurseDamageMax += wDamageMax;
    }

	// 소환서
	ITEM* pEquipLeft = &Equipment[EQUIPMENT_WEAPON_LEFT];
	if (pEquipLeft->Type != -1 && pEquipLeft->Durability != 0
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_CURSEDAMAGE
		&& IsRequireEquipItem( pEquipLeft )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_CURSEDAMAGE
		)
	{
        ITEM_ATTRIBUTE* pAttribute = &ItemAttribute[pEquipLeft->Type];
		WORD wDamageMin = 0;
		WORD wDamageMax = 0;

		// 추가 저주력 계산.
		::PlusSpecial(&wDamageMin, AT_IMPROVE_CURSE, pEquipLeft);
		::PlusSpecial(&wDamageMax, AT_IMPROVE_CURSE, pEquipLeft);
		
		// 내구력에 따른 공격력 감소.
		float fPercent = CalcDurabilityPercent(pEquipLeft->Durability,
			pAttribute->Durability, pEquipLeft->Level, pEquipLeft->Option1,
			pEquipLeft->ExtOption);

        Character.CurseDamageMin += wDamageMin - WORD(wDamageMin * fPercent);
        Character.CurseDamageMax += wDamageMax - WORD(wDamageMax * fPercent);

#ifdef KJH_FIX_WOPS_K27580_EQUIPED_SUMMONING_ITEM
		// 액설런트 옵션
		PlusSpecial(&Character.CurseDamageMin,AT_IMPROVE_MAGIC_LEVEL,pEquipLeft);
		PlusSpecial(&Character.CurseDamageMax,AT_IMPROVE_MAGIC_LEVEL,pEquipLeft);
		PlusSpecialPercent(&Character.CurseDamageMin,AT_IMPROVE_MAGIC_PERCENT,pEquipLeft,2);
		PlusSpecialPercent(&Character.CurseDamageMax,AT_IMPROVE_MAGIC_PERCENT,pEquipLeft,2);
#endif // KJH_FIX_WOPS_K27580_EQUIPED_SUMMONING_ITEM
	}
#ifdef YDG_ADD_SANTABUFF_MAGIC_DAMAGE
	if(g_isCharacterBuff((&Hero->Object), eBuff_StrengthOfSanta))	//산타의 강화
	{
		int _Temp = 30;

		Character.CurseDamageMin += _Temp;
		Character.CurseDamageMax += _Temp;
	}
#endif //YDG_ADD_SANTABUFF_MAGIC_DAMAGE
}

void CHARACTER_MACHINE::CalculateAttackRating()
{
	WORD Strength, Dexterity, Charisma;

	Strength = Character.Strength + Character.AddStrength;
	Dexterity= Character.Dexterity+ Character.AddDexterity;
    Charisma = Character.Charisma + Character.AddCharisma;

    //  공격 성공율.
    if ( GetBaseClass( Character.Class )==CLASS_DARK_LORD )
	    Character.AttackRating  = ((Character.Level*5)+(Dexterity*5)/2)+(Strength/6)+(Charisma/10);
#ifdef PBG_ADD_NEWCHAR_MONK
	else if(GetBaseClass( Character.Class )==CLASS_RAGEFIGHTER)
	{
		Character.AttackRating = ((Character.Level*3)+(Dexterity*5)/4)+(Strength/6);
	}
#endif //PBG_ADD_NEWCHAR_MONK
    else
	    Character.AttackRating  = ((Character.Level*5)+(Dexterity*3)/2)+(Strength/4);

	//	세트 옵션을 적용한다.
	g_csItemOption.PlusSpecial ( &Character.AttackRating, AT_SET_OPTION_IMPROVE_ATTACKING_PERCENT );	//	공격 성공률 증가

#ifdef ADD_SOCKET_STATUS_BONUS
	Character.AttackRating += g_SocketItemMgr.m_StatusBonus.m_iAttackRateBonus;
#endif	// ADD_SOCKET_STATUS_BONUS
}

void CHARACTER_MACHINE::CalculateAttackRatingPK()
{
	WORD Dexterity;
	Dexterity= Character.Dexterity+ Character.AddDexterity;
    int CharacterClass = GetBaseClass ( Character.Class );

	float tmpf = 0.f;
	switch(CharacterClass)
	{
		case CLASS_KNIGHT:
			tmpf = (float)Character.Level * 3 + (float)Dexterity * 4.5f;
			break;
		case CLASS_DARK_LORD:
			tmpf = (float)Character.Level * 3 + (float)Dexterity * 4.f;
			break;
		case CLASS_ELF:
			tmpf = (float)Character.Level * 3 + (float)Dexterity * 0.6f;
			break;		
		case CLASS_DARK:
		case CLASS_SUMMONER:
			tmpf = (float)Character.Level * 3 + (float)Dexterity * 3.5f;
			break;
		case CLASS_WIZARD:
			tmpf = (float)Character.Level * 3 + (float)Dexterity * 4.f;
			break;
#ifdef PBG_ADD_NEWCHAR_MONK
		case CLASS_RAGEFIGHTER:
			tmpf = (float)Character.Level * 2.6f + (float)Dexterity * 3.6f;
			break;
#endif //PBG_ADD_NEWCHAR_MONK
	}

	Character.AttackRatingPK = (WORD)tmpf;

#ifndef YDG_FIX_SOCKET_BALANCE_PATCH
#ifdef ADD_SOCKET_STATUS_BONUS
	Character.AttackRatingPK += g_SocketItemMgr.m_StatusBonus.m_iPvPAttackRateBonus;
#endif	// ADD_SOCKET_STATUS_BONUS
#endif	// YDG_FIX_SOCKET_BALANCE_PATCH
}

void CHARACTER_MACHINE::CalculateAttackSpeed()
{
	WORD Dexterity;
	///////////////////////////////////////////// 캐릭터 기본 민첩 계산 /////////////////////////////////////////////
	Dexterity= Character.Dexterity+ Character.AddDexterity;

    int CharacterClass = GetBaseClass ( Character.Class );

	if ( CharacterClass==CLASS_ELF )    //  요정.
    {
		Character.AttackSpeed = Dexterity/50;
        Character.MagicSpeed = Dexterity/50;
    }
	else if ( CharacterClass==CLASS_KNIGHT || CharacterClass==CLASS_DARK )  //  20 --> 18  기사. 마검사.
    {
        Character.AttackSpeed = Dexterity/15;
        Character.MagicSpeed = Dexterity/20;
    }
	
    else if ( CharacterClass==CLASS_DARK_LORD )
    {
		Character.AttackSpeed = Dexterity/10;
        Character.MagicSpeed = Dexterity/10;
    }
	else if (CharacterClass == CLASS_SUMMONER)
    {
		Character.AttackSpeed = Dexterity/20;
        Character.MagicSpeed = Dexterity/20;
    }
#ifdef PBG_ADD_NEWCHAR_MONK
	else if(CharacterClass == CLASS_RAGEFIGHTER)
	{
		Character.AttackSpeed = Dexterity/9;
        Character.MagicSpeed = Dexterity/9;
	}
#endif //PBG_ADD_NEWCHAR_MONK
	else  //  법사.
    {
        Character.AttackSpeed = Dexterity/20;
        Character.MagicSpeed = Dexterity/10;
    }
	///////////////////////////////////////////// 캐릭터에 장착한 아이템 /////////////////////////////////////////////
	ITEM *r = &Equipment[EQUIPMENT_WEAPON_RIGHT];
	ITEM *l = &Equipment[EQUIPMENT_WEAPON_LEFT ];
	ITEM *g = &Equipment[EQUIPMENT_GLOVES      ];
	ITEM *rl = &Equipment[EQUIPMENT_RING_LEFT   ];
	ITEM *rr = &Equipment[EQUIPMENT_RING_RIGHT  ];
#ifdef LDK_ADD_PC4_GUARDIAN
	ITEM *phelper = &Equipment[EQUIPMENT_HELPER  ];
#endif //LDK_ADD_PC4_GUARDIAN

	SEASON3B::CNewUIPickedItem* pPickedItem = SEASON3B::CNewUIInventoryCtrl::GetPickedItem();
	if(pPickedItem && pPickedItem->GetOwnerInventory() == NULL)	// 장비창은 Owner가 NULL이다.
	{	
		// 아이템을 든 경우
		ITEM* pItem = pPickedItem->GetItem();
		
		if( pItem )
		{
			int iSourceIndex = pPickedItem->GetSourceLinealPos();
			switch(iSourceIndex)
			{
			case EQUIPMENT_WEAPON_RIGHT:
				r = pItem;
				break;
			case EQUIPMENT_WEAPON_LEFT:
				l = pItem;
				break;
			case EQUIPMENT_GLOVES:
				g = pItem;
				break;
			case EQUIPMENT_RING_LEFT:
				rl = pItem;
				break;
			case EQUIPMENT_RING_RIGHT:
				rr = pItem;
				break;

#ifdef LDK_ADD_PC4_GUARDIAN
			case EQUIPMENT_HELPER:
				phelper = pItem;
				break;
#endif //LDK_ADD_PC4_GUARDIAN
			}
		}
	}

	bool Right		= false;
	bool Left		= false;
	bool Glove		= false;
	bool RingRight	= false;
	bool RingLeft	= false;
#ifdef LDK_ADD_PC4_GUARDIAN
	bool Helper     = false;
#endif //LDK_ADD_PC4_GUARDIAN
    if((r->Type!=ITEM_BOW+7 && r->Type!=ITEM_BOW+15 ) && r->Type>=ITEM_SWORD && r->Type<ITEM_STAFF+MAX_ITEM_INDEX) {
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_ATTACKSPEED
		if( IsRequireEquipItem(r) && r->Durability!=0 )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_ATTACKSPEED
		{
			Right = true;
		}
	}

    if((l->Type!=ITEM_BOW+7 && l->Type!=ITEM_BOW+15 ) && l->Type>=ITEM_SWORD && l->Type<ITEM_STAFF+MAX_ITEM_INDEX) {
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_ATTACKSPEED
		if( IsRequireEquipItem(l) && l->Durability!=0 )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_ATTACKSPEED
		{
			Left = true;
		}
	}

	if(g->Durability!=0 && g->Type != -1) {
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_ATTACKSPEED
		if( IsRequireEquipItem(g) )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_ATTACKSPEED
		{
			Glove = true;
		}
	}

    if ( rl->Durability!=0 &&
		(rl->Type == ITEM_HELPER+20
#ifdef YDG_ADD_CS7_CRITICAL_MAGIC_RING
		|| rl->Type == ITEM_HELPER+107		// 치명 마법반지
#endif	// YDG_ADD_CS7_CRITICAL_MAGIC_RING
		) )
    {
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_ATTACKSPEED
		if( IsRequireEquipItem(rl) )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_ATTACKSPEED
		{
#ifdef LDK_FIX_EQUIPED_EXPIREDPERIOD_RING_EXCEPTION
			if(false == rl->bPeriodItem || false == rl->bExpiredPeriod)
			{
				switch ( rl->Level>>3 )
				{
				case 0:
				case 3:
					RingRight = true;
					break;
				}
			}
#else // LDK_FIX_EQUIPED_EXPIREDPERIOD_RING_EXCEPTION
			switch ( rl->Level>>3 )
			{
			case 0:
			case 3:
				RingRight = true;
				break;
			}
#endif // LDK_FIX_EQUIPED_EXPIREDPERIOD_RING_EXCEPTION
		}
    }

    if ( rr->Durability!=0 && (rr->Type == ITEM_HELPER+20
#ifdef YDG_ADD_CS7_CRITICAL_MAGIC_RING
		|| rr->Type == ITEM_HELPER+107		// 치명 마법반지
#endif	// YDG_ADD_CS7_CRITICAL_MAGIC_RING
		) ) {
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_ATTACKSPEED
		if( IsRequireEquipItem(rr) )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_ATTACKSPEED
		{
#ifdef LDK_FIX_EQUIPED_EXPIREDPERIOD_RING_EXCEPTION
			if(false == rr->bPeriodItem || false == rr->bExpiredPeriod)
			{
				switch ( rr->Level>>3 )
				{
				case 0:
				case 3:
					RingLeft = true;
					break;
				}
			}
#else // LDK_FIX_EQUIPED_EXPIREDPERIOD_RING_EXCEPTION
			switch ( rr->Level>>3 )
			{
			case 0:
			case 3:
				RingLeft = true;
				break;
			}
#endif // LDK_FIX_EQUIPED_EXPIREDPERIOD_RING_EXCEPTION
		}
    }

#ifdef LDK_ADD_PC4_GUARDIAN
	if( phelper->Durability != 0 && 
		(phelper->Type == ITEM_HELPER+64
#ifdef YDG_ADD_SKELETON_PET
		|| phelper->Type == ITEM_HELPER+123	// 스켈레톤 펫
#endif	// YDG_ADD_SKELETON_PET
		)
		)
	{
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_ATTACKSPEED
		if( IsRequireEquipItem(phelper) )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_ATTACKSPEED
		{
#ifdef LDK_MOD_PC4_GUARDIAN_EXPIREDPERIOD_NOTPRINT_INFO
			//만료된 기간제 아이템이면 출력 안함
			if(false == phelper->bPeriodItem || false == phelper->bExpiredPeriod)
			{
				Helper = true;
			}
#else //LDK_MOD_PC4_GUARDIAN_EXPIREDPERIOD_NOTPRINT_INFO
			Helper = true;
#endif //LDK_MOD_PC4_GUARDIAN_EXPIREDPERIOD_NOTPRINT_INFO
		}
	}
#endif //LDK_ADD_PC4_GUARDIAN

    int RightWeaponSpeed = r->WeaponSpeed;
    int LeftWeaponSpeed  = l->WeaponSpeed;

    if ( r->Durability<=0 ) RightWeaponSpeed = 0;
    if ( l->Durability<=0 ) LeftWeaponSpeed = 0;

    if(Right && Left)
    {
     	Character.AttackSpeed += (RightWeaponSpeed+LeftWeaponSpeed)/2;
     	Character.MagicSpeed += (RightWeaponSpeed+LeftWeaponSpeed)/2;
    }
	else if(Right)
    {
        Character.AttackSpeed += RightWeaponSpeed;
        Character.MagicSpeed += RightWeaponSpeed;
    }
	else if(Left)
    {
        Character.AttackSpeed += LeftWeaponSpeed;
        Character.MagicSpeed += LeftWeaponSpeed;
    }
	
	if(RingRight)	// 마법사의 반지 효과
    {
        Character.AttackSpeed += 10;
        Character.MagicSpeed += 10;
		CharacterAttribute->Ability |= ABILITY_FAST_ATTACK_RING;
    }
	else if(RingLeft)	// 마법사의 반지 효과
    {
        Character.AttackSpeed += 10;
        Character.MagicSpeed += 10;
		CharacterAttribute->Ability |= ABILITY_FAST_ATTACK_RING;
    }
	else if (CharacterAttribute->Ability & ABILITY_FAST_ATTACK_RING)
	{
		CharacterAttribute->Ability &= (~ABILITY_FAST_ATTACK_RING);
	}

    //  장갑 속도.
	if(Glove)
    {
        Character.AttackSpeed += g->WeaponSpeed;
        Character.MagicSpeed += g->WeaponSpeed;
    }

#ifdef LDK_ADD_PC4_GUARDIAN
	if(Helper)
	{
		const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ITEM_HELPER+64);
        Character.AttackSpeed += Item_data.m_byValue2;
        Character.MagicSpeed += Item_data.m_byValue2;
	}
#endif //LDK_ADD_PC4_GUARDIAN

#ifdef YDG_ADD_SKELETON_PET
	if(Helper)	// 스켈레톤 펫
	{
		const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ITEM_HELPER+123);
        Character.AttackSpeed += Item_data.m_byValue2;
        Character.MagicSpeed += Item_data.m_byValue2;
	}
#endif	// YDG_ADD_SKELETON_PET

	if(Character.Ability & ABILITY_FAST_ATTACK_SPEED)
    {
        Character.AttackSpeed += 20;
        Character.MagicSpeed += 20;
    }
	else if ( Character.Ability&ABILITY_FAST_ATTACK_SPEED2 )	//. 영혼의 물약 공속 증가
    {
        Character.AttackSpeed += 20;
        Character.MagicSpeed += 20;
    }
#ifdef LDS_MOD_INGAMESHOPITEM_RING_AMULET_CHARACTERATTR
	ITEM *Amulet = &Equipment[EQUIPMENT_AMULET];
	
#ifdef LDK_FIX_EQUIPED_EXPIREDPERIOD_AMULET_EXCEPTION
	//만료된 기간제 아이템이면 출력 안함
	if( (false == Amulet->bPeriodItem || false == Amulet->bExpiredPeriod)
		&& Amulet->Type == ITEM_HELPER + 114 )
#else //LDK_FIX_EQUIPED_EXPIREDPERIOD_AMULET_EXCEPTION
	if( Amulet->Type == ITEM_HELPER + 114 )		// AMULETEMERALD
#endif //LDK_FIX_EQUIPED_EXPIREDPERIOD_AMULET_EXCEPTION
	{
		//공속증가 +7
		int _Temp = 7;
		
		Character.AttackSpeed += _Temp;
		Character.MagicSpeed += _Temp;
		//Character.AttackSpeed += Amulet->WeaponSpeed;	// 서버에서 넘어오는 값? 추후 확인사항.
		//Character.MagicSpeed += Amulet->WeaponSpeed;
	}
#endif // LDS_MOD_INGAMESHOPITEM_RING_AMULET_CHARACTERATTR
	
	///////////////////////////////////////// 캐릭터 상태 ( 버프 ) ///////////////////////////////////////// 
#ifdef PSW_SCROLL_ITEM
	if(g_isCharacterBuff((&Hero->Object), eBuff_EliteScroll1))  // 신속의 스크롤
	{
		const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ITEM_POTION + 72);
		Character.AttackSpeed += Item_data.m_byValue1;
		Character.MagicSpeed += Item_data.m_byValue1;
	}
#endif// PSW_SCROLL_ITEM
#ifdef HELLOWIN_EVENT
	if(g_isCharacterBuff((&Hero->Object), eBuff_Hellowin1))	// 잭오랜턴의 축복
	{
		ITEM_ADD_OPTION Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ITEM_POTION + 46);
		Character.AttackSpeed += Item_data.m_byValue1;
		Character.MagicSpeed += Item_data.m_byValue1;
	}
#endif //HELLOWIN_EVENT
#ifdef PBG_ADD_SANTABUFF
	if(g_isCharacterBuff((&Hero->Object), eBuff_QuickOfSanta))	//산타의 신속
	{
		//공속증가 +15
		int _Temp = 15;
		Character.AttackSpeed += _Temp;
		Character.MagicSpeed += _Temp;
	}
#endif //PBG_ADD_SANTABUFF

	///////////////////////////////////////////// 엑셀런트 옵션 ///////////////////////////////////////////// 
	// 공격 스피드 같은 경우 엑셀런트 옵션에 스피드 증가 옵션이 있으면 서버랑 옵션값이 두배차이 난다.
	// 서버에서는 아이템을 만들때 엑셀런트 옵션을 더해주고 또 최종 단계의 캐릭터 공속 계산을 할때 더 하므로
	// 엑셀옵션이 공속 7증가가 있으면 서버는 14 클라 7이 증가 된다.
	PlusSpecial(&Character.AttackSpeed,AT_IMPROVE_ATTACK_SPEED,&Equipment[EQUIPMENT_WEAPON_RIGHT]);
	PlusSpecial(&Character.MagicSpeed, AT_IMPROVE_ATTACK_SPEED,&Equipment[EQUIPMENT_WEAPON_RIGHT]);
	PlusSpecial(&Character.AttackSpeed,AT_IMPROVE_ATTACK_SPEED,&Equipment[EQUIPMENT_WEAPON_LEFT]);
	PlusSpecial(&Character.MagicSpeed, AT_IMPROVE_ATTACK_SPEED,&Equipment[EQUIPMENT_WEAPON_LEFT]);
	PlusSpecial(&Character.AttackSpeed,AT_IMPROVE_ATTACK_SPEED,&Equipment[EQUIPMENT_AMULET]);
	PlusSpecial(&Character.MagicSpeed, AT_IMPROVE_ATTACK_SPEED,&Equipment[EQUIPMENT_AMULET]);
	PlusSpecial(&Character.AttackSpeed, AT_IMPROVE_ATTACK_SPEED,&Equipment[EQUIPMENT_HELPER]);
	PlusSpecial(&Character.MagicSpeed, AT_IMPROVE_ATTACK_SPEED,&Equipment[EQUIPMENT_HELPER]);
	///////////////////////////////////////////// 소켓 아이템 /////////////////////////////////////////////
#ifdef ADD_SOCKET_STATUS_BONUS
	Character.AttackSpeed += g_SocketItemMgr.m_StatusBonus.m_iAttackSpeedBonus;
	Character.MagicSpeed += g_SocketItemMgr.m_StatusBonus.m_iAttackSpeedBonus;
#endif	// ADD_SOCKET_STATUS_BONUS
	///////////////////////////////////////////// 소켓 아이템 /////////////////////////////////////////////
}

void CHARACTER_MACHINE::CalculateSuccessfulBlockingPK()
{
	WORD Dexterity;
	Dexterity= Character.Dexterity+ Character.AddDexterity;
    int CharacterClass = GetBaseClass ( Character.Class );

	float tmpf = 0.f;
	switch(CharacterClass)
	{
		case CLASS_KNIGHT:
			tmpf = (float)Character.Level * 2 + (float)Dexterity * 0.5f;
			break;
		case CLASS_DARK_LORD:
			tmpf = (float)Character.Level * 2 + (float)Dexterity * 0.5f;
			break;
		case CLASS_ELF:
			tmpf = (float)Character.Level * 2 + (float)Dexterity * 0.1f;
			break;		
		case CLASS_DARK:
			tmpf = (float)Character.Level * 2 + (float)Dexterity * 0.25f;
			break;
		case CLASS_WIZARD:
			tmpf = (float)Character.Level * 2 + (float)Dexterity * 0.25f;
			break;
		case CLASS_SUMMONER:
			tmpf = (float)Character.Level * 2 + (float)Dexterity * 0.5f;
			break;
#ifdef PBG_ADD_NEWCHAR_MONK
		case CLASS_RAGEFIGHTER:
			tmpf = (float)Character.Level * 1.5f + (float)Dexterity * 0.2f;
			break;
#endif //PBG_ADD_NEWCHAR_MONK
	}
	Character.SuccessfulBlockingPK = (WORD)tmpf;
	hanguo_check3();
}

// 방어율 계산 함수
void CHARACTER_MACHINE::CalculateSuccessfulBlocking()
{
	WORD Dexterity;

	// 캐릭터의 민첩성 + 추가 민첩성을 구한다.
	Dexterity= Character.Dexterity+ Character.AddDexterity;

    int CharacterClass = GetBaseClass ( Character.Class );

	if (CharacterClass==CLASS_ELF || CharacterClass==CLASS_SUMMONER)
	{
		// 요정, 소환술사면 구한 민첩성의 / 4 를 입력
        Character.SuccessfulBlocking = Dexterity/4;
	}
    else if ( CharacterClass==CLASS_DARK_LORD )
	{
		// 다크로드면 구한 민첩성의 / 7 를 설정
        Character.SuccessfulBlocking = Dexterity/7;
	}
#ifdef PBG_ADD_NEWCHAR_MONK
	else if(CharacterClass==CLASS_RAGEFIGHTER)
	{
		Character.SuccessfulBlocking = Dexterity/10;
	}
#endif //PBG_ADD_NEWCHAR_MONK
	else
	{
		// 기사, 흑마법사, 마검은 구한 민첩성의 / 3 를 설정
      	Character.SuccessfulBlocking = Dexterity/3;
	}

	// 왼쪽 무기가 내구도가 0이 아니라면
	ITEM* Left = &Equipment[EQUIPMENT_WEAPON_LEFT];
    if ( Left->Type!=-1 && Left->Durability!=0 
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_SUCCESSFULBLOCKING
		&& IsRequireEquipItem( Left )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_SUCCESSFULBLOCKING	
		)
	{
        ITEM_ATTRIBUTE* p = &ItemAttribute[Left->Type];
        float percent = CalcDurabilityPercent(Left->Durability,p->Durability,Left->Level,Left->Option1,Left->ExtOption);

        WORD SuccessBlocking = 0;
        SuccessBlocking = Left->SuccessfulBlocking - (WORD)(Left->SuccessfulBlocking*percent);

		// 왼쪽 무기의 방어률을 더해주고
		Character.SuccessfulBlocking += SuccessBlocking;
        
        SuccessBlocking = 0;
        PlusSpecial(&SuccessBlocking,AT_IMPROVE_BLOCKING,Left);

		// 왼쪽 무기에 추가 방어율이 있다면 방어률 더해준다.
        Character.SuccessfulBlocking += SuccessBlocking - (WORD)(SuccessBlocking*percent);

		// 왼쪽 무기에 방어 성공율이 있다면 계산해서 더해준다.
        PlusSpecialPercent(&Character.SuccessfulBlocking,AT_IMPROVE_BLOCKING_PERCENT,Left,10);
	}

	// 다른 장비들에도 방어 성공율이 있다면 계산해서 더해준다.
	// 핼멧
    PlusSpecialPercent ( &Character.SuccessfulBlocking, AT_IMPROVE_BLOCKING_PERCENT, &Equipment[EQUIPMENT_HELM],10 );
	// 갑옷
    PlusSpecialPercent ( &Character.SuccessfulBlocking, AT_IMPROVE_BLOCKING_PERCENT, &Equipment[EQUIPMENT_ARMOR],10 );
	// 바지
    PlusSpecialPercent ( &Character.SuccessfulBlocking, AT_IMPROVE_BLOCKING_PERCENT, &Equipment[EQUIPMENT_PANTS],10 );
	// 장갑
    PlusSpecialPercent ( &Character.SuccessfulBlocking, AT_IMPROVE_BLOCKING_PERCENT, &Equipment[EQUIPMENT_GLOVES],10 );
	// 신발
    PlusSpecialPercent ( &Character.SuccessfulBlocking, AT_IMPROVE_BLOCKING_PERCENT, &Equipment[EQUIPMENT_BOOTS],10 );
	// 반지 왼쪽
    PlusSpecialPercent ( &Character.SuccessfulBlocking, AT_IMPROVE_BLOCKING_PERCENT, &Equipment[EQUIPMENT_RING_LEFT],10 );
	// 반지 오른쪽
    PlusSpecialPercent ( &Character.SuccessfulBlocking, AT_IMPROVE_BLOCKING_PERCENT, &Equipment[EQUIPMENT_RING_RIGHT],10 );
	hanguo_check3();

#ifdef ADD_SOCKET_STATUS_BONUS

#ifndef YDG_FIX_SOCKET_SHIELD_BONUS	// 정리할때 삭제해야함
	// 왼쪽 무기가 내구도가 0이 아니라면
    if ( Left->Type!=-1 && Left->Durability!=0 
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_SUCCESSFULBLOCKING
		&& IsRequireEquipItem( Left )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_SUCCESSFULBLOCKING	
		)
	{
		// 방패차고있을때만 적용된다
		Character.SuccessfulBlocking += g_SocketItemMgr.m_StatusBonus.m_iShieldDefenceBonus;
	}
#endif	// YDG_FIX_SOCKET_SHIELD_BONUS

#ifdef YDG_FIX_SOCKET_BALANCE_PATCH
	Character.SuccessfulBlocking *= g_SocketItemMgr.m_StatusBonus.m_fDefenceRateBonus;
#else	// YDG_FIX_SOCKET_BALANCE_PATCH
#ifdef YDG_FIX_SOCKET_ITEM_DEFENCE_RATE_BONUS
	Character.SuccessfulBlocking *= 1.0f + g_SocketItemMgr.m_StatusBonus.m_iDefenceRateBonus * 0.01f;
#else	// YDG_FIX_SOCKET_ITEM_DEFENCE_RATE_BONUS
	Character.SuccessfulBlocking += g_SocketItemMgr.m_StatusBonus.m_iDefenceRateBonus;
#endif	// YDG_FIX_SOCKET_ITEM_DEFENCE_RATE_BONUS
#endif	// YDG_FIX_SOCKET_BALANCE_PATCH
#endif	// ADD_SOCKET_STATUS_BONUS
}

void CHARACTER_MACHINE::CalculateDefense()
{
	WORD Dexterity, Vitality;

	Dexterity= Character.Dexterity+ Character.AddDexterity;
    Vitality = Character.Vitality + Character.AddVitality;

    int CharacterClass = GetBaseClass ( Character.Class );

	if ( CharacterClass==CLASS_ELF )
    {
        Character.Defense  = Dexterity/10;
    }
	else if (CharacterClass==CLASS_KNIGHT || CharacterClass==CLASS_SUMMONER)
    {
        Character.Defense  = Dexterity/3;
    }
	else if ( CharacterClass==CLASS_WIZARD )
    {
        Character.Defense  = Dexterity/4;
    }
	else if ( CharacterClass==CLASS_DARK_LORD )
    {
        Character.Defense  = Dexterity/7;
    }
#ifdef PBG_ADD_NEWCHAR_MONK
	else if ( CharacterClass==CLASS_RAGEFIGHTER )
    {
        Character.Defense  = Dexterity/8;
    }
#endif //PBG_ADD_NEWCHAR_MONK
	else
    {
		// 마검사 / 몽크
        Character.Defense  = Dexterity/5;
    }

    WORD    Defense = 0;
    for( int i=EQUIPMENT_WEAPON_LEFT; i<=EQUIPMENT_WING; ++i )
    {
        if( Equipment[i].Durability!=0
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_DEFENSE	
			&& IsRequireEquipItem(&Equipment[i])
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_DEFENSE
			)
        {
            WORD defense = ItemDefense(&Equipment[i]);

            //  내구력에 따른 공격력 감소.
            ITEM_ATTRIBUTE *p = &ItemAttribute[Equipment[i].Type];
            float percent;
            if ( i==EQUIPMENT_WING )
            {
                //  날개.
                percent = CalcDurabilityPercent(Equipment[i].Durability,p->Durability,Equipment[i].Level,0);//Equipment[i].Option1);
            }
            else
            {
                percent = CalcDurabilityPercent(Equipment[i].Durability,p->Durability,Equipment[i].Level,Equipment[i].Option1,Equipment[i].ExtOption);
            }

            defense -= (WORD)(defense*percent);

            Defense += defense;
        }
    }
    Character.Defense += Defense;

    //  +10, +11일 경우 추가 방어력.
    if ( g_bAddDefense )
    {
        float addDefense = 0.f;
        switch ( EquipmentLevelSet )
        {
        case 10:
            addDefense = 0.05f;
            break;
        case 11:
            addDefense = 0.1f;
            break;
        case 12:
            addDefense = 0.15f;
            break;
        case 13:
            addDefense = 0.2f;
            break;
#ifdef LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
        case 14:
            addDefense = 0.25f;
            break;
        case 15:
            addDefense = 0.3f;
            break;
#endif //LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
        }
        Character.Defense += (WORD)(Character.Defense*addDefense);
    }

#ifdef PSW_SCROLL_ITEM
	if(g_isCharacterBuff((&Hero->Object), eBuff_EliteScroll2))  // 방어의 스크롤
	{
		const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ITEM_POTION + 73);
		Character.Defense += (WORD)Item_data.m_byValue1;
	}
#endif// PSW_SCROLL_ITEM
#ifdef HELLOWIN_EVENT
	if(g_isCharacterBuff((&Hero->Object), eBuff_Hellowin3)) // 잭오랜턴의 외침
	{
		ITEM_ADD_OPTION Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ITEM_POTION + 48);
		Character.Defense += (WORD)Item_data.m_byValue1;
	}
#endif //HELLOWIN_EVENT
#ifdef PBG_ADD_SANTABUFF
	if(g_isCharacterBuff((&Hero->Object), eBuff_BlessingOfXmax))	//크리스마스의 축복
	{
		int _Temp = 0;
		_Temp = Character.Level / 5 + 50;
		
		Character.Defense += _Temp;
	}

	if(g_isCharacterBuff((&Hero->Object), eBuff_DefenseOfSanta))	//산타의 방어
	{
		int _Temp = 100;
		Character.Defense += _Temp;
	}
#endif //PBG_ADD_SANTABUFF

#ifdef LDS_ADD_PCROOM_ITEM_JPN_6TH
	if(g_isCharacterBuff((&Hero->Object), eBuff_StrongCharm))		//강함의 인장(PC방 아이템)
	{
		int _Temp = 60;
		Character.Defense += _Temp;
	}
#endif // LDS_ADD_PCROOM_ITEM_JPN_6TH

	//	세트 옵션을 적용한다.
    g_csItemOption.PlusSpecial ( &Character.Defense, AT_SET_OPTION_IMPROVE_DEFENCE );         //	방어력 증가

	g_csItemOption.PlusSpecialLevel ( &Character.Defense, Dexterity, AT_SET_OPTION_IMPROVE_DEFENCE_3 );         //	특정 캐릭터 방어력 증가 민첩성/50	
	g_csItemOption.PlusSpecialLevel ( &Character.Defense, Vitality, AT_SET_OPTION_IMPROVE_DEFENCE_4 );          //	특정 캐릭터 방어력 증가 체력/40		    

    if ( Equipment[EQUIPMENT_WEAPON_LEFT].Type>=ITEM_SHIELD && Equipment[EQUIPMENT_WEAPON_LEFT].Type<ITEM_SHIELD+MAX_ITEM_INDEX )
    {
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_DEFENSE
		if( IsRequireEquipItem( &Equipment[EQUIPMENT_WEAPON_LEFT]) )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_DEFENSE
		{
			g_csItemOption.PlusSpecialPercent ( &Character.Defense, AT_SET_OPTION_IMPROVE_SHIELD_DEFENCE );         //	방패 장비시 방어력 5% 증가
		}
    }

    PlusSpecial ( &Character.Defense, AT_SET_OPTION_IMPROVE_DEFENCE, &Equipment[EQUIPMENT_HELPER] );
    if ( Equipment[EQUIPMENT_WING].Type==ITEM_HELPER+30 
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		|| Equipment[EQUIPMENT_WING].Type==ITEM_WING+49
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		)   //  망토.
    {
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_DEFENSE
		if( IsRequireEquipItem( &Equipment[EQUIPMENT_WING]) )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_DEFENSE
		{
			PlusSpecial ( &Character.Defense, AT_SET_OPTION_IMPROVE_DEFENCE, &Equipment[EQUIPMENT_WING] );
		}
    }

#ifdef ADD_SOCKET_STATUS_BONUS
	Character.Defense += g_SocketItemMgr.m_StatusBonus.m_iDefenceBonus;
#endif	// ADD_SOCKET_STATUS_BONUS

#ifdef YDG_FIX_SOCKET_SHIELD_BONUS
	// 왼쪽 무기가 내구도가 0이 아니라면
    if ( Equipment[EQUIPMENT_WEAPON_LEFT].Type!=-1 && Equipment[EQUIPMENT_WEAPON_LEFT].Durability!=0 
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_SUCCESSFULBLOCKING
		&& IsRequireEquipItem( Equipment[EQUIPMENT_WEAPON_LEFT] )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_SUCCESSFULBLOCKING	
		)
	{
		// 방패차고있을때만 적용된다
		Character.Defense += Equipment[EQUIPMENT_WEAPON_LEFT].Defense * g_SocketItemMgr.m_StatusBonus.m_iShieldDefenceBonus * 0.01f;
	}
#endif	// YDG_FIX_SOCKET_SHIELD_BONUS
}

void CHARACTER_MACHINE::CalculateMagicDefense()
{
    for( int i=EQUIPMENT_HELM; i<=EQUIPMENT_WING; ++i )
    {
        if( Equipment[i].Durability!=0 
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_MAGICDEFENSE
			&& IsRequireEquipItem( &Equipment[i] )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_MAGICDEFENSE
			)
        {
            Character.MagicDefense  = ItemMagicDefense(&Equipment[i]);
        }
    }
//	Character.MagicDefense  = ItemMagicDefense(&Equipment[EQUIPMENT_HELM]);
//	Character.MagicDefense += ItemMagicDefense(&Equipment[EQUIPMENT_ARMOR]);
//	Character.MagicDefense += ItemMagicDefense(&Equipment[EQUIPMENT_PANTS]);
//	Character.MagicDefense += ItemMagicDefense(&Equipment[EQUIPMENT_GLOVES]);
//	Character.MagicDefense += ItemMagicDefense(&Equipment[EQUIPMENT_BOOTS]);
//	Character.MagicDefense += ItemMagicDefense(&Equipment[EQUIPMENT_WING]);
}

void CHARACTER_MACHINE::CalculateWalkSpeed()
{
    if( Equipment[EQUIPMENT_BOOTS].Durability!=0
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_WALKSPEED
		&& IsRequireEquipItem( &Equipment[EQUIPMENT_BOOTS] )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_WALKSPEED
		)
    {
	    Character.WalkSpeed  = ItemWalkSpeed(&Equipment[EQUIPMENT_BOOTS]);
    }

    if( Equipment[EQUIPMENT_WING].Durability!=0 
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_WALKSPEED
		&& IsRequireEquipItem( &Equipment[EQUIPMENT_WING] )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_WALKSPEED
		)
    {
	    Character.WalkSpeed += ItemWalkSpeed(&Equipment[EQUIPMENT_WING]);
    }
}

//(10+l)*l/4
//(4+l)*l*l*20
void CHARACTER_MACHINE::CalculateNextExperince()
{
	// 레벨업 했을 경우이니깐
	// 현재 경험치를 다음경험치로 초기화
	Character.Experience = Character.NextExperince;

	// 다음 경험치 새로 구해줌
	Character.NextExperince = (9+Character.Level)*(Character.Level)*(Character.Level)*10;

    if ( Character.Level > 255 )
    {
        int LevelOver_N = Character.Level - 255;
        Character.NextExperince += (9+LevelOver_N)*LevelOver_N*LevelOver_N*1000;
    }
}

void CHARACTER_MACHINE::CalulateMasterLevelNextExperience()
{
	// 레벨업 했을 경우이니깐
	// 현재 경험치를 다음경험치로 초기화
	Master_Level_Data.lMasterLevel_Experince = Master_Level_Data.lNext_MasterLevel_Experince;

	__int64 iTotalLevel = CharacterAttribute->Level + Master_Level_Data.nMLevel + 1;	// 종합레벨
	__int64 iTOverLevel = iTotalLevel - 255;		// 255레벨 이상 기준 레벨
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	__int64 iBaseExperience = 0;					// 레벨 초기 경험치
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	
	__int64 iData_Master =	// A
		(
		(
		(__int64)9 + (__int64)iTotalLevel
		)
		* (__int64)iTotalLevel
		* (__int64)iTotalLevel
		* (__int64)10
		)
		+
		(
		(
		(__int64)9 + (__int64)iTOverLevel
		)
		* (__int64)iTOverLevel
		* (__int64)iTOverLevel
		* (__int64)1000
		);

	Master_Level_Data.lNext_MasterLevel_Experince = ( iData_Master - (__int64)3892250000 ) / (__int64)2;	// B
}

bool CHARACTER_MACHINE::IsZeroDurability()
{
    for ( int i=EQUIPMENT_WEAPON_RIGHT; i<MAX_EQUIPMENT; ++i )
    {
        if ( Equipment[i].Durability==0 )
        {
            return  true;
        }
    }

    return  false;
}

#ifdef PSW_SECRET_ITEM
void CHARACTER_MACHINE::CalculateBasicState()
{
	if(g_isCharacterBuff((&Hero->Object), eBuff_SecretPotion1)) // 힘
	{
		ITEM_ADD_OPTION Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ITEM_POTION + 78);
		Character.AddStrength += (WORD)Item_data.m_byValue1;
	}
	else if(g_isCharacterBuff((&Hero->Object), eBuff_SecretPotion2)) // 민첩
	{
		ITEM_ADD_OPTION Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ITEM_POTION + 79);
		Character.AddDexterity += (WORD)Item_data.m_byValue1;
	}
	else if(g_isCharacterBuff((&Hero->Object), eBuff_SecretPotion3)) // 체력
	{
		ITEM_ADD_OPTION Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ITEM_POTION + 80);
		Character.AddVitality += (WORD)Item_data.m_byValue1;
	}
	else if(g_isCharacterBuff((&Hero->Object), eBuff_SecretPotion4)) // 에너지
	{
		ITEM_ADD_OPTION Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ITEM_POTION + 81);
		Character.AddEnergy += (WORD)Item_data.m_byValue1;
	}
	else if(g_isCharacterBuff((&Hero->Object), eBuff_SecretPotion5)) // 통솔
	{
		ITEM_ADD_OPTION Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ITEM_POTION + 82);
		Character.AddCharisma += (WORD)Item_data.m_byValue1;
	}
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	if(g_isCharacterBuff((&Hero->Object), eBuff_Hp_up_Ourforces))
	{
#ifdef PBG_MOD_RAGEFIGHTERSOUND
		WORD _AddStat = (WORD)(30+(WORD)((Character.Energy-132)/10));
#else //PBG_MOD_RAGEFIGHTERSOUND
		WORD _AddStat = (WORD)(30+(WORD)((Character.Energy-380)/10));
#endif //PBG_MOD_RAGEFIGHTERSOUND
		if(_AddStat > 200)
		{
			_AddStat = 200;
		}
		Character.AddVitality += _AddStat;
	}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
#ifndef YDG_FIX_DOUBLE_SOCKETITEM_BONUS
#ifdef SOCKET_SYSTEM
	// 소켓 세트 옵션을 검사한다.
	g_SocketItemMgr.CheckSocketSetOption();
	g_SocketItemMgr.CalcSocketStatusBonus();
	CharacterMachine->Character.AddStrength  += g_SocketItemMgr.m_StatusBonus.m_iStrengthBonus;
	CharacterMachine->Character.AddDexterity += g_SocketItemMgr.m_StatusBonus.m_iDexterityBonus;
	CharacterMachine->Character.AddVitality  += g_SocketItemMgr.m_StatusBonus.m_iVitalityBonus;
	CharacterMachine->Character.AddEnergy    += g_SocketItemMgr.m_StatusBonus.m_iEnergyBonus;
#endif	// SOCKET_SYSTEM
#endif	// YDG_FIX_DOUBLE_SOCKETITEM_BONUS
}

#endif //PSW_SECRET_ITEM


#ifdef LDS_FIX_APPLY_BUFF_STATEVALUES_WHEN_CALCULATE_CHARACTERSTATE_WITH_SETITEM
void CHARACTER_MACHINE::getAllAddStateOnlyExValues( int &iAddStrengthExValues, 
												int &iAddDexterityExValues, 
												int &iAddVitalityExValues,
												int &iAddEnergyExValues, 
												int &iAddCharismaExValues )
{
	if(g_isCharacterBuff((&Hero->Object), eBuff_SecretPotion1)) // 힘
	{
		ITEM_ADD_OPTION Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ITEM_POTION + 78);
		iAddStrengthExValues += (WORD)Item_data.m_byValue1;
	}
	else if(g_isCharacterBuff((&Hero->Object), eBuff_SecretPotion2)) // 민첩
	{
		ITEM_ADD_OPTION Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ITEM_POTION + 79);
		iAddDexterityExValues += (WORD)Item_data.m_byValue1;
	}
	else if(g_isCharacterBuff((&Hero->Object), eBuff_SecretPotion3)) // 체력
	{
		ITEM_ADD_OPTION Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ITEM_POTION + 80);
		iAddVitalityExValues += (WORD)Item_data.m_byValue1;
	}
	else if(g_isCharacterBuff((&Hero->Object), eBuff_SecretPotion4)) // 에너지
	{
		ITEM_ADD_OPTION Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ITEM_POTION + 81);
		iAddEnergyExValues += (WORD)Item_data.m_byValue1;
	}
	else if(g_isCharacterBuff((&Hero->Object), eBuff_SecretPotion5)) // 통솔
	{
		ITEM_ADD_OPTION Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ITEM_POTION + 82);
		iAddCharismaExValues += (WORD)Item_data.m_byValue1;
	}
	
#ifdef SOCKET_SYSTEM
	// 소켓 세트 옵션을 검사한다. 이미 위에서 소켓 아이템 계산 함수를 한번 호출 하였으므로 계산부는 제거.
	//g_SocketItemMgr.CheckSocketSetOption();
	//g_SocketItemMgr.CalcSocketStatusBonus();
	iAddStrengthExValues  += g_SocketItemMgr.m_StatusBonus.m_iStrengthBonus;
	iAddDexterityExValues += g_SocketItemMgr.m_StatusBonus.m_iDexterityBonus;
	iAddVitalityExValues  += g_SocketItemMgr.m_StatusBonus.m_iVitalityBonus;
	iAddEnergyExValues    += g_SocketItemMgr.m_StatusBonus.m_iEnergyBonus;
#endif	// SOCKET_SYSTEM
}
#endif // LDS_FIX_APPLY_BUFF_STATEVALUES_WHEN_CALCULATE_CHARACTERSTATE_WITH_SETITEM



#ifdef ANTIHACKING_ENABLE
extern BOOL g_bNewFrame;
#endif //ANTIHACKING_ENABLE

void CHARACTER_MACHINE::CalculateAll()
{
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_SETITEM

	// 캐릭터 add 스텟 초기화
	InitAddValue ();

#ifdef PSW_SECRET_ITEM
	// 버프 효과가 중에 기본 스텟을 올려 줄것이 있으면 여기서 한다. 
	// SOCKET_SYSTEM define 추가 되어있음.
	CalculateBasicState();
#endif //PSW_SECRET_ITEM

	//	아이템이 갱신되었다면 다시 세트 옵션을 검사한다.
	//	캐릭터 정보창이 띄어있을때.
	g_csItemOption.CheckItemSetOptions ();
	g_csItemOption.ClearListOnOff();
 
#else //PSW_BUGFIX_REQUIREEQUIPITEM_SETITEM           // 정리 해야 할 코드

#if defined PSW_SECRET_ITEM && defined LDK_FIX_USE_SECRET_ITEM_TO_SETITEM_OPTION_BUG
	// 비약 적용시 add값 갱신.
	// 버프 효과가 중에 기본 스텟을 올려 줄것이 있으면 여기서 한다. 
	// SOCKET_SYSTEM define 추가 되어있음.
	CalculateBasicState();
#endif //PSW_SECRET_ITEM

	//	아이템이 갱신되었다면 다시 세트 옵션을 검사한다.
	//	캐릭터 정보창이 띄어있을때.
	g_csItemOption.CheckItemSetOptions ();
	g_csItemOption.ClearListOnOff();
	// 캐릭터 add 스텟 초기화
	InitAddValue ();

#ifdef SOCKET_SYSTEM
	// 소켓 세트 옵션을 검사한다.
	g_SocketItemMgr.CheckSocketSetOption();
	g_SocketItemMgr.CalcSocketStatusBonus();
	CharacterMachine->Character.AddStrength  += g_SocketItemMgr.m_StatusBonus.m_iStrengthBonus;
	CharacterMachine->Character.AddDexterity += g_SocketItemMgr.m_StatusBonus.m_iDexterityBonus;
	CharacterMachine->Character.AddVitality  += g_SocketItemMgr.m_StatusBonus.m_iVitalityBonus;
	CharacterMachine->Character.AddEnergy    += g_SocketItemMgr.m_StatusBonus.m_iEnergyBonus;
#endif	// SOCKET_SYSTEM
 
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_SETITEM           // 정리 해야 할 코드
#ifdef PBG_FIX_SETOPTION_CALCULATE_SECRET_POTION
	CalculateBasicState();
#endif //PBG_FIX_SETOPTION_CALCULATE_SECRET_POTION

	// 세트 아이템 들 옵션 적용 부분 시작
	// ============================================================================== >
	// 현재 아이템을 착용에 대한 모든 증감 수치 값을 얻어 오며, 이는 
	// 세트 아이템이 가지고 있는 세트 옵션 과 기본 옵션 스탯 수치도 포함됩니다.
#ifdef LDS_FIX_WRONG_CALCULATEEQUIPEDITEMOPTIONVALUE
	WORD	wAddStrength, wAddDexterity, wAddEnergy, wAddVitality,
		wStrengthResult, wDexterityResult, wEnergyResult, wVitalityResult;
	wAddStrength = wAddDexterity = wAddEnergy = wAddVitality = 0;
	wStrengthResult = wDexterityResult = wEnergyResult = wVitalityResult = 0;

	// 세트 옵션 수치에 "통솔" 수치를 반영 합니다.
#ifdef LDS_ADD_CHARISMAVALUE_TOITEMOPTION
	WORD wAddCharisma, wCharismaResult; 
	wAddCharisma = wCharismaResult = 0;
#endif // LDS_ADD_CHARISMAVALUE_TOITEMOPTION
	
	// 세트 옵션 수치로 추가된 추가 스탯 수치만을 가져옵니다.	// 1.버프 능력치 2. 소켓 아이템 능력치 누락
#ifdef LDS_ADD_CHARISMAVALUE_TOITEMOPTION
	g_csItemOption.getAllAddStateOnlyAddValue( &wAddStrength, &wAddDexterity, &wAddEnergy, &wAddVitality, &wAddCharisma );
#else // LDS_ADD_CHARISMAVALUE_TOITEMOPTION
	g_csItemOption.getAllAddStateOnlyAddValue( &wAddStrength, &wAddDexterity, &wAddEnergy, &wAddVitality );
#endif // LDS_ADD_CHARISMAVALUE_TOITEMOPTION
	
	// 세트 아이템들에 대한 아이템 요구 수치 계산전에 
#ifdef LDS_FIX_APPLY_BUFF_STATEVALUES_WHEN_CALCULATE_CHARACTERSTATE_WITH_SETITEM
	int iAddStrengthByExValues, iAddDexterityByExValues, 
		iAddEnergyByExValues, iAddVitalityByExValues, 
		iAddCharismaExValues;

	iAddStrengthByExValues = iAddDexterityByExValues 
		= iAddEnergyByExValues = iAddVitalityByExValues = iAddCharismaExValues = 0;

	getAllAddStateOnlyExValues( iAddStrengthByExValues, 
								iAddDexterityByExValues, 
								iAddVitalityByExValues, 
								iAddEnergyByExValues, 
								iAddCharismaExValues );

	wAddStrength	+= iAddStrengthByExValues;
	wAddDexterity	+= iAddDexterityByExValues;
	wAddEnergy		+= iAddEnergyByExValues;
	wAddVitality	+= iAddVitalityByExValues;
#endif // LDS_FIX_APPLY_BUFF_STATEVALUES_WHEN_CALCULATE_CHARACTERSTATE_WITH_SETITEM

	// 케릭터의 기본 스탯 수치에 추가 스탯 수치를 더하여 스탯이 적용된 결과 스탯 수치를 산출합니다.
	wStrengthResult		= CharacterMachine->Character.Strength + wAddStrength;
	wDexterityResult	= CharacterMachine->Character.Dexterity + wAddDexterity;
	wEnergyResult		= CharacterMachine->Character.Energy + wAddEnergy;
	wVitalityResult		= CharacterMachine->Character.Vitality + wAddVitality;

	// 다크로드만이 가진 "통솔" 수치 또한 결과 스탯 수치를 산출합니다.
#ifdef LDS_ADD_CHARISMAVALUE_TOITEMOPTION
	wCharismaResult		= CharacterMachine->Character.Charisma + wAddCharisma;
#endif // LDS_ADD_CHARISMAVALUE_TOITEMOPTION

	
	// 세트 최종 반영된 수치값이 아이템 요구수치에 허용되는 수치 인지를 판단하여 
	// 각 아이템별 스탯 수치를 다시 산출 및 적용 합니다.
#ifdef LDS_ADD_CHARISMAVALUE_TOITEMOPTION
	g_csItemOption.getAllAddOptionStatesbyCompare( &Character.AddStrength, 
									&Character.AddDexterity, &Character.AddEnergy, &Character.AddVitality, &Character.AddCharisma, 
									wStrengthResult, wDexterityResult, wEnergyResult, wVitalityResult, wCharismaResult );
#else // LDS_ADD_CHARISMAVALUE_TOITEMOPTION
	g_csItemOption.getAllAddOptionStatesbyCompare( &Character.AddStrength, 
									&Character.AddDexterity, &Character.AddEnergy, &Character.AddVitality,
									wStrengthResult, wDexterityResult, wEnergyResult, wVitalityResult );
#endif // LDS_ADD_CHARISMAVALUE_TOITEMOPTION
	
#else // LDS_FIX_WRONG_CALCULATEEQUIPEDITEMOPTIONVALUE
	// 셋트 아이템 일반 옵션( 장비에 붙어 있는 옵션 )
    for ( int i=EQUIPMENT_WEAPON_RIGHT; i<MAX_EQUIPMENT; ++i )
    {
    	ITEM* item = &Equipment[i];

        if ( item->Durability<=0 
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_SETITEM
			|| IsRequireEquipItem( item ) == false
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_SETITEM			
			) 
		{
			continue;
		}

        WORD Result = 0;
		// Result*5 = 5를 곱 하는 이유는 기본 단위가 5부터 시작
        switch ( g_csItemOption.GetDefaultOptionValue( item, &Result ) )
        {
        case SET_OPTION_STRENGTH:
            Character.AddStrength += Result*5;
            break;

        case SET_OPTION_DEXTERITY:
            Character.AddDexterity += Result*5;
            break;

        case SET_OPTION_ENERGY:
            Character.AddEnergy += Result*5;
            break;

        case SET_OPTION_VITALITY:
            Character.AddVitality += Result*5;
            break;
        }
    }

	// 셋트 아이템 셋트 옵션( 셋트 아이템 몇개 이상을 착용 했을 경우 발생 하는 옵션 )
	g_csItemOption.GetSpecial ( &Character.AddStrength, AT_SET_OPTION_IMPROVE_STRENGTH );	    //	힘 증가
	g_csItemOption.GetSpecial ( &Character.AddDexterity, AT_SET_OPTION_IMPROVE_DEXTERITY );	    //	민첩성 증가
	g_csItemOption.GetSpecial ( &Character.AddEnergy, AT_SET_OPTION_IMPROVE_ENERGY );		    //	에너지 증가
	g_csItemOption.GetSpecial ( &Character.AddVitality, AT_SET_OPTION_IMPROVE_VITALITY );	    //	체력 증가
	g_csItemOption.GetSpecial ( &Character.AddCharisma, AT_SET_OPTION_IMPROVE_CHARISMA );	    //	통솔 증가
#endif // LDS_FIX_WRONG_CALCULATEEQUIPEDITEMOPTIONVALUE
	
#ifdef YDG_FIX_SETITEM_REQUIRED_STATUS_BUG
	// 최종 스탯을 적용하기 위해 셋트 아이템 옵션 검사를 한번 더 해준다
#ifdef YDG_FIX_SETITEM_REQUIRED_STATUS_SECRET_POTION
#ifndef PBG_FIX_SETOPTION_CALCULATE_SECRET_POTION
	CalculateBasicState();
#endif //PBG_FIX_SETOPTION_CALCULATE_SECRET_POTION
#endif	// YDG_FIX_SETITEM_REQUIRED_STATUS_SECRET_POTION
	g_csItemOption.CheckItemSetOptions ();
#endif	// YDG_FIX_SETITEM_REQUIRED_STATUS_BUG

	// 세트 아이템 들 옵션 적용 부분 끝
	// ============================================================================== <

    //  망토에 해당되는 통솔을 증가시킨다.
	if ( (CharacterMachine->Equipment[EQUIPMENT_WING].Type+MODEL_ITEM)==MODEL_HELPER+30)
    {
        PlusSpecial ( &Character.AddCharisma, AT_SET_OPTION_IMPROVE_CHARISMA, &CharacterMachine->Equipment[EQUIPMENT_WING] );
    }

#ifdef	LJH_FIX_NOT_CALCULATED_BUFF
	// 버프 효과가 중에 기본 스텟을 올려 줄것이 있으면 여기서 한다. 
	CalculateBasicState();
#else	//LJH_FIX_NOT_CALCULATED_BUFF
#ifndef PSW_BUGFIX_REQUIREEQUIPITEM_SETITEM  // 정리 해야 할 코드
#ifdef PSW_SECRET_ITEM
#ifndef PBG_FIX_SETOPTION_CALCULATE_SECRET_POTION
	// 버프 효과가 중에 기본 스텟을 올려 줄것이 있으면 여기서 한다. 
	CalculateBasicState();
#endif //PBG_FIX_SETOPTION_CALCULATE_SECRET_POTION
#endif //PSW_SECRET_ITEM
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_SETITEM // 정리 해야 할 코드
#endif	//LJH_FIX_NOT_CALCULATED_BUFF

	///// PSW_BUGFIX_REQUIREEQUIPITEM ////
	// 장비를 착용 하고 있더라도 장착한 장비의 
	// 요구 조건이 안 맞을 경우 장착한 
	// 장비의 효과을 적용 하지 않는다

	CalculateDamage();				// 수정( 셋트 옵션은 빼고 )
	CalculateMagicDamage();			// 수정( 셋트 옵션은 빼고 )
	CalculateCurseDamage();			// 수정
	CalculateAttackRating();		// 수정 할 필요 없음
	CalculateAttackSpeed();			// 수정
	CalculateSuccessfulBlocking();  // 수정
	CalculateDefense();             // 수정
	CalculateMagicDefense();        // 수정
	CalculateWalkSpeed();           // 수정
	//CalculateNextExperince();
	CalculateSuccessfulBlockingPK(); // 수정 할 필요 없음
	CalculateAttackRatingPK();       // 수정 할 필요 없음

	///// PSW_BUGFIX_REQUIREEQUIPITEM ////


	MONSTER_ATTRIBUTE *c   = &Enemy.Attribute;
	FinalAttackDamageRight = AttackDamageRight - c->Defense;
	FinalAttackDamageLeft  = AttackDamageLeft  - c->Defense;
	int EnemyAttackDamage  = c->AttackDamageMin + rand()%(c->AttackDamageMax-c->AttackDamageMin+1);
	FinalHitPoint          = EnemyAttackDamage - Character.Defense;
	FinalAttackRating      = Character.AttackRating - c->SuccessfulBlocking;
	FinalDefenseRating     = Character.SuccessfulBlocking - c->AttackRating;

#ifdef ANTIHACKING_ENABLE
	// 체크 코드
	if ( g_bNewFrame)
	{
		switch ( GetTickCount() % 5000)
		{
		case 1321:
			hanguo_check4();
			break;
		case 4321:
			hanguo_check2();
			break;
		}
	}
#endif //ANTIHACKING_ENABLE

	if(FinalAttackDamageRight < 0)
		FinalAttackDamageRight = 0;
	if(FinalAttackDamageLeft < 0)
		FinalAttackDamageLeft = 0;
	if(FinalHitPoint < 0)
		FinalHitPoint = 0;
	if(FinalAttackRating < 0)
		FinalAttackRating = 0;
	else if(FinalAttackRating > 100)
		FinalAttackRating = 100;
	if(FinalDefenseRating < 0)
		FinalDefenseRating = 0;
	else if(FinalDefenseRating > 100)
		FinalDefenseRating = 100;
	if(rand()%100 < FinalAttackRating)
		FinalSuccessAttack = true;
	else
		FinalSuccessAttack = false;
	if(rand()%100 < FinalDefenseRating)
		FinalSuccessDefense = true;
	else
		FinalSuccessDefense = false;
}

void CHARACTER_MACHINE::GetMagicSkillDamage( int iType, int *piMinDamage, int *piMaxDamage)
{
	// 소환수 스킬은 저주력과 관련.
	if (AT_SKILL_SUMMON_EXPLOSION <= iType && iType <= AT_SKILL_SUMMON_REQUIEM)
	{
		*piMinDamage = Character.MagicDamageMin;
		*piMaxDamage = Character.MagicDamageMax;
		return;
	}


	SKILL_ATTRIBUTE *p = &SkillAttribute[iType];
	
    int Damage = p->Damage;

	*piMinDamage = Character.MagicDamageMin+Damage;
	*piMaxDamage = Character.MagicDamageMax+Damage+Damage/2;

#ifdef YDG_FIX_MAGIC_DAMAGE_CALC_ORDER
	Damage = 0;
	g_csItemOption.PlusSpecial ( (WORD*)&Damage,  AT_SET_OPTION_IMPROVE_MAGIC_POWER );	//	최종 마력 증가
	if(Damage != 0)
	{
		float fratio = 1.f + (float)Damage/100.f;
		*piMinDamage *= fratio;
		*piMaxDamage *= fratio;
	}

    Damage = 0;
    g_csItemOption.ClearListOnOff();
    g_csItemOption.PlusMastery ( &Damage, p->MasteryType );
    g_csItemOption.PlusSpecial ( (WORD*)&Damage,  AT_SET_OPTION_IMPROVE_SKILL_ATTACK );	//	스킬 공격력 증가.
    *piMinDamage += Damage;
    *piMaxDamage += Damage;
#else	// YDG_FIX_MAGIC_DAMAGE_CALC_ORDER
    Damage = 0;
    g_csItemOption.ClearListOnOff();
    g_csItemOption.PlusMastery ( &Damage, p->MasteryType );
    g_csItemOption.PlusSpecial ( (WORD*)&Damage,  AT_SET_OPTION_IMPROVE_SKILL_ATTACK );	//	스킬 공격력 증가.
    *piMinDamage += Damage;
    *piMaxDamage += Damage;
	
	Damage = 0;
	g_csItemOption.PlusSpecial ( (WORD*)&Damage,  AT_SET_OPTION_IMPROVE_MAGIC_POWER );	//	최종 마력 증가
	if(Damage != 0)
	{
		float fratio = 1.f + (float)Damage/100.f;
		*piMinDamage *= fratio;
		*piMaxDamage *= fratio;
	}
#endif	// YDG_FIX_MAGIC_DAMAGE_CALC_ORDER
}

// 저주력 스킬 데미지.
void CHARACTER_MACHINE::GetCurseSkillDamage(int iType, int *piMinDamage, int *piMaxDamage)
{
	if (CLASS_SUMMONER != GetBaseClass(Character.Class))	// 소환술사만 저주력이 필요하다.
		return;

	// 소환수 스킬은 저주력과 관련.
	if (AT_SKILL_SUMMON_EXPLOSION <= iType && iType <= AT_SKILL_SUMMON_REQUIEM)
	{
		SKILL_ATTRIBUTE *p = &SkillAttribute[iType];
		*piMinDamage = Character.CurseDamageMin+p->Damage;
		*piMaxDamage = Character.CurseDamageMax+p->Damage+p->Damage/2;
	}
	else
	{
		*piMinDamage = Character.CurseDamageMin;
		*piMaxDamage = Character.CurseDamageMax;
	}
}

void CHARACTER_MACHINE::GetSkillDamage( int iType, int *piMinDamage, int *piMaxDamage)
{

	SKILL_ATTRIBUTE *p = &SkillAttribute[iType];
	
    int Damage = p->Damage;

	*piMinDamage = Damage;
	*piMaxDamage = Damage+Damage/2;

    Damage = 0;
    //  세트 옵션 적용.
    g_csItemOption.ClearListOnOff();
    g_csItemOption.PlusMastery ( &Damage, p->MasteryType );
    g_csItemOption.PlusSpecial ( (WORD*)&Damage,  AT_SET_OPTION_IMPROVE_SKILL_ATTACK );	//	스킬 공격력 증가.
    *piMinDamage += Damage;
    *piMaxDamage += Damage;

}

BYTE CHARACTER_MACHINE::GetSkillMasteryType ( int iType )
{
    BYTE MasteryType = 255;
    SKILL_ATTRIBUTE* p = &SkillAttribute[iType];

    MasteryType = p->MasteryType;

    return MasteryType;
}

// 스킬 타입을 넣으면 스킬에 대한 정보등을 넘겨준다.
void GetSkillInformation( int iType, int iLevel, char *lpszName, int *piMana, int *piDistance, int *piSkillMana)
{

	SKILL_ATTRIBUTE *p = &SkillAttribute[iType];
	if ( lpszName)
	{
#ifdef USE_SKILL_LEVEL
		if ( iLevel == 0)
		{
			strcpy( lpszName, p->Name);
		}
		else
		{
			sprintf( lpszName, "%s +%d", p->Name, iLevel);
		}
#else
		strcpy( lpszName, p->Name);
#endif
	}
	if ( piMana)
	{
		*piMana = p->Mana;
	}
	if ( piDistance)
	{
		*piDistance = p->Distance;
	}
	if (piSkillMana)
	{
		*piSkillMana = p->AbilityGuage;
	}

}

void GetSkillInformation_Energy(int iType, int *piEnergy)
{

	SKILL_ATTRIBUTE *p = &SkillAttribute[iType];

	if(piEnergy)
	{
		if(p->Energy == 0)
		{
			*piEnergy = 0;
		}
		else
		{
			*piEnergy = 20 + (p->Energy*p->Level*4/100);

			if( iType == AT_SKILL_SUMMON_EXPLOSION || iType == AT_SKILL_SUMMON_REQUIEM ){
				*piEnergy = 20 + (p->Energy*p->Level*3/100);
			}

#ifdef PSW_BUGFIX_CLASS_KNIGHT_REQUIRESKILL
			if( GetBaseClass ( Hero->Class ) == CLASS_KNIGHT ) {
				*piEnergy = 10 + (p->Energy*p->Level*4/100);
			}
#endif //PSW_BUGFIX_CLASS_KNIGHT_REQUIRESKILL
		}
	}

}

void GetSkillInformation_Charisma(int iType, int *piCharisma)
{
	
	SKILL_ATTRIBUTE *p = &SkillAttribute[iType];

	if(piCharisma)
	{
		*piCharisma = p->Charisma;
	}

}

void GetSkillInformation_Damage(int iType, int *piDamage)
{
	
	SKILL_ATTRIBUTE *p = &SkillAttribute[iType];

	if(piDamage)
	{
		*piDamage = p->Damage;
	}

}

//////////////////////////////////////////////////////////////////////////
//  스킬의 거리를 알려준다.
//////////////////////////////////////////////////////////////////////////
float GetSkillDistance(int Index, CHARACTER* c)
{
    float Distance = (float)( SkillAttribute[Index].Distance );

	if( Index == AT_SKILL_BLOW_UP + 4 ) 
	{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
		if(Distance != 3)
			int aaa = 0;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	}

    if(c != NULL)
    {
        // 다크로드의 경우 다크호스를 탔을 경우는 거리 +2를 해준다.
        if(c->Helper.Type == MODEL_HELPER+4)
		{
            Distance += 2;
		}
    }

    return Distance;
}

//////////////////////////////////////////////////////////////////////////
//  현재 사용하는 스킬을 발동할수 있는지 검사한다. ( 스킬 딜레이가 없을때 가능 );
//////////////////////////////////////////////////////////////////////////
bool    CheckSkillDelay ( int SkillIndex )
{
	int Skill = CharacterAttribute->Skill[SkillIndex];  //  스킬.

    int Delay = SkillAttribute[Skill].Delay;            //  스킬 딜레이.

#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	if(!CheckAttack() && (Skill == AT_SKILL_GIANTSWING || Skill == AT_SKILL_DRAGON_LOWER ||
		Skill == AT_SKILL_DRAGON_KICK))
	{
		return false;
	}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
    //  스킬 딜레이가 존재하는 스킬.
    if ( Delay>0 )
    {
        if ( CharacterAttribute->SkillDelay[SkillIndex] > 0 )
        {
            return false;
        }

		int iCharisma;
		GetSkillInformation_Charisma(Skill, &iCharisma);	// 통솔포인트
		if(iCharisma > (CharacterAttribute->Charisma + CharacterAttribute->AddCharisma))
		{
            return false;
		}

		CharacterAttribute->SkillDelay[SkillIndex] = Delay;
    }
    return true;
}


//////////////////////////////////////////////////////////////////////////
//  스킬 딜레이를 계산한다.
//////////////////////////////////////////////////////////////////////////
void    CalcSkillDelay ( int time )
{

	//^ 펜릴 스킬 관련(딜레이 계산)
	int iSkillNumber;
	iSkillNumber = CharacterAttribute->SkillNumber+2;
	iSkillNumber = min(iSkillNumber, MAX_SKILLS);

    for ( int i=0; i<iSkillNumber; ++i )
    {
		if ( CharacterAttribute->SkillDelay[i] <= 0 ) 
			continue;

        CharacterAttribute->SkillDelay[i] -= time;
        if ( CharacterAttribute->SkillDelay[i]<0 )
        {
            CharacterAttribute->SkillDelay[i] = 0;
        }
    }
	

#ifdef STATE_LIMIT_TIME
    if ( g_iWorldStateTime>0 )
    {
        g_iWorldStateTime -= time;
        if ( g_iWorldStateTime<=0 )
        {
            if ( ((Hero->Object.State&STATE_FREEZE)==STATE_FREEZE) )
                Hero->Object.State -= STATE_FREEZE;
            if ( ((Hero->Object.State&STATE_STUN)==STATE_STUN) )
                Hero->Object.State -= STATE_STUN;
            if ( ((Hero->Object.State&STATE_PARALYZE)==STATE_PARALYZE) )
                Hero->Object.State -= STATE_PARALYZE;

            g_iWorldStateTime = 0;
        }
    }
#endif// STATE_LIMIT_TIME
}


//////////////////////////////////////////////////////////////////////////
//  현재 주인공의 상태 마법이 갱신되었는지 설정한다.
//////////////////////////////////////////////////////////////////////////
#ifdef STATE_LIMIT_TIME
void    SetWorldStateTime ( DWORD State, int Time )
{
    if ( g_dwWorldStateBack!=State )
    {
        g_dwWorldStateBack = State;
        g_iWorldStateTime = Time;
    }
}
#endif// STATE_LIMIT_TIME


//////////////////////////////////////////////////////////////////////////
//  스킬 딜레이 값을 표시한다.
//////////////////////////////////////////////////////////////////////////
void    RenderSkillDelay ( int SkillIndex, int x, int y, int Width, int Height )
{
    int Delay = CharacterAttribute->SkillDelay[SkillIndex];
    if ( Delay>0 )
    {
        int Type  = CharacterAttribute->Skill[SkillIndex];

        int MaxDelay = SkillAttribute[Type].Delay;

        float Persent = (float)(Delay/(float)MaxDelay);
        
        EnableAlphaTest ();
        glColor4f ( 1.f, 0.5f, 0.5f, 0.5f );
        float deltaH = Height*Persent;
        RenderColor ( (float)x, (float)y+Height-deltaH, (float)Width, deltaH );
        DisableAlphaBlend();
    }
}

// 길마인가
bool IsGuildMaster()
{
	return ( Hero->GuildStatus == G_MASTER );
}
// 부길마인가
bool IsSubGuildMaster()
{
	return ( Hero->GuildStatus == G_SUB_MASTER );
}		
// 배틀마스터인가
bool IsBattleMaster()
{
	return ( Hero->GuildStatus == G_BATTLE_MASTER );
}		
