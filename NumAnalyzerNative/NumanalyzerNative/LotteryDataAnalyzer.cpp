#include "stdafx.h"

#include "LotteryDataAnalyzer.h"
#include "StringUtils.h"

#include "boost/algorithm/string.hpp"


LotteryDataAnalyzer::LotteryDataAnalyzer(const std::wstring &dataPath)
	:mDataPath(dataPath)
{

}

ErrorType LotteryDataAnalyzer::ConstructData()
{
	auto result = ReadDataFromFile();
	if (result == ErrorType::ET_NoError)
	{
		for (auto &line : mLotteryData)
		{
			for (uint32 ii = 0; ii < line.lineData.size(); ++ii)
			{
				BOOST_ASSERT(ii < uint32(std::numeric_limits<uint8>::max()));

				auto &lottery = line.lineData[ii];
				lottery.idxInLine = uint8(ii);
				
			}
		}
	}
	return result;
}

ErrorType LotteryDataAnalyzer::Analyze(LotteryAnalyzeOutputData &output)
{
	if (mLotteryData.empty())
		return ErrorType::ET_AnalyzeEmptyData;

	struct BigNumChecker {
		static bool IsPositive(uint32 num) { return IsBigNum(num); }
	};

	struct OddNumChecker {
		static bool IsPositive(uint32 num) { return IsOddNum(num); }
	};

	std::tuple<CounterContainer, CounterContainer>	bigCounterContainer;
	std::tuple<CounterContainer, CounterContainer>	oddCounterContainer;

	ReciprocalCounter<BigNumChecker> bigNumCheckerCounter;	
	ReciprocalCounter<OddNumChecker> oddNumCheckerCounter;

	//{@ continue case
	for (uint32 ii = 0; ii < 10; ++ii)
	{
		for (const auto &lottery : mLotteryData)
		{
			const uint32 num = lottery.lineData[ii].num;
			if (bigNumCheckerCounter.Calc(num))
			{
				bigNumCheckerCounter.StoreInContainer(bigCounterContainer);
			}		

			if (oddNumCheckerCounter.Calc(num))
			{
				oddNumCheckerCounter.StoreInContainer(oddCounterContainer);
			}
		}
	}
	//@}

	//{@
	
	//@}
	return ErrorType::ET_NoError;
}

void LotteryDataAnalyzer::CalcOddData(LotteryAnalyzeOutputData &output)
{

}

ErrorType LotteryDataAnalyzer::ReadDataFromFile()
{
	std::ifstream iff(mDataPath.c_str());

	if (!iff)
		return ErrorType::ET_FilePathIsNotValid;

	std::string line;
	while (std::getline(iff, line))
	{
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

		if (dataParts.size() != 10)
			return ErrorType::ET_FileFormatErrorWithWrongData;

		for (uint32 ii = 0; ii < dataParts.size(); ++ii)
		{
			lottery.lineData.push_back(LotteryLineData::LotteryData(std::stoi(dataParts[ii]), ii));
		}
	}
	return ErrorType::ET_NoError;
}

bool IsOddNum(uint32 num)
{
	return num & 0x01;
}

bool IsBigNum(uint32 num)
{
	return num > 5;
}
