#include "stdafx.h"
#include "CSProtocol.h"
#include "CharacterManager.h"
#include "Protect.h"
#include "QueryManager.h"
#include "ServerManager.h"
#include "Util.h"

void CSDataSend(int index,BYTE* lpMsg,int size)
{
	switch(lpMsg[0])
	{
		case 0xC1:
			{
				BYTE send[8192];

				PSBMSG_HEAD pMsg;

				pMsg.set(0xE1,lpMsg[2],(size=(size+1)));

				memcpy(&send[0],&pMsg,sizeof(pMsg));

				memcpy(&send[4],&lpMsg[3],(size-4));

				gSocketManager.DataSend(index,send,size);
			}
			break;
		case 0xC2:
			{
				BYTE send[8192];

				PSWMSG_HEAD pMsg;

				pMsg.set(0xE1,lpMsg[3],(size=(size+1)));

				memcpy(&send[0],&pMsg,sizeof(pMsg));

				memcpy(&send[5],&lpMsg[4],(size-5));

				gSocketManager.DataSend(index,send,size);
			}
			break;
	}
}

void CSDataRecv(int index,BYTE head,BYTE* lpMsg,int size) // OK
{
	switch(lpMsg[0])
	{
		case 0xC1:
			{
				BYTE recv[8192];

				PBMSG_HEAD pMsg;

				pMsg.set(lpMsg[3],(size=(size-1)));

				memcpy(&recv[0],&pMsg,sizeof(pMsg));

				memcpy(&recv[3],&lpMsg[4],(size-3));

				ChatServerProtocolCore(index,pMsg.head,recv,size);
			}
			break;
		case 0xC2:
			{
				BYTE recv[8192];

				PWMSG_HEAD pMsg;

				pMsg.set(lpMsg[4],(size=(size-1)));

				memcpy(&recv[0],&pMsg,sizeof(pMsg));

				memcpy(&recv[4],&lpMsg[5],(size-4));

				ChatServerProtocolCore(index,pMsg.head,recv,size);
			}
			break;
	}
}

void ChatServerProtocolCore(int index,BYTE head,BYTE* lpMsg,int size) // OK
{
	PROTECT_START

	switch(head)
	{
		case 0x60:
			FriendListRequest((FHP_FRIENDLIST_REQ*)lpMsg,index);
			break;
		case 0x62:
			FriendStateClientRecv((FHP_FRIEND_STATE_C*)lpMsg,index);
			break;
		case 0x63:
			FriendAddRequest((FHP_FRIEND_ADD_REQ*)lpMsg,index);
			break;
		case 0x64:
			WaitFriendAddRequest((FHP_WAITFRIEND_ADD_REQ*)lpMsg,index);
			break;
		case 0x65:
			FriendDelRequest((FHP_FRIEND_ADD_REQ*)lpMsg,index);
			break;
		case 0x70:
			FriendMemoSend((FHP_FRIEND_MEMO_SEND*)lpMsg,index);
			break;
		case 0x71:
			FriendMemoListReq((FHP_FRIEND_MEMO_LIST_REQ*)lpMsg,index);
			break;
		case 0x72:
			FriendMemoReadReq((FHP_FRIEND_MEMO_RECV_REQ*)lpMsg,index);
			break;
		case 0x73:
			FriendMemoDelReq((FHP_FRIEND_MEMO_DEL_REQ*)lpMsg,index);
			break;
	}

	PROTECT_FINAL
}

