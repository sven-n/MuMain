#pragma once

#ifdef _EDITOR

namespace MuEditor
{
// Which of the editor's tool windows are open; the Windows menu shows and
// hides them, and a tool can open another (an item number opens the item in
// the item tools).
struct ToolWindows
{
    bool itemBrowser = false;
    bool itemBasics = false;
    bool itemRequirements = false;
    bool itemRules = false;
    bool itemCategories = false;
    bool itemModel = false;
    bool itemGlowColors = false;
    bool itemFormulas = false;
    bool itemSync = false;
    bool itemChecks = false;
    bool itemTranslations = false;
    bool monsterBrowser = false;
    bool monsterTranslations = false;
    bool monsterChecks = false;
    bool monsterModel = false;
    bool monsterPreview = false;
    bool monsterAttacks = false;
    bool serverList = false;
    bool skillEditor = false;
    bool devEditor = false;
    bool mapEditor = false;
    bool effectBrowser = false;
    bool console = true;
};
} // namespace MuEditor

#endif // _EDITOR
