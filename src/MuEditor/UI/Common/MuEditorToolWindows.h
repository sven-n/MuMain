#pragma once

#ifdef _EDITOR

namespace MuEditor
{
// Which of the editor's tool windows are open; the Windows menu shows and
// hides them, and a tool can open another (an item number opens the item in
// the item tools).
struct ToolWindows
{
    bool itemEditor = false;
    bool itemTranslations = false;
    bool skillEditor = false;
    bool devEditor = false;
    bool mapEditor = false;
    bool effectBrowser = false;
    bool console = true;
};
} // namespace MuEditor

#endif // _EDITOR
