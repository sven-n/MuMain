//************************************************************************
//
// Decompiled by @myheart, @synth3r
// <https://forum.ragezone.com/members/2000236254.html>
//
//
// FILE: StringToken.cpp
//
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "StringToken.h"

CStringToken::CStringToken() // OK
{
    this->data.clear();
    this->delimiter.clear();
    this->tokens.clear();
    //this->index = this->tokens.begin();
}

CStringToken::~CStringToken() // OK
{
}

CStringToken::CStringToken(const std::wstring& dataLine, const std::wstring& delim) // OK
{
    this->data = dataLine;
    this->delimiter = delim;
    this->tokens.clear();
    this->index = this->tokens.begin();
    this->split();
}

size_t CStringToken::countTokens() // OK
{
    return this->tokens.size();
}

bool CStringToken::hasMoreTokens()
{
    return this->index != this->tokens.end();
}

std::wstring CStringToken::nextToken() // OK
{
    std::wstring result;

    if (this->index == this->tokens.end())
    {
        result.assign(L"\0");
    }
    else
    {
        result.assign((*this->index));

        this->index++;
    }

    return result;
}

void CStringToken::split() // OK
{
    std::size_t first_not = this->data.find_first_not_of(this->delimiter, 0);
    std::size_t first = this->data.find_first_of(this->delimiter, first_not);

    while (first != std::wstring::npos || first_not != std::wstring::npos)
    {
        std::wstring subdata = this->data.substr(first_not, first - first_not);

        this->tokens.push_back(subdata);

        this->IsNullString(first);

        first_not = this->data.find_first_not_of(this->delimiter, first);
        first = this->data.find_first_of(this->delimiter, first_not);
    }

    this->index = this->tokens.begin();
}

void CStringToken::IsNullString(std::wstring::size_type pos) // OK
{
    std::wstring search = this->data.substr(pos + 1, this->delimiter.length());

    if (search == this->delimiter)
    {
        this->tokens.push_back(L"\0");

        this->IsNullString(pos + 1);
    }
}
#endif