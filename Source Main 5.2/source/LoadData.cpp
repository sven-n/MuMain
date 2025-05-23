// LoadData.cpp: implementation of the CLoadData class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LoadData.h"

#include <codecvt>
#include <locale>
#include <shlwapi.h>

#include "GlobalBitmap.h"

#include "ZzzBMD.h"
#include "ZzzTexture.h"

CLoadData gLoadData;

CLoadData::CLoadData() // OK
{
}

CLoadData::~CLoadData() // OK
{
}

void CLoadData::AccessModel(int Type, wchar_t* Dir, wchar_t* FileName, int i)
{
    wchar_t Name[64];
    if (i == -1)
        swprintf(Name, L"%s.bmd", FileName);
    else if (i < 10)
        swprintf(Name, L"%s0%d.bmd", FileName, i);
    else
        swprintf(Name, L"%s%d.bmd", FileName, i);

    bool Success = false;

    Models[Type].m_iBMDSeqID = Type;

    Success = Models[Type].Open2(Dir, Name);

    if (Success == false && (wcscmp(FileName, L"Monster") == NULL || wcscmp(FileName, L"Player") == NULL || wcscmp(FileName, L"PlayerTest") == NULL || wcscmp(FileName, L"Angel") == NULL))
    {
        wchar_t Text[256];
        swprintf(Text, L"%s file does not exist.", Name);
        MessageBox(g_hWnd, Text, NULL, MB_OK);
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
    }
}

void CLoadData::OpenTexture(int Model, wchar_t* SubFolder, int Wrap, int Type, bool Check)
{
    BMD* pModel = &Models[Model];

    for (int i = 0; i < pModel->NumMeshs; i++)
    {
        Texture_t* pTexture = &pModel->Textures[i];

        int wchars_num = MultiByteToWideChar(CP_UTF8, 0, pTexture->FileName, -1, NULL, 0);
        auto* textureFileName = new wchar_t[wchars_num];
        MultiByteToWideChar(CP_UTF8, 0, pTexture->FileName, -1, textureFileName, wchars_num);

        wchar_t szFullPath[256] = { 0, };
        wcscpy(szFullPath, L"Data\\");
        wcscat(szFullPath, SubFolder);
        wcscat(szFullPath, textureFileName);

        wchar_t __ext[_MAX_EXT] = { 0, };
        _wsplitpath(textureFileName, NULL, NULL, NULL, __ext);
        if (pTexture->FileName[0] == 'h' && pTexture->FileName[1] == 'i' && pTexture->FileName[2] == 'd')
        {
            pModel->IndexTexture[i] = BITMAP_HIDE;
        }
        else if (tolower(__ext[1]) == 't') // TGA
        {
            pModel->IndexTexture[i] = Bitmaps.LoadImage(szFullPath, GL_NEAREST, Wrap);
        }
        else if (tolower(__ext[1]) == 'j') // JPG
        {
            pModel->IndexTexture[i] = Bitmaps.LoadImage(szFullPath, Type, Wrap);
        }

        bool isSkin = (pTexture->FileName[0] == 's' && pTexture->FileName[1] == 'k' && pTexture->FileName[2] == 'i')
            || !wcsnicmp(textureFileName, L"level", 5);
        bool isHair = pTexture->FileName[0] == 'h' && pTexture->FileName[1] == 'a' && pTexture->FileName[2] == 'i' && pTexture->FileName[3] == 'r';
        
        if (isSkin || isHair)
        {
            BITMAP_t* pBitmap =
                pModel->IndexTexture[i] != BITMAP_UNKNOWN
                ? Bitmaps.FindTexture(pModel->IndexTexture[i])
                : Bitmaps.FindTextureByName(textureFileName);

            if (pBitmap)
            {
                pBitmap->IsSkin = isSkin;
                pBitmap->IsHair = isHair;
            }
        }
        
        if (pModel->IndexTexture[i] == BITMAP_UNKNOWN)
        {
            if (auto pBitmap = Bitmaps.FindTextureByName(textureFileName))
            {
                // we try to find an already loaded texture based on the file name
                Bitmaps.LoadImage(pBitmap->BitmapIndex, pBitmap->FileName);
                pModel->IndexTexture[i] = pBitmap->BitmapIndex;
            }
            else
            {
                wchar_t szErrorMsg[256] = { 0, };
                swprintf(szErrorMsg, L"OpenTexture Failed: %s of %hs", szFullPath, pModel->Name);
#ifdef FOR_WORK
                PopUpErrorCheckMsgBox(szErrorMsg);
#else // FOR_WORK
                PopUpErrorCheckMsgBox(szErrorMsg, true);
#endif // FOR_WORK
            }
        }

        delete[] textureFileName;
    }
}