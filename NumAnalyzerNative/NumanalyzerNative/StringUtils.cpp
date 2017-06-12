#include "stdafx.h"

using UTF8_UTF16_CVT = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>;// utf16conv;

std::wstring utf8_to_utf16(const std::string& str)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.from_bytes(str);
}

std::string utf16_to_utf8(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

bool is_white_space(int32 c)
{
	const char whitespace[] = { ' ', '\t', '\r', '\n' };

	for (auto &ws : whitespace)
	{
		if (ws == c)
			return true;
	}

	return false;
}
