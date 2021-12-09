/**********************************************************************

Filename    :   FontConfigParser.cpp
Content     :   Parsing logic for GFxPlayer font configuration file
Created     :
Authors     :   Michael Antonov

Copyright   :   (c) 2005-2006 Scaleform Corp. All Rights Reserved.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/
#include "stdafx.h"

#ifdef LDK_ADD_SCALEFORM

#include "CGFxFontConfigParser.h"

#include "GFxFontLib.h"
#include "GFxPlayer.h"
#include "GSysFile.h"

#include <stdarg.h>

// ***** ConfigParser implementation

ConfigParser::ConfigParser(const char* pfilename)
{
    Pos         = 0;
    Line        = 1;

    // if absolute path, then save the parent path
    ParentPath.AppendString(pfilename);
    GFxURLBuilder::ExtractFilePath(&ParentPath);
    if (ParentPath.CompareNoCase(pfilename) == 0)
    {
        ParentPath = "";
    } 

    GSysFile file(pfilename);
    if (file.IsValid())
    {
        SInt flen = 0;
        if ((flen = file.GetLength()) != 0)
        {
            UByte* td = (UByte*) GALLOC(flen, GStat_Default_Mem);
            file.Read(td, flen);

            // PPS: the following converts byte stream to appropriate endianness
            // PPS: only for UTF16
            UInt16* prefix16 = (UInt16*)td;
/*/
			if (prefix16[0] == GByteUtil::BEToSystem( (UInt16)0xFFFE )) // little endian
            {
                prefix16++;
                SInt numChars = (flen / 2) - 1;
                if (sizeof(wchar_t) == 2)
                {
                    UInt16* chars = (UInt16*) GALLOC(numChars * 2,GStat_Default_Mem);
                    Buffer.Resize(numChars);
                    for (SInt i=0; i < numChars; i++)
                        chars[i] = GByteUtil::LEToSystem(prefix16[i]);
                    Buffer.SetString( ((const wchar_t*)chars), numChars );
                    GFREE(chars);
                }
                else if (sizeof(wchar_t) == 4)
                {
                    UInt32* chars = (UInt32*) GALLOC(numChars * 4, GStat_Default_Mem);
                    Buffer.Resize(numChars);
                    for (SInt i=0; i < numChars; i++)
                        chars[i] = GByteUtil::LEToSystem(prefix16[i]);
                    Buffer.SetString( ((const wchar_t*)chars), numChars );
                    GFREE(chars);
                }
                else
                {
                    GFC_DEBUG_ASSERT1(0, "Unsupported wchar_t size (%d)\n", sizeof(wchar_t));
                }
            }
			else if (prefix16[0] == GByteUtil::BEToSystem((UInt16)0xFEFF)) // big endian
            {
                prefix16++;
                SInt numChars = (flen / 2) - 1;
                if (sizeof(wchar_t) == 2)
                {
                    UInt16* chars = (UInt16*) GALLOC(numChars * 2, GStat_Default_Mem);
                    Buffer.Resize(numChars);
                    for (SInt i=0; i < numChars; i++)
                        chars[i] = GByteUtil::BEToSystem(prefix16[i]);
                    Buffer.SetString( ((const wchar_t*)chars), numChars );
                    GFREE(chars);
                }
                else if (sizeof(wchar_t) == 4)
                {
                    UInt32* chars = (UInt32*) GALLOC(numChars * 4, GStat_Default_Mem);
                    Buffer.Resize(numChars);
                    for (SInt i=0; i < numChars; i++)
                        chars[i] = GByteUtil::BEToSystem(prefix16[i]);
                    Buffer.SetString( ((const wchar_t*)chars), numChars );
                    GFREE(chars);
                }
                else
                {
                    GFC_DEBUG_ASSERT1(0, "Unsupported wchar_t size (%d)\n", sizeof(wchar_t));
                }            
            }

            else/*/ if (flen > 2 && td[0] == 0xEF && td[1] == 0xBB && td[2] == 0xBF)
            {
                // UTF-8 BOM was explicitly specified
                Buffer.SetString( (char*)td + 3, GUTF8Util::GetLength((char*)td + 3, flen - 3) );
            }

            else
            {
                // UTF-8 BOM not specified
                Buffer.SetString( (char*)td, GUTF8Util::GetLength((char*)td, flen) );
            }

            GFREE(td);
        }
    }
}

ConfigParser::~ConfigParser()
{
    
}


ConfigParser::TokenType ConfigParser::NextToken(Token* ptoken)
{
    if (!IsValid() || (Pos == (SInt)Buffer.GetLength()))
        return ptoken->Set(Tok_EOF);

    SkipWhitespace();

    // Skip EOF byte, if any.
    if (Buffer[Pos] == 0x1A)
    {
        Pos++;
        if (Pos == (SInt)Buffer.GetLength())
            return ptoken->Set(Tok_EOF);
    }

    // Handle '\r\n' and just '\n' combinations.
    if ((Buffer[Pos] == '\r') && (Buffer[Pos+1] == '\n'))
    {
        Line++;
        Pos+=2;
        return ptoken->Set(Tok_EOL, &Buffer[Pos-2], 2);
    }

    if (Buffer[Pos] == '\n')
    {
        Line++;
        Pos++;
        return ptoken->Set(Tok_EOL, &Buffer[Pos-1], 1);
    }

    // Process identifiers.
    if (IsFirstIdChar(Buffer[Pos]))
    {
        wchar_t * pstart = &Buffer[Pos];
        UInt   start = Pos;
        Pos++;

        while((Pos < (SInt)Buffer.GetLength()) && IsNextIdChar(Buffer[Pos]))
            Pos++;
        return ptoken->Set(Tok_Id, pstart, Pos-start);
    }

    int sign = IsSignChar(Buffer[Pos]) ? 1 : 0;
    if (sign)
    {
        ++Pos;
        if (Pos == (SInt)Buffer.GetLength())
            return ptoken->Set(Tok_EOF);
    }

    if (IsNumberChar(Buffer[Pos]))
    {
        Pos -= sign;
        wchar_t * pstart = &Buffer[Pos];
        UInt   start = Pos;
        bool sawDot = Buffer[Pos] == L'.';
        Pos++;

        while((Pos < (SInt)Buffer.GetLength()) && IsNumberChar(Buffer[Pos]))
        {
            if (Buffer[Pos] == L'.')
            {
                if (sawDot)
                    break;
                sawDot = true;
            }
            Pos++;
        }
        if (!sign)
            return ptoken->Set(Tok_Number, pstart, Pos-start);
        else
            return ptoken->Set(Tok_SignedNumber, pstart, Pos-start);
    }
    // Process Quoted strings.
    if (Buffer[Pos] == '\"')
    {
        Pos++;
        wchar_t* pstart = &Buffer[Pos];
        UInt   start  = Pos;

        while((Pos < (SInt)Buffer.GetLength()) &&
            (Buffer[Pos] != '\"') )
            Pos++;

        if (Buffer[Pos] == '\"')
        {
            Pos++;
            return ptoken->Set(Tok_String, pstart, Pos-start-1);
        }
        else
        {
            // Incorrect.
            ptoken->Set(Tok_Error);
            ptoken->pError = "closing quote \" character expected";
            return ptoken->Type;
        }
    }

    // Report other characters as Tok_Char.
    Pos++;
    return ptoken->Set(Tok_Char, &Buffer[Pos-1], 1);
}

bool    ConfigParser::TokenizeLine(TokenLine* ptokens, UInt max)
{
    GUNUSED(max);
    Token tok;
    ptokens->ErrorIndex = -1;
    ptokens->LineIndex  = Line;
    ptokens->Tokens.Clear();
    
    do {
        NextToken(&tok);
        if ((tok.Type == Tok_Error) && (ptokens->ErrorIndex == -1))
            ptokens->ErrorIndex = (SInt)ptokens->Tokens.GetSize();
        ptokens->Tokens.PushBack(tok);
    } while(tok.Type > Tok_EOF);

    return (ptokens->Tokens.GetSize() > 1) || (tok.Type != Tok_EOF);
}

bool    ConfigParser::MatchLine(GArray<Token> &tokens, const wchar_t* pformat, ...)
{
    va_list vl;
    va_start(vl, pformat);

    UInt tokenIndex = 0;

    while(*pformat != 0)
    {
        // Must have tokens for format string.
        if (tokenIndex >= tokens.GetSize())
        {
            va_end(vl);
            return 0;
        }

        Token &tok = tokens[tokenIndex];

        switch (*pformat)
        {
        case '%':

            // %i = matching id
            if (*(pformat+1) == 'i')
            {
                if (!tok.IsId(va_arg(vl, const wchar_t*)))
                {
                    va_end(vl);
                    return 0;
                }
            }
            // %s feed back string
            else if (*(pformat+1) == 's')
            {
                if (!tok.IsString())
                {
                    va_end(vl);
                    return 0;
                }
                GString* pstr = va_arg(vl, GString*);
                *pstr = tok.GetString();
            }

            // %I feed back identifier
            else if (*(pformat+1) == 'I')
            {
                if (!tok.IsId())
                {
                    va_end(vl);
                    return 0;
                }
                GString* pstr = va_arg(vl, GString*);
                *pstr = tok.GetString();
            }

            // %n feed back number
            else if (*(pformat+1) == 'n')
            {
                if (!tok.IsNumber())
                {
                    va_end(vl);
                    return 0;
                }
                Float* pnumber = va_arg(vl, Float*);
                *pnumber = tok.GetNumber();
            }

            // %p feed back number (if % sign is added at the end, then value will be converted to 
            // scale of 1, i.e. 99% will be converted to 0.99). Negative values are NOT supported.
            else if (*(pformat+1) == 'p')
            {
                if (!tok.IsNumber())
                {
                    va_end(vl);
                    return 0;
                }
                Float* pnumber = va_arg(vl, Float*);
                *pnumber = tok.GetNumber();

                if (tokenIndex + 1 < tokens.GetSize())
                {
                    Token &tok = tokens[tokenIndex + 1];
                    if (tok.IsChar(L'%'))
                    {
                        *pnumber /= 100.f;
                        ++tokenIndex;
                    }
                }
            }
            // %P feed back number (if % sign is added at the end, then value will be converted to 
            // scale of 1, i.e. 99% will be converted to 0.99). Negative values ARE supported.
            else if (*(pformat+1) == 'P')
            {
                if (!tok.IsSignedNumber() && !tok.IsNumber())
                {
                    va_end(vl);
                    return 0;
                }
                Float* pnumber = va_arg(vl, Float*);
                *pnumber = tok.GetNumber();

                if (tokenIndex + 1 < tokens.GetSize())
                {
                    Token &tok = tokens[tokenIndex + 1];
                    if (tok.IsChar(L'%'))
                    {
                        *pnumber /= 100.f;
                        ++tokenIndex;
                    }
                }
            }

            pformat++;
            break;

            // By default, token must match a character.
        default:
            if (!tok.IsChar(*pformat))
            {
                va_end(vl);
                return 0;
            }
        }

        // Go to next character and 
        pformat++;
        tokenIndex++;
    }

    va_end(vl);

    // Last token must be new line or error.
    if (tokenIndex >= tokens.GetSize())    
        return 0;
    Token &tok = tokens[tokenIndex];
    if ((tok.Type == Tok_EOL) || (tok.Type == Tok_EOF))
        return 1;

    // Errors at the end of line would yield parse error.
    return 0;
}


// ***** Font translation
//
void FontTranslator::AddMapping(const GString &key, const GString &value)
{
    Mappings.Add(key, value);
}

void FontTranslator::Translate(TranslateInfo* ptranslateInfo)
{
    GString key;
    key = ptranslateInfo->GetKey();
    GString* pvalue = Mappings.Get(key);
    if (pvalue)
    {
        ptranslateInfo->SetResult(pvalue->ToCStr(), pvalue->GetLength());
    }
}


// ***** FontFonfig parsing

// Apply config settings to loader.
void FontConfig::Apply(GFxLoader* ploader)
{
    ploader->SetFontLib(0);
    ploader->SetFontMap(pFontMap.GetPtr());

    ploader->SetTranslator(pTranslation);

    // Load font SWF/GFX files
    if (FontLibFiles.GetSize())
    {
        GPtr<GFxFontLib> plib = *new GFxFontLib;
        ploader->SetFontLib(plib);
        for(UInt i=0; i<FontLibFiles.GetSize(); i++)
        {
            GPtr<GFxMovieDef> pdef = *ploader->CreateMovie(FontLibFiles[i]);
            if (pdef)
                plib->AddFontsFrom(pdef, 0);
        }
    }
   
}

FontConfigSet::FontConfigSet()
{
}

// Helper function used to decode font style
static GFxFontMap::MapFontFlags UpdateFontFlags(GFxFontMap::MapFontFlags flags, const GString& symbol, UInt lineIndex)
{    
    GFxFontMap::MapFontFlags newFlags = GFxFontMap::MFF_Original;

    if (!symbol.CompareNoCase("bold"))
        newFlags = GFxFontMap::MFF_Bold;
    else if (!symbol.CompareNoCase("normal"))
        newFlags = GFxFontMap::MFF_Normal;
    else if (!symbol.CompareNoCase("italic"))
        newFlags = GFxFontMap::MFF_Italic;
    else if (!symbol.CompareNoCase("fauxbold"))
        newFlags = GFxFontMap::MFF_FauxBold;
    else if (!symbol.CompareNoCase("fauxitalic"))
        newFlags = GFxFontMap::MFF_FauxItalic;
    else if (!symbol.CompareNoCase("noautofit"))
        newFlags = GFxFontMap::MFF_NoAutoFit;
    else
    {
        fprintf(stderr, "Warning: FontConfig(%d) - unknown map font style '%s'\n",
                        lineIndex, symbol.ToCStr());
        return flags;
    }

    if (flags == GFxFontMap::MFF_Original)
    {
        flags = newFlags;
    }
    else if ( ((flags == GFxFontMap::MFF_Normal) && ((newFlags & GFxFontMap::MFF_BoldItalic) != 0)) ||
              ((newFlags == GFxFontMap::MFF_Normal) && ((flags & GFxFontMap::MFF_BoldItalic) != 0)) )
    {
        // Normal combined with incorrect flag.
        fprintf(stderr, "Warning: FontConfig(%d) - unexpected map font style '%s'\n",
                lineIndex, symbol.ToCStr());
    }
    else
    {
          flags = (GFxFontMap::MapFontFlags) (newFlags | flags);
    }

    return flags;
}


void FontConfigSet::Parse(ConfigParser* pparser)
{
    // Process file with tokens    
    ConfigParser::TokenLine     line;
    GArray<Token>&              tokens = line.Tokens;

    GString s1, s2, s3, s4;

    Configs.Clear();
    if (!pparser->IsValid())
        return;

    while(pparser->TokenizeLine(&line, 6))
    {
        if (line.HasErrors())
        {
            fprintf(stderr, "Error: FontConfig(%d) - %s\n",
                             line.LineIndex, tokens[line.ErrorIndex].pError);
            break;            
        }

        // [FontConfig] - unnamed
        if (ConfigParser::MatchLine(tokens, L"[%i]", L"FontConfig"))
        {
            GPtr<FontConfig> pfontConfig = *new FontConfig;
            pfontConfig->ConfigName = "Unnamed";
            Configs.PushBack(pfontConfig);
        }
        // [FontConfig "English"]
        else if (ConfigParser::MatchLine(tokens, L"[%i%s]", L"FontConfig", &s1))
        {
            GPtr<FontConfig> pfontConfig = *new FontConfig;
            pfontConfig->ConfigName = s1;
            Configs.PushBack(pfontConfig);
        }

        else
        {
            // Empty lines ok.
            if (ConfigParser::MatchLine(tokens, L""))
                continue;

            if (Configs.GetSize() == 0)
            {
                fprintf(stderr, "Error: FontConfig(%d) - file mappings outside of [FontConfig] section\n",
                                line.LineIndex);
                break;
            }

            FontConfig* pconfig = Configs[Configs.GetSize()-1];
            GASSERT(pconfig);
            Float scaleFactor = 1.0f;
            Float offx = 0, offy = 0;

            // map "Arial" = "takoma"
            if (ConfigParser::MatchLine(tokens, L"%i%s=%s", L"map", &s1, &s2))
            {
                if (!pconfig->pFontMap)                
                    pconfig->pFontMap = *new GFxFontMap;
                pconfig->pFontMap->MapFont(s1.ToCStr(), s2.ToCStr());
            }

            // map "Arial" = "takoma" Bold
            else if (ConfigParser::MatchLine(tokens, L"%i%s=%s%I", L"map", &s1, &s2, &s3))
            {
                if (!pconfig->pFontMap)                
                    pconfig->pFontMap = *new GFxFontMap;

                GFxFontMap::MapFontFlags mapFlags =
                        UpdateFontFlags(GFxFontMap::MFF_Original, s3, line.LineIndex);
                pconfig->pFontMap->MapFont(s1.ToCStr(), s2.ToCStr(), mapFlags);
            }
            // map "Arial" = "takoma" 1.3
            else if (ConfigParser::MatchLine(tokens, L"%i%s=%s%p", L"map", &s1, &s2, &scaleFactor))
            {
                if (!pconfig->pFontMap)                
                    pconfig->pFontMap = *new GFxFontMap;
                pconfig->pFontMap->MapFont(s1.ToCStr(), s2.ToCStr(), GFxFontMap::MFF_Original, scaleFactor);
            }

            // map "Arial" = "takoma" Bold 1.3
            else if (ConfigParser::MatchLine(tokens, L"%i%s=%s%I%p", L"map", &s1, &s2, &s3, &scaleFactor))
            {
                if (!pconfig->pFontMap)                
                    pconfig->pFontMap = *new GFxFontMap;

                GFxFontMap::MapFontFlags mapFlags =
                    UpdateFontFlags(GFxFontMap::MFF_Original, s3, line.LineIndex);
                pconfig->pFontMap->MapFont(s1.ToCStr(), s2.ToCStr(), mapFlags, scaleFactor);
            }
            // map "Arial" = "takoma" Bold 1.3 12.2 93.3
            else if (ConfigParser::MatchLine(tokens, L"%i%s=%s%I%p%P%P", L"map", &s1, &s2, &s3, &scaleFactor, &offx, &offy))
            {
                if (!pconfig->pFontMap)                
                    pconfig->pFontMap = *new GFxFontMap;

                GFxFontMap::MapFontFlags mapFlags =
                    UpdateFontFlags(GFxFontMap::MFF_Original, s3, line.LineIndex);
                pconfig->pFontMap->MapFont(s1.ToCStr(), s2.ToCStr(), mapFlags, scaleFactor, offx, offy);
            }

            // map "Arial" = "takoma" Bold Italic
            else if (ConfigParser::MatchLine(tokens, L"%i%s=%s%I%I", L"map", &s1, &s2, &s3, &s4))
            {
                if (!pconfig->pFontMap)                
                    pconfig->pFontMap = *new GFxFontMap;

                GFxFontMap::MapFontFlags mapFlags =
                           UpdateFontFlags(GFxFontMap::MFF_Original, s3, line.LineIndex);
                mapFlags = UpdateFontFlags(mapFlags, s4, line.LineIndex);
                pconfig->pFontMap->MapFont(s1.ToCStr(), s2.ToCStr(), mapFlags);
            }
            // map "Arial" = "takoma" Bold Italic 1.3
            else if (ConfigParser::MatchLine(tokens, L"%i%s=%s%I%I%p", L"map", &s1, &s2, &s3, &s4, &scaleFactor))
            {
                if (!pconfig->pFontMap)                
                    pconfig->pFontMap = *new GFxFontMap;

                GFxFontMap::MapFontFlags mapFlags =
                    UpdateFontFlags(GFxFontMap::MFF_Original, s3, line.LineIndex);
                mapFlags = UpdateFontFlags(mapFlags, s4, line.LineIndex);
                pconfig->pFontMap->MapFont(s1.ToCStr(), s2.ToCStr(), mapFlags, scaleFactor);
            }
            // map "Arial" = "takoma" Bold Italic 1.3 12.2 93.3
            else if (ConfigParser::MatchLine(tokens, L"%i%s=%s%I%I%p%P%P", L"map", &s1, &s2, &s3, &s4, &scaleFactor, &offx, &offy))
            {
                if (!pconfig->pFontMap)                
                    pconfig->pFontMap = *new GFxFontMap;

                GFxFontMap::MapFontFlags mapFlags =
                    UpdateFontFlags(GFxFontMap::MFF_Original, s3, line.LineIndex);
                mapFlags = UpdateFontFlags(mapFlags, s4, line.LineIndex);
                pconfig->pFontMap->MapFont(s1.ToCStr(), s2.ToCStr(), mapFlags, scaleFactor, offx, offy);
            }

            // fontlib "myfile.swf"
            else if (ConfigParser::MatchLine(tokens, L"%i%s", L"fontlib", &s1))
            {
                // prepend parent path to the filename.
                GString path( pparser->ParentPath );
                path += s1;
                pconfig->FontLibFiles.PushBack(path);
            }

            // tr "Good morning" = "Guten Morgen"
            else if (ConfigParser::MatchLine(tokens, L"%i%s=%s", L"tr", &s1, &s2))
            {

                pconfig->pTranslation->AddMapping(s1, s2);
            }            

            else
            {
                // Unsupported line
                fprintf(stderr, "Error: FontConfig(%d) - unexpected statement\n",
                                line.LineIndex);
                break;
            }
        }       
    }
}

#endif //LDK_ADD_SCALEFORM
