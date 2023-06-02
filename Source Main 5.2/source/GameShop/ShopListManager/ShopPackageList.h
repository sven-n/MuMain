/**************************************************************************************************

전체 페키지 목록 객체

iterator를 이용하여 순차적으로 페키지 객체를 가져올 수 있다.
페키지 번호를 이용하여 패키지 객체를 가져올 수 있다.

**************************************************************************************************/

#pragma once

#include "ShopPackage.h"
#include <map>

class CShopPackageList
{
public:
    CShopPackageList(void);
    ~CShopPackageList(void);

    int GetSize();
    void Clear();

    virtual void Append(CShopPackage package);

    void SetFirst();											// 패키지 목록에서 첫 번째 패키지를 가리키게 한다.
    bool GetNext(CShopPackage& package);						// 현재 패키지 객체를 넘기고 다음 패키지 객체를 가리키게 한다.

    bool GetValueByKey(int nKey, CShopPackage& package);		// 패키지 번호로 해당 패키지 객체 가져오기
    bool GetValueByIndex(int nIndex, CShopPackage& package);	// 인덱스 번호로 해당 패키지 객체 가져오기

    bool SetPacketLeftCount(int PackageSeq, int nCount);

protected:
    std::map<int, CShopPackage> m_Packages;
    std::map<int, CShopPackage>::iterator m_iter;
    std::vector<int> m_PackageIndex;
};
