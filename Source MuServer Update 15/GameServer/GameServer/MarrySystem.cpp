#include "stdafx.h"

#if(GAMESERVER_TYPE==0)

#include "MarrySystem.h"
#include "Util.h"
#include "Notice.h"
#include "NewsProtocol.h"
#include "DSProtocol.h"
#include "ServerInfo.h"
#include "Protocol.h"
#include "Message.h"
#include "MemScript.h"
#include "PG_Custom.h"

Marry gMarry;

void Marry::LoadData(char * FilePath)
{
	this->Enabled = FALSE;

	CMemScript* lpMemScript = new CMemScript;

	if(lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR,FilePath);
		return;
	}

	if(lpMemScript->SetBuffer(FilePath) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

	for(int Level; Level < MAX_MAP; Level++)
	{
		Level = 0;

		this->MapIndex[Level] = 0;
	}

	try
	{
		while(true)
		{
			if(lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}
		
			int section = lpMemScript->GetNumber();

			while(true)
			{
				if(section == 0)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					this->Enabled = lpMemScript->GetNumber();

					this->MarryMap  = lpMemScript->GetAsNumber();

					this->MarryMaxTime  = lpMemScript->GetAsNumber();

				}
				else if (section == 1)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					this->ProposeX   = lpMemScript->GetNumber();
				
					this->ProposeY  = lpMemScript->GetAsNumber();

					this->AcceptX  = lpMemScript->GetAsNumber();
				
					this->AcceptY  = lpMemScript->GetAsNumber();

				}
				else if (section == 2)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					this->CostDivorce  = lpMemScript->GetNumber();
				
					this->CostForceDivorce  = lpMemScript->GetAsNumber();
				
					this->ShopEnabled  = lpMemScript->GetAsNumber();

				}
				else if (section == 3)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}
					
					int	Level = lpMemScript->GetNumber();
					
					this->MapIndex[Level] = lpMemScript->GetAsNumber();

				}
				else 
					break;
				}
			}
		}
	
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;	
}

void Marry::SendInfo(int aIndex)
{
	
	if(OBJECT_RANGE(aIndex) == 0)
	{ 
		return;
	}

	if(this->Enabled == true)
	{
		LPOBJ lpObj = &gObj[aIndex];

		SDHP_RECV_MARRY lpInfo;

		PHeadSubSetB((LPBYTE)&lpInfo, 0xFA,0x10, sizeof(lpInfo));

		memcpy(lpInfo.Name,lpObj->MarryCharacterInfo.MarriedName,sizeof(lpInfo.Name));
		
		lpInfo.CostDivorce = this->CostDivorce;
		lpInfo.CostForceDivorce = this->CostForceDivorce;
		lpInfo.TraceCost = 0;
		lpInfo.MAP = this->MarryMap;
		lpInfo.ProposeX = this->ProposeX;
		lpInfo.ProposeY = this->ProposeY;
		lpInfo.AcceptX = this->AcceptX;
		lpInfo.AcceptY = this->AcceptY;

		DataSend(lpObj->Index, (LPBYTE)&lpInfo, sizeof(lpInfo));	
	}
}

bool Marry::GetSex(LPOBJ lpObj)
{
	if( lpObj->Class == CLASS_SUMMONER || lpObj->Class == CLASS_ELF )
	{
		return 0;
	}else
	{
		return 1;
	}
}

int Marry::GetPos(LPOBJ lpObj)
{
	BYTE Map = lpObj->Map;

	BYTE X = lpObj->X;

	BYTE Y = lpObj->Y;

	if( ((X >= ProposeX) || (X <= ProposeY) ) && ((Y >= AcceptX) || 
		(Y <= AcceptY)) && (Map== MarryMap))
	{
		return 1;
	}
	return 0;
}

