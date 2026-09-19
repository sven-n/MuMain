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

#include <climits>
#include <filesystem>
#include <fstream>
#include <vector>

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

    const auto narrowPath = mu_narrow_path(szFilePath);
    ifs.open(narrowPath, std::ifstream::in);

    DWORD LastError = GetLastError();

    for (int n = 0; !ifs.is_open() && n < 10; ++n)
    {
        Sleep(0x64);
        ifs.open(narrowPath, std::ifstream::in);
        LastError = GetLastError();
    }

    char buff[1024] = { 0 };

    if (ifs.is_open())
    {
        this->GetCategoryListPtr()->Clear();

        int linesRead = 0;
        int rowsParsed = 0;

        while (true)
        {
            memset(buff, 0, sizeof(buff));

            if (!ifs.getline(buff, sizeof(buff)))
                break;

            ++linesRead;

            CShopCategory cat;

            const std::wstring data = this->GetDecodedString(buff, enc);

            if (cat.SetCategory(data))
            {
                this->GetCategoryListPtr()->Append(cat);
                ++rowsParsed;
            }
        }

        ifs.close();

        // Make a zero-row decode impossible to mistake for success.
        // If the decoder ever mangles the catalog again, rowsParsed=0 (with a
        // non-zero linesRead) will be obvious in muConsoleDebug instead of
        // silently producing an empty shop grid with no error dialog.
        g_ConsoleDebug->Write(MCD_NORMAL,
            L"[XShop] LoadCategroy: parsed %d category rows from %d lines (encode=%d) <%ls>",
            rowsParsed, linesRead, (int)enc, szFilePath);
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

    const auto narrowPath = mu_narrow_path(szFilePath);
    ifs.open(narrowPath, std::ifstream::in);

    DWORD LastError = GetLastError();

    for (int n = 0; !ifs.is_open() && n < 10; ++n)
    {
        Sleep(0x64);
        ifs.open(narrowPath, std::ifstream::in);
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

    const auto narrowPath = mu_narrow_path(szFilePath);
    ifs.open(narrowPath, std::ifstream::in);

    DWORD LastError = GetLastError();

    for (int n = 0; !ifs.is_open() && n < 10; ++n)
    {
        Sleep(0x64);
        ifs.open(narrowPath, std::ifstream::in);
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

    ifs.open(mu_narrow_path(szFilePath), std::ifstream::in);

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

    if (encode == FE_UNICODE)
    {
        const auto* bytes = reinterpret_cast<const unsigned char*>(buffer);
        for (std::size_t index = 0; bytes[index] != 0 || bytes[index + 1] != 0; index += 2)
        {
            const std::uint16_t codeUnit = static_cast<std::uint16_t>(bytes[index]) |
                (static_cast<std::uint16_t>(bytes[index + 1]) << 8);
#if WCHAR_MAX > 0xFFFF
            if (codeUnit >= 0xD800 && codeUnit <= 0xDBFF)
            {
                const std::uint16_t low = static_cast<std::uint16_t>(bytes[index + 2]) |
                    (static_cast<std::uint16_t>(bytes[index + 3]) << 8);
                if (low >= 0xDC00 && low <= 0xDFFF)
                {
                    result.push_back(static_cast<wchar_t>(0x10000 + ((codeUnit - 0xD800) << 10) + (low - 0xDC00)));
                    index += 2;
                    continue;
                }
            }
#endif
            result.push_back(static_cast<wchar_t>(codeUnit));
        }
        return result;
    }

    // FE_ANSI -> CP_ACP, FE_UTF8 -> CP_UTF8. Convert the narrow bytes to UTF-16
    // properly. The old decompiled code reinterpret-cast the narrow ASCII bytes
    // as wchar_t* ("todo: check if that's correct"), which turned a row like
    // "10@Item@200@..." into a single garbage token with no wide '@'
    // delimiters. CStringToken then yielded 1 field instead of 7, so every row
    // decoded to Root=0 -> zero category zones -> no tabs and an empty grid.
    const UINT codePage = (encode == FE_UTF8) ? CP_UTF8 : CP_ACP;

    int cchWideChar = MultiByteToWideChar(codePage, 0, buffer, -1, 0, 0);
    if (cchWideChar <= 0)
    {
        return result; // empty string on conversion failure
    }

    std::vector<wchar_t> wide(static_cast<std::size_t>(cchWideChar));
    if (MultiByteToWideChar(codePage, 0, buffer, -1, wide.data(), cchWideChar) > 0)
        result = wide.data();

    return result;
}
#endif
