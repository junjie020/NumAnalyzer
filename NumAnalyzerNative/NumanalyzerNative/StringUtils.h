#pragma once


std::wstring utf8_to_utf16(const std::string &wstr);
std::string utf16_to_utf8(const std::wstring &str);

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

	if (leftIdx == 0 && rightIdx == str.length() - 1)
		return;

	str = str.substr(leftIdx, rightIdx);
}

template<typename CharType>
bool is_empty_c_str(const CharType *ch)
{
	return ch == nullptr || *ch == '\0';
}

