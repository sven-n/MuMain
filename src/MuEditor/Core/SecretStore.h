#pragma once

#ifdef _EDITOR

#include <optional>
#include <string>

// MuEditor's secrets (the OpenMU admin panel's API key) in the operating system's key store: the
// Windows Credential Manager, the macOS Keychain or, when MuEditor was built with libsecret, the
// Linux keyring. Never in a file of the editor or a log.
namespace MuEditor::SecretStore
{
// Whether this build has a key store.
bool IsAvailable();
// The key store as "in {0}" names it, in the UI language ("the Windows Credential Manager").
const char* GetStoreName();
// The secret stored as `name`; nothing when there is none or the store refuses.
std::optional<std::string> Read(const std::string& name);
// Stores the secret `name`, or replaces it; false when the store refuses.
bool Write(const std::string& name, const std::string& secret);
// Removes the secret `name`; true when it is gone, also when there was none.
bool Remove(const std::string& name);
} // namespace MuEditor::SecretStore

#endif // _EDITOR
