// Test stubs for game symbols referenced by MUCore/MUPlatform/MURenderFX/MUData
// that are defined in game source files not linked into MuTests.
// These provide minimal definitions to satisfy the linker.
//
// Categories:
//   1. Global variable stubs (Winmain, ZzzInterface, WSclient, zzzpath)
//   2. Free function stubs (SceneManager, Winmain, UIControls, ZzzInterface)
//   3. Class method stubs (CGlobalBitmap, Connection, CUIRenderText, SEASON3B UI)
//   4. ShopListManager stubs (WZResult, DownloadFileInfo, CListManager, CShopListManager)
//   5. TurboJPEG C-linkage stubs
//   6. MUData texture helpers (real implementations — tested directly)

#ifdef _WIN32
#include <windows.h>
#else
#include "Platform/PlatformTypes.h"
#endif

#include <cstdint>
#include <cstring>
#include <cwchar>
#include <string>
#include <vector>

// Self-contained ShopListManager headers (no game/PCH deps)
#include "GameShop/ShopListManager/interface/WZResult/WZResult.h"
#include "GameShop/ShopListManager/interface/DownloadInfo.h"

// ===========================================================================
// 1. Global variable stubs
// ===========================================================================

// From Winmain.cpp
bool Destroy = false;
BOOL g_bUseWindowMode = TRUE;
bool g_bWndActive = false;
bool g_HasInactiveFpsOverride = false;
HDC g_hDC = nullptr;
HWND g_hWnd = nullptr;
int g_iMousePopPosition_x = 0;
int g_iMousePopPosition_y = 0;
int g_iNoMouseTime = 0;
double g_TargetFpsBeforeInactive = -1.0;

// From ZzzInterface.cpp
int EditFlag = 0;

// From WSclient.cpp — Connection* type; stub as void*
void* SocketClient = nullptr;

// From Connection.cpp
[[maybe_unused]] extern const std::string g_dotnetLibPath = "";

// From zzzpath.cpp
bool g_bShowPath = false;

// ===========================================================================
// 2. Free function stubs
// ===========================================================================

// From Winmain.cpp — called by ZzzTexture.cpp (MURenderFX)
void DestroySound() {}
void KillGLWindow() {}
void SetMaxMessagePerCycle(int /*messages*/) {}

// From SceneManager.cpp — called by MUPlatform, MUCore
double GetTargetFps() { return -1.0; }
void SetTargetFps(double /*targetFps*/) {}
void ResetFrameStats() {}
void SetShowDebugInfo(bool /*enabled*/) {}
void SetShowFpsCounter(bool /*enabled*/) {}

// From UIControls.cpp — called by UsefulDef.cpp (MUCore)
int CutText3(const wchar_t* /*pszText*/, wchar_t* /*pTextOut*/, int /*TargetWidth*/, int /*iMaxOutLine*/,
             int /*iOutStrLength*/, int /*iFirstLineTab*/, int /*bReverseWrite*/)
{
    return 0;
}

// From ZzzInterface.cpp — called by muConsoleDebug.cpp (MUCore)
bool PressKey(int /*Key*/) { return false; }

// From NewUICommon.cpp — called by muConsoleDebug.cpp (MUCore)
namespace SEASON3B
{
bool IsPress(int /*iVirtKey*/) { return false; }
} // namespace SEASON3B

// ===========================================================================
// 3. CGlobalBitmap stubs — defined in MUData (not linked into MuTests)
//    ZzzTexture.cpp (MURenderFX) provides the global storage; we provide methods.
// ===========================================================================

struct BITMAP_t;

class CGlobalBitmap
{
public:
    CGlobalBitmap();
    virtual ~CGlobalBitmap();

    unsigned int LoadImage(const std::wstring& filename, unsigned int uiFilter, unsigned int uiWrapMode);
    bool LoadImage(unsigned int uiBitmapIndex, const std::wstring& filename, unsigned int uiFilter,
                   unsigned int uiWrapMode);
    void UnloadImage(unsigned int uiBitmapIndex, bool bForce);
    BITMAP_t* GetTexture(unsigned int uiBitmapIndex);
    std::uint32_t GetUsedTextureMemory() const;
    size_t GetNumberOfTexture() const;
};

CGlobalBitmap::CGlobalBitmap() {}
CGlobalBitmap::~CGlobalBitmap() {}
unsigned int CGlobalBitmap::LoadImage(const std::wstring& /*filename*/, unsigned int /*uiFilter*/,
                                      unsigned int /*uiWrapMode*/)
{
    return 0;
}
bool CGlobalBitmap::LoadImage(unsigned int /*uiBitmapIndex*/, const std::wstring& /*filename*/,
                              unsigned int /*uiFilter*/, unsigned int /*uiWrapMode*/)
{
    return false;
}
void CGlobalBitmap::UnloadImage(unsigned int /*uiBitmapIndex*/, bool /*bForce*/) {}
BITMAP_t* CGlobalBitmap::GetTexture(unsigned int /*uiBitmapIndex*/) { return nullptr; }
std::uint32_t CGlobalBitmap::GetUsedTextureMemory() const { return 0; }
size_t CGlobalBitmap::GetNumberOfTexture() const { return 0; }

// ===========================================================================
// 3b. Connection stub — Dotnet/Connection.cpp (MUGame, not linked)
//     Called by PopUpErrorCheckMsgBox in ZzzTexture.cpp (MURenderFX)
// ===========================================================================

class Connection
{
public:
    void Close();
};
void Connection::Close() {}

// ===========================================================================
// 3c. CUIRenderText stubs — ThirdParty/UIControls.cpp (MUThirdParty, not linked)
//     Called by UsefulDef.cpp (MUCore)
// ===========================================================================

class CUIRenderText
{
public:
    virtual ~CUIRenderText();
    static CUIRenderText* GetInstance();
    HDC GetFontDC() const;
};
CUIRenderText::~CUIRenderText() {}
CUIRenderText* CUIRenderText::GetInstance() { return nullptr; }
HDC CUIRenderText::GetFontDC() const { return nullptr; }

// ===========================================================================
// 3d. SEASON3B UI stubs — UI/ (MUGame, not linked)
//     Called by ZzzOpenglUtil.cpp (MURenderFX)
// ===========================================================================

namespace SEASON3B
{

class CNewUIMiniMap
{
public:
    void SetBtnPos(int Num, float x, float y, float nx, float ny);
};
void CNewUIMiniMap::SetBtnPos(int /*Num*/, float /*x*/, float /*y*/, float /*nx*/, float /*ny*/) {}

class CNewUISystem
{
public:
    static CNewUISystem* GetInstance();
    CNewUIMiniMap* GetUI_pNewUIMiniMap() const;
};
CNewUISystem* CNewUISystem::GetInstance() { return nullptr; }
CNewUIMiniMap* CNewUISystem::GetUI_pNewUIMiniMap() const { return nullptr; }

} // namespace SEASON3B

// ===========================================================================
// 4. ShopListManager stubs — GameShop/ (MUGame, not linked)
//    Needed by test_shoplist_download.cpp.
//    WZResult & DownloadFileInfo headers are included above for correct layout.
// ===========================================================================

// -- WZResult (real implementation — tests check return values) --
WZResult::WZResult() { SetSuccessResult(); }
WZResult::~WZResult() {}

bool WZResult::IsSuccess() { return !m_dwErrorCode && !m_dwWindowErrorCode; }

uint32_t WZResult::GetErrorCode() { return m_dwErrorCode; }
wchar_t* WZResult::GetErrorMessage() { return m_szErrorMessage; }
uint32_t WZResult::GetWindowErrorCode() { return m_dwWindowErrorCode; }

