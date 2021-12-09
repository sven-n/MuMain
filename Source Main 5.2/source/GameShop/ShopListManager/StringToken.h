
#pragma once

#include "include.h"

class CStringToken  
{
public:
	CStringToken();
	virtual ~CStringToken();
	CStringToken(const std::string& dataLine, const std::string& delim);
	
	size_t countTokens();						//토큰의 갯수 
	bool hasMoreTokens();						//토큰이 존재하는지 확인 
	std::string nextToken();					//다음 토큰 

private: 
	std::string data;
	std::string delimiter;						//데이터, 구분자 
	std::vector<std::string> tokens;			//토큰을 벡터에 저장 
	std::vector<std::string>::iterator index;	//벡터에 대한 반복자 

	void split();								//스트링을 구분자로 나눠서 벡터에 저장 
	void IsNullString(std::string::size_type pos);						//토큰에 널이 있으면 벡터에 널값 넣어주기
};
