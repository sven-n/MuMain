//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TextureScript.h"

void TextureScript::setScript( const TextureScript& rhs )
{
    m_bBright       = rhs.m_bBright;
    m_bHiddenMesh   = rhs.m_bHiddenMesh;
    m_bStreamMesh   = rhs.m_bStreamMesh;
    m_bNoneBlendMesh= rhs.m_bNoneBlendMesh;
    m_byShadowMesh  = rhs.m_byShadowMesh;
}

bool TextureScriptParsing::parsingTScript( char* filename )
{
    int   ch = '_';
    char  str[] = "RHSN";
    char* strDest;
    char* strTokenFile;
    char  strFileName[32];

    memcpy (strFileName, filename, 32 );
    strTokenFile = strchr ( strFileName, ch );
    if ( strTokenFile!=NULL )
    {
        strDest = strtok ( strTokenFile, "." );
        int length = min ( 5, strlen( strDest ) );

        int result = strcspn ( strDest, str );
        if ( result )//if ( m_strDest!=NULL )
        {
            for ( int i=1; i<length; ++i )
            {
                switch ( strTokenFile[i] )
                {
                case 'R' :
                    m_bBright = true;
                    m_bBeScript = true;
                    break;

                case 'H' :
                    m_bHiddenMesh = true;
                    m_bBeScript = true;
                    break;

                case 'S' :
                    m_bStreamMesh = true;
                    m_bBeScript = true;
                    break;

                case 'N' :
                    m_bNoneBlendMesh = true;
                    m_bBeScript = true;
                    break;

                case 'DC' :
                    m_byShadowMesh = 1; //  NoneTexture.
                    m_bBeScript = true;
                    break;

                case 'DT' :
                    m_byShadowMesh = 2; //  Texture.
                    m_bBeScript = true;
                    break;

                default :
                    m_bBeScript = false;
#ifdef PJH_ADD_PANDA_CHANGERING
					if(strcmp("mu_rgb_lights.jpg",filename) == 0)
					{
						m_bBright = true;
						m_bBeScript = true;
					}
#endif //PJH_ADD_PANDA_CHANGERING
                    return m_bBeScript;
                }
            }
        }
    }
    return m_bBeScript;
}