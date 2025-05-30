#pragma once

SmartPointer(BuffStateValueControl);

class BuffStateValueControl
{
private:
    struct BuffStateValueInfo
    {
        DWORD    s_Value1;
        DWORD    s_Value2;
        DWORD    s_Time;

        BuffStateValueInfo() : s_Value1(0), s_Value2(0), s_Time(0) {}
    };

public:
    static BuffStateValueControlPtr Make();
    virtual ~BuffStateValueControl();

public:
    const BuffStateValueInfo GetValue(eBuffState bufftype);
    void GetBuffInfoString(std::list<std::wstring>& outstr, eBuffState bufftype);
    void GetBuffValueString(std::wstring& outstr, eBuffState bufftype);
    eBuffValueLoadType CheckValue(eBuffState bufftype);

private:
    void SetValue(eBuffState bufftype, BuffStateValueInfo& valueinfo);
    void Initialize();
    void Destroy();
    BuffStateValueControl();

private:
    typedef std::map<eBuffState, BuffStateValueInfo>  BuffStateValueMap;

private:
    BuffStateValueMap				m_BuffStateValue;
};
