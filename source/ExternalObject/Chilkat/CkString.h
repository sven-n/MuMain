// CkString.h: interface for the CkString class.
//
//////////////////////////////////////////////////////////////////////


#ifndef _CKSTRING_H
#define _CKSTRING_H

#pragma once

#include "CkObject.h"

#if !defined(WIN32) && !defined(WINCE)
#include "SystemTime.h"
#endif

class CkStringArray;

#pragma pack (push, 8) 

// CLASS: CkString
class CkString : public CkObject
{
    public:
	CkString();
	~CkString();

	CkString(const CkString &);
	CkString &operator=(const CkString &);

	CkString &operator=(const char *);
	CkString &operator=(int);
	CkString &operator=(bool);
	CkString &operator=(char);

       // 
       // operator to cast to a const char *
       // 
       operator const char *();
       operator const wchar_t *();

        void appendHexData(const char *data, int dataLen);

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);

	// Load the contents of a text file into the CkString object.
	// The string is cleared before loading.
	// Charset can be "unicode", "ansi", "utf-8", or any other charset
	// such as "iso-8859-1", "windows-1253", "Shift_JIS", etc.
        bool loadFile(const char *fileName, const char *charset);

	// Returns the ANSI character starting at an index.
	// The first character is at index 0.
	// IMPORTANT: This is not a byte index, but a character position index.
	char charAt(int idx) const;
	wchar_t charAtU(int idx) const;   // Returns Unicode character at position idx.

	// If this string is "43" for example, this returns 43.
        int intValue(void) const;
        double doubleValue(void) const;

	// New methods as of 17-Aug-2006
	CkString *clone(void) const;
	void setStr(CkString &s);
	bool endsWith(const char *s) const;
	bool endsWithStr(CkString &s) const;
	//bool beginsWith(const char *s);
	bool beginsWithStr(CkString &s) const;
	int indexOf(const char *s) const;
	int indexOfStr(CkString &s) const;
        int replaceAll(CkString &str, CkString &replacement);
        bool replaceFirst(CkString &str, CkString &replacement);
	CkString *substring(int startCharIdx, int numChars) const;
	bool matchesStr(CkString &str) const;
	bool matches(const char *s) const;
	bool matchesNoCase(const char *s) const;
	CkString *getChar(int idx) const;
        int removeAll(CkString &str);
        bool removeFirst(CkString &str);
	void chopAtStr(CkString &str);
	void urlDecode(const char *charset);
	void urlEncode(const char *charset);
	void base64Decode(const char *charset);
	void base64Encode(const char *charset);
	void qpDecode(const char *charset);
	void qpEncode(const char *charset);
	void hexDecode(const char *charset);
	void hexEncode(const char *charset);
	void entityDecode(void);
	void entityEncode(void);
	void appendUtf8(const char *s);	    
	void appendAnsi(const char *s);	    
	void appendCurrentDateRfc822(void);
	// SYSTEMTIME is defined in <windows.h>
	// For more information, Google "SYSTEMTIME"
	void appendDateRfc822(SYSTEMTIME &sysTime);

	// Self explanatory, right?
	void clear(void);
	void prepend(const char *s);	    
	void appendInt(int n);
	void append(const char *s);	    
        void appendChar(char c);
	void appendN(const char *s, int numBytes);	
        void appendStr(const CkString &str);
	void appendEnc(const char *s, const char *charEncoding);	// such as "utf-8", "windows-1252", "shift_JIS", etc.
	void appendRandom(int numBytes, const char *encoding);	// such as "base64", "hex", "qp", "url", etc.



	// Convert the binary data to a hex string representation and append.
        void appendHexData(const unsigned char *data, int dataLen);

	// Same as clearing the string and appending.
        void setString(const char *s);
        void setStringAnsi(const char *s);
        void setStringUtf8(const char *s);

	// To/From Unicode...
	void setStringU(const wchar_t *unicode);
	void appendU(const wchar_t *unicode);
	void appendNU(const wchar_t *unicode, int numChars);
	const wchar_t *getUnicode(void) const;

	// To ANSI...
	const char *getAnsi(void) const;

	// To utf-8...
	const char *getUtf8(void) const;

	// Get any multi-byte encoding.
	const char *getEnc(const char *encoding) const;

	// Same as strcmp
	int compareStr(const CkString &str) const;	// Compare against another CkString

	const char *getString(void) const;

	int getSizeUtf8(void) const;  // Returns size in bytes of utf-8 string.
	int getSizeAnsi(void) const; // Returns size in bytes of ANSI string.
	int getSizeUnicode(void) const; // Returns size in bytes of Unicode string.
	int getNumChars(void) const;	// Returns number of characters in the string.

        // Trim whitespace from both ends of the string.
        void trim(void);    // Does not include newline
        void trim2(void);   // Includes newline
	void trimInsideSpaces(void);

        // Case sensitive replacement functions.
	// Return the number of occurances replaced.
        int replaceAllOccurances(const char *pattern, const char *replacement);
        bool replaceFirstOccurance(const char *pattern, const char *replacement);

        // CRLF
        void toCRLF(void);                  // Make all end of lines a CRLF ("\r\n")
        void toLF(void);                    // Make all end of lines a "\n"

        // Eliminate all occurances of a particular ANSI character.
        void eliminateChar(char ansiChar, int startIndex);

        // Return the last (ANSI) character in the string.
        char lastChar(void);

	// Return the number of occurances of ch in the string.
	int countCharOccurances(char ch);

        // Remove the last N chars from the string.
        void shorten(int n);    

        // Convert to lower or upper case
        void toLowerCase(void);
        void toUpperCase(void);

        // Convert XML special characters to their representations
        // Example: '<' is converted to &lt;
        void encodeXMLSpecial(void);    // Convert '<' to &lt; (etc.)
        void decodeXMLSpecial(void);    // Convert &lt; to '<' (etc.)

	// Pattern can contain * and ? wildcards.
        bool containsSubstring(const char *pattern) const;	
        bool containsSubstringNoCase(const char *pattern) const;	

	// For many Win32 SDK functions, such as CreateFile, error information
	// must be retrieved by using the Win32 functions GetLastError and FormatMessage.
	// This method calls these Win32 functions to format the error and append it
	// to the string.
