#pragma once

#define MAX_BUFF_NAME_LENGTH	50

#define MAX_DESCRIPT_LENGTH		100

struct _BUFFINFO
{
    short	s_BuffIndex;
    BYTE	s_BuffEffectType;
    BYTE	s_ItemType;
    BYTE	s_ItemIndex;
    char	s_BuffName[MAX_BUFF_NAME_LENGTH];
    BYTE	s_BuffClassType;
    BYTE	s_NoticeType;
    BYTE	s_ClearType;
    char	s_BuffDescript[MAX_DESCRIPT_LENGTH];
};

class BuffInfo
{
public:
    BuffInfo();
    virtual ~BuffInfo();

public:
    short	s_BuffIndex;
    BYTE	s_BuffEffectType;
    BYTE	s_ItemType;
    BYTE	s_ItemIndex;
    wchar_t	s_BuffName[MAX_BUFF_NAME_LENGTH];
    BYTE	s_BuffClassType;
    BYTE	s_NoticeType;
    BYTE	s_ClearType;
    wchar_t	s_BuffDescript[MAX_DESCRIPT_LENGTH];
    std::list<std::wstring> s_BuffDescriptlist;
};

SmartPointer(BuffScriptLoader);
class BuffScriptLoader
{
public:
    static BuffScriptLoaderPtr Make();
    virtual ~BuffScriptLoader();

private:
    bool Load(const std::wstring& pchFileName);

public:
    const BuffInfo GetBuffinfo(eBuffState type) const;
    eBuffClass IsBuffClass(eBuffState type) const;

#ifdef KJH_PBG_ADD_INGAMESHOP_SYSTEM
    int GetBuffIndex(int iItemCode);
    int GetBuffType(int iItemCode);
#endif // KJH_PBG_ADD_INGAMESHOP_SYSTEM

private:
    BuffScriptLoader();

private:
    typedef std::map<eBuffState, BuffInfo> BuffInfoMap;

private:
    BuffInfoMap			m_Info;
};
