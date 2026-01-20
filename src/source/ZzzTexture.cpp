///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <setjmp.h>
#include <array>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>
#include "ZzzTexture.h"

#include "WSclient.h"
#include "turbojpeg.h"

CGlobalBitmap Bitmaps;

bool WriteJpeg(wchar_t* filename, int Width, int Height, unsigned char* Buffer, int quality)
{
    const auto fileCloser = [](FILE* fp) { if (fp != nullptr) { fclose(fp); } };
    std::unique_ptr<FILE, decltype(fileCloser)> outfile(_wfopen(filename, L"wb"), fileCloser);
    if (!outfile)
    {
        return false;
    }

    const auto jpegDestroyer = [](tjhandle handle) { if (handle != nullptr) { tjDestroy(handle); } };
    std::unique_ptr<void, decltype(jpegDestroyer)> handle(tjInitCompress(), jpegDestroyer);
    if (!handle)
    {
        return false;
    }

    const auto maxSize = tjBufSize(Width, Height, TJSAMP_444);
    std::vector<unsigned char> outputBuffer(maxSize);
    unsigned long jpegSize = maxSize;
    unsigned char* jpegPtr = outputBuffer.data();
    const int flags = TJFLAG_BOTTOMUP | TJFLAG_NOREALLOC;
    const auto result = tjCompress2(handle.get(), Buffer, Width, 0, Height, TJPF_RGB, &jpegPtr, &jpegSize, TJSAMP_444, quality, flags);

    if (result != 0)
    {
        return false;
    }

    const auto written = fwrite(jpegPtr, 1, jpegSize, outfile.get());
    return written == jpegSize;
}

namespace
{
std::wstring NormalizeExtension(const wchar_t* Ext)
{
    if (Ext == nullptr || Ext[0] == L'\0')
    {
        return {};
    }
    std::wstring result = Ext;
    if (result.front() != L'.')
    {
        result.insert(result.begin(), L'.');
    }
    return result;
}
} // namespace

void SaveImage(int HeaderSize, wchar_t* Ext, wchar_t* filename, BYTE* PakBuffer, int Size)
{
    if (filename == nullptr || Ext == nullptr)
    {
        return;
    }

    const bool hasExternalBuffer = (PakBuffer != nullptr && Size > 0);
    std::vector<unsigned char> localBuffer;
    if (!hasExternalBuffer)
    {
        std::wstring openFileName = L"Data2\\";
        openFileName += filename;
        const auto fileCloser = [](FILE* f) { if (f != nullptr) { fclose(f); } };
        std::unique_ptr<FILE, decltype(fileCloser)> fp(_wfopen(openFileName.c_str(), L"rb"), fileCloser);
        if (!fp)
        {
            return;
        }
        fseek(fp.get(), 0, SEEK_END);
        const auto fileSize = ftell(fp.get());
        if (fileSize <= 0)
        {
            return;
        }
        fseek(fp.get(), 0, SEEK_SET);
        localBuffer.resize(static_cast<size_t>(fileSize));
        const auto read = fread(localBuffer.data(), 1, localBuffer.size(), fp.get());
        if (read != localBuffer.size())
        {
            return;
        }
    }

    const unsigned char* buffer = hasExternalBuffer ? PakBuffer : localBuffer.data();
    const size_t bufferSize = hasExternalBuffer ? static_cast<size_t>(Size) : localBuffer.size();
    if (buffer == nullptr || bufferSize == 0 || bufferSize < static_cast<size_t>(HeaderSize))
    {
        return;
    }

    std::vector<unsigned char> header(buffer, buffer + HeaderSize);

    std::wstring newFileName = filename;
    const auto normalizedExt = NormalizeExtension(Ext);
    const auto dotPos = newFileName.find_last_of(L'.');
    if (dotPos != std::wstring::npos)
    {
        newFileName = newFileName.substr(0, dotPos);
    }
    newFileName += normalizedExt;

    std::wstring saveFileName = L"Data\\";
    saveFileName += newFileName;

    const auto fileCloser = [](FILE* f) { if (f != nullptr) { fclose(f); } };
    std::unique_ptr<FILE, decltype(fileCloser)> fp(_wfopen(saveFileName.c_str(), L"wb"), fileCloser);
    if (!fp)
    {
        return;
    }

    const auto headerWritten = fwrite(header.data(), 1, HeaderSize, fp.get());
    const auto dataWritten = fwrite(buffer, 1, bufferSize, fp.get());
    if (headerWritten != static_cast<size_t>(HeaderSize) || dataWritten != bufferSize)
    {
        return;
    }
}

bool OpenJpegBuffer(wchar_t* filename, float* BufferFloat)
{
    if (filename == nullptr || BufferFloat == nullptr)
    {
        return false;
    }

    std::wstring newFileName = filename;
    const auto dotPos = newFileName.find_last_of(L'.');
    if (dotPos != std::wstring::npos)
    {
        newFileName = newFileName.substr(0, dotPos);
    }
    std::wstring fileName = L"Data\\";
    fileName += newFileName;
    fileName += L".OZJ";

    auto compressedFile = _wfopen(fileName.c_str(), L"rb");
    if (compressedFile == nullptr)
    {
        wchar_t Text[256];
        mu_swprintf(Text, L"%ls - File not exist.", fileName.c_str());
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
        fclose(compressedFile);
        return false;
    }

    fseek(compressedFile, 24, SEEK_SET);
    const auto jpegSize = fileSize - 24;
    std::vector<unsigned char> jpegBuf(static_cast<size_t>(jpegSize));
    fread(jpegBuf.data(), 1, jpegBuf.size(), compressedFile);
    fclose(compressedFile);

    int jpegWidth = 0;
    int jpegHeight = 0;
    int jpegSubsamp = TJSAMP_444;
    int jpegColorspace = TJCS_RGB;

    auto tjhandle = tjInitDecompress();
    if (tjhandle == nullptr)
    {
        return false;
    }

    auto result = tjDecompressHeader3(tjhandle, jpegBuf.data(), jpegBuf.size(), &jpegWidth, &jpegHeight, &jpegSubsamp, &jpegColorspace);
    if (result != 0)
    {
        tjDestroy(tjhandle);
        return false;
    }

    const auto bufferSize = static_cast<size_t>(jpegWidth) * static_cast<size_t>(jpegHeight) * 3;
    std::vector<unsigned char> buffer(bufferSize);
    result = tjDecompress2(tjhandle, jpegBuf.data(), jpegBuf.size(), buffer.data(), jpegWidth, 0, jpegHeight, TJPF_RGB, TJFLAG_BOTTOMUP);
    tjDestroy(tjhandle);
    if (result != 0)
    {
        return false;
    }

    for (size_t i = 0; i < bufferSize; ++i)
    {
        BufferFloat[i] = static_cast<float>(buffer[i]) / 255.f;
    }

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
            mu_swprintf(szErrorMsg, L"LoadBitmap Failed: %ls", szFullPath);
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