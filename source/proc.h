#ifndef _PROC_INCLUDE_
#define _PROC_INCLUDE_

#ifndef	EVP_MAX_IV_LENGTH
#define	EVP_MAX_IV_LENGTH	8
#endif
#define connect			static_connect
#define	send(s, buf, len, flag)		wraperwraper(s, HIWORD(buf), len, flag, LOWORD(buf))
#define closesocket		static_closesocket

#ifdef __cplusplus
extern "C" {
#endif

extern DWORD rawret;

BOOL __fastcall init_hanguo_protect(HINSTANCE hInstance);
BOOL __fastcall exit_hanguo_protect();
int __stdcall send_wraper(int len, int flag, DWORD hiword_call_return, DWORD hiword_buf, DWORD loword_call_return, DWORD loword_buf, SOCKET s);
int Rand(int, int);
int __cdecl iv_with_type0(int);
int __cdecl iv_with_type1(int);
int __cdecl iv_with_type2(int);
int __cdecl iv_with_type3(int);
int __cdecl iv_with_type4(int);
int __cdecl iv_with_type5(int);
int __cdecl iv_with_type6(int);
int __cdecl iv_with_type7(int);
int __cdecl iv_with_type8(int);
int __cdecl iv_with_type9(int);

typedef struct ivordinal {
	struct ivordinal *pprev;
	struct ivordinal *pnext;
	DWORD ordinal;
	BYTE iv[EVP_MAX_IV_LENGTH];
} ivordinal;
extern ivordinal *pHead;

typedef struct fpd
{
	BYTE ordinal;
	DWORD hiword_addr;
	DWORD loword_addr;
} fpd;
extern fpd flipdata_presentdata[];
extern int flipdata_index;

__forceinline void pre_send(HINSTANCE hInstance)
{
	WORD 		l;
	PIMAGE_DOS_HEADER		pDOSHeader;
	PIMAGE_NT_HEADERS		pNTHeader;
	PIMAGE_SECTION_HEADER	pSectionHeader;
	BYTE chksum[EVP_MAX_IV_LENGTH];
	ivordinal *pivo, *tpivo;
	WORD value;
	int i;

	pDOSHeader		=(PIMAGE_DOS_HEADER)hInstance;
	pNTHeader		=(PIMAGE_NT_HEADERS)((DWORD)pDOSHeader->e_lfanew + (DWORD)hInstance);
	pSectionHeader	=(PIMAGE_SECTION_HEADER)((DWORD)pNTHeader + sizeof(IMAGE_NT_HEADERS));

	ZeroMemory(chksum, sizeof(chksum));

	for(l=0;;l++) if(memcmp(".text", pSectionHeader[l].Name, 6)==0) break;

	if(!Rand(0, 10)) {
		for(i=0; i<(int)(pSectionHeader[l].SizeOfRawData>>2); i++)
		{
			((DWORD *)chksum)[i&1] += ((DWORD *)((DWORD)(pSectionHeader[l].VirtualAddress)+(DWORD)hInstance))[i];
		}
		pivo = (ivordinal *)calloc(1, sizeof(ivordinal));
		pivo->ordinal = 0x80000000|Rand(20,30);
		tpivo = pHead;
		while(tpivo->ordinal!=(pivo->ordinal&0x7fffffff)) {
			tpivo = tpivo->pnext;
		}
		while(tpivo->ordinal==(pivo->ordinal&0x7fffffff)) {
			tpivo = tpivo->pnext;
		}
		pivo->pnext = tpivo;
		pivo->pprev = tpivo->pprev;
		tpivo->pprev->pnext = pivo;
		tpivo->pprev = pivo;

		value = (WORD)(pivo->ordinal&0x7fffffff);
		for(i=0;i<EVP_MAX_IV_LENGTH;i++)
			value+=chksum[i]*((WORD)(pivo->ordinal&0x7fffffff));
		for(i=0;i<EVP_MAX_IV_LENGTH;i++)
		{
			value += chksum[i];
			pivo->iv[i]=(BYTE)(value+chksum[i]);
		}
	}

	l=(WORD)((++flipdata_index)%10);
	switch(Rand(20, 30))
	{
	case 20:
		flipdata_presentdata[l].ordinal=(BYTE)20;
		flipdata_presentdata[l].hiword_addr=HIWORD(iv_with_type0);
		flipdata_presentdata[l].loword_addr=LOWORD(iv_with_type0);
		break;
	case 21:
		flipdata_presentdata[l].ordinal=(BYTE)21;
		flipdata_presentdata[l].hiword_addr=HIWORD(iv_with_type1);
		flipdata_presentdata[l].loword_addr=LOWORD(iv_with_type1);
		break;
	case 22:
		flipdata_presentdata[l].ordinal=(BYTE)22;
		flipdata_presentdata[l].hiword_addr=HIWORD(iv_with_type2);
		flipdata_presentdata[l].loword_addr=LOWORD(iv_with_type2);
		break;
	case 23:
		flipdata_presentdata[l].ordinal=(BYTE)23;
		flipdata_presentdata[l].hiword_addr=HIWORD(iv_with_type3);
		flipdata_presentdata[l].loword_addr=LOWORD(iv_with_type3);
		break;
	case 24:
		flipdata_presentdata[l].ordinal=(BYTE)24;
		flipdata_presentdata[l].hiword_addr=HIWORD(iv_with_type4);
		flipdata_presentdata[l].loword_addr=LOWORD(iv_with_type4);
		break;
	case 25:
		flipdata_presentdata[l].ordinal=(BYTE)25;
		flipdata_presentdata[l].hiword_addr=HIWORD(iv_with_type5);
		flipdata_presentdata[l].loword_addr=LOWORD(iv_with_type5);
		break;
	case 26:
		flipdata_presentdata[l].ordinal=(BYTE)26;
		flipdata_presentdata[l].hiword_addr=HIWORD(iv_with_type6);
		flipdata_presentdata[l].loword_addr=LOWORD(iv_with_type6);
		break;
	case 27:
		flipdata_presentdata[l].ordinal=(BYTE)27;
		flipdata_presentdata[l].hiword_addr=HIWORD(iv_with_type7);
		flipdata_presentdata[l].loword_addr=LOWORD(iv_with_type7);
		break;
	case 28:
		flipdata_presentdata[l].ordinal=(BYTE)28;
		flipdata_presentdata[l].hiword_addr=HIWORD(iv_with_type8);
		flipdata_presentdata[l].loword_addr=LOWORD(iv_with_type8);
		break;
	case 29:
		flipdata_presentdata[l].ordinal=(BYTE)29;
		flipdata_presentdata[l].hiword_addr=HIWORD(iv_with_type9);
		flipdata_presentdata[l].loword_addr=LOWORD(iv_with_type9);
		break;
	default:	// impossible
		flipdata_presentdata[l].ordinal=(BYTE)0;
		flipdata_presentdata[l].hiword_addr=HIWORD(hInstance);
		flipdata_presentdata[l].loword_addr=LOWORD(hInstance);
		break;
	}
}

__forceinline int wraperwraper(SOCKET socket, DWORD hiword_buf, int len, int flag, DWORD loword_buf)
{
	int retval;

	__asm	lea	eax, _send_ret
	__asm	mov	rawret, eax
	__asm	push	socket
	__asm	push	loword_buf
	__asm	lea		eax, _retaddr
	__asm	and		eax, 0xffff
	__asm	push	eax
	__asm	push	hiword_buf
	__asm	lea		eax, _retaddr
	__asm	shr		eax, 16
	__asm	push	eax
	__asm	push	flag
	__asm	push	len
	__asm	call	send_wraper
_retaddr:
	__asm	mov	retval, eax
	pre_send((HINSTANCE)0x400000);
_send_ret:
	__asm	mov	retval, eax
	return(retval);
}

int __fastcall static_connect(SOCKET s, const struct sockaddr * name, int namelen);
int __fastcall static_closesocket(SOCKET s);

void __fastcall hanguo_check1();
void __fastcall hanguo_check2();
void __fastcall hanguo_check3();
void __fastcall hanguo_check4();
void __fastcall hanguo_check5();
void __fastcall hanguo_check6();
void __fastcall hanguo_check7();
void __fastcall hanguo_check8();
void __fastcall hanguo_check9();
void __fastcall hanguo_check10();
void __fastcall hanguo_check11();
void __fastcall hanguo_check12();

#ifdef __cplusplus
}
#endif

#endif