void FriendListRequest(FHP_FRIENDLIST_REQ* lpMsg,int index)
{
	BYTE send[2048];

	FHP_FRIENDLIST_COUNT pMsg;

	pMsg.h.set(0x60,0);

	int size = sizeof(pMsg);

	pMsg.Number = lpMsg->Number;

	memcpy(pMsg.Name,lpMsg->Name,sizeof(pMsg.Name));

	pMsg.Count = 0;

	pMsg.MailCount = 0;

	gQueryManager.ExecQuery("WZ_UserGuidCreate '%s'",lpMsg->Name);

	gQueryManager.Fetch();

	gQueryManager.Close();

	gQueryManager.ExecQuery("SELECT GUID,MemoTotal FROM T_FriendMain WHERE Name='%s'",lpMsg->Name);

	gQueryManager.Fetch();

	DWORD guid = gQueryManager.GetAsInteger("GUID");

	pMsg.MailCount = gQueryManager.GetAsInteger("MemoTotal");

	gQueryManager.Close();

	if(gQueryManager.ExecQuery("SELECT FriendName,Del FROM T_FriendList WHERE GUID=%d",guid) != 0)
	{
		FHP_FRIENDLIST FriendList;

		while(gQueryManager.Fetch() != SQL_NO_DATA)
		{
			gQueryManager.GetAsString("FriendName",FriendList.Name,sizeof(FriendList.Name));

			FriendList.Server = ((gQueryManager.GetAsInteger("Del")==0)?(BYTE)GetServerCodeByName(FriendList.Name):0xFF);

			memcpy(&send[size],&FriendList,sizeof(FriendList));

			size += sizeof(FriendList);

			pMsg.Count++;
		}
	}

	gQueryManager.Close();

	pMsg.h.size[0] = SET_NUMBERHB(size);
	pMsg.h.size[1] = SET_NUMBERLB(size);

	memcpy(send,&pMsg,sizeof(pMsg));

	CSDataSend(index,send,size);

	WaitFriendListResult(index,guid,lpMsg->Number,lpMsg->Name);

	FriendMemoList(index,guid,lpMsg->Number,lpMsg->Name);
}

void FriendStateClientRecv(FHP_FRIEND_STATE_C* lpMsg,int index)
{

}

void FriendAddRequest(FHP_FRIEND_ADD_REQ* lpMsg,int index)
{
	gQueryManager.ExecQuery("WZ_WaitFriendAdd '%s','%s'",lpMsg->Name,lpMsg->FriendName);

	gQueryManager.Fetch();

	BYTE result = gQueryManager.GetResult(0);

	gQueryManager.Close();

	if(result == 8)
	{
		FHP_FRIEND_ADD_RESULT pMsg;

		pMsg.h.set(0x63,sizeof(pMsg));

		pMsg.Number = lpMsg->Number;

		pMsg.Result = 0;

		memcpy(pMsg.Name,lpMsg->Name,sizeof(pMsg.Name));

		memcpy(pMsg.FriendName,lpMsg->FriendName,sizeof(pMsg.FriendName));

		pMsg.Server = (BYTE)GetServerCodeByName(lpMsg->FriendName);

		gQueryManager.ExecQuery("WZ_FriendAdd '%s','%s'",lpMsg->Name,lpMsg->FriendName);

		gQueryManager.Fetch();

		pMsg.Result = gQueryManager.GetResult(0);

		gQueryManager.Close();

		CSDataSend(index,(BYTE*)&pMsg,pMsg.h.size);

		if(pMsg.Result == 1)
		{
			CHARACTER_INFO CharacterInfo;

			if(gCharacterManager.GetCharacterInfo(&CharacterInfo,lpMsg->FriendName) != 0)
			{
				FHP_FRIEND_STATE pMsg;

				pMsg.h.set(0x62,sizeof(pMsg));

				pMsg.Number = CharacterInfo.UserIndex;

				memcpy(pMsg.Name,lpMsg->FriendName,sizeof(pMsg.Name));

				memcpy(pMsg.FriendName,lpMsg->Name,sizeof(pMsg.FriendName));

				pMsg.State = (BYTE)GetServerCodeByName(lpMsg->FriendName);

				CServerManager* lpServerManager = FindServerByCode(CharacterInfo.GameServerCode);

				if(lpServerManager != 0){CSDataSend(lpServerManager->m_index,(BYTE*)&pMsg,pMsg.h.size);}
			}
		}
	}
	else
	{
		FHP_FRIEND_ADD_RESULT pMsg;

		pMsg.h.set(0x63,sizeof(pMsg));

		pMsg.Number = lpMsg->Number;

		pMsg.Result = result;

		memcpy(pMsg.Name,lpMsg->Name,sizeof(pMsg.Name));

		memcpy(pMsg.FriendName,lpMsg->FriendName,sizeof(pMsg.FriendName));

		pMsg.Server = 0xFF;

		CSDataSend(index,(BYTE*)&pMsg,pMsg.h.size);

		if(pMsg.Result == 1)
		{
			CHARACTER_INFO CharacterInfo;

			if(gCharacterManager.GetCharacterInfo(&CharacterInfo,lpMsg->FriendName) != 0)
			{
				FHP_WAITFRIENDLIST_COUNT pMsg;

				pMsg.h.set(0x61,sizeof(pMsg));

				pMsg.Number = CharacterInfo.UserIndex;

				memcpy(pMsg.Name,lpMsg->FriendName,sizeof(pMsg.Name));

				memcpy(pMsg.FriendName,lpMsg->Name,sizeof(pMsg.FriendName));

				CServerManager* lpServerManager = FindServerByCode(CharacterInfo.GameServerCode);

				if(lpServerManager != 0){CSDataSend(lpServerManager->m_index,(BYTE*)&pMsg,pMsg.h.size);}
			}
		}
	}
}

