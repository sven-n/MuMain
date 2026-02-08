//************************************************************************
//
// Decompiled by @myheart, @synth3r
// <https://forum.ragezone.com/members/2000236254.html>
//
//
// FILE: ShopList.cpp
//
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "ShopList.h"

#include <fstream>

CShopList::CShopList() // OK
{
    this->m_CategoryListPtr = new CShopCategoryList;
    this->m_PackageListPtr = new CShopPackageList;
    this->m_ProductListPtr = new CShopProductList;
}

CShopList::~CShopList() // OK
{
    SAFE_DELETE(m_CategoryListPtr);
    SAFE_DELETE(m_PackageListPtr);
    SAFE_DELETE(m_ProductListPtr);
}

WZResult CShopList::LoadCategroy(const wchar_t* szFilePath) // OK
{
    WZResult result;

    FILE_ENCODE enc = this->IsFileEncodingUtf8(szFilePath);

    std::ifstream ifs;

    ifs.open(szFilePath, std::ifstream::in);

    DWORD LastError = GetLastError();

    for (int n = 0; !ifs.is_open() && n < 10; ++n)
    {
        Sleep(0x64);
        ifs.open(szFilePath, std::ifstream::in);
        LastError = GetLastError();
    }

    char buff[1024] = { 0 };

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
        result.SetResult(PT_LOADLIBRARY, LastError, L"package file open fail");
    }

    return result;
}

WZResult CShopList::LoadPackage(const wchar_t* szFilePath) // OK
{
    WZResult result;

    FILE_ENCODE enc = this->IsFileEncodingUtf8(szFilePath);

    std::ifstream ifs;

    ifs.open(szFilePath, std::ifstream::in);

    DWORD LastError = GetLastError();

    for (int n = 0; !ifs.is_open() && n < 10; ++n)
    {
        Sleep(0x64);
        ifs.open(szFilePath, std::ifstream::in);
        LastError = GetLastError();
    }

    char buff[1024] = { 0 };

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
        result.SetResult(4, LastError, L"package file open fail");
    }

    return result;
}

WZResult CShopList::LoadProduct(const wchar_t* szFilePath) // OK
{
    static WZResult result;

    result.BuildSuccessResult();

    FILE_ENCODE enc = this->IsFileEncodingUtf8(szFilePath);

    std::ifstream ifs;

    ifs.open(szFilePath, std::ifstream::in);

    DWORD LastError = GetLastError();

    for (int n = 0; !ifs.is_open() && n < 10; ++n)
    {
        Sleep(0x64);
        ifs.open(szFilePath, std::ifstream::in);
        LastError = GetLastError();
    }

    char buff[1024] = { 0 };

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
        result.SetResult(4, LastError, L"package file open fail");
    }

    return result;
}

void CShopList::SetCategoryListPtr(CShopCategoryList* CategoryListPtr) // OK
{
    m_CategoryListPtr = CategoryListPtr;
}

void CShopList::SetPackageListPtr(CShopPackageList* PackagePtr) // OK
{
    m_PackageListPtr = PackagePtr;
}

void CShopList::SetProductListPtr(CShopProductList* ProductListPtr) // OK
{
    m_ProductListPtr = ProductListPtr;
}

FILE_ENCODE CShopList::IsFileEncodingUtf8(const wchar_t* szFilePath) // OK
{
    std::ifstream ifs;

    ifs.open(szFilePath, std::ifstream::in);

    if (!ifs.is_open())
    {
        return FE_ANSI;
    }

    char buff[16] = { 0 };

    ifs.getline(buff, sizeof(buff));

    ifs.close();

    if (strlen(buff) < 3)
    {
        return FE_ANSI;
    }

    if (buff[0] == 0xEF && buff[1] == 0xBB && buff[2] == 0xBF)
    {
        return FE_UTF8;
    }

    if (buff[0] == 0xFF && buff[1] == 0xFE)
    {
        return FE_UNICODE;
    }

    return FE_ANSI;
}

std::wstring CShopList::GetDecodedString(const char* buffer, FILE_ENCODE encode) // OK
{
    std::wstring result;

    if (encode == FE_UTF8)
    {
        int cchWideChar = MultiByteToWideChar(CP_UTF8, 0, buffer, -1, 0, 0);
        auto lpWideCharStr = new WCHAR[cchWideChar + 1];
        MultiByteToWideChar(CP_UTF8, 0, buffer, -1, lpWideCharStr, cchWideChar);

        cchWideChar = WideCharToMultiByte(0, 0, lpWideCharStr, -1, 0, 0, 0, 0);
        char* buff = new char[cchWideChar + 1];
        WideCharToMultiByte(0, 0, lpWideCharStr, -1, buff, cchWideChar, 0, 0);

        // todo: check if that's correct
        result = (wchar_t*) buff;

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
            // todo: check if that's correct
            result = (wchar_t*)(buffer);
        }
    }

    return result;
}
#endif