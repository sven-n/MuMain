/*
작성일: 2009-07-24
작성자: 문상현
요약: 샵리스트 관리를 위해 사용되는 객체
*/

#pragma once

#include "include.h"
#include "ListManager.h"
#include "ShopList.h"

class CShopListManager : public CListManager
{
public:
    CShopListManager();
    virtual ~CShopListManager();

    CShopList* GetListPtr() { return m_ShopList; };

private:
    CShopList* m_ShopList;

    WZResult		LoadScript(bool bDonwLoad);
};