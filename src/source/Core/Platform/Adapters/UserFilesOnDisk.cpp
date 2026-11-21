#include "Core/Platform/UserFolder.h"

namespace Core::Platform
{

// The folder is on the device's disk: each write is stored already.
void TargetCommitUserFiles(const std::filesystem::path& /*folder*/) {}

} // namespace Core::Platform
