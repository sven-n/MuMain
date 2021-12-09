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

CStringToken::CStringToken(const std::string& dataLine,const std::string& delim) // OK
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
	return this->index!=this->tokens.end();
}

std::string CStringToken::nextToken() // OK
{
	std::string result;

	if(this->index==this->tokens.end())
	{
		result.assign("\0");
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
	std::size_t first_not = this->data.find_first_not_of(this->delimiter,0);
	std::size_t first = this->data.find_first_of(this->delimiter,first_not);

	while(first!=std::string::npos||first_not!=std::string::npos)
	{
		std::string subdata = this->data.substr(first_not,first-first_not);

		this->tokens.push_back(subdata);

		this->IsNullString(first);

		first_not = this->data.find_first_not_of(this->delimiter,first);
		first = this->data.find_first_of(this->delimiter,first_not);
	}

	this->index = this->tokens.begin();
}

void CStringToken::IsNullString(std::string::size_type pos) // OK
{
	std::string search = this->data.substr(pos+1,this->delimiter.length());

	if(search==this->delimiter)
	{
		this->tokens.push_back("\0");

		this->IsNullString(pos+1);
	}
}
#endif