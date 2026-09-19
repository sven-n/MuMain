#pragma once

#include <string>

namespace GameConfigValidation
{
int ValidateServerPort(int value, int defaultPort);
std::wstring ValidateServerIP(const std::wstring& value, const std::wstring& defaultIP);
}