void Marry::Propose(LPOBJ lpObj,char * Wife)
{
	int lpTargetID = gObjGetIndex(Wife);
	
	int lpObjSex = GetSex(lpObj);
					
	int lpTargetSex = GetSex(&gObj[lpTargetID]);

	if(lpObj->MarryCharacterInfo.Married == 1)
	{
		gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"[Propose] You are already married!");
		return;
	}

	if(gObj[lpTargetID].MarryCharacterInfo.Married == 1)
	{
		gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"[Propose] %s already married!",gObj[lpTargetID].Name);
		return;
	}

	if(lpTargetID <= 0)
	{
		gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"[Propose] Character %s doesnt exist or is disconnected",Wife);
		return;
	}

	if(this->GetPos(lpObj) > 0 )
	{		
		if(this->GetPos(lpObj) > 0 && this->GetPos(&gObj[lpTargetID]) > 0 )
		{
			if(lpObjSex != lpTargetSex)
			{			
				if(gObj[lpTargetID].MarryCharacterInfo.Married == 1)
				{
					gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"[Propose] %s is already married",gObj[lpTargetID].Name);
					return;			
				}	
		
				lpObj->MarryCharacterInfo.MarrySeconds = 0;
				wsprintf(gObj[lpTargetID].MarryCharacterInfo.MarryTarget,"%s",lpObj->Name);
		
				gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"[Propose] waiting for response: %s"
				,gObj[lpTargetID].Name);
					
				gObj[lpTargetID].MarryCharacterInfo.MarrySeconds = this->MarryMaxTime;
				wsprintf(lpObj->MarryCharacterInfo.MarryTarget,"%s",gObj[lpTargetID].Name);
	
				gNotice.NewNoticeSend(gObj[lpTargetID].Index,0,0,0,0,0,"[Propose] Accept marry with %s?",lpObj->Name);


			}else{gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"You cant marry with some one with the same sex!");}
	
		}else
		{
			gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"[Propose] %s incorrect Position!",gObj[lpTargetID].Name);
			gNotice.NewNoticeSend(gObj[lpTargetID].Index,0,0,0,0,0,"[Propose] %s You position is %d,%d!",gObj[lpTargetID].Name,this->AcceptX,this->AcceptY);
		}
		
	}else{gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"[Propose] %s You position is %d,%d!",lpObj->Name,this->ProposeX,this->ProposeY);
			gNotice.NewNoticeSend(gObj[lpTargetID].Index,0,0,0,0,0,"[Propose] %s You position is %d,%d!",gObj[lpTargetID].Name,this->AcceptX,this->AcceptY);}

	
}

void Marry::Accept(LPOBJ lpObj)
{
	int lpTargetID = gObjGetIndex(lpObj->MarryCharacterInfo.MarryTarget);

	if(lpTargetID >= 0)
	{
		if(lpObj->MarryCharacterInfo.MarrySeconds >= this->MarryMaxTime) 
		{
			gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"[Accept] Time for answer is finished!");
			return;
		}

		if (lpObj->MarryCharacterInfo.Married == 0)
		{
			lpObj->MarryCharacterInfo.Married = 1;
			lpObj->MarryCharacterInfo.MarrySeconds = this->MarryMaxTime;
	
			wsprintf(lpObj->MarryCharacterInfo.MarriedName,"%s",lpObj->MarryCharacterInfo.MarryTarget);
			memset( lpObj->MarryCharacterInfo.MarryTarget, 0, 10);

			gObj[lpTargetID].MarryCharacterInfo.Married = 1;
			gObj[lpTargetID].MarryCharacterInfo.MarrySeconds = this->MarryMaxTime;
	
			wsprintf(gObj[lpTargetID].MarryCharacterInfo.MarriedName,"%s",gObj[lpTargetID].MarryCharacterInfo.MarryTarget);
			memset( gObj[lpTargetID].MarryCharacterInfo.MarryTarget, 0, 10);
		
			GCFireworksSend(lpObj,this->ProposeX,this->ProposeY);
			GCFireworksSend(lpObj,this->AcceptX,this->AcceptY);
			
			gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"[Accept] Congratulations you are married with %s" // - Ok
				,gObj[lpTargetID].Name);
		
			gNotice.NewNoticeSend(gObj[lpTargetID].Index,0,0,0,0,0,"[Accept] Congratulations you are married with %s" // - Ok
				,lpObj->Name);
					
			this->SendInfo(lpTargetID);
			this->SendInfo(lpObj->Index);
		
			gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(580));
			gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(581),gObj[lpTargetID].Name,lpObj->Name);

		}else{gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"[Accept] You are already married!");}

	}else{gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"[Accept] Character %s doesnt exist or is disconnected",lpObj->MarryCharacterInfo.MarryTarget);}
}

