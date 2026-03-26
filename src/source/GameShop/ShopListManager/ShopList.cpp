//************************************************************************
//
// FILE: ShopList.cpp
// Removed #ifdef _WIN32 guard (Story 7.6.6)
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "ShopList.h"

#include <fstream>
#include <cstring>

CShopList::CShopList()
{
    // cppcheck-suppress [noCopyConstructor, noOperatorEq]
    this->m_CategoryListPtr = new CShopCategoryList;
    this->m_PackageListPtr = new CShopPackageList;
    this->m_ProductListPtr = new CShopProductList;
}

CShopList::~CShopList()
{
    SAFE_DELETE(m_CategoryListPtr);
    SAFE_DELETE(m_PackageListPtr);
    SAFE_DELETE(m_ProductListPtr);
}

WZResult CShopList::LoadCategroy(const wchar_t* szFilePath)
{
    WZResult result;

    FILE_ENCODE enc = this->IsFileEncodingUtf8(szFilePath);

    std::ifstream ifs;

    char narrowPath[MAX_PATH * 4] = {0};
    wcstombs(narrowPath, szFilePath, sizeof(narrowPath) - 1);

    ifs.open(narrowPath, std::ifstream::in);

    for (int n = 0; !ifs.is_open() && n < 10; ++n)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        ifs.open(narrowPath, std::ifstream::in);
    }

    char buff[1024] = {0};

    if (ifs.is_open())
    {
        this->GetCategoryListPtr()->Clear();

        while (true)
        {
            memset(buff, 0, sizeof(buff));

            if (!ifs.getline(buff, sizeof(buff)))
                break;

            CShopCategory cat;

            const std::wstring data = this->GetDecodedString(buff, enc);

            if (cat.SetCategory(data))
            {
                this->GetCategoryListPtr()->Append(cat);
            }
        }

        ifs.close();
    }
    else
    {
        result.SetResult(PT_LOADLIBRARY, 0, L"package file open fail");
    }

    return result;
}

WZResult CShopList::LoadPackage(const wchar_t* szFilePath)
{
    WZResult result;

    FILE_ENCODE enc = this->IsFileEncodingUtf8(szFilePath);

    std::ifstream ifs;

    char narrowPath[MAX_PATH * 4] = {0};
    wcstombs(narrowPath, szFilePath, sizeof(narrowPath) - 1);

    ifs.open(narrowPath, std::ifstream::in);

    for (int n = 0; !ifs.is_open() && n < 10; ++n)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        ifs.open(narrowPath, std::ifstream::in);
    }

    char buff[1024] = {0};

    if (ifs.is_open())
    {
        this->GetPackageListPtr()->Clear();

        while (true)
        {
            if (!ifs.getline(buff, sizeof(buff)))
                break;

            CShopPackage pack;

            if (pack.SetPackage(this->GetDecodedString(buff, enc)))
            {
                this->GetPackageListPtr()->Append(pack);
                this->GetCategoryListPtr()->InsertPackage(pack.ProductDisplaySeq, pack.PackageProductSeq);
            }
        }

        ifs.close();
    }
    else
    {
        result.SetResult(4, 0, L"package file open fail");
    }

    return result;
}

WZResult CShopList::LoadProduct(const wchar_t* szFilePath)
{
    static WZResult result;

    result.BuildSuccessResult();

    FILE_ENCODE enc = this->IsFileEncodingUtf8(szFilePath);

    std::ifstream ifs;

    char narrowPath[MAX_PATH * 4] = {0};
    wcstombs(narrowPath, szFilePath, sizeof(narrowPath) - 1);

    ifs.open(narrowPath, std::ifstream::in);

    for (int n = 0; !ifs.is_open() && n < 10; ++n)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        ifs.open(narrowPath, std::ifstream::in);
    }

    char buff[1024] = {0};

    if (ifs.is_open())
    {
        this->GetProductListPtr()->Clear();

        while (true)
        {
            memset(buff, 0, sizeof(buff));

            if (!ifs.getline(buff, sizeof(buff)))
                break;

            CShopProduct product;

            std::wstring data = this->GetDecodedString(buff, enc);

            if (product.SetProduct(data))
            {
                this->GetProductListPtr()->Append(product);
            }
        }

        ifs.close();
    }
    else
    {
        result.SetResult(4, 0, L"package file open fail");
    }

    return result;
}

void CShopList::SetCategoryListPtr(CShopCategoryList* CategoryListPtr)
{
    m_CategoryListPtr = CategoryListPtr;
}

void CShopList::SetPackageListPtr(CShopPackageList* PackagePtr)
{
    m_PackageListPtr = PackagePtr;
}

void CShopList::SetProductListPtr(CShopProductList* ProductListPtr)
{
    m_ProductListPtr = ProductListPtr;
}

FILE_ENCODE CShopList::IsFileEncodingUtf8(const wchar_t* szFilePath)
{
    char narrowPath[MAX_PATH * 4] = {0};
    wcstombs(narrowPath, szFilePath, sizeof(narrowPath) - 1);

    std::ifstream ifs;

    ifs.open(narrowPath, std::ifstream::in);

    if (!ifs.is_open())
    {
        return FE_ANSI;
    }

    char buff[16] = {0};

    ifs.getline(buff, sizeof(buff));

    ifs.close();

    if (strlen(buff) < 3)
    {
        return FE_ANSI;
    }

    if (buff[0] == (char)0xEF && buff[1] == (char)0xBB && buff[2] == (char)0xBF)
    {
        return FE_UTF8;
    }

    if (buff[0] == (char)0xFF && buff[1] == (char)0xFE)
    {
        return FE_UNICODE;
    }

    return FE_ANSI;
}

std::wstring CShopList::GetDecodedString(const char* buffer, FILE_ENCODE encode)
{
    std::wstring result;

    if (encode == FE_UTF8)
    {
        // UTF-8 to wide char conversion
        int cchWideChar = MultiByteToWideChar(CP_UTF8, 0, buffer, -1, 0, 0);
        auto lpWideCharStr = new wchar_t[cchWideChar + 1];
        MultiByteToWideChar(CP_UTF8, 0, buffer, -1, lpWideCharStr, cchWideChar);

        int cbMultiByte = WideCharToMultiByte(0, 0, lpWideCharStr, -1, 0, 0, 0, 0);
        char* buff = new char[cbMultiByte + 1];
        WideCharToMultiByte(0, 0, lpWideCharStr, -1, buff, cbMultiByte, 0, 0);

        // cppcheck-suppress dangerousTypeCast
        result = (wchar_t*)buff;

        delete[] lpWideCharStr;
        delete[] buff;
    }
    else
    {
        if (encode == FE_UNICODE)
        {
            result = L"\0";
        }
        else
        {
            // cppcheck-suppress dangerousTypeCast
            result = (wchar_t*)(buffer);
        }
    }

    return result;
}
#endif
