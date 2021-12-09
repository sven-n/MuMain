// CkStringArray.h: interface for the CkStringArray class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "CkString.h"
class CkByteData;

#include "CkObject.h"

#pragma pack (push, 8) 

// CLASS: CkStringArray
class CkStringArray : public CkObject  
{
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkStringArray(const CkStringArray &c) { } 
	CkStringArray &operator=(const CkStringArray &) { return *this; }
	CkStringArray(void *impl) : m_impl(impl) { }


    public:
	void *getImpl(void) const { return m_impl; }
	void inject(void *impl);

	CkStringArray();
	virtual ~CkStringArray();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);


	// Add a string to the array.
	void Append(const char *str);

	// Return the number of strings in the array.
	long get_Count();

	// Loads a file into an array of strings -- one per line.
	// CRLFs are not included in each string.
	// Blank lines are ignored and not included in the array.
	bool LoadFromFile(const char *filename);

	// Saves the string array to a file.  Line endings are controlled
	// by the Crlf property (get_Crlf/put_Crlf)
	bool SaveToFile(const char *filename);

	// Return the last string in the array, and remove it.
	bool Pop(CkString &str);

	bool LastString(CkString &str);

	bool RemoveAt(long index);

	void InsertAt(long index, const char *str);

	// Find an exact match, case sensitive.
	// Begin searching at firstIndex.
	long Find(const char *str, long firstIndex);

	void Prepend(const char *str);

        // If true, always convert to CRLF before adding, and before returning.
        // If false, convert to bare LF before adding and before returning.
	bool get_Crlf();
	void put_Crlf(bool newVal);

        // If true, always trim whitespace from both ends of the string before
        // adding.
	bool get_Trim();
	void put_Trim(bool newVal);

        // If true, then duplicates cannot exist in the collection.
	// Methods such as Append will do nothing if the string already
	// exists.
	// The default is false.
	bool get_Unique();
	void put_Unique(bool newVal);

	// Serialize the entire string array into a single Base64-encoded string.
	void Serialize(CkString &encodedStr);
	// Append from a previously serialized string array.
	bool AppendSerialized(const char *encodedStr);

	bool GetString(long index, CkString &str);
	const char *GetString(long index);
        const char *getString(long index);

	void Subtract(CkStringArray &array);

	// Remove the string that exactly matches str (case sensitive).
	void Remove(const char *str);

	// Return true if the string array contains the str (case sensitive)
	bool Contains(const char *str);

	// Remove all strings from the array.
	void Clear();

	// Sort the string array in lexicographic order.
	void Sort(bool ascending);

	// Split a string at a boundary (1 character or more)
	// and append the individual strings to this string array.
	void SplitAndAppend(const char *str, const char *boundary);

	CkString m_resultString;
        const char *strAt(long index);
        const char *serialize(void);
        const char *pop(void);
	const char *lastStr(void);
	// LOADFROMTEXT_BEGIN
	void LoadFromText(const char *str);
	// LOADFROMTEXT_END
	// SAVENTHTOFILE_BEGIN
	bool SaveNthToFile(int index, const char *filename);
	// SAVENTHTOFILE_END
	// SAVETOTEXT_BEGIN
	void SaveToText(CkString &outStr);
	const char *saveToText(void);
	// SAVETOTEXT_END
	// FINDFIRSTMATCH_BEGIN
	int FindFirstMatch(const char *str, int firstIndex);
	// FINDFIRSTMATCH_END
	// UNION_BEGIN
	void Union(CkStringArray &array);
	// UNION_END
	// GETSTRINGLEN_BEGIN
	int GetStringLen(int index);
	// GETSTRINGLEN_END

	// STRINGARRAY_INSERT_POINT

	// END PUBLIC INTERFACE

};
#pragma pack (pop)

