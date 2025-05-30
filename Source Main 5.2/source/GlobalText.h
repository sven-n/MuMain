#pragma once

#include "stringset.h"

template <class T>
class TGlobalText
{
#pragma pack(push, 1)
    struct GLOBALTEXT_HEADER
    {
        WORD	wSignature;	
        DWORD	dwNumberOfText;
    };
    struct GLOBALTEXT_STRING_HEADER
    {
        DWORD	dwKey;
        DWORD	dwSizeOfString;
    };
#pragma pack(pop)

    class CKeyCode
    {
        std::wstring	m_strKeyCode;
    public:
        CKeyCode(int key)
        {
            wchar_t szKeyCode[12] = { 0, };
            _itow(key, szKeyCode, 10);
            m_strKeyCode = szKeyCode;

            if (m_strKeyCode.size() < 9)
            {
                int nInsertChar = 9 - m_strKeyCode.size();
                m_strKeyCode.insert(m_strKeyCode.begin(), nInsertChar, '0');
            }
            m_strKeyCode.resize(9);
        }
        ~CKeyCode() { }

        WORD GetTypeCode()
        {
            std::wstring strTypeCode(m_strKeyCode, 0, 2);
            return _wtoi(strTypeCode.c_str());
        }
        WORD GetCountryCode()
        {
            std::wstring strCountryCode(m_strKeyCode, 2, 3);
            return _wtoi(strCountryCode.c_str());
        }
        WORD GetIndexCode()
        {
            std::wstring strIndexCode(m_strKeyCode, 5, 4);
            return _wtoi(strIndexCode.c_str());
        }
    };

    leaf::TStringSet<T>		m_StringSet;

public:

    TGlobalText() {}
    ~TGlobalText() { RemoveAll(); }

    enum
    {
        MAX_NUMBER_OF_TEXTS = 9999,

        LD_COMMON_TEXTS = 0x00000000,

        LD_USA_CANADA_TEXTS = 0x00000001,
        LD_JAPAN_A_TEXTS = 0x00000002,
        LD_TAIWAN_TEXTS = 0x00000004,
        LD_PHILIPPINES_TEXTS = 0x00000008,
        LD_CHINA_A_TEXTS = 0x00000010,
        LD_BRAZIL_A_TEXTS = 0x00000020,
        LD_SOUTH_KOREA_TEXTS = 0x00000040,
        LD_THAILAND_TEXTS = 0x00000080,
        LD_VIETNAM_TEXTS = 0x00000100,

        LD_FOREIGN_TEXTS = 0X80000000,

        LD_ALL_TEXTS = 0xFFFFFFFF
    };

    bool Load(const std::wstring& strFilePath, DWORD dwLoadDisposition)
    {
        FILE* fp = _wfopen(strFilePath.c_str(), L"rb");
        if (NULL == fp)
            return false;

        GLOBALTEXT_HEADER GTHeader;
        fread(&GTHeader, sizeof(GLOBALTEXT_HEADER), 1, fp);

        if (GTHeader.wSignature != 0x5447)
            return false;

        for (int i = 0; i < (int)(GTHeader.dwNumberOfText); i++)
        {
            GLOBALTEXT_STRING_HEADER GTStringHeader;
            fread(&GTStringHeader, sizeof(GLOBALTEXT_STRING_HEADER), 1, fp);

            char* pStringBuffer = new char[GTStringHeader.dwSizeOfString + 1];
            fread(pStringBuffer, sizeof(char), GTStringHeader.dwSizeOfString, fp);

            if (CheckLoadDisposition(GTStringHeader.dwKey, dwLoadDisposition) || GTStringHeader.dwKey < MAX_NUMBER_OF_TEXTS)
            {
                BuxConvert(pStringBuffer, sizeof(char) * GTStringHeader.dwSizeOfString);		//. decoding
                pStringBuffer[GTStringHeader.dwSizeOfString] = '\0';

                int wchars_num = MultiByteToWideChar(CP_UTF8, 0, pStringBuffer, -1, NULL, 0);
                auto* text = new wchar_t[wchars_num];
                MultiByteToWideChar(CP_UTF8, 0, pStringBuffer, -1, text, wchars_num);

                m_StringSet.Add(GTStringHeader.dwKey, text);
            }

            delete[] pStringBuffer;
        }
        fclose(fp);
        return true;
    }
    bool Save(const std::wstring& strFilePath)
    {
        FILE* fp = _wfopen(strFilePath.c_str(), L"wb");
        if (NULL == fp)
            return false;

        GLOBALTEXT_HEADER GTHeader;
        GTHeader.wSignature = 0x5447;
        GTHeader.dwNumberOfText = m_StringSet.GetCount();
        fwrite(&GTHeader, sizeof(GLOBALTEXT_HEADER), 1, fp);

        for (int i = 0; i < GTHeader.dwNumberOfText; i++)
        {
            int key = m_StringSet.GetKey(i);
            if (-1 == key)
            {
                fclose(fp);
                return false;
            }

            GLOBALTEXT_STRING_HEADER GTStringHeader;
            GTStringHeader.dwKey = key;
            GTStringHeader.dwSizeOfString = m_StringSet.FindObj(key).size();

            fwrite(&GTStringHeader, sizeof(GLOBALTEXT_STRING_HEADER), 1, fp);

            T* pStringBuffer = new T[GTStringHeader.dwSizeOfString];
            memcpy(pStringBuffer, m_StringSet[key], sizeof(T) * GTStringHeader.dwSizeOfString);

            BuxConvert(pStringBuffer, sizeof(T) * GTStringHeader.dwSizeOfString);		//. encoding
            fwrite(pStringBuffer, sizeof(T), GTStringHeader.dwSizeOfString, fp);

            delete[] pStringBuffer;
        }
        fclose(fp);
        return true;
    }