WZResult& WZResult::operator=(const WZResult& a2)
{
    m_dwErrorCode = a2.m_dwErrorCode;
    m_dwWindowErrorCode = a2.m_dwWindowErrorCode;
    wcsncpy(m_szErrorMessage, a2.m_szErrorMessage, MAX_ERROR_MESSAGE - 1);
    m_szErrorMessage[MAX_ERROR_MESSAGE - 1] = L'\0';
    return *this;
}

void WZResult::SetResult(uint32_t dwErrorCode, uint32_t dwWindowErrorCode, const wchar_t* szFormat, ...)
{
    va_list va;
    va_start(va, szFormat);
    m_dwErrorCode = dwErrorCode;
    m_dwWindowErrorCode = dwWindowErrorCode;
    vswprintf(m_szErrorMessage, MAX_ERROR_MESSAGE, szFormat, va);
    va_end(va);
}

void WZResult::SetSuccessResult()
{
    m_dwErrorCode = WZ_SUCCESS;
    m_dwWindowErrorCode = 0;
    wcsncpy(m_szErrorMessage, L"Success", MAX_ERROR_MESSAGE - 1);
    m_szErrorMessage[MAX_ERROR_MESSAGE - 1] = L'\0';
}

WZResult WZResult::BuildSuccessResult()
{
    WZResult result;
    result.SetSuccessResult();
    return result;
}

WZResult WZResult::BuildResult(uint32_t dwErrorCode, uint32_t dwWindowErrorCode, const wchar_t* szFormat, ...)
{
    WZResult result;
    wchar_t Buffer[MAX_ERROR_MESSAGE];
    va_list args;
    va_start(args, szFormat);
    std::memset(Buffer, 0, sizeof(Buffer));
    vswprintf(Buffer, MAX_ERROR_MESSAGE, szFormat, args);
    va_end(args);
    result.SetResult(dwErrorCode, dwWindowErrorCode, Buffer);
    return result;
}

// -- DownloadFileInfo (real layout from DownloadInfo.h included above) --
DownloadFileInfo::DownloadFileInfo() : m_uFileLength(0)
{
    std::memset(m_szFileName, 0, sizeof(m_szFileName));
    std::memset(m_szLocalFilePath, 0, sizeof(m_szLocalFilePath));
    std::memset(m_szRemoteFilePath, 0, sizeof(m_szRemoteFilePath));
    std::memset(m_szTargerDirPath, 0, sizeof(m_szTargerDirPath));
}
DownloadFileInfo::~DownloadFileInfo() {}

// -- CListManager / CShopListManager --
// These headers have heavy deps (curl, FTP), so define minimal stubs here.
// Virtual method order must match: ~CListManager (virtual), LoadScript (pure virtual).

class CListVersionInfo
{
public:
    unsigned short Zone;
    unsigned short year;
    unsigned short yearId;
};

enum FTP_SERVICE_MODE
{
    FTP_MODE_ACTIVE,
    FTP_MODE_PASSIVE
};

class CListManager
{
public:
    CListManager();
    virtual ~CListManager();

    void SetListManagerInfo(DownloaderType type, const wchar_t* ServerIP, const wchar_t* UserID, const wchar_t* Pwd,
                            const wchar_t* RemotePath, const wchar_t* LocalPath, CListVersionInfo Version,
                            uint32_t dwDownloadMaxTime);

    void SetListManagerInfo(DownloaderType type, const wchar_t* ServerIP, unsigned short PortNum,
                            const wchar_t* UserID, const wchar_t* Pwd, const wchar_t* RemotePath,
                            const wchar_t* LocalPath, FTP_SERVICE_MODE ftpMode, CListVersionInfo Version,
                            uint32_t dwDownloadMaxTime);

    WZResult LoadScriptList(bool bDonwLoad);

protected:
    virtual WZResult LoadScript(bool bDonwLoad) = 0;
};

CListManager::CListManager() {}
CListManager::~CListManager() {}