void WaitFriendAddRequest(FHP_WAITFRIEND_ADD_REQ* lpMsg,int index)
{
	FHP_WAITFRIEND_ADD_RESULT pMsg;

	pMsg.h.set(0x64,sizeof(pMsg));

	pMsg.Number = lpMsg->Number;

	pMsg.Result = lpMsg->Result;

	memcpy(pMsg.Name,lpMsg->Name,sizeof(pMsg.Name));

	memcpy(pMsg.FriendName,lpMsg->FriendName,sizeof(pMsg.FriendName));

	pMsg.pServer = (BYTE)GetServerCodeByName(lpMsg->FriendName);

	if(lpMsg->Result == 0)
	{
		gQueryManager.ExecQuery("WZ_WaitFriendDel '%s','%s'",lpMsg->Name,lpMsg->FriendName);

		gQueryManager.Fetch();

		pMsg.Result = gQueryManager.GetResult(0);

		gQueryManager.Close();

		CSDataSend(index,(BYTE*)&pMsg,pMsg.h.size);
	}
	else
	{
		gQueryManager.ExecQuery("WZ_FriendAdd '%s','%s'",lpMsg->Name,lpMsg->FriendName);

		gQueryManager.Fetch();

		pMsg.Result = gQueryManager.GetResult(0);

		gQueryManager.Close();

		CSDataSend(index,(BYTE*)&pMsg,pMsg.h.size);

		if(pMsg.Result == 1)
		{
			CHARACTER_INFO CharacterInfo;

			if(gCharacterManager.GetCharacterInfo(&CharacterInfo,lpMsg->FriendName) != 0)
			{
				FHP_FRIEND_STATE pMsg;

				pMsg.h.set(0x62,sizeof(pMsg));

				pMsg.Number = CharacterInfo.UserIndex;

				memcpy(pMsg.Name,lpMsg->FriendName,sizeof(pMsg.Name));

				memcpy(pMsg.FriendName,lpMsg->Name,sizeof(pMsg.FriendName));

				pMsg.State = (BYTE)GetServerCodeByName(lpMsg->FriendName);

				CServerManager* lpServerManager = FindServerByCode(CharacterInfo.GameServerCode);

				if(lpServerManager != 0){CSDataSend(lpServerManager->m_index,(BYTE*)&pMsg,pMsg.h.size);}
			}
		}
	}
}

