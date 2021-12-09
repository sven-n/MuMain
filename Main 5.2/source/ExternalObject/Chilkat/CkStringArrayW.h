#ifndef _CkStringArrayW_H
#define _CkStringArrayW_H
#pragma once

#include "CkString.h"
#include "CkStringArray.h"


#pragma pack (push, 8) 


class CkStringArrayW : public CkStringArray
{
    private:
	unsigned long nextIdxW(void)
	    {
	    m_resultIdxW++;
	    if (m_resultIdxW >= 10)
		{
		m_resultIdxW = 0;
		}
	    return m_resultIdxW;
	    }
	unsigned long m_resultIdxW;
	CkString m_resultStringW[10];

    public:
	CkStringArrayW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkStringArrayW() { }
	void appendW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->Append(sTemp1.getUtf8());
		}
	bool appendSerializedW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AppendSerialized(sTemp1.getUtf8());
		}
	bool containsW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->Contains(sTemp1.getUtf8());
		}
	long findW(const wchar_t *w1,long firstIndex)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->Find(sTemp1.getUtf8(),firstIndex);
		}
	int findFirstMatchW(const wchar_t *w1,int firstIndex)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->FindFirstMatch(sTemp1.getUtf8(),firstIndex);
		}
	void insertAtW(long index,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->InsertAt(index,sTemp1.getUtf8());
		}
	bool loadFromFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadFromFile(sTemp1.getUtf8());
		}
	void loadFromTextW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->LoadFromText(sTemp1.getUtf8());
		}
	void prependW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->Prepend(sTemp1.getUtf8());
		}
	void removeW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->Remove(sTemp1.getUtf8());
		}
	bool saveNthToFileW(int index,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveNthToFile(index,sTemp1.getUtf8());
		}
	bool saveToFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveToFile(sTemp1.getUtf8());
		}
	void splitAndAppendW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->SplitAndAppend(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	const wchar_t *getStringW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetString(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lastStrW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->LastString(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *popW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->Pop(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *saveToTextW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->SaveToText(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *serializeW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->Serialize(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *strAtW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetString(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}

};
#endif
