#pragma once

#ifdef _EDITOR

#include <string>
#include <vector>

// "O. Browse": pick an object model from any map's Object folder and import it
// onto the current map (see Editor::ObjectImport). List-based (no 3D thumbnails
// yet); shows model file names grouped by source map with a name filter.
class CMapObjectBrowser
{
public:
    static CMapObjectBrowser& GetInstance();

    // Draws the browser. If the user imports a model, *outImportedType is set to
    // the resulting object type (model slot) so the caller can select it for
    // placement. Left unchanged otherwise.
    void Render(int currentWorld, int* outImportedType);

private:
    CMapObjectBrowser() = default;
    ~CMapObjectBrowser() = default;

    void ScanSourceWorlds();
    void LoadFileList(int sourceWorld);

    bool             m_scanned = false;
    int              m_sourceWorld = -1;
    std::vector<int> m_worlds;             // Object folder numbers found in the Data tree
    std::vector<std::wstring> m_files;     // *.bmd files in the selected source folder
    std::vector<unsigned int> m_thumbs;    // preview textures (parallel to m_files; 0 = not yet)
    int              m_selected = -1;      // index into m_files
    char             m_filter[64] = { 0 }; // name filter
    std::string      m_status;             // last import result
};

#define g_MapObjectBrowser CMapObjectBrowser::GetInstance()

#endif // _EDITOR