void FriendDelRequest(FHP_FRIEND_ADD_REQ* lpMsg,int index)
{
	FHP_FRIEND_DEL_RESULT pMsg;

	pMsg.h.set(0x65,sizeof(pMsg));

	pMsg.Number = lpMsg->Number;

	pMsg.Result = 0;

	memcpy(pMsg.Name,lpMsg->Name,sizeof(pMsg.Name));

	memcpy(pMsg.FriendName,lpMsg->FriendName,sizeof(pMsg.FriendName));

	gQueryManager.ExecQuery("WZ_FriendDel '%s','%s'",lpMsg->Name,lpMsg->FriendName);

	gQueryManager.Fetch();

	pMsg.Result = gQueryManager.GetResult(0);

	gQueryManager.Close();

	CSDataSend(index,(BYTE*)&pMsg,pMsg.h.size);

	if(pMsg.Result == 1)
	{
		CHARACTER_INFO CharacterInfo;

		if(gCharacterManager.GetCharacterInfo(&CharacterInfo,lpMsg->FriendName) != 0)
		{
			FHP_FRIEND_STATE pMsg;

			pMsg.h.set(0x62,sizeof(pMsg));

			pMsg.Number = CharacterInfo.UserIndex;

			memcpy(pMsg.Name,lpMsg->FriendName,sizeof(pMsg.Name));

			memcpy(pMsg.FriendName,lpMsg->Name,sizeof(pMsg.FriendName));

			pMsg.State = 0xFF;

			CServerManager* lpServerManager = FindServerByCode(CharacterInfo.GameServerCode);

			if(lpServerManager != 0){CSDataSend(lpServerManager->m_index,(BYTE*)&pMsg,pMsg.h.size);}
		}
	}
}

void FriendMemoSend(FHP_FRIEND_MEMO_SEND* lpMsg,int index)
{
	FHP_FRIEND_MEMO_SEND_RESULT pMsg;

	pMsg.h.set(0x70,sizeof(pMsg));

	pMsg.Number = lpMsg->Number;

	memcpy(pMsg.Name,lpMsg->Name,sizeof(pMsg.Name));

	pMsg.Result = 1;

	pMsg.WindowGuid = lpMsg->WindowGuid;

    gQueryManager.BindParameterAsString(1,lpMsg->ToName,sizeof(lpMsg->ToName));

    gQueryManager.BindParameterAsString(2,lpMsg->Subject,sizeof(lpMsg->Subject));

	gQueryManager.ExecQuery("WZ_WriteMail '%s',?,?,'%d','%d'",lpMsg->Name,lpMsg->Dir,lpMsg->Action);

	gQueryManager.Fetch();

	DWORD memo = gQueryManager.GetResult(0);

	DWORD guid = gQueryManager.GetResult(1);

	gQueryManager.Close();

	if(memo <= 10)
	{
		pMsg.Result = (BYTE)memo;
		CSDataSend(index,(BYTE*)&pMsg,pMsg.h.size);
		return;
	}

	BYTE MemoBuff[1000];

	memset(MemoBuff,0,sizeof(MemoBuff));

	memcpy(MemoBuff,lpMsg->Memo,lpMsg->MemoSize);

	gQueryManager.BindParameterAsBinary(1,MemoBuff,sizeof(MemoBuff));

	gQueryManager.BindParameterAsBinary(2,lpMsg->Photo,sizeof(lpMsg->Photo));

	gQueryManager.ExecQuery("UPDATE T_FriendMail SET Memo=?,Photo=? WHERE MemoIndex=%d AND GUID=%d",memo,guid);

	gQueryManager.Close();

	CSDataSend(index,(BYTE*)&pMsg,pMsg.h.size);

	if(pMsg.Result == 1)
	{
		CHARACTER_INFO CharacterInfo;

		if(gCharacterManager.GetCharacterInfo(&CharacterInfo,lpMsg->ToName) != 0)
		{
			gQueryManager.ExecQuery("SELECT MemoIndex,FriendName,wDate,Subject,bRead FROM T_FriendMail WHERE MemoIndex=%d AND GUID=%d",memo,guid);

			gQueryManager.Fetch();

			FHP_FRIEND_MEMO_LIST pMsg;

			pMsg.h.set(0x71,sizeof(pMsg));

			pMsg.Number = CharacterInfo.UserIndex;

			pMsg.MemoIndex = gQueryManager.GetAsInteger("MemoIndex");

			gQueryManager.GetAsString("FriendName",pMsg.SendName,sizeof(pMsg.SendName));

			memcpy(pMsg.RecvName,lpMsg->ToName,sizeof(pMsg.RecvName));

			gQueryManager.GetAsString("wDate",pMsg.Date,sizeof(pMsg.Date));

			gQueryManager.GetAsString("Subject",pMsg.Subject,sizeof(pMsg.Subject));

			pMsg.read = gQueryManager.GetAsInteger("bRead");

			CServerManager* lpServerManager = FindServerByCode(CharacterInfo.GameServerCode);

			if(lpServerManager != 0){CSDataSend(lpServerManager->m_index,(BYTE*)&pMsg,sizeof(pMsg));}

			gQueryManager.Close();
		}
	}
}

