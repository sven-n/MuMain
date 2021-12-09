// BadSyntax.h: interface for the CBadSyntax class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

struct BAD_SYNTAX_INFO
{
	char syntax[32];
};

class CBadSyntax
{
public:
	CBadSyntax();
	virtual ~CBadSyntax();
	void Load(char* path);
	bool CheckSyntax(char* text);
private:
	std::vector<BAD_SYNTAX_INFO> m_BadSyntaxInfo;
};

extern CBadSyntax gBadSyntax;
