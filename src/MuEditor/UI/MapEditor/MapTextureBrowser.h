#pragma once

#ifdef _EDITOR

#include <string>
#include <vector>

// Texture browser for the Map Editor: scans every Data\World* folder and previews
// the terrain tile textures each map ships (Tile*/ExtTile*/AlphaTile*), so you can
// see what textures exist across all maps - not just the ones loaded for the map
// you're standing on.
//
// Preview images are loaded into a dedicated high scratch range of bitmap slots
// (well clear of the game's own texture indices) and freed when you switch worlds,
// so at most one world's tiles are resident at a time.
class CMapTextureBrowser
{
public:
    static CMapTextureBrowser& GetInstance();

    // Draws the browser UI. `defaultWorld` seeds the world selector the first time.
    void Render(int defaultWorld);

private:
    CMapTextureBrowser() = default;
    ~CMapTextureBrowser() = default;

    struct Preview
    {
        std::wstring file;        // file name (for the tooltip)
        unsigned int slot;        // bitmap index the image was loaded into (GLuint)
        bool         loaded;      // false if the file failed to decode
    };

    void ScanWorlds();
    void LoadWorld(int world);
    void ClearPreviews();

    bool             m_scanned = false;
    int              m_selectedWorld = -1;
    std::vector<int> m_worlds;      // World folder numbers found in the Data tree
    std::vector<Preview> m_previews;
    std::string      m_hoveredName; // filename under the cursor, shown as a label
                                    // (the game cursor can cover ImGui tooltips)
    int              m_selectedIndex = -1;  // clicked preview (index into m_previews)
    std::string      m_status;              // result of the last import action
};

#define g_MapTextureBrowser CMapTextureBrowser::GetInstance()

#endif // _EDITOR
