#pragma once

#include "Include.h"

class CStringToken
{
public:
    CStringToken();
    virtual ~CStringToken();
    CStringToken(const std::wstring& dataLine, const std::wstring& delim);

    size_t countTokens();     // ��ū�� ����
    bool hasMoreTokens();     // ��ū�� �����ϴ��� Ȯ��
    std::wstring nextToken(); // ���� ��ū

private:
    std::wstring data;
    std::wstring delimiter;                    // ������, ������
    std::vector<std::wstring> tokens;          // ��ū�� ���Ϳ� ����
    std::vector<std::wstring>::iterator index; // ���Ϳ� ���� �ݺ���

    void split();                                   // ��Ʈ���� �����ڷ� ������ ���Ϳ� ����
    void IsNullString(std::wstring::size_type pos); // ��ū�� ���� ������ ���Ϳ� �ΰ� �־��ֱ�
};