void FriendMemoListReq(FHP_FRIEND_MEMO_LIST_REQ* lpMsg,int index)
{
	gQueryManager.ExecQuery("SELECT GUID FROM T_FriendMain WHERE Name='%s'",lpMsg->Name);

	gQueryManager.Fetch();

	DWORD guid = gQueryManager.GetAsInteger("GUID");

	gQueryManager.Close();

	FriendMemoList(index,guid,lpMsg->Number,lpMsg->Name);
}

void FriendMemoReadReq(FHP_FRIEND_MEMO_RECV_REQ* lpMsg,int index)
{
	gQueryManager.ExecQuery("SELECT GUID FROM T_FriendMain WHERE Name='%s'",lpMsg->Name);

	gQueryManager.Fetch();

	DWORD guid = gQueryManager.GetAsInteger("GUID");

	gQueryManager.Close();

	FHP_FRIEND_MEMO_RECV pMsg;

	pMsg.h.set(0x72,sizeof(pMsg));

	pMsg.Number = lpMsg->Number;

	memcpy(pMsg.Name,lpMsg->Name,sizeof(pMsg.Name));

	pMsg.MemoIndex = lpMsg->MemoIndex;

	gQueryManager.ExecQuery("SELECT Memo,Photo,Dir,Act FROM T_FriendMail WHERE MemoIndex=%d AND GUID=%d",lpMsg->MemoIndex,guid);

	gQueryManager.Fetch();

	gQueryManager.GetAsBinary("Photo",pMsg.Photo,sizeof(pMsg.Photo));

	pMsg.Dir = gQueryManager.GetAsInteger("Dir");

	pMsg.Action = gQueryManager.GetAsInteger("Act");

	gQueryManager.GetAsBinary("Memo",(BYTE*)pMsg.Memo,sizeof(pMsg.Memo));

	pMsg.MemoSize = strlen(pMsg.Memo);

	gQueryManager.Close();

	gQueryManager.ExecQuery("UPDATE T_FriendMail SET bRead=1 WHERE MemoIndex=%d AND GUID=%d",lpMsg->MemoIndex,guid);

	gQueryManager.Close();

	CSDataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
}

void FriendMemoDelReq(FHP_FRIEND_MEMO_DEL_REQ* lpMsg,int index)
{
	FHP_FRIEND_MEMO_DEL_RESULT pMsg;

	pMsg.h.set(0x73,sizeof(pMsg));

	pMsg.Result = 0;

	pMsg.MemoIndex = lpMsg->MemoIndex;

	pMsg.Number = lpMsg->Number;

	memcpy(pMsg.Name,lpMsg->Name,sizeof(pMsg.Name));

	gQueryManager.ExecQuery("WZ_DelMail '%s',%d",lpMsg->Name,lpMsg->MemoIndex);

	gQueryManager.Fetch();

	pMsg.Result = gQueryManager.GetResult(0);

	gQueryManager.Close();

	CSDataSend(index,(BYTE*)&pMsg,pMsg.h.size);
}

