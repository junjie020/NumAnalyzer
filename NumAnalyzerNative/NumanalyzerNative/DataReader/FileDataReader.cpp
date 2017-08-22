#include "stdafx.h"
#include "FileDataReader.h"
#include "StringUtils.h"
#include "boost/algorithm/string.hpp"

FileDataReader::FileDataReader(const std::wstring &path)
	:mPath(path)
{

}

std::wregex build_num_reg_obj()
{
	std::wstring regMatchString(L"\t?");

	for (uint32 ii = 0; ii < 9; ++ii)
		regMatchString += L"\\d+\\s*,?";

	regMatchString += L"\\d+";

	return std::wregex(regMatchString.c_str(), std::regex_constants::icase);
}

ErrorType convert_string_data_num_to_lottery(const std::wstring &dataNum, LotteryLineData &lottery)
{
	std::vector<std::string> dataParts;
	boost::split(dataParts, dataNum, boost::is_any_of(", "));
	if (dataParts.size() != 10)
		return ErrorType::ET_FileFormatErrorWithWrongData;

	lottery.data.resize(dataParts.size());
	lottery.indices.resize(dataParts.size());

	for (uint32 ii = 0; ii < dataParts.size(); ++ii)
	{
		uint8 num = uint8(std::stoi(dataParts[ii]));

		lottery.data[ii].num = num;
		lottery.data[ii].idxInLine = ii;

		BOOST_ASSERT(1 <= num && num <= 10);
		lottery.indices[num - 1] = ii;
	}

	return ErrorType::ET_NoError;
}

ErrorType FileDataReader::ConstructData(LotteryLineDataArray &lotterys)
{
	std::ifstream iff(mPath.c_str());

	if (!iff)
		return ErrorType::ET_FilePathIsNotValid;

	std::wregex regDataNum = std::move(build_num_reg_obj());

	std::string line;
	while (std::getline(iff, line))
	{
		/*
		trim(line);
		const char tab = '\t';

		std::vector<std::string> parts;
		boost::split(parts, line, boost::is_any_of("\t"));

		if (parts.size() != 3)
		return ErrorType::ET_FileFormatError;

		mLotteryData.push_back(LotteryLineData());
		LotteryLineData &lottery = mLotteryData.back();
		lottery.lotteryNum = std::stoi(parts[0]);

		lottery.date = parts[2];

		std::vector<std::string> dataParts;
		boost::split(dataParts, parts[1], boost::is_any_of(","));
		*/

		auto wLine = StringUtils::utf8_to_utf16(line);
		line = StringUtils::utf16_to_utf8(wLine);

		std::wsmatch matchResult;
		if (std::regex_search(wLine, matchResult, regDataNum))
		{
			std::wstring strData = *matchResult.begin();
			StringUtils::trim(strData);

			lotterys.push_back(LotteryLineData());

			auto result = convert_string_data_num_to_lottery(strData, lotterys.back());
			if (ErrorType::ET_NoError != result)
				return result;
		}


	}	
	return ErrorType::ET_NoError;
}
