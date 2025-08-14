#pragma once
#define MIN(a, b)			(((a) < (b)) ? (a) : (b))
#define MAX(a, b)			(((a) > (b)) ? (a) : (b))
#define LIMIT(val, l, h)	((val) < (l) ? (l) : (val) > (h) ? (h) : (val))
#define SWAP(a, b)			{ (a) ^= (b) ^= (a) ^= (b); }
#define	ARGB(a,r,g,b)	(((DWORD)(a))<<24 | (DWORD)(r) | ((DWORD)(g))<<8 | ((DWORD)(b))<<16)


inline int Random(int nMin, int nMax)
{
    return rand() % (nMax - nMin + 1) + nMin;
}

bool ReduceStringByPixel(LPTSTR lpszDst, int nDstSize, LPCTSTR lpszSrc, int nPixel);
#if defined KJH_ADD_INGAMESHOP_UI_SYSTEM || defined LJH_MOD_TO_USE_DIVIDESTRINGBYPIXEL_FUNC
int DivideStringByPixel(wchar_t* alpszDst, int nDstRow, int nDstColumn, const wchar_t* lpszSrc,
    int nPixelPerLine, bool bSpaceInsert = true, const wchar_t szNewLineChar = L';');
#else // define KJH_ADD_INGAMESHOP_UI_SYSTEM || define LJH_MOD_TO_USE_DIVIDESTRINGBYPIXEL_FUNC
int DivideStringByPixel(LPTSTR alpszDst, int nDstRow, int nDstColumn, LPCTSTR lpszSrc,
    int nPixelPerLine, bool bSpaceInsert = true);
#endif // define KJH_ADD_INGAMESHOP_UI_SYSTEM || define LJH_MOD_TO_USE_DIVIDESTRINGBYPIXEL_FUNC
int DivideString(LPTSTR alpszDst, int nDstRow, int nDstColumn, LPCTSTR lpszSrc);

BOOL CheckErrString(LPTSTR lpszTarget);