void Marry::Trace(LPOBJ lpObj)
{
	int lpTargetID = gObjGetIndex(lpObj->MarryCharacterInfo.MarriedName);
		
	int lpTargetMap = gObj[lpTargetID].Map;

	if(lpObj->MarryCharacterInfo.Married == 0)
	{
		gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"[Trace] You are not married!");
		return;
	}
	
	if(lpTargetID >= 0)
	{	
		if(lpTargetMap == MAP_ICARUS)
		{
			if(!(lpObj->Inventory[8].m_Index == GET_ITEM(13,3)|| lpObj->Inventory[7].m_Index == GET_ITEM(13,30)
				|| (lpObj->Inventory[7].m_Index >= GET_ITEM(12,0) && lpObj->Inventory[7].m_Index <=GET_ITEM(12,6))
				|| (lpObj->Inventory[7].m_Index >= GET_ITEM(12,130) && lpObj->Inventory[7].m_Index <=GET_ITEM(12,135))
				|| (lpObj->Inventory[7].m_Index >= GET_ITEM(12,36) && lpObj->Inventory[7].m_Index <=GET_ITEM(12,43))
				|| (lpObj->Inventory[7].m_Index >= GET_ITEM(12,49) && lpObj->Inventory[7].m_Index <=GET_ITEM(12,50))
				|| (lpObj->Inventory[7].m_Index >= GET_ITEM(12,200) && lpObj->Inventory[7].m_Index <= GET_ITEM(12,254))))
			{
				gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"You need wings to teleport to this map");
				return;
			}
		}

		if(lpTargetMap == MAP_KANTURU3)
		{
			if(!(lpObj->Inventory[8].m_Index == GET_ITEM(13,3)|| lpObj->Inventory[7].m_Index == GET_ITEM(13,30)
				|| (lpObj->Inventory[7].m_Index >= GET_ITEM(12,0)&& lpObj->Inventory[7].m_Index <=GET_ITEM(12,6))
				|| (lpObj->Inventory[7].m_Index >= GET_ITEM(12,130)&& lpObj->Inventory[7].m_Index <=GET_ITEM(12,135))
				|| (lpObj->Inventory[7].m_Index >= GET_ITEM(12,36)&& lpObj->Inventory[7].m_Index <=GET_ITEM(12,43))
				|| (lpObj->Inventory[7].m_Index >= GET_ITEM(12,49)&& lpObj->Inventory[7].m_Index <=GET_ITEM(12,50))
				|| (lpObj->Inventory[7].m_Index >= GET_ITEM(12,200) && lpObj->Inventory[7].m_Index <= GET_ITEM(12,254))))
			{
				gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"You need wings to teleport to this map");
				return;
			}
		}

		if(lpObj->PKLevel >= 5)
		{
			gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"Pk lvl can not teleport");
			return;
		}

		if( (lpTargetMap == 30) && (BC_MAP_RANGE(lpTargetMap) == 0) && 
			(CC_MAP_RANGE(lpTargetMap) == 0) && (IT_MAP_RANGE(lpTargetMap) == 0) && 
			(DS_MAP_RANGE(lpTargetMap) == 0) && (KALIMA_MAP_RANGE(lpTargetMap) == 0) && 
			(IG_MAP_RANGE(lpTargetMap) == 0) && 
			#if(SKY_EVENT == 1)
			//(SKYEVENT_MAP_RANGE(lpTargetMap,gObj[lpTargetID].X,gObj[lpTargetID].Y) == 0) && 
			#endif
			(DG_MAP_RANGE(lpTargetMap) == 0) && (lpTargetMap != 64))
		{
			gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"%s is inside of one event, you cant warp there!"
				,lpObj->MarryCharacterInfo.MarriedName);
			return;
		}
	// - REQ-Gens
	
		if(lpObj->Level < this->MapIndex[lpTargetMap])
		{
			gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"You need level %d for teleporting",this->MapIndex[lpTargetMap]);
			return;
		}

		#if(BOT_RACER == 1)

		if(lpObj->RaceCheck == 1)
		{
			BotRacer.ResetTime(lpObj->Index);
			gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"You cant use warp/tracemarry under the race, race its over.");
		}

		#endif

		int x = rand()%1;
		int y = rand()%1;
			
		x += gObj[lpTargetID].X;
		y += gObj[lpTargetID].Y;
		gObjTeleport(lpObj->Index, gObj[lpTargetID].Map, x, y);	
	}
	else{gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"%s is disconnected",lpObj->MarryCharacterInfo.MarriedName);}
}

