///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <setjmp.h>
#include "ZzzTexture.h"

#include "WSclient.h"
#include "turbojpeg.h"

CGlobalBitmap Bitmaps;

bool WriteJpeg(wchar_t* filename, int Width, int Height, unsigned char* Buffer, int quality)
{
    FILE* outfile;
    if ((outfile = _wfopen(filename, L"wb")) == NULL)
    {
        //fprintf(stderr, L"can't open %s\n", filename);
        //exit(1);
        return FALSE;
    }

    const auto handle = tjInitCompress();
    unsigned long jpegSize = 0;
    auto maxSize = tjBufSize(Width, Height, TJSAMP_444);
    auto outputBuffer = new unsigned char[maxSize];
    
    tjCompress2(handle, Buffer, Width, 0, Height, TJPF_RGB, &outputBuffer, &jpegSize, TJSAMP_444, quality, TJFLAG_BOTTOMUP);
    fwrite(outputBuffer, 1, jpegSize, outfile);
    fclose(outfile);
    tjDestroy(handle);

    return TRUE;
}

void SaveImage(int HeaderSize, wchar_t* Ext, wchar_t* filename, BYTE* PakBuffer, int Size)
{
    if (PakBuffer == NULL || Size == 0)
    {
        wchar_t OpenFileName[256];
        wcscpy(OpenFileName, L"Data2\\");
        wcscat(OpenFileName, filename);
        FILE* fp = _wfopen(OpenFileName, L"rb");
        if (fp == NULL)
        {
            return;
        }
        fseek(fp, 0, SEEK_END);
        Size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        PakBuffer = new unsigned char[Size];
        fread(PakBuffer, 1, Size, fp);
        fclose(fp);
    }

    wchar_t Header[24];
    memcpy(Header, PakBuffer, HeaderSize);

    wchar_t NewFileName[256];
    int iTextcnt = 0;
    for (int i = 0; i < (int)wcslen(filename); i++)
    {
        iTextcnt = i;
        NewFileName[i] = filename[i];
        if (filename[i] == '.') break;
    }
    NewFileName[iTextcnt + 1] = NULL;
    wcscat(NewFileName, Ext);
    wchar_t SaveFileName[256];
    wcscpy(SaveFileName, L"Data\\");
    wcscat(SaveFileName, NewFileName);
    FILE* fp = _wfopen(SaveFileName, L"wb");
    if (fp == NULL) return;
    fwrite(Header, 1, HeaderSize, fp);
    fwrite(PakBuffer, 1, Size, fp);
    fclose(fp);

    if (PakBuffer == NULL || Size == 0)
    {
        SAFE_DELETE_ARRAY(PakBuffer);
    }
}

bool OpenJpegBuffer(wchar_t* filename, float* BufferFloat)
{
    
    wchar_t FileName[256];

    wchar_t NewFileName[256];
    int iTextcnt = 0;
    for (int i = 0; i < (int)wcslen(filename); i++)
    {
        iTextcnt = i;
        NewFileName[i] = filename[i];
        if (filename[i] == '.') break;
    }
    NewFileName[iTextcnt + 1] = NULL;
    wcscpy(FileName, L"Data\\");
    wcscat(FileName, NewFileName);
    wcscat(FileName, L"OZJ");

    auto compressedFile = _wfopen(FileName, L"rb");
    if (compressedFile == nullptr)
    {
        wchar_t Text[256];
        swprintf(Text, L"%s - File not exist.", FileName);
        g_ErrorReport.Write(Text);
        g_ErrorReport.Write(L"\r\n");
        MessageBox(g_hWnd, Text, NULL, MB_OK);
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
        return false;
    }

    fseek(compressedFile, 0, SEEK_END);
    const auto fileSize = ftell(compressedFile);
    if (fileSize < 24)
    {
        return false;
    }

    // Skip first 24 bytes, because these are added by the OZJ format
    fseek(compressedFile, 24, SEEK_SET);

    const auto jpegSize = fileSize - 24;
    int jpegWidth = 0, jpegHeight = 0;
    int jpegSubsamp = TJSAMP_444;
    int jpegColorspace = TJCS_RGB;

    auto tjhandle = tjInitDecompress();

    auto jpegBuf = new BYTE[jpegSize];
    fread(jpegBuf, 1, jpegSize, compressedFile);
    fclose(compressedFile);

    // First reading the header with the size information
    auto result = tjDecompressHeader3(tjhandle, jpegBuf, jpegSize, &jpegWidth, &jpegHeight, &jpegSubsamp, &jpegColorspace);
    if (result != 0)
    {
        delete[] jpegBuf;
        return false;
    }

    // decompress into the buffer
    const auto bufferSize = jpegWidth * jpegHeight * 3;
    const auto buffer = new BYTE[bufferSize];
    result = tjDecompress2(tjhandle, jpegBuf, jpegSize, buffer, jpegWidth, 0, jpegHeight, TJPF_RGB, TJFLAG_BOTTOMUP);
    delete[] jpegBuf;
    jpegBuf = nullptr;
    if (result != 0)
    {
        delete[] buffer;
        return false;
    }

    for (int i = 0; i < bufferSize; ++i)
    {
        BufferFloat[i] = static_cast<float>(buffer[i]) / 255.f;
    }

    delete[] buffer;
    
    return true;
}

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
bool LoadBitmap(const wchar_t* szFileName, GLuint uiTextureIndex, GLuint uiFilter, GLuint uiWrapMode, bool bCheck, bool bFullPath)
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
bool LoadBitmap(const wchar_t* szFileName, GLuint uiTextureIndex, GLuint uiFilter, GLuint uiWrapMode, bool bCheck)
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
{
    wchar_t szFullPath[256] = { 0, };
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    if (bFullPath == true)
    {
        wcscpy(szFullPath, szFileName);
    }
    else
    {
        wcscpy(szFullPath, L"Data\\");
        wcscat(szFullPath, szFileName);
    }
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
    wcscpy(szFullPath, L"Data\\");
    wcscat(szFullPath, szFileName);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
    if (bCheck)
    {
        if (false == Bitmaps.LoadImage(uiTextureIndex, szFullPath, uiFilter, uiWrapMode))
        {
            wchar_t szErrorMsg[256] = { 0, };
            swprintf(szErrorMsg, L"LoadBitmap Failed: %s", szFullPath);
#ifdef FOR_WORK
            PopUpErrorCheckMsgBox(szErrorMsg);
#else // FOR_WORK
            PopUpErrorCheckMsgBox(szErrorMsg, true);
#endif // FOR_WORK
            return false;
        }
        return true;
    }
    return Bitmaps.LoadImage(uiTextureIndex, szFullPath, uiFilter, uiWrapMode);
}
void DeleteBitmap(GLuint uiTextureIndex, bool bForce)
{
    Bitmaps.UnloadImage(uiTextureIndex, bForce);
}
void PopUpErrorCheckMsgBox(const wchar_t* szErrorMsg, bool bForceDestroy)
{
    wchar_t szMsg[1024] = { 0, };
    wcscpy(szMsg, szErrorMsg);

    if (bForceDestroy)
    {
        MessageBox(g_hWnd, szErrorMsg, L"ErrorCheckBox", MB_OK | MB_ICONERROR);
    }
    else
    {
        int iResult = MessageBox(g_hWnd, szMsg, L"ErrorCheckBox", MB_YESNO | MB_ICONERROR);
        if (IDYES == iResult)
        {
            return;
        }
    }

    if (SocketClient != nullptr)
    {
        SocketClient->Close();
    }

    KillGLWindow();
    DestroySound();
    DestroyWindow();
    ExitProcess(0);
}