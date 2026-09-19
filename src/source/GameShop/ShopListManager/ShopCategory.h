/**************************************************************************************************

ī�װ��� ��ü

���� ī�װ��� ������ ������ �ִ�.
���� ī�װ��� ������ "ī�װ��� ��ȣ"�� ������� ������ �ִ�.
���� ī�װ����� �� ���� ī�װ������ "��Ű�� ��ȣ" ����� ������ �ִ�.

**************************************************************************************************/

#pragma once

#include "Include.h"

class CShopCategory
{
public:
    CShopCategory();
    virtual ~CShopCategory();

    bool SetCategory(std::wstring strdata);

    // clang-format off
    void SetCategoryFirst(); // ���� ī�װ��� ����� ù ��° �׷���
                             // ����Ű���� �����Ѵ�.
    bool GetCategoryNext(int& CategorySeq); // ���� ī�װ��� ��ȣ�� �����ϰ� ���� ���� ī�װ��� ��ȣ�� ����Ų��.

    void SetPackagSeqFirst(); // ī�װ����� ��ϵǾ� �ִ� ��Ű�� ����� ù ��° �׸���
                              // ����Ű���� �����Ѵ�.
    bool GetPackagSeqNext(int& PackagSeq); // ��Ű�� ��ȣ�� �����ϰ� ���� ��Ű�� ��ȣ�� ����Ų��.
    // clang-format on

    void AddPackageSeq(int PackageSeq);
    void ClearPackageSeq();

public:
    int ProductDisplaySeq;                              // 1. ī�װ��� ��ȣ
    wchar_t CategroyName[SHOPLIST_LENGTH_CATEGORYNAME]; // 2. ī�װ��� �̸�
    int EventFlag;                                      // 3. �̺�Ʈ ī�װ��� ���� (199:�̺�Ʈ, 200:�Ϲ�)
    int OpenFlag;                                       // 4. ���� ���� (201:����, 202: �����)
    int ParentProductDisplaySeq;                        // 5. �θ� ī�װ��� ��ȣ
    int DisplayOrder;                                   // 6. ���� ����
    int Root;                                           // 7. �ֻ��� ī�װ��� ���� (1: �ֻ���, 0: ����)

    std::vector<int> CategoryList; // ���� ī�װ����� ���� "ī�װ��� ��ȣ" ���
    std::vector<int>::iterator Categoryiter;

    std::vector<int> PackageList; // ���� ī�װ����� ���Ե� "��Ű�� ��ȣ" ���
    std::vector<int>::iterator Packageiter;
};
