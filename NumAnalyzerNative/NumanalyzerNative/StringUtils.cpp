#include "stdafx.h"

using UTF8_UTF16_CVT = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>;// utf16conv;

//std::string utf8_to_utf16(const std::wstring &wstr)
//{
//	UTF8_UTF16_CVT cvt;
//	return cvt.to_bytes(wstr);
//}
//
//std::wstring utf16_to_utf8(const std::string &str)
//{
//	UTF8_UTF16_CVT cvt;
//	return cvt.from_bytes(str);
//}

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
