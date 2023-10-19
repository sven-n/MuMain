#ifndef _PERSONALSHOPTITLEIMP_H_
#define _PERSONALSHOPTITLEIMP_H_

// - ÀÌÇö

#include "zzzinfomation.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"

inline POINT MakePos(long x, long y)
{
    POINT pos = { x, y };
    return pos;
}
inline SIZE MakeSize(int cx, int cy)
{
    SIZE size = { cx, cy };
    return size;
}

//. CPersonalShopTitleImp
class CPersonalShopTitleImp
{
    class CShopTitleDrawObj
    {
    public:
        enum { EXTRA_SPACE = 25 };

        CShopTitleDrawObj();
        ~CShopTitleDrawObj();

        bool Create(int key, const std::wstring& name, const std::wstring& title, POINT& pos);
        void Release();

        int	GetKey() const;

        void SetBoxContent(const std::wstring& name, const std::wstring& title);
        void SetBoxPos(POINT& pos);
        void GetFullTitle(std::wstring& title);

        void GetBoxSize(SIZE& size);
        void GetBoxPos(POINT& pos);
        void GetBoxRect(RECT& rect);

        void EnableDraw();
        void DisableDraw();
        bool IsVisible() const;

        void EnableHighlight();
        void DisableHighlight();
        bool IsHighlight() const;
        void Draw(int iPkLevel);

    private:
        void Init();
        void SeparateShopTitle(IN const std::wstring& title, OUT std::wstring& topTitle, OUT std::wstring& bottomTitle);
        void CalculateBooleanSize(IN const std::wstring& name, IN const std::wstring& topTitle, IN const std::wstring& bottomTitle, OUT SIZE& size);

    private:
        std::wstring	m_fullname;
        std::wstring	m_fulltitle;
        std::wstring	m_topTitle;
        std::wstring	m_bottomTitle;

        int		m_key;
        bool	m_bDraw;
        POINT	m_pos;
        SIZE	m_size, m_icon;
        bool	m_bHighlight;
    };

public:
    ~CPersonalShopTitleImp();

    bool AddShopTitle(int key, CHARACTER* pPlayer, const std::wstring& title);
    void RemoveShopTitle(CHARACTER* pPlayer);
    void RemoveAllShopTitle();
    void RemoveAllShopTitleExceptHero();

    CHARACTER* FindCharacter(int key) const;

    void ShowShopTitles();
    void HideShopTitles();
    bool IsShowShopTitles() const;

    void EnableShopTitleDraw(CHARACTER* pPlayer);
    void DisableShopTitleDraw(CHARACTER* pPlayer);
    bool IsShopTitleVisible(CHARACTER* pPlayer);

    bool IsShopTitleHighlight(CHARACTER* pPlayer) const;

    bool IsInViewport(CHARACTER* pPlayer);
    void GetShopTitle(CHARACTER* pPlayer, std::wstring& title);
    void GetShopTitleSummary(CHARACTER* pPlayer, std::wstring& summary);

    DWORD GetShopTextColor(CHARACTER* pPlayer);
    DWORD GetShopText2Color(CHARACTER* pPlayer);
    DWORD GetShopBGColor(CHARACTER* pPlayer);

    void Update();
    void Draw();

    static CPersonalShopTitleImp* GetObjPtr();		//. singleton obj pointer

protected:
    CPersonalShopTitleImp();	//. ban create instance

    void UpdatePosition();
    void RevisionPosition();
    void CheckKeyIntegrity();

    void CalculateBooleanPos(IN CHARACTER* pPlayer, IN const SIZE& size, OUT POINT& pos);

private:
    typedef std::map<CHARACTER*, CShopTitleDrawObj*> type_drawobj_map;
    type_drawobj_map m_listShopTitleDrawObj;

    int	m_iHighlightFrame;
    bool m_bShow;
};

//. wrapping to C-style function
inline bool AddShopTitle(int key, CHARACTER* pPlayer, const std::wstring& title) {
    return CPersonalShopTitleImp::GetObjPtr()->AddShopTitle(key, pPlayer, title);
}
inline void RemoveShopTitle(CHARACTER* pPlayer) {
    CPersonalShopTitleImp::GetObjPtr()->RemoveShopTitle(pPlayer);
}
inline void RemoveAllShopTitle() {
    CPersonalShopTitleImp::GetObjPtr()->RemoveAllShopTitle();
}
inline void RemoveAllShopTitleExceptHero() {
    CPersonalShopTitleImp::GetObjPtr()->RemoveAllShopTitleExceptHero();
}
inline CHARACTER* FindCharacterTagShopTitle(int key) {
    return CPersonalShopTitleImp::GetObjPtr()->FindCharacter(key);
}

inline void ShowShopTitles()
{
    CPersonalShopTitleImp::GetObjPtr()->ShowShopTitles();
}

inline void HideShopTitles()
{
    CPersonalShopTitleImp::GetObjPtr()->HideShopTitles();
}

inline bool IsShowShopTitles() {
    return CPersonalShopTitleImp::GetObjPtr()->IsShowShopTitles();
}

inline void EnableShopTitleDraw(CHARACTER* pPlayer) {
    CPersonalShopTitleImp::GetObjPtr()->EnableShopTitleDraw(pPlayer);
}
inline void DisableShopTitleDraw(CHARACTER* pPlayer) {
    CPersonalShopTitleImp::GetObjPtr()->DisableShopTitleDraw(pPlayer);
}
inline bool IsShopTitleVisible(CHARACTER* pPlayer) {
    return CPersonalShopTitleImp::GetObjPtr()->IsShopTitleVisible(pPlayer);
}

