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

ErrorType LotteryDataAnalyzer::Analyze(std::string &outputInfo)
{
	if (mLotteryData.empty())
		return ErrorType::ET_AnalyzeEmptyData;

	//ContinueAnalyzer ca;
	//AnalyzeContinueData(ca);

	//StepAnalyzer sa;
	//AnalyzeStepData(sa);


	//NumanalyerArray analyzers = { &ca, &sa };
	//FormatOutput(std::move(analyzers), outputInfo);


	mFilter.Filter(mLotteryData);

	AnalyzeResultAll results;

	BigSmallAnalyzer bs;
	bs.Analyze(mLotteryData, mFilter, results.bigSmall);

	OddEvenAnalyzer oe;
	oe.Analyze(mLotteryData, mFilter, results.oddEven);


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
	ReciprocalCounter<BigSmallNumChecker> &bigNumChecker, 
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

//void LotteryDataAnalyzer::AnalyzeContinueData(ContinueAnalyzer &analyer)
//{
//	analyer.Analyze(mLotteryData);
//}
//
//void LotteryDataAnalyzer::AnalyzeStepData(StepAnalyzer &analyer)
//{
//	analyer.Analyze(mLotteryData);
//}

//#ifdef _DEBUG
//static void check_valid(const CounterContainerArray &arr, uint32 totallNum)
//{
//	const CounterContainer &firstNum = std::get<0>(arr);
//	const CounterContainer &secondNum = std::get<1>(arr);
//
//	uint32 counter = 0;
//	for (auto n : firstNum)
//	{
//		counter += n;
//	}
//
//	for (auto n : secondNum)
//	{
//		counter += n;
//	}
//
//	BOOST_ASSERT(counter == totallNum);
//
//	
//
//	auto maxNum = std::max(firstNum.size(), secondNum.size());
//	auto minNum = std::min(firstNum.size(), secondNum.size());
//	auto delta = maxNum - minNum;
//	BOOST_ASSERT(delta == 0 || delta == 1);
//}
//#endif // _DEBUG
//
//void LotteryDataAnalyzer::FormatOutput(NumanalyerArray &&analyzers, std::string &outputBuffer)
//{
//	std::ostringstream oss;
//
//	for (auto analyzer : analyzers)
//	{
//		const CounterContainerArray& bigCC = analyzer->GetBigCounterArray();
//
//#ifdef _DEBUG
//		check_valid(bigCC, mLotteryData.size() * 10);
//#endif // _DEBUG
//
//		oss << "Name : " << analyzer->GetName() << "\n";
//		oss << "Big : ";
//
//		auto bigC = std::get<0>(bigCC);
//		for (auto n : bigC)
//		{
//			oss << n << ",";
//		}
//
//		oss << "\n";
//		oss << "Small : ";
//
//		auto smallC = std::get<1>(bigCC);
//		for (auto n : smallC)
//		{
//			oss << n << ",";
//		}
//
//		oss << "\n";
//			
//
//		const CounterContainerArray& oddCC = analyzer->GetOddCounterArray();
//
//#ifdef _DEBUG
//		check_valid(oddCC, mLotteryData.size() * 10);
//#endif // _DEBUG
//
//		oss << "Odd : ";
//
//		auto oddC = std::get<0>(oddCC);
//		for (auto n : oddC)
//		{
//			oss << n << ",";
//		}
//
//		oss << "\n";
//		oss << "Even : ";
//
//		auto evenC = std::get<1>(oddCC);
//
//		for (auto n : evenC)
//		{
//			oss << n << ",";
//		}
//
//		oss << "\n";
//	}
//
//	outputBuffer = oss.str();
//
//	OutputDebugStringA(outputBuffer.c_str());
//}

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



//void ContinueAnalyzer::Analyze(const LotteryLineDataArray &lotteryDataArray)
//{
//	//{@ continue case
//	for (uint32 ii = 0; ii < DATA_COLUMN_NUM; ++ii)
//	{
//		for (const auto &lotteryLine : lotteryDataArray)
//		{
//			analyze_num(lotteryLine.lineData[ii].num, mBigNumChecker, mOddNumChecker, mBigCounterContainer[ii], mOddCounterContainer[ii]);
//		}
//	}
//
//	auto bigPair = mBigNumChecker.GetCounter();
//	store_in_container(bigPair, mBigCounterContainer);
//
//	auto oddPair = mOddNumChecker.GetCounter();
//	store_in_container(oddPair, mOddCounterContainer);
//	//@}
//}
//
//void StepAnalyzer::Analyze(const LotteryLineDataArray &lotteryDataArray)
//{
//	for (uint32 ii = 0; ii < 10; ++ii)
//	{
//		Analyze(lotteryDataArray, ii, 0);
//		Analyze(lotteryDataArray, ii, 1);
//	}
//
//	auto bigPair = mBigNumChecker.GetCounter();
//	store_in_container(bigPair, mBigCounterContainer);
//
//	auto oddPair = mOddNumChecker.GetCounter();
//	store_in_container(oddPair, mOddCounterContainer);
//}
//
//void StepAnalyzer::Analyze(const LotteryLineDataArray &lotteryDataArray, uint32 colIdx, uint32 begIdx)
//{
//	for (uint32 iStep = 0; iStep < lotteryDataArray.size(); iStep += 2)
//	{
//		analyze_num(lotteryDataArray[iStep].lineData[colIdx].num, mBigNumChecker, mOddNumChecker, mBigCounterContainer, mOddCounterContainer);
//	}
//}

void DataFilter::Filter(const LotteryLineDataArray &lotteryDataArray)
{
	for (uint32 iCol = 0; iCol < DATA_COLUMN_NUM; ++iCol)
	{
		auto &counter = mCounters[iCol];
		for (uint32 iRow = 0; iRow < lotteryDataArray.size(); ++iRow)
		{
			analyze_num(lotteryDataArray[iRow].lineData[iCol].num, counter.bigNumChecker, counter.oddNumChecker, counter.bigCounterContainer, counter.oddCounterContainer);
		}

		auto bigPair = counter.bigNumChecker.GetCounter();
		store_in_container(bigPair, counter.bigCounterContainer);
		
		auto oddPair = counter.oddNumChecker.GetCounter();
		store_in_container(oddPair, counter.oddCounterContainer);		
	}
}

void IDataAnalyzer::Analyze(const LotteryLineDataArray &lotteryLines, const ColumnContainers &containers, 
	const std::tuple<AnalyzeResult::ResultCounter::NumType, AnalyzeResult::ResultCounter::NumType> &types, 
	AnalyzeResult &result)
{
	BOOST_ASSERT(!std::get<0>(*containers[0]).empty() || !std::get<1>(*containers[0]).empty());
	BOOST_ASSERT(containers.size() == result.continueRecords.size() && result.continueRecords.size() == result.stepRecords.size());

	for (uint32 ii = 0; ii < containers.size(); ++ii)
	{
		auto &counter = *containers[ii];

		auto &container0 = std::get<0>(counter);
		auto &container1 = std::get<1>(counter);
		const uint32 minCount = std::min(container0.size(), container1.size());

		//{@	merge in one vector
		CounterContainer fullContainer;
		fullContainer.reserve(container0.size() + container1.size());

		for (uint32 jj = 0; jj < minCount; ++jj)
		{
			fullContainer.push_back(container0[jj]);
			fullContainer.push_back(container1[jj]);
		}

		if (container0.size() > container1.size())
		{
			fullContainer.push_back(container0.back());
		}
		else if (container1.size() > container0.size())
		{
			fullContainer.push_back(container1.back());
		}
		//@}

		CounterContainer::iterator itFull = std::begin(fullContainer);
		while (itFull != std::end(fullContainer))
		{
			auto dis = std::distance(std::begin(fullContainer), itFull);
			
			const uint32 value = *itFull;
			if (value != 1)
			{
				const AnalyzeResult::ResultCounter::NumType type = (dis % 2 == 0) ? std::get<0>(types) : std::get<1>(types);
				AnalyzeResult::ResultCounterMap &rsm = result.continueRecords[ii];

				auto &resultValue = rsm[value];

				resultValue.info.push_back(AnalyzeResult::ResultCounter::OriginInfo());
				resultValue.info.back().type = type;
				resultValue.numCounter = value;

				uint32 index = 0;
				std::for_each(fullContainer.begin(), itFull, [&index](uint32 num) {index += num; });

				BOOST_ASSERT(index + *itFull < lotteryLines.size());

				for (uint32 iOrg = 0; iOrg < *itFull; ++iOrg)
				{
					resultValue.info.back().numbers.push_back(lotteryLines[index + iOrg].lineData[ii].num);
				}

				++itFull;
			}
			else
			{
				AnalyzeResult::ResultCounterMap &rsm = result.stepRecords[ii];
				auto itFound = std::find_if_not(itFull, std::end(fullContainer), [](uint32 vv) { return vv == 1; });				
				BOOST_ASSERT(itFull != itFound);

				uint32 numCounter = uint32(std::distance(itFull, itFound));

				std::vector<std::vector<uint32>::const_iterator>	iterContainers;

				uint32 extraNum = 0;

				if (itFull != std::begin(fullContainer))
				{
					++extraNum;

					auto itTemp = itFull;
					--itTemp;
					iterContainers.push_back(itTemp);

					BOOST_ASSERT(*itTemp > 1);
				}

				if (itFound != std::end(fullContainer))
				{
					++extraNum;
					iterContainers.push_back(itFound);

					BOOST_ASSERT(*itFound > 1);
				}
	
				for (auto it = itFull; it != itFound; ++it)
				{
					iterContainers.push_back(it);
				}
			
				// the calculate method is : the number of 1 in the array
				// and the begin and end count, but the step is need two number to generate, so we need to minus 1.
				// ex : 9, 8, 6, 1, 7, 1, 2 ==> 3, 1, 1, 2 ==> 3,1 has one step, 1, 1 has one step, 1, 2 has one step, 4 number(6, 1, 7, 1) but 3 steps.
				const uint32 finalNumCounter = numCounter + extraNum - 1;	
				auto &resultValue = rsm[finalNumCounter];

				resultValue.numCounter = finalNumCounter;
				resultValue.info.back().type = AnalyzeResult::ResultCounter::Unknown;
				resultValue.info.push_back(AnalyzeResult::ResultCounter::OriginInfo());

				for (auto it : iterContainers)
				{
					resultValue.info.back().numbers.push_back(*it);
				}

				itFull = itFound;
			}
		}

	}
}

void BigSmallAnalyzer::Analyze(const LotteryLineDataArray &lotteryDataArray, const DataFilter &filter, AnalyzeResult &result)
{
	ColumnContainers containers;

	for (uint32 ii = 0; ii < containers.size(); ++ii)
	{
		containers[ii] = &(filter.mCounters[ii].bigCounterContainer);
	}

	IDataAnalyzer::Analyze(lotteryDataArray, containers, std::make_tuple(AnalyzeResult::ResultCounter::Big, AnalyzeResult::ResultCounter::Small), result);
}

void OddEvenAnalyzer::Analyze(const LotteryLineDataArray &lotteryDataArray, const DataFilter &filter, AnalyzeResult &result)
{
	ColumnContainers containers;

	for (uint32 ii = 0; ii < containers.size(); ++ii)
	{
		containers[ii] = &(filter.mCounters[ii].oddCounterContainer);
	}

	IDataAnalyzer::Analyze(lotteryDataArray, containers, std::make_tuple(AnalyzeResult::ResultCounter::Odd, AnalyzeResult::ResultCounter::Even), result);
}
