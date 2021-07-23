//////////////////////////////////////////////////////////////////////////
//  
//  Texture Script.
//
//  텍스쳐 이름으로 MESH의 특성 설정한다.
//  
//  _ + R, H, S, N값으로 설정.
//
//  R : bRright 효과가 적용되는 메쉬.
//  H : Hidden 메쉬 설정.
//  S : bright 되지 않은 메쉬가 텍스쳐 애니메이션 되는 효과. Streammesh
//  N : +효과가 적용되지 않는 메쉬.
//  
//  예) Test_R.tga, Test_RH.tga, Test_H.tga, Test_HRNS.tga 등등등.
//    
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//  INCLUDE.
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "TextureScript.h"


//////////////////////////////////////////////////////////////////////////
//  FUNCTION.
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//  TextureScript
//////////////////////////////////////////////////////////////////////////
void    TextureScript::setScript( const TextureScript& rhs )
{
    m_bBright       = rhs.m_bBright;
    m_bHiddenMesh   = rhs.m_bHiddenMesh;
    m_bStreamMesh   = rhs.m_bStreamMesh;
    m_bNoneBlendMesh= rhs.m_bNoneBlendMesh;
    m_byShadowMesh  = rhs.m_byShadowMesh;
}


//////////////////////////////////////////////////////////////////////////
//  TextureScriptParsing
//////////////////////////////////////////////////////////////////////////
bool    TextureScriptParsing::parsingTScript( char* filename )
{
    //  _B가 붙은 파일을 Bright를 먹게한다.
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