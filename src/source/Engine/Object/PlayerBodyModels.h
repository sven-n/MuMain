#pragma once

namespace Engine::Object
{
// OpenPlayers' stage * MAX_CLASS + base-class offset. Unchanged classes stay at the root.
const wchar_t* PlayerBodyModelDirectory(int classOffset, bool useS6PlayerModels);
} // namespace Engine::Object
