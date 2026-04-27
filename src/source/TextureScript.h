#pragma once

//////////////////////////////////////////////////////////////////////////
//  DEFINE.
//////////////////////////////////////////////////////////////////////////
const   BYTE    SHADOW_NONE = 0;
const   BYTE    SHADOW_RENDER_COLOR = 1;
const   BYTE    SHADOW_RENDER_TEXTURE = 2;

//////////////////////////////////////////////////////////////////////////
//  CLASS.
//////////////////////////////////////////////////////////////////////////
class TextureScript
{
protected:
    bool    m_bBright;          //
    bool    m_bHiddenMesh;      //
    bool    m_bStreamMesh;      //
    bool    m_bNoneBlendMesh;   //
    BYTE    m_byShadowMesh;

public:
    TextureScript(void) : m_bBright(false), m_bHiddenMesh(false), m_bStreamMesh(false), m_bNoneBlendMesh(false), m_byShadowMesh(0) {};
    ~TextureScript(void) {};
    TextureScript(const TextureScript& r)
    {
        m_bBright = r.m_bBright;
        m_bHiddenMesh = r.m_bHiddenMesh;
        m_bStreamMesh = r.m_bStreamMesh;
        m_bNoneBlendMesh = r.m_bNoneBlendMesh;
        m_byShadowMesh = r.m_byShadowMesh;
    }

    inline  bool getHiddenMesh(void) { return m_bHiddenMesh; }
    inline  bool getBright(void) { return m_bBright; }
    inline  bool getStreamMesh(void) { return m_bStreamMesh; }
    inline  bool getNoneBlendMesh(void) { return m_bNoneBlendMesh; }
    inline  BYTE getShadowMesh(void) { return m_byShadowMesh; }

    void    setScript(const TextureScript& rhs);
};

class TextureScriptParsing : public TextureScript
{
private:
    bool    m_bBeScript;

public:
    TextureScriptParsing(void) : m_bBeScript(false) {};
    ~TextureScriptParsing(void) {};

    bool    IsScript(void) { return m_bBeScript; }
    bool    parsingTScript(wchar_t* filename);
    bool    parsingTScriptA(char* filename);
};