void CListManager::SetListManagerInfo(DownloaderType /*type*/, const wchar_t* /*ServerIP*/, const wchar_t* /*UserID*/,
                                      const wchar_t* /*Pwd*/, const wchar_t* /*RemotePath*/,
                                      const wchar_t* /*LocalPath*/, CListVersionInfo /*Version*/,
                                      uint32_t /*dwDownloadMaxTime*/)
{
}

void CListManager::SetListManagerInfo(DownloaderType /*type*/, const wchar_t* /*ServerIP*/,
                                      unsigned short /*PortNum*/, const wchar_t* /*UserID*/, const wchar_t* /*Pwd*/,
                                      const wchar_t* /*RemotePath*/, const wchar_t* /*LocalPath*/,
                                      FTP_SERVICE_MODE /*ftpMode*/, CListVersionInfo /*Version*/,
                                      uint32_t /*dwDownloadMaxTime*/)
{
}

WZResult CListManager::LoadScriptList(bool /*bDonwLoad*/) { return WZResult(); }

class CShopListManager : public CListManager
{
public:
    CShopListManager();
    ~CShopListManager() override;

private:
    WZResult LoadScript(bool bDonwLoad) override;
};

CShopListManager::CShopListManager() {}
CShopListManager::~CShopListManager() {}
WZResult CShopListManager::LoadScript(bool /*bDonwLoad*/) { return WZResult(); }

// ===========================================================================
// 5. TurboJPEG stubs — linked to Main target but not MuTests
// ===========================================================================

extern "C"
{
    unsigned long tjBufSize(int /*width*/, int /*height*/, int /*jpegSubsamp*/) { return 0; }
    void* tjInitCompress(void) { return nullptr; }
    void* tjInitDecompress(void) { return nullptr; }
    int tjCompress2(void*, const unsigned char*, int, int, int, int, unsigned char**, unsigned long*, int, int, int)
    {
        return -1;
    }
    int tjDecompress2(void*, const unsigned char*, unsigned long, unsigned char*, int, int, int, int, int)
    {
        return -1;
    }
    int tjDecompressHeader3(void*, const unsigned char*, unsigned long, int*, int*, int*, int*) { return -1; }
    int tjDestroy(void*) { return 0; }
}

// ===========================================================================
// 6. MUData texture helpers — real implementations from GlobalBitmap.cpp.
//    Tested directly by test_texturesystemmigration.cpp.
// ===========================================================================

[[nodiscard]] int MapGLFilterToSDL(unsigned int uiFilter)
{
    switch (uiFilter)
    {
    case 0x2600u: // GL_NEAREST -> SDL_GPU_FILTER_NEAREST
        return 0;
    case 0x2601u: // GL_LINEAR  -> SDL_GPU_FILTER_LINEAR
        return 1;
    default:
        return 0;
    }
}

[[nodiscard]] int MapGLWrapToSDL(unsigned int uiWrapMode)
{
    switch (uiWrapMode)
    {
    case 0x812Fu: // clamp-to-edge -> SDL_GPU_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
        return 2;
    case 0x2901u: // repeat -> SDL_GPU_SAMPLER_ADDRESS_MODE_REPEAT
        return 0;
    default:
        return 2;
    }
}

[[nodiscard]] std::vector<std::uint8_t> PadRGBToRGBA(const std::uint8_t* rgbData, int width, int height)
{
    const std::size_t pixelCount = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    std::vector<std::uint8_t> rgba(pixelCount * 4u);
    for (std::size_t i = 0; i < pixelCount; ++i)
    {
        rgba[i * 4u + 0u] = rgbData[i * 3u + 0u]; // R
        rgba[i * 4u + 1u] = rgbData[i * 3u + 1u]; // G
        rgba[i * 4u + 2u] = rgbData[i * 3u + 2u]; // B
        rgba[i * 4u + 3u] = 255u;                 // A
    }
    return rgba;
}
