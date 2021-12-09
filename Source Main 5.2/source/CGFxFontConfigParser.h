/**********************************************************************

Filename    :   FontConfigParser.h
Content     :   Parsing logic for GFxPlayer font configuration file
Created     :
Authors     :   Michael Antonov

Copyright   :   (c) 2005-2006 Scaleform Corp. All Rights Reserved.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/
#pragma once

#ifdef LDK_ADD_SCALEFORM

#ifndef INC_FontConfigParser_H
#define INC_FontConfigParser_H

#include "GFile.h"
#include "GString.h"
#include "GFxLoader.h"

// ***** Font configuration settings


class ConfigParser
{
    GFxWStringBuffer Buffer;

    SInt    Pos;
    UInt    Line;

    inline void SkipWhitespace()
    {
        while ((Pos < (SInt)Buffer.GetLength()) && 
            ((Buffer[Pos] == L' ') || (Buffer[Pos] == L'\t')))
            Pos++;
    }

    static inline bool IsFirstIdChar(const wchar_t c)
    {
        return ((c >= L'a') && (c <= L'z')) ||
            ((c >= L'A') && (c <= L'Z')) ||
            (c == L'_');
    }

    static inline bool IsNextIdChar(const wchar_t c)
    {
        return IsFirstIdChar(c) ||
            ((c >= L'0') && (c <= L'9'));
    }
    static inline bool IsNumberChar(const wchar_t c)
    {
        return ((c >= L'0') && (c <= L'9')) || c == L'.';
    }
    static inline bool IsSignChar(const wchar_t c)
    {
        return (c == L'+' || c == L'-');
    }

public:

    GString ParentPath;

    ConfigParser(const char* pfilename);
    ~ConfigParser();

    bool    IsValid() const { return Buffer.GetLength() != 0; }


    enum TokenType
    {
        // Order relied on by TokenizeLine.
        Tok_Error,  // Error
        Tok_EOL,    // End of line
        Tok_EOF,    // End of file

        Tok_Char,   // Control character
        Tok_String, // Quoted string
        Tok_Id,     // Identifier
        Tok_Number, // Number
        Tok_SignedNumber // Signed Number (+,-)
    };

    struct Token
    {
        TokenType   Type;
        const wchar_t* pData;
        UInt        Length;
        const char* pError;

        // *** Initialization
        TokenType        Set(TokenType t, const wchar_t* pdata = 0, UInt length = 0)
        {
            pError = "";
            Type = t;
            pData = pdata;
            Length = length;
            return t;
        }

        // *** State/Type query

        GString   GetString() const   
        { 
            if (pData)
            {
                GString str;

                // Strip newlines because Flash multiline text uses \r instead of \n
                for (UInt i=0; i < Length; i++)
                {
                    if (pData[i] != '\n')
                        str.AppendChar(pData[i]);
                }

                return str;
            }
            else
            {
                return GString(); 
            }
        }
        
        wchar_t        GetChar() const     { return pData ? pData[0] : 0; }

        Float          GetNumber() const
        {
            GString s1;
            s1.AppendString(pData, Length);
            return (Float)atof(s1.ToCStr());
        }

        bool        IsId() const        { return (Type == Tok_Id); }
        bool        IsString() const    { return (Type == Tok_String); }
        bool        IsNumber() const    { return (Type == Tok_Number); }
        bool        IsSignedNumber() const { return (Type == Tok_SignedNumber); }

        bool        IsId(const wchar_t* pid) const
        {
            GString s1, s2;
            s1.AppendString(pData, Length);
            s2.AppendString(pid);
            return ((Type == Tok_Id) &&
                Length == s2.GetLength() &&
                !s1.CompareNoCase(s2));
        }

        bool        IsChar(const wchar_t c) const
        {
            return ((Type == Tok_Char) && GetChar() == c);
        }

    };

    // Reads next token and returns its type. Used by TokenizeLine
    TokenType   NextToken(Token* ptoken);

    struct TokenLine
    {
        // Tokens for the line, including new line in the end
        GArray<Token>      Tokens;
        // Index of Tok_Error, if any. -1 for no errors.
        SInt               ErrorIndex;
        // Line index that we tokenized.
        UInt               LineIndex;

        bool    HasErrors() { return ErrorIndex != -1; }
    };

    // Tokenize a line, returning 1 if successful and 0 for eof.
    bool        TokenizeLine(TokenLine* pline, UInt max);

    // Matches a token array based on format signature, returns 1 if match found 0 otherwise.
    static bool MatchLine(GArray<Token> &tokens, const wchar_t* pformat, ...);
};


/* *** Font configuration file structure:

[FontConfig "English"]
fontlib "fonts_english1.swf"
fontlib "fonts_english2.swf"

[FontConfig "Chinese"]
fontlib "fonts_chinese.swf"
map "Arial" = "Chinese Font Name"

*/


class FontTranslator : public GFxTranslator
{
    GHash<GString, GString, GString::HashFunctor> Mappings;
public:    
    virtual UInt    GetCaps() const         { return Cap_StripTrailingNewLines; }

    // Add a mapping between two strings
    void AddMapping(const GString &key, const GString &value);

    // Callback
    virtual void Translate(TranslateInfo* ptranslateInfo);
};

class FontConfig : public GRefCountBase<FontConfig, GStat_Default_Mem>
{
public:
    enum ProviderType
    {
        Provider_Undefined,
        Provider_None,
        Provider_Win32,
        Provider_FT2
    };

    GString               ConfigName;
    ProviderType            Provider;

    GArray<GString>       FontLibFiles;
    GPtr<GFxFontMap>        pFontMap;

    GPtr<FontTranslator>    pTranslation;

    FontConfig()
    {
        Provider = Provider_Undefined;
        pTranslation = *new FontTranslator();
    }

    // Apply config settings to loader.
    void Apply(GFxLoader* ploader);
    //void Apply(GFxMovieView* pmovie, GFxLoader* ploader);

};

class FontConfigSet
{
public:

    typedef ConfigParser::Token     Token;
    typedef ConfigParser::TokenLine TokenLine;

    GArray<GPtr<FontConfig> >   Configs;

    FontConfigSet();
    
    // Parse font configuration set.
    void Parse(ConfigParser* pparser);

    // Array-style access
    UInt GetSize() const { return (UInt)Configs.GetSize(); }
    FontConfig* operator [] (UInt index) { return Configs[index]; }
};


#endif

#endif //LDK_ADD_SCALEFORM
