#pragma once

#ifdef _EDITOR

#include <string>

// Item Data Saving Operations
class ItemDataSaver
{
public:
    static bool Save(wchar_t* fileName, std::string* outChangeLog = nullptr);
};

#endif // _EDITOR
