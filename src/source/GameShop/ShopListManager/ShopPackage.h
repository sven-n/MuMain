/**************************************************************************************************

��Ű�� ��ü

���� ��Ű�� ������ ������ �ִ�.
���� ��Ű���� ��� �Ǿ��ִ� "��ǰ ��ȣ" ��ϰ� "���� ��ȣ" ����� ������ �ִ�.

��Ű���� ���� ��ǰ�� ����ִ� ��쿡�� ���� ��ȣ�� �� �� �̴�.
��Ű���� ���� ������ �����Ǿ� �ִٸ� ��ǰ�� �� �� �̴�.

(exe 1)
A ��Ű���� a�� a' ��� ��ǰ�� �� �� �ִٸ� ���� ������ ���� �� �� ����.
-> A��ǰ a, a' 5000��

(exe 2)
B ��Ű���� b ��� ��ǰ�� �ϳ��� �ִٸ� 1000��, 2000��, 3000�� ���� ���� ������ ���� �� �� �ִ�.
-> B��ǰ b 1���� 1000��
-> B��ǰ b 2���� 2000��
-> B��ǰ b 4���� 3000��
�� ������ �ٸ� ��Ű���� �����־�� �Ѵ�.

**************************************************************************************************/

#pragma once

#include "Include.h"
#include <time.h>

class CShopPackage
{
public:
    CShopPackage();
    virtual ~CShopPackage();

    bool SetPackage(std::wstring strdata);
    void SetLeftCount(int nCount);

    int GetProductCount();                    // ��Ű�� ������ ��ǰ �� ��������
    void SetProductSeqFirst();                // ��Ű�� ������ ù��° ��ǰ ��ȣ�� ��ġ
    bool GetProductSeqFirst(int& ProductSeq); // ��Ű�� ������ ù��° ��ǰ ��ȣ�� �������� ���� ��ǰ ��ȣ�� �̵�
    bool GetProductSeqNext(int& ProductSeq);  // ��ǰ ��ȣ �������� ���� ��ġ�� �̵�

    int GetPriceCount();                  // ��Ű�� ������ ���� �� ��������
    void SetPriceSeqFirst();              // ��Ű�� ������ ù��° ���� ��ȣ�� ��ġ
    bool GetPriceSeqFirst(int& PriceSeq); // ��Ű�� ������ ù��° ���� ��ȣ�� �������� ���� ���� ��ȣ�� �̵�
    bool GetPriceSeqNext(int& PriceSeq);  // ���� ��ȣ �������� ���� ��ġ�� �̵�

public:
    int ProductDisplaySeq;                                   //  1. ��Ű���� �����ִ� ī�װ��� ��ȣ
    int ViewOrder;                                           //  2. ���� ����
    int PackageProductSeq;                                   //  3. ��Ű�� ��ȣ
    wchar_t PackageProductName[SHOPLIST_LENGTH_PACKAGENAME]; //  4. ��Ű�� ��
    int PackageProductType;                                  //  5. ��Ű�� ���� (170:�Ϲ� ��ǰ, 171:�̺�Ʈ ��ǰ)
    int Price;                                               //  6. ����
    wchar_t Description[SHOPLIST_LENGTH_PACKAGEDESC];        //  7. �� ����
    wchar_t Caution[SHOPLIST_LENGTH_PACKAGECAUTION];         //  8. ���� ����
    // clang-format off
    int SalesFlag; //  9. ���� ���� ����(���Ź�ư ���⿩��) (182:����,
                   //  183:�Ұ�)
    int GiftFlag;  // 10. ���� ���� ����(������ư ���⿩��)
                   // (184:����, 185:�Ұ�)
    tm StartDate;                                             // 11. �Ǹ� ������
    tm EndDate;                                               // 12. �Ǹ� ������
    // clang-format on
    int CapsuleFlag;                                          // 13. ĸ�� ��ǰ ���� (176:ĸ��, 177:�Ϲ�)
    int CapsuleCount;                                         // 14. ��Ű���� ���Ե� ��ǰ ����
    wchar_t ProductCashName[SHOPLIST_LENGTH_PACKAGECASHNAME]; // 15. ���� ĳ�� ��
    wchar_t PricUnitName[SHOPLIST_LENGTH_PACKAGEPRICEUNIT];   // 16. ���� ���� ǥ�� ��
    int DeleteFlag;                                           // 17. ���� ���� (180:����, 181:Ȱ��)
    int EventFlag;                                            // 18. �̺�Ʈ ��ǰ ���� (199:�̺�Ʈ ��ǰ, 200:�Ϲ� ��ǰ)
    int ProductAmount;                                        // 19. ���� ��ǰ ����
    wchar_t InGamePackageID[SHOPLIST_LENGTH_INGAMEPACKAGEID]; // 21. ��Ű�� ������ �ڵ�
    int ProductCashSeq;                                       // 22. ���� ĳ�� ���� �ڵ�
    int PriceCount;         // 23. ���� ��ǰ ���� ��å ���� ���� (���� ��ǰ�� 1���� ��쿡�� PriceSeq�� ���� ���� �� �ִ�.)
    bool DeductMileageFlag; // 25. ���ϸ����� ���� ��ǰ ���� (false : �Ϲ�, true : ���ϸ��� ���� ��ǰ)
    int CashType;           // 26. �۷ι� ���� : Wcoin(C), WCoin(P) ����
    int CashTypeFlag;       // 27. �۷ι� ���� : Wcoin(C), WCoin(P) ���� or �ڵ� ����(668: ���μ���, 669: �ڵ�����)

    int LeftCount; // �ܿ� ����

private:
    void SetProductSeqList(std::wstring strdata);
    void SetPriceSeqList(std::wstring strdata);

    std::vector<int> ProductSeqList; // 20. ��Ű���� ���Ե� ��ǰ ��ȣ ���
    std::vector<int>::iterator ProductSeqIter;

    std::vector<int> PriceSeqList; // 24. ��Ű���� ���ԵǴ� ���� ��ȣ ��� (��ǰ�� 1���� ��쿡�� ���� ����.)
    std::vector<int>::iterator PriceSeqIter;
};