void WaitFriendListResult(int index,DWORD guid,WORD aIndex,char* name)
{
	if(gQueryManager.ExecQuery("SELECT FriendName FROM T_WaitFriend WHERE GUID=%d",guid) != 0)
	{
		while(gQueryManager.Fetch() != SQL_NO_DATA)
		{
			FHP_WAITFRIENDLIST_COUNT pMsg;

			pMsg.h.set(0x61,sizeof(pMsg));

			pMsg.Number = aIndex;

			memcpy(pMsg.Name,name,sizeof(pMsg.Name));

			gQueryManager.GetAsString("FriendName",pMsg.FriendName,sizeof(pMsg.FriendName));

			CSDataSend(index,(BYTE*)&pMsg,pMsg.h.size);
		}
	}

	gQueryManager.Close();
}

void FriendStateRecv(char* name,BYTE state)
{
	DWORD guid;

	if(gQueryManager.ExecQuery("SELECT GUID FROM T_FriendMain WHERE Name='%s'",name) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		return;
	}
	else
	{
		guid = gQueryManager.GetAsInteger("GUID");

		gQueryManager.Close();
	}

	if(gQueryManager.ExecQuery("SELECT FriendName,Del FROM T_FriendList WHERE GUID=%d",guid,0) != 0)
	{
		while(gQueryManager.Fetch() != SQL_NO_DATA)
		{
			FHP_FRIEND_STATE pMsg;

			pMsg.h.set(0x62,sizeof(pMsg));

			gQueryManager.GetAsString("FriendName",pMsg.Name,sizeof(pMsg.Name));

			CHARACTER_INFO CharacterInfo;

			if(gCharacterManager.GetCharacterInfo(&CharacterInfo,pMsg.Name) != 0)
			{
				pMsg.Number = CharacterInfo.UserIndex;

				memcpy(pMsg.FriendName,name,sizeof(pMsg.FriendName));

				pMsg.State = ((state==0)?((gQueryManager.GetAsInteger("Del")==0)?GetServerCodeByName(pMsg.FriendName):0xFF):0xFF);

				CServerManager* lpServerManager = FindServerByCode(CharacterInfo.GameServerCode);

				if(lpServerManager != 0){CSDataSend(lpServerManager->m_index,(BYTE*)&pMsg,pMsg.h.size);}
			}
		}
	}

	gQueryManager.Close();
}

void FriendMemoList(int index,DWORD guid,WORD aIndex,char* name)
{
	if(gQueryManager.ExecQuery("SELECT MemoIndex,FriendName,wDate,Subject,bRead FROM T_FriendMail WHERE GUID=%d",guid) != 0)
	{
		while(gQueryManager.Fetch() != SQL_NO_DATA)
		{
			FHP_FRIEND_MEMO_LIST pMsg;

			pMsg.h.set(0x71,sizeof(pMsg));

			pMsg.Number = aIndex;

			pMsg.MemoIndex = gQueryManager.GetAsInteger("MemoIndex");

			gQueryManager.GetAsString("FriendName",pMsg.SendName,sizeof(pMsg.SendName));

			memcpy(pMsg.RecvName,name,sizeof(pMsg.RecvName));

			gQueryManager.GetAsString("wDate",pMsg.Date,sizeof(pMsg.Date));

			gQueryManager.GetAsString("Subject",pMsg.Subject,sizeof(pMsg.Subject));

			pMsg.read = gQueryManager.GetAsInteger("bRead");

			CSDataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
		}
	}

	gQueryManager.Close();
}