void Marry::Divorce(LPOBJ lpObj)
{	
	int lpTargetID = gObjGetIndex(lpObj->MarryCharacterInfo.MarriedName);

	if(lpObj->MarryCharacterInfo.Married == 0)
	{
		gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"[Divorce] You are not married!");
		return;
	}

	if(lpTargetID <= 0)
	{
		gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"[Divorce] %s is disconnected"
			,lpObj->MarryCharacterInfo.MarriedName);
		return;
	}

	if(this->CostDivorce > lpObj->Money )
	{
		gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"[Divorce] You need %d zen to divorce"
			,this->CostDivorce);
		return;
	}

	lpObj->Money -= this->CostDivorce;
	GCMoneySend(lpObj->Index,lpObj->Money);				

	gObj[lpTargetID].Money += this->CostDivorce;
	GCMoneySend(gObj[lpTargetID].Index,gObj[lpTargetID].Money);

	lpObj->MarryCharacterInfo.Married = 0;
	memset( lpObj->MarryCharacterInfo.MarryTarget, 0, MAX_ACCOUNT_LEN);
	memset( lpObj->MarryCharacterInfo.MarriedName, 0, MAX_ACCOUNT_LEN);
	
	gObj[lpTargetID].MarryCharacterInfo.Married = 0;
	memset( gObj[lpTargetID].MarryCharacterInfo.MarryTarget, 0, MAX_ACCOUNT_LEN);
	memset( gObj[lpTargetID].MarryCharacterInfo.MarriedName, 0, MAX_ACCOUNT_LEN);
	
	gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"[Divorce] You get divorced, your Zen is decreased by %d"
		,this->CostDivorce);

	gNotice.NewNoticeSend(gObj[lpTargetID].Index,0,0,0,0,0,"[Divorce] You get divorced, your Zen is increase by %d"
		,this->CostDivorce);

	gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"[Divorce] Successfull divorce ");
	gNotice.NewNoticeSend(gObj[lpTargetID].Index,0,0,0,0,0,"[Divorce] %s decided to divorce ",lpObj->Name);

	this->SendInfo(lpObj->Index);			
	this->SendInfo(lpTargetID);	

	LogAdd(LOG_GREEN,"[Marry] %s and %s are no longer married",lpObj->Name,gObj[lpTargetID].Name);

}

void Marry::ForceDivorce(LPOBJ lpObj)
{
	int lpTargetID = gObjGetIndex(lpObj->MarryCharacterInfo.MarriedName);

	if(lpObj->MarryCharacterInfo.Married == 0)
	{
		gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"[ForceDivorce] You are not married!");
		return;
	}

	if(this->CostForceDivorce > lpObj->Money )
	{
		gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"[ForceDivorce] You need %d zen to force the divorce"
			,this->CostForceDivorce);
		return;
	}

	if(lpTargetID > 0)
	{
		gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"[ForceDivorce] %s is online please use /forcedivorce command"
			,lpObj->MarryCharacterInfo.MarriedName);
		return;
	}

	lpObj->Money -= this->CostForceDivorce;
	GCMoneySend(lpObj->Index,lpObj->Money);		

	JGSetPlayerMarryDivorseInfo(lpObj->MarryCharacterInfo.MarriedName);

	lpObj->MarryCharacterInfo.Married = 0;
	lpObj->MarryCharacterInfo.MarrySeconds = 0;
	memset( lpObj->MarryCharacterInfo.MarryTarget, 0, MAX_ACCOUNT_LEN);
	memset( lpObj->MarryCharacterInfo.MarriedName, 0, MAX_ACCOUNT_LEN);

	this->SendInfo(lpObj->Index);

	gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"[ForceDivorce] Successfull divorce");

}

void Marry::Status(LPOBJ lpObj)
{
	if(this->Enabled == 1)
	{
		if(lpObj->MarryCharacterInfo.Married == 1)
		{
			gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"Status Marry is: %d",lpObj->MarryCharacterInfo.Married);
			gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"You are married to: %s",lpObj->MarryCharacterInfo.MarriedName);
		}else{gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"%s, You are not married",lpObj->Name);}
	}
}

void Marry::CommandCore(LPOBJ lpObj,BYTE Command)
{
	if(this->Enabled == 1)
	{
		if(gServerInfo.CmdMarryAF > 0)
		{
			if(lpObj->MarryCharacterInfo.aFloodMarryCmd != 0)
			{
				gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"You must wait before use marry command again");
				return;	
			}
			lpObj->MarryCharacterInfo.aFloodMarryCmd = gServerInfo.CmdMarryAF;
		}

		switch(Command)
		{
			case 0:
			{
				if(lpObj->MarryCharacterInfo.Married == 1)
				{
					gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,"You are married with %s",lpObj->MarryCharacterInfo.MarryTarget);
				}else
				{
					gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,"You are not married!");
				}

			}break;
			case 1:
			{
				this->Accept(lpObj);
			}break;
			case 2:
			{
				this->Divorce(lpObj);
			}break;
			case 3:
			{
				this->ForceDivorce(lpObj);
			}break;
			case 4:
			{
				this->Trace(lpObj);
			}break;
			case 5:
			{
				this->Status(lpObj);
			}break;
		}
	}
}

#endif