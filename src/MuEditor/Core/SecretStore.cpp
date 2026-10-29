#include "stdafx.h"

#ifdef _EDITOR

#include "SecretStore.h"

#include "I18N/All.h"

#if defined(_WIN32)
#include "Core/Text/Utf8.h"
#include <windows.h>
#include <wincred.h>
#elif defined(__APPLE__)
#include <Security/Security.h>
#elif defined(MU_HAS_LIBSECRET)
#include <libsecret/secret.h>
#endif

namespace MuEditor::SecretStore
{
#if defined(_WIN32) || defined(__APPLE__) || defined(MU_HAS_LIBSECRET)
namespace
{
// The service the secrets are stored for, in front of each name.
constexpr const char* Service = "MuEditor";
} // namespace
#endif

#if defined(_WIN32)
namespace
{
// A generic credential "MuEditor/<name>".
std::wstring Target(const std::string& name)
{
    return Core::Text::FromUtf8(std::string(Service) + "/" + name);
}
} // namespace

bool IsAvailable()
{
    return true;
}

const char* GetStoreName()
{
    return I18N::Editor::TheWindowsCredentialManager;
}

std::optional<std::string> Read(const std::string& name)
{
    PCREDENTIALW credential = nullptr;
    if (!CredReadW(Target(name).c_str(), CRED_TYPE_GENERIC, 0, &credential))
    {
        return std::nullopt;
    }
    std::string secret(reinterpret_cast<const char*>(credential->CredentialBlob), credential->CredentialBlobSize);
    CredFree(credential);
    return secret;
}

bool Write(const std::string& name, const std::string& secret)
{
    std::wstring target = Target(name);
    std::wstring user = Core::Text::FromUtf8(Service);
    std::string blob = secret;
    CREDENTIALW credential{};
    credential.Type = CRED_TYPE_GENERIC;
    credential.TargetName = target.data();
    credential.UserName = user.data();
    credential.CredentialBlobSize = static_cast<DWORD>(blob.size());
    credential.CredentialBlob = reinterpret_cast<LPBYTE>(blob.data());
    credential.Persist = CRED_PERSIST_LOCAL_MACHINE;
    const bool written = CredWriteW(&credential, 0) != FALSE;
    SecureZeroMemory(blob.data(), blob.size());
    return written;
}

bool Remove(const std::string& name)
{
    return CredDeleteW(Target(name).c_str(), CRED_TYPE_GENERIC, 0) != FALSE || GetLastError() == ERROR_NOT_FOUND;
}

#elif defined(__APPLE__)
namespace
{
// Releases a Core Foundation object when it goes.
template <typename T> class CfRef
{
public:
    explicit CfRef(T ref) : m_ref(ref) {}
    ~CfRef()
    {
        if (m_ref != nullptr)
        {
            CFRelease(m_ref);
        }
    }
    CfRef(const CfRef&) = delete;
    CfRef& operator=(const CfRef&) = delete;

    T Get() const
    {
        return m_ref;
    }

private:
    T m_ref;
};

CfRef<CFStringRef> MakeString(const std::string& text)
{
    return CfRef<CFStringRef>(CFStringCreateWithCString(kCFAllocatorDefault, text.c_str(), kCFStringEncodingUTF8));
}

// The query of the generic password `account` of `service`.
CfRef<CFMutableDictionaryRef> MakeQuery(CFStringRef service, CFStringRef account)
{
    CFMutableDictionaryRef query = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
                                                             &kCFTypeDictionaryValueCallBacks);
    CFDictionarySetValue(query, kSecClass, kSecClassGenericPassword);
    CFDictionarySetValue(query, kSecAttrService, service);
    CFDictionarySetValue(query, kSecAttrAccount, account);
    return CfRef<CFMutableDictionaryRef>(query);
}
} // namespace

bool IsAvailable()
{
    return true;
}

const char* GetStoreName()
{
    return I18N::Editor::TheMacOSKeychain;
}

std::optional<std::string> Read(const std::string& name)
{
    const CfRef<CFStringRef> service = MakeString(Service);
    const CfRef<CFStringRef> account = MakeString(name);
    const CfRef<CFMutableDictionaryRef> query = MakeQuery(service.Get(), account.Get());
    CFDictionarySetValue(query.Get(), kSecReturnData, kCFBooleanTrue);
    CFDictionarySetValue(query.Get(), kSecMatchLimit, kSecMatchLimitOne);
    CFTypeRef result = nullptr;
    if (SecItemCopyMatching(query.Get(), &result) != errSecSuccess || result == nullptr)
    {
        return std::nullopt;
    }
    const CfRef<CFTypeRef> found(result);
    if (CFGetTypeID(result) != CFDataGetTypeID())
    {
        return std::nullopt;
    }
    const auto data = static_cast<CFDataRef>(result);
    return std::string(reinterpret_cast<const char*>(CFDataGetBytePtr(data)),
                       static_cast<size_t>(CFDataGetLength(data)));
}

bool Write(const std::string& name, const std::string& secret)
{
    const CfRef<CFStringRef> service = MakeString(Service);
    const CfRef<CFStringRef> account = MakeString(name);
    const CfRef<CFMutableDictionaryRef> query = MakeQuery(service.Get(), account.Get());
    // A secret stored before is replaced.
    SecItemDelete(query.Get());
    const CfRef<CFDataRef> data(CFDataCreate(kCFAllocatorDefault, reinterpret_cast<const UInt8*>(secret.data()),
                                             static_cast<CFIndex>(secret.size())));
    CFDictionarySetValue(query.Get(), kSecValueData, data.Get());
    return SecItemAdd(query.Get(), nullptr) == errSecSuccess;
}

bool Remove(const std::string& name)
{
    const CfRef<CFStringRef> service = MakeString(Service);
    const CfRef<CFStringRef> account = MakeString(name);
    const CfRef<CFMutableDictionaryRef> query = MakeQuery(service.Get(), account.Get());
    const OSStatus status = SecItemDelete(query.Get());
    return status == errSecSuccess || status == errSecItemNotFound;
}

#elif defined(MU_HAS_LIBSECRET)
namespace
{
// MuEditor's secrets, each with the attribute "name".
const SecretSchema* GetSchema()
{
    static const SecretSchema schema = []
    {
        SecretSchema made{};
        made.name = "org.mumain.MuEditor";
        made.flags = SECRET_SCHEMA_NONE;
        made.attributes[0] = {"name", SECRET_SCHEMA_ATTRIBUTE_STRING};
        made.attributes[1] = {nullptr, SECRET_SCHEMA_ATTRIBUTE_STRING};
        return made;
    }();
    return &schema;
}

// Whether a call failed; frees its error.
bool Failed(GError* error)
{
    if (error == nullptr)
    {
        return false;
    }
    g_error_free(error);
    return true;
}
} // namespace

bool IsAvailable()
{
    return true;
}

const char* GetStoreName()
{
    return I18N::Editor::TheSystemKeyring;
}

std::optional<std::string> Read(const std::string& name)
{
    GError* error = nullptr;
    gchar* secret = secret_password_lookup_sync(GetSchema(), nullptr, &error, "name", name.c_str(), nullptr);
    if (Failed(error) || secret == nullptr)
    {
        if (secret != nullptr)
        {
            secret_password_free(secret);
        }
        return std::nullopt;
    }
    std::string text(secret);
    secret_password_free(secret);
    return text;
}

bool Write(const std::string& name, const std::string& secret)
{
    GError* error = nullptr;
    const std::string label = std::string(Service) + ": " + name;
    const gboolean stored = secret_password_store_sync(GetSchema(), SECRET_COLLECTION_DEFAULT, label.c_str(),
                                                       secret.c_str(), nullptr, &error, "name", name.c_str(), nullptr);
    return !Failed(error) && stored != FALSE;
}

bool Remove(const std::string& name)
{
    GError* error = nullptr;
    secret_password_clear_sync(GetSchema(), nullptr, &error, "name", name.c_str(), nullptr);
    return !Failed(error);
}

#else
// No key store: the key is kept only while the editor runs.
bool IsAvailable()
{
    return false;
}

const char* GetStoreName()
{
    return I18N::Editor::TheSystemKeyring;
}

std::optional<std::string> Read(const std::string&)
{
    return std::nullopt;
}

bool Write(const std::string&, const std::string&)
{
    return false;
}

bool Remove(const std::string&)
{
    return true;
}
#endif
} // namespace MuEditor::SecretStore

#endif // _EDITOR
