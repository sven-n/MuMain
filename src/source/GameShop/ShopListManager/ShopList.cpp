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

namespace
{
constexpr wchar_t ByteOrderMark = 0xFEFF;

// A UTF-16 script is read as bytes: a text-mode stream splits lines at any 0x0A byte, which is half of a UTF-16
// line break, and Windows' text mode also changes the bytes of some characters.
std::ios::openmode ScriptOpenMode(FILE_ENCODE encode)
{
    return encode == FE_UNICODE ? std::ifstream::in | std::ifstream::binary : std::ifstream::in;
}
} // namespace

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

    const auto narrowPath = mu_resolved_narrow_path(szFilePath);
    ifs.open(narrowPath, ScriptOpenMode(enc));

    DWORD LastError = GetLastError();

    for (int n = 0; !ifs.is_open() && n < 10; ++n)
    {
        Sleep(0x64);
        ifs.open(narrowPath, ScriptOpenMode(enc));
        LastError = GetLastError();
    }

    if (ifs.is_open())
    {
        this->GetCategoryListPtr()->Clear();

        int linesRead = 0;
        int rowsParsed = 0;

        std::wstring data;
        while (this->ReadLine(ifs, enc, data))
        {
            ++linesRead;

            CShopCategory cat;

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

    const auto narrowPath = mu_resolved_narrow_path(szFilePath);
    ifs.open(narrowPath, ScriptOpenMode(enc));

    DWORD LastError = GetLastError();

    for (int n = 0; !ifs.is_open() && n < 10; ++n)
    {
        Sleep(0x64);
        ifs.open(narrowPath, ScriptOpenMode(enc));
        LastError = GetLastError();
    }

    if (ifs.is_open())
    {
        this->GetPackageListPtr()->Clear();

        std::wstring data;
        while (this->ReadLine(ifs, enc, data))
        {
            CShopPackage pack;

            if (pack.SetPackage(data))
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

    const auto narrowPath = mu_resolved_narrow_path(szFilePath);
    ifs.open(narrowPath, ScriptOpenMode(enc));

    DWORD LastError = GetLastError();

    for (int n = 0; !ifs.is_open() && n < 10; ++n)
    {
        Sleep(0x64);
        ifs.open(narrowPath, ScriptOpenMode(enc));
        LastError = GetLastError();
    }

    if (ifs.is_open())
    {
        this->GetProductListPtr()->Clear();

        std::wstring data;
        while (this->ReadLine(ifs, enc, data))
        {
            CShopProduct product;

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
    // The byte order mark is read as bytes, before any text-mode translation.
    std::ifstream ifs(mu_resolved_narrow_path(szFilePath), std::ifstream::in | std::ifstream::binary);
    if (!ifs.is_open())
    {
        return FE_ANSI;
    }

    unsigned char bytes[3] = {};
    ifs.read(reinterpret_cast<char*>(bytes), sizeof(bytes));
    const std::streamsize count = ifs.gcount();

    if (count >= 3 && bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF)
    {
        return FE_UTF8;
    }

    if (count >= 2 && bytes[0] == 0xFF && bytes[1] == 0xFE)
    {
        return FE_UNICODE;
    }

    return FE_ANSI;
}

bool CShopList::ReadLine(std::istream& stream, FILE_ENCODE encode, std::wstring& line)
{
    if (encode == FE_UNICODE)
    {
        // UTF-16LE: two bytes per code unit, the line ends at the code unit 0x000A.
        std::string bytes;
        char unit[2] = {};
        while (stream.read(unit, sizeof(unit)) && !(unit[0] == '\n' && unit[1] == '\0'))
        {
            bytes.append(unit, sizeof(unit));
        }
        if (bytes.empty() && !stream)
        {
            return false;
        }
        if (bytes.size() >= 2 && bytes[bytes.size() - 2] == '\r' && bytes.back() == '\0')
        {
            bytes.resize(bytes.size() - 2);
        }
        bytes.append(2, '\0');
        line = this->GetDecodedString(bytes.data(), encode);
    }
    else
    {
        char buff[1024] = {0};
        if (!stream.getline(buff, sizeof(buff)))
        {
            return false;
        }
        line = this->GetDecodedString(buff, encode);
    }

    // The byte order mark opens the first line; it is no part of the row.
    if (!line.empty() && line.front() == ByteOrderMark)
    {
        line.erase(0, 1);
    }
    return true;
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