    bool Add(int key, const T* szString) { return m_StringSet.Add(key, szString); }
    bool Remove(int key) { return m_StringSet.Remove(key); }
    void RemoveAll() { m_StringSet.RemoveAll(); }

    const T* Get(int key) { return m_StringSet[key]; }
    size_t GetStringSize(int key) { return m_StringSet.FindObj(key).size(); }

    WORD GetNumCountryCode(const std::wstring& strAlpha3Code)
    {	//. strAlpha3Code: Official Alpha-3 Code
        if (0 == strAlpha3Code.compare(L"USA") || 0 == strAlpha3Code.compare(L"CAN")) return 130;
        if (0 == strAlpha3Code.compare(L"JPN")) return 450;
        if (0 == strAlpha3Code.compare(L"TPE")) return 471;
        if (0 == strAlpha3Code.compare(L"PHI")) return 480;
        if (0 == strAlpha3Code.compare(L"CHN")) return 690;
        if (0 == strAlpha3Code.compare(L"BRA")) return 789;
        if (0 == strAlpha3Code.compare(L"KOR")) return 880;
        if (0 == strAlpha3Code.compare(L"THA")) return 885;
        if (0 == strAlpha3Code.compare(L"VIE")) return 893;
        if (0 == strAlpha3Code.compare(L"FRN")) return 995;

        return 0;
    }

    const T* operator [] (int key)
    {
        return Get(key);
    }

protected:
    void BuxConvert(PVOID pvBuffer, DWORD dwSize)
    {
        PBYTE pbyBuffer = (PBYTE)(pvBuffer);
        BYTE byBuxCode[3] = { 0xfc,0xcf,0xab };

        for (int i = 0; i < (int)dwSize; i++)
            pbyBuffer[i] ^= byBuxCode[i % 3];
    }
    bool CheckLoadDisposition(int key, DWORD dwLoadDisposition)
    {
        CKeyCode KeyCode(key);
        if (((dwLoadDisposition & LD_USA_CANADA_TEXTS) == LD_USA_CANADA_TEXTS) &&
            (KeyCode.GetCountryCode() == GetNumCountryCode(L"USA") || KeyCode.GetCountryCode() == GetNumCountryCode(L"CAN")))
            return true;
        if (((dwLoadDisposition & LD_JAPAN_A_TEXTS) == LD_JAPAN_A_TEXTS) &&
            (KeyCode.GetCountryCode() == GetNumCountryCode(L"JPN")))
            return true;
        if (((dwLoadDisposition & LD_TAIWAN_TEXTS) == LD_TAIWAN_TEXTS) &&
            (KeyCode.GetCountryCode() == GetNumCountryCode(L"TPE")))
            return true;
        if (((dwLoadDisposition & LD_PHILIPPINES_TEXTS) == LD_PHILIPPINES_TEXTS) &&
            (KeyCode.GetCountryCode() == GetNumCountryCode(L"PHI")))
            return true;
        if (((dwLoadDisposition & LD_CHINA_A_TEXTS) == LD_CHINA_A_TEXTS) &&
            (KeyCode.GetCountryCode() == GetNumCountryCode(L"CHN")))
            return true;
        if (((dwLoadDisposition & LD_BRAZIL_A_TEXTS) == LD_BRAZIL_A_TEXTS) &&
            (KeyCode.GetCountryCode() == GetNumCountryCode(L"BRA")))
            return true;
        if (((dwLoadDisposition & LD_SOUTH_KOREA_TEXTS) == LD_SOUTH_KOREA_TEXTS) &&
            (KeyCode.GetCountryCode() == GetNumCountryCode(L"KOR")))
            return true;
        if (((dwLoadDisposition & LD_THAILAND_TEXTS) == LD_THAILAND_TEXTS) &&
            (KeyCode.GetCountryCode() == GetNumCountryCode(L"THA")))
            return true;
        if (((dwLoadDisposition & LD_VIETNAM_TEXTS) == LD_VIETNAM_TEXTS) &&
            (KeyCode.GetCountryCode() == GetNumCountryCode(L"VIE")))
            return true;
        if (((dwLoadDisposition & LD_FOREIGN_TEXTS) == LD_FOREIGN_TEXTS) &&
            (KeyCode.GetCountryCode() == GetNumCountryCode(L"FRN")))
            return true;
        return false;
    }
};

typedef TGlobalText<char>		CGlobalText;
typedef TGlobalText<wchar_t>	CGlobalTextW;

extern CGlobalTextW GlobalText;

