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

	ContinueNumAnalyer ca;
	AnalyzeContinueData(ca);

	StepNumAnalyer sa;
	AnalyzeStepData(sa);


	NumanalyerArray analyzers = { &ca, &sa };

	std::string outputInfo;
	FormatOutput(std::move(analyzers), outputInfo);

	return ErrorType::ET_NoError;
}

static void store_in_container(const ReciprocalCounterPair &pair, CounterContainerArray &container)
{
	auto first = std::get<0>(pair);
	auto second = std::get<1>(pair);

	if (first != 0)
		std::get<0>(container).push_back(first);

	if (second != 0)
		std::get<1>(container).push_back(second);
}

static void analyze_num(uint32 num, 
	ReciprocalCounter<BigNumChecker> &bigNumChecker, 
	ReciprocalCounter<OddNumChecker> &oddNumChecker, 
	CounterContainerArray &bigCounterContainer, 
	CounterContainerArray &oddCounterContainer)
{
	ReciprocalCounterPair pairBig;
	if (bigNumChecker.Calc(num, pairBig))
	{
		store_in_container(pairBig, bigCounterContainer);
	}

	ReciprocalCounterPair pairOdd;
	if (oddNumChecker.Calc(num, pairOdd))
	{
		store_in_container(pairOdd, oddCounterContainer);
	}
}

void LotteryDataAnalyzer::AnalyzeContinueData(ContinueNumAnalyer &analyer)
{
	analyer.Analyze(mLotteryData);
}

void LotteryDataAnalyzer::AnalyzeStepData(StepNumAnalyer &analyer)
{
	analyer.Analyze(mLotteryData);
}

#ifdef _DEBUG
static void check_valid(const CounterContainerArray &arr, uint32 totallNum)
{
	const CounterContainer &firstNum = std::get<0>(arr);
	const CounterContainer &secondNum = std::get<1>(arr);

	uint32 counter = 0;
	for (auto n : firstNum)
	{
		counter += n;
	}

	for (auto n : secondNum)
	{
		counter += n;
	}

	BOOST_ASSERT(counter == totallNum);

	auto maxNum = std::max(firstNum.size(), secondNum.size());
	auto minNum = std::min(firstNum.size(), secondNum.size());
	auto delta = maxNum - minNum;
	BOOST_ASSERT(delta == 0 || delta == 1);
}
#endif // _DEBUG

void LotteryDataAnalyzer::FormatOutput(NumanalyerArray &&analyzers, std::string &outputBuffer)
{
	std::ostringstream oss;

	for (auto analyzer : analyzers)
	{
		const CounterContainerArray& bigCC = analyzer->GetBigCounterArray();

#ifdef _DEBUG
		check_valid(bigCC, mLotteryData.size());
#endif // _DEBUG

		oss << "Name : " << analyzer->GetName() << "\n";
		oss << "Big : ";

		auto bigC = std::get<0>(bigCC);
		for (auto n : bigC)
		{
			oss << n << ",";
		}

		oss << "\n";

		auto smallC = std::get<1>(bigCC);
		for (auto n : smallC)
		{
			oss << n << ",";
		}

		oss << "\n";
			

		const CounterContainerArray& oddCC = analyzer->GetOddCounterArray();

#ifdef _DEBUG
		check_valid(oddCC, mLotteryData.size());
#endif // _DEBUG

		oss << "Odd : ";

		auto oddC = std::get<0>(oddCC);
		for (auto n : oddC)
		{
			oss << n << ",";
		}

		oss << "\n";

		auto evenC = std::get<1>(oddCC);

		for (auto n : evenC)
		{
			oss << n << ",";
		}

		oss << "\n";
	}
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

void ContinueNumAnalyer::Analyze(const LotteryLineDataArray &lotteryDataArray)
{
	//{@ continue case
	for (uint32 ii = 0; ii < 10; ++ii)
	{
		for (const auto &lotteryLine : lotteryDataArray)
		{
			analyze_num(lotteryLine.lineData[ii].num, mBigNumChecker, mOddNumChecker, mBigCounterContainer, mOddCounterContainer);
		}
	}
	//@}
}

void StepNumAnalyer::Analyze(const LotteryLineDataArray &lotteryDataArray)
{
	for (uint32 ii = 0; ii < 10; ++ii)
	{
		Analyze(lotteryDataArray, ii, 0);
		Analyze(lotteryDataArray, ii, 1);
	}
}

void StepNumAnalyer::Analyze(const LotteryLineDataArray &lotteryDataArray, uint32 colIdx, uint32 begIdx)
{
	for (uint32 iStep = 0; iStep < lotteryDataArray.size(); iStep += 2)
	{
		analyze_num(lotteryDataArray[iStep].lineData[colIdx].num, mBigNumChecker, mOddNumChecker, mBigCounterContainer, mOddCounterContainer);
	}
}
