#pragma once


std::string utf8_to_utf16(const std::wstring &wstr);
std::wstring utf16_to_utf8(const std::string &str);

bool is_white_space(int32 c);

template<class StrType>
void trim(StrType &str)
{
	size_t leftIdx = 0;
	size_t rightIdx = str.length() - 1;
	for (auto &c : str)
	{
		if (!is_white_space(c))
			break;

		++leftIdx;
	}

	for (auto it = std::rbegin(str); it != std::rend(str); ++it)
	{
		if (!is_white_space(*it))
			break;

		--rightIdx;		
	}

	str = str.substr(leftIdx, rightIdx);
}

