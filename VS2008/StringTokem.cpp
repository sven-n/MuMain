#include "stdafx.h"
#include "StringToken.h"

CStringToken::CStringToken()
{

}

CStringToken::~CStringToken()
{

}

CStringToken::CStringToken(const std::string& _str, const std::string& _delim)
{
	if ((_str.length() == 0) || (_delim.length() == 0)) return;

	data		= _str;
	delimiter	= _delim;

	/* Remove sequential delimiter */
	std::size_t curr_pos = 0;

	while (true)
	{
		if ((curr_pos = data.find(delimiter, curr_pos)) != std::string::npos)
		{
			curr_pos += delimiter.length();

			while (data.find(delimiter, curr_pos) == curr_pos)
			{
				data.erase(curr_pos, delimiter.length());
			}
		}
		else
			break;
	}

	/* Trim leading delimiter */
	if (data.find(delimiter, 0) == 0)
	{
		data.erase(0, delimiter.length());
	}

	/* Trim ending delimiter */
	if ((curr_pos = data.rfind(delimiter)) != std::string::npos)
	{
		if (curr_pos != (data.length() - delimiter.length())) return;
		data.erase(data.length() - delimiter.length(), delimiter.length());
	}
}

size_t CStringToken::countTokens()
{
	return this->tokens.begin() - this->tokens.end();
}

bool CStringToken::hasMoreTokens()
{
	return (data.length() > 0);
}

std::string CStringToken::nextToken() 
{
	if (data.length() == 0)
		return "";

	std::string tmp_str = "";
	std::size_t pos = data.find(delimiter, 0);

	if (pos != std::string::npos)
	{
		tmp_str = data.substr(0, pos);
		data = data.substr(pos + delimiter.length(), data.length() - pos);
	}
	else
	{
		tmp_str = data.substr(0, data.length());
		data = "";
	}
	return tmp_str;
}