/**************************************************************************************************

전체 카테고리 목록 객체

iterator를 이용하여 순차적으로 카테고리 객체를 가져올 수 있다.
카테고리 번호를 이용하여 카테고리 객체를 가져올 수 있다.

**************************************************************************************************/

#pragma once

#include "ShopCategory.h"
#include <map>

class CShopCategoryList
{
public:
    CShopCategoryList(void);
    ~CShopCategoryList(void);

    void Clear();

    int GetSize();
    virtual void Append(CShopCategory category);

    void SetFirst();											// 카테고리 목록에서 첫 번째 카테고리를 가리키게 한다.
    bool GetNext(CShopCategory& category);						// 현재 카테고리 객체를 넘기고 다음 카테고리 객체를 가리키게 한다.

    bool GetValueByKey(int nKey, CShopCategory& category);		// 카테고리 순번으로 카테고리 객체를 가져온다.
    bool GetValueByIndex(int nIndex, CShopCategory& category);	// 인덱스 번호로 카테고리 객체를 가져온다.

    bool InsertPackage(int Category, int Package);
    bool RefreshPackageSeq(int Category, int PackageSeqs[], int PackageCount);

protected:
    std::map<int, CShopCategory> m_Categroys;				// 카테고리 객체 맵
    std::map<int, CShopCategory>::iterator m_Categoryiter;	// 카테고리 iterator
    std::vector<int> m_CategoryIndex;						// 카테고리 번호 인덱스 목록
};
