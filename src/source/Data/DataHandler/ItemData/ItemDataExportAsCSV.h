#pragma once

#ifdef _EDITOR

// Item Data Export/Import Operations
class ItemDataExportAsCSV
{
public:
    static bool ExportToCsv(wchar_t* fileName);
    // Future: static bool ImportFromCsv(wchar_t* fileName);
};

#endif // _EDITOR