#ifdef WIN32
        void appendLastWindowsError(void);
#endif
	// Returns true if the strings are equal, or false.  (Not the same as 
	// "compare", which returns 0 if equal, and 1 or -1 if the strings are lexicographically
	// less than or greater than)
        bool equals(const char *s) const;   
        bool equalsStr(CkString &s) const;
        bool equalsIgnoreCase(const char *s) const;
        bool equalsIgnoreCaseStr(CkString &s) const;
        
        // Remove a chunk from the string.
        void removeChunk(int charStartPos, int numChars);

        // Remove all occurances of a particular character.
        void removeCharOccurances(char c);

	// Replace all occurance of c1 with c2.
	void replaceChar(char c1, char c2);

	// Replace the first occurance of c1 with a null terminator
	void chopAtFirstChar(char c1);

	void obfuscate(void);
	void unobfuscate(void);

	// Save the string to a file.
	// charset can be "ansi", "utf-8", "unicode", or anything else such as "iso-8859-1"
	bool saveToFile(const char *filename, const char *charset);

	CkStringArray *split(char splitChar, bool exceptDoubleQuoted, bool exceptEscaped, bool keepEmpty) const;
	CkStringArray *split2(const char *splitCharSet, bool exceptDoubleQuoted, bool exceptEscaped, bool keepEmpty) const;
	CkStringArray *tokenize(char *punctuation) const;

	// Equivalent to split2(" \t\r\n",true,true,false)
	CkStringArray *splitAtWS(void) const;

	// Return true if this string begins with substr (case sensitive)
	bool beginsWith(const char *sSubstr) const;


	CkString *getDelimited(const char *beginSearchAfter, 
				   const char *startDelim, const char *endDelim);

	void minimizeMemory(void);

	// STRING_INSERT_POINT

	// END PUBLIC INTERFACE

	bool isInternalPtr(const char *str);

    private:
	// Internal implementation.
	void *m_x;
	bool m_utf8;	// If true, all input "const char *" arguments are utf-8, otherwise they are ANSI strings.
	void *m_sb;	// Used for getEnc()

    public:
	// For internal use
	void *getImplX(void) const { return m_x; }

};
#pragma pack (pop)

#endif