inline bool IsShopTitleHighlight(CHARACTER* pPlayer) {
    return CPersonalShopTitleImp::GetObjPtr()->IsShopTitleHighlight(pPlayer);
}

inline bool IsShopInViewport(CHARACTER* pPlayer) {
    return CPersonalShopTitleImp::GetObjPtr()->IsInViewport(pPlayer);
}
inline void GetShopTitle(CHARACTER* pPlayer, std::wstring& title) {
    CPersonalShopTitleImp::GetObjPtr()->GetShopTitle(pPlayer, title);
}
inline void GetShopTitleSummary(CHARACTER* pPlayer, std::wstring& summary) {
    CPersonalShopTitleImp::GetObjPtr()->GetShopTitleSummary(pPlayer, summary);
}

inline DWORD GetShopTextColor(CHARACTER* pPlayer)
{
    return CPersonalShopTitleImp::GetObjPtr()->GetShopTextColor(pPlayer);
}

inline DWORD GetShopText2Color(CHARACTER* pPlayer)
{
    return CPersonalShopTitleImp::GetObjPtr()->GetShopText2Color(pPlayer);
}

inline DWORD GetShopBGColor(CHARACTER* pPlayer)
{
    return CPersonalShopTitleImp::GetObjPtr()->GetShopBGColor(pPlayer);
}

inline void UpdatePersonalShopTitleImp()
{
    CPersonalShopTitleImp::GetObjPtr()->Update();
}

inline void DrawPersonalShopTitleImp()
{
    CPersonalShopTitleImp::GetObjPtr()->Draw();
}

//. CPersonalItemPriceTable
class CPersonalItemPriceTable
{
    std::map<int, int>	m_mapTable;

    static CPersonalItemPriceTable* ms_pSeller;
    static CPersonalItemPriceTable* ms_pBuyer;

public:
    enum { TYPE_NONE = -1, TYPE_SELLER = 1, TYPE_BUYER };

    CPersonalItemPriceTable()
    {
    }
    ~CPersonalItemPriceTable()
    {
        RemoveAllItemPrice();
    }

    void AddItemPrice(int index, int price)
    {
        auto mi = m_mapTable.find(index);
        if (mi != m_mapTable.end())
        {
            m_mapTable.erase(mi);
        }
        m_mapTable.insert(std::map<int, int>::value_type(index, price));
    }
    void RemoveItemPrice(int index)
    {
        auto mi = m_mapTable.find(index);
        if (mi != m_mapTable.end())
        {
            m_mapTable.erase(mi);
        }
    }
    void RemoveAllItemPrice()
    {
        m_mapTable.clear();
    }

    bool GetItemPrice(int index, int& price)
    {
        auto mi = m_mapTable.find(index);
        if (mi != m_mapTable.end())
        {
            price = (*mi).second;
            return true;
        }
        return false;
    }

    static bool CreatePersonalItemTable()
    {
        if (ms_pSeller != NULL || ms_pBuyer != NULL)
            return false;	//. error

        ms_pSeller = new CPersonalItemPriceTable;
        ms_pBuyer = new CPersonalItemPriceTable;

        return true;
    }
    static void ReleasePersonalItemTable()
    {
        if (ms_pSeller)
        {
            delete ms_pSeller;
            ms_pSeller = NULL;
        }
        if (ms_pBuyer)
        {
            delete ms_pBuyer;
            ms_pBuyer = NULL;
        }
    }
    static CPersonalItemPriceTable* GetObjPtr(int type)
    {
        CPersonalItemPriceTable* pPersonalItemTable = NULL;
        switch (type)
        {
        case TYPE_SELLER:
            pPersonalItemTable = ms_pSeller; break;
        case TYPE_BUYER:
            pPersonalItemTable = ms_pBuyer; break;
        }

        return pPersonalItemTable;
    }
};

//. wrapping to C-style function
inline bool CreatePersonalItemTable()
{
    return CPersonalItemPriceTable::CreatePersonalItemTable();
}

inline void ReleasePersonalItemTable()
{
    CPersonalItemPriceTable::ReleasePersonalItemTable();
}

inline void AddPersonalItemPrice(int index, int price, int type)
{
    CPersonalItemPriceTable* pPersonalItemTable = CPersonalItemPriceTable::GetObjPtr(type);
    if (pPersonalItemTable)
    {
        pPersonalItemTable->AddItemPrice(index, price);
    }
}

inline void RemovePersonalItemPrice(int index, int type)
{
    CPersonalItemPriceTable* pPersonalItemTable = CPersonalItemPriceTable::GetObjPtr(type);
    if (pPersonalItemTable)
    {
        pPersonalItemTable->RemoveItemPrice(index);
    }
}

inline void RemoveAllPerosnalItemPrice(int type)
{
    CPersonalItemPriceTable* pPersonalItemTable = CPersonalItemPriceTable::GetObjPtr(type);
    if (pPersonalItemTable)
    {
        pPersonalItemTable->RemoveAllItemPrice();
    }
}

inline bool GetPersonalItemPrice(int index, int& price, int type)
{
    CPersonalItemPriceTable* pPersonalItemTable = CPersonalItemPriceTable::GetObjPtr(type);
    if (!pPersonalItemTable)
        return false;

    return pPersonalItemTable->GetItemPrice(index, price);
}

inline bool CheckPriceIntegrity(const wchar_t* szZen, int size)
{
    if (size > 255) return false;
    for (int i = 0; i < size; i++)
    {
        if (szZen[i] == '\0')
            break;
        if (szZen[i] & 0x80)
        {
            return false;
        }
    }
    return true;
}

#endif // _PERSONALSHOPTITLEIMP_H_