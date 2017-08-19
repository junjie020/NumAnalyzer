#include "stdafx.h"

#include "LotteryDataAnalyzer.h"
#include "StringUtils.h"
#include "boost/algorithm/string.hpp"

#include "rapidjson/include/rapidjson/document.h"
#include "rapidjson/include/rapidjson/stringbuffer.h"
#include "rapidjson/include/rapidjson/writer.h"
#include "rapidjson/include/rapidjson/prettywriter.h"

#include "curl/include/curl/curl.h"

LotteryDataAnalyzer::LotteryDataAnalyzer(const std::wstring &dataPath)
	:mDataPath(dataPath)
{

}

ErrorType LotteryDataAnalyzer::ConstructData(bool isURL)
{
	if (isURL)
		return ReadDataFromURL();

	return ReadDataFromFile();
}

ErrorType LotteryDataAnalyzer::Analyze(std::string &outputInfo)
{
	if (mLotteryData.empty())
		return ErrorType::ET_AnalyzeEmptyData;

	mFilter.Filter(mLotteryData);

	AnalyzeResultAll results;

	BigSmallAnalyzer bs;
	bs.Analyze(mLotteryData, mFilter, results.bigSmall);

	OddEvenAnalyzer oe;
	oe.Analyze(mLotteryData, mFilter, results.oddEven);

	NumberBigSmallAnalyzer nbs;
	nbs.Analyze(mLotteryData, mFilter, results.numBigSmall);

	NumberOddEvenAnalyzer noe;
	noe.Analyze(mLotteryData, mFilter, results.numOddEven);

	FormatOutput(results, outputInfo);

	return ErrorType::ET_NoError;
}

static void store_in_container(const ReciprocalCounterPair &pair, CounterContainerPair &container)
{
	auto first = std::get<0>(pair);
	auto second = std::get<1>(pair);

	const bool isNotInit = container.cc0.empty() && container.cc1.empty();
	
	if (first != 0)
	{
		if (isNotInit)
			container.orderIdx = 0;

		container.cc0.push_back(first);
	}
		

	if (second != 0)
	{
		if (isNotInit)
			container.orderIdx = 1;

		container.cc1.push_back(second);
	}
		
}

static void analyze_num(uint32 num, 
	ReciprocalCounter<BigSmallNumChecker> &bigNumChecker, 
	ReciprocalCounter<OddNumChecker> &oddNumChecker, 
	CounterContainerPair &bigCounterContainer, 
	CounterContainerPair &oddCounterContainer)
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

static const char* get_column_name(uint32 iCol)
{
	const char* names[] = { "Col0", "Col1", "Col2", "Col3", "Col4", "Col5" , "Col6", "Col7" , "Col8", "Col9" };

	BOOST_ASSERT(iCol < _countof(names));

	return names[iCol];
}

static const char* get_data_type_name(NumType type)
{
	switch (type)
	{
	case AnalyzeResult::ResultCounter::Big:
		return "Big";
	case AnalyzeResult::ResultCounter::Small:
		return "Small";
	case AnalyzeResult::ResultCounter::Odd:
		return "Odd";
	case AnalyzeResult::ResultCounter::Even:
		return "Even";
	case AnalyzeResult::ResultCounter::Unknown:
	default:
		return "Unknown";
	}
}

static void format_record(const AnalyzeResult::ColumnRecords &records, rapidjson::Document &doc, const char* analyzeName, const char* recordTypename)
{
	using namespace rapidjson;
	Value::MemberIterator recordMem = doc[analyzeName].FindMember(recordTypename);
	BOOST_ASSERT(recordMem != doc[analyzeName].MemberEnd());

	const char* colName = "Column";
	BOOST_ASSERT(recordMem->value.FindMember(colName) == recordMem->value.MemberEnd());

	auto &docAllocator = doc.GetAllocator();

	Value colArrayValue;
	colArrayValue.SetArray();

	for (uint32 ii = 0; ii < records.size(); ++ii)
	{
		Value resuletsArrayValue;
		resuletsArrayValue.SetArray();

		const AnalyzeResult::ResultCounterMap &colResults = records[ii];
		for (const auto &cr : colResults)
		{
			Value resultValueObj;
			resultValueObj.SetObject();

			Value counterValue;
			counterValue.SetUint(cr.second.numCounter);
			resultValueObj.AddMember("Counter", counterValue, docAllocator);
			
			Value originInfoValue;
			originInfoValue.SetArray();
			
			for (auto &info : cr.second.info)
			{
				Value infoValue;
				infoValue.SetObject();

				Value dataType;
				dataType.SetString(get_data_type_name(info.type), docAllocator);
				infoValue.AddMember("DataType", dataType, docAllocator);
				
				Value OriginNumArrayValue;
				OriginNumArrayValue.SetArray();
				
				for (auto &num : info.numbers)
				{	
					OriginNumArrayValue.PushBack(num, docAllocator);
				}

				infoValue.AddMember("OriginNum", OriginNumArrayValue, docAllocator);

				originInfoValue.PushBack(infoValue, docAllocator);
			}
			resultValueObj.AddMember("OriginInfo", originInfoValue, docAllocator);

			resuletsArrayValue.PushBack(resultValueObj, docAllocator);
		}

		colArrayValue.PushBack(resuletsArrayValue, docAllocator);
	}

	recordMem->value.AddMember(StringRef(colName), colArrayValue, docAllocator);

}

static void format_result(const AnalyzeResult &result, rapidjson::Document &doc, const char* analyzeName)
{
	const char* ContinueName = "Continue";
	const char* StepName = "Step";

	BOOST_ASSERT(doc.FindMember(analyzeName) != doc.MemberEnd());

	format_record(result.continueRecords, doc, analyzeName, ContinueName);
	format_record(result.stepRecords, doc, analyzeName, StepName);
}

static std::string find_json_template_folder()
{
	std::wstring buffer;
	buffer.resize(1024);
	::_wgetcwd(&*buffer.begin(), buffer.size());

	return utf16_to_utf8(get_parent_path(buffer));
}

static std::string read_json_template_content()
{
	//const std::string jsonTemplatePath = "G:/github/NumAnalyzer/NumAnalyzerNative/NumAnalyzerConsoleTest/jsontest1.json";//join_path(find_json_template_folder(), "Resources\\JSONTemplate.json");
	const std::string jsonTemplatePath = join_path(find_json_template_folder(), "Resources\\JSONTemplate.json");

	std::ifstream iff(jsonTemplatePath.c_str());
	if (iff)
	{
		iff.seekg(0, std::ios::end);
		size_t fileSize = size_t(iff.tellg());
		iff.seekg(0, std::ios::beg);

		std::string buffer;
		buffer.resize(fileSize);
		iff.read(&*buffer.begin(), fileSize);

		return buffer;
	}

	return std::string();
}

void LotteryDataAnalyzer::FormatOutput(const AnalyzeResultAll &results, std::string &outputInfo)
{
	const std::string jsonTemplateContent = read_json_template_content();
	
	rapidjson::Document doc;
	if (doc.Parse(jsonTemplateContent).HasParseError())
	{
		std::ostringstream oss;
		oss << "Json template has error, error code is : " << uint32(doc.GetParseError()) << std::endl;
		OutputDebugStringA(oss.str().c_str());
		return;
	}

	struct AnalyzeResutPack
	{
		const AnalyzeResult *result;
		const char* name;
	};

	AnalyzeResutPack resultPacks[] = {
		&results.bigSmall, "BigSmall",
		&results.oddEven, "OddEven",
		&results.numBigSmall, "NumBigSmall",
		&results.numOddEven, "NumOddEven",
	};

	for (const auto &pack : resultPacks)
	{
		format_result(*pack.result, doc, pack.name);
	}

	rapidjson::StringBuffer sb;
	rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
	doc.Accept(writer); 
	outputInfo = sb.GetString();

	if (false)
	{
		rapidjson::StringBuffer tmpSB;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> pw(tmpSB);
		doc.Accept(pw);

		std::ofstream off("jsontest.json");
		off << sb.GetString();
	}
}

ErrorType LotteryDataAnalyzer::ReadDataFromFile()
{
	std::ifstream iff(mDataPath.c_str());

	if (!iff)
		return ErrorType::ET_FilePathIsNotValid;

	std::wstring regMatchString(L"\t");

	for (uint32 ii = 0; ii < 9; ++ii)
		regMatchString += L"\\d+\\s*,?";

	regMatchString += L"\\d+";

	std::wregex regDataNum(regMatchString.c_str(), std::regex_constants::icase);

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

		auto wLine = utf8_to_utf16(line);
		line = utf16_to_utf8(wLine);

		std::wsmatch matchResult;
		if (std::regex_search(wLine, matchResult, regDataNum))
		{
			std::wstring strData = *matchResult.begin();
			trim(strData);

			std::vector<std::string> dataParts;
			boost::split(dataParts, strData, boost::is_any_of(", "));
			if (dataParts.size() != 10)
				return ErrorType::ET_FileFormatErrorWithWrongData;

			mLotteryData.push_back(LotteryLineData());
			LotteryLineData &lottery = mLotteryData.back();
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
		}


	}
	return ErrorType::ET_NoError;
}

ErrorType LotteryDataAnalyzer::ReadDataFromURL()
{
	curl_easy_init();
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

void DataFilter::Filter(const LotteryLineDataArray &lotteryDataArray)
{
	for (uint32 iCol = 0; iCol < DATA_COLUMN_NUM; ++iCol)
	{
		auto &counter = mCounters[iCol];
		for (uint32 iRow = 0; iRow < lotteryDataArray.size(); ++iRow)
		{
			analyze_num(lotteryDataArray[iRow].data[iCol].num, counter.bigNumChecker, counter.oddNumChecker, counter.bigCounterContainer, counter.oddCounterContainer);
		}

		auto bigPair = counter.bigNumChecker.GetCounter();
		store_in_container(bigPair, counter.bigCounterContainer);
		
		auto oddPair = counter.oddNumChecker.GetCounter();
		store_in_container(oddPair, counter.oddCounterContainer);
	}

	for (uint32 iCol = 0; iCol < DATA_COLUMN_NUM; ++iCol)
	{
		auto &counter = mNumCounters[iCol];
		for (uint32 iRow = 0; iRow < lotteryDataArray.size() - 1; ++iRow)
		{
			const auto& line = lotteryDataArray[iRow];
			const uint32 idx = line.indices[iCol];

			BOOST_ASSERT(line.data[idx].num == (iCol + 1));

			const auto& nextLine = lotteryDataArray[iRow + 1];
		
			analyze_num(nextLine.data[idx].num, counter.bigNumChecker, counter.oddNumChecker, counter.bigCounterContainer, counter.oddCounterContainer);
		}

		auto bigPair = counter.bigNumChecker.GetCounter();
		store_in_container(bigPair, counter.bigCounterContainer);

		auto oddPair = counter.oddNumChecker.GetCounter();
		store_in_container(oddPair, counter.oddCounterContainer);
	}
}

static CounterContainer rebuild_full_container(const CounterContainerPair &pair)
{
	//{@	merge in one vector
	CounterContainer fullContainer;
	fullContainer.reserve(pair.cc0.size() + pair.cc1.size());

	const CounterContainer *first = &pair.cc0;
	const CounterContainer *second = &pair.cc1;
	if (pair.orderIdx != 0)
		std::swap(first, second);


	BOOST_ASSERT(std::abs(int32(first->size() - second->size())) <= 1);

	const uint32 minSize = std::min(first->size(), second->size());

	for (uint32 jj = 0; jj < minSize; ++jj)
	{
		fullContainer.push_back((*first)[jj]);
		fullContainer.push_back((*second)[jj]);
	}

	if (first->size() > second->size())
		fullContainer.push_back(first->back());
	else if (second->size() > first->size())
		fullContainer.push_back(second->back());
	//@}

	return std::move(fullContainer);
}

static uint32 sum_container(CounterContainer::const_iterator itBeg,  CounterContainer::const_iterator itEnd)
{
	uint32 index = 0;
	std::for_each(itBeg, itEnd, [&index](uint32 num) {index += num; });

	return index;
}

void IDataAnalyzer::Analyze(const LotteryLineDataArray &lotteryLines, const ColumnContainers &containers, 
	const std::tuple<NumType, NumType> &types, 
	AnalyzeResult &result)
{
	BOOST_ASSERT(!(*containers[0]).cc0.empty() || !(*containers[0]).cc1.empty());
	BOOST_ASSERT(containers.size() == result.continueRecords.size() && result.continueRecords.size() == result.stepRecords.size());

	for (uint32 ii = 0; ii < containers.size(); ++ii)
	{
		const NumType firstType = (containers[ii]->orderIdx == 0)  ? std::get<0>(types) : std::get<1>(types);
		const NumType secondType = (containers[ii]->orderIdx == 0) ? std::get<1>(types) : std::get<0>(types);

		CounterContainer fullContainer = std::move(rebuild_full_container(*containers[ii]));

		CounterContainer::iterator itFull = std::begin(fullContainer);
		while (itFull != std::end(fullContainer))
		{
			auto dis = std::distance(std::begin(fullContainer), itFull);
			
			const uint32 value = *itFull;
			if (value != 1)
			{
				const NumType type = (dis % 2 == 0) ? firstType : secondType;
				AnalyzeResult::ResultCounterMap &rsm = result.continueRecords[ii];

				auto &resultValue = rsm[value];

				resultValue.info.push_back(AnalyzeResult::ResultCounter::OriginInfo());
				resultValue.info.back().type = type;
				resultValue.numCounter = value;

				resultValue.info.back().numbers = BuildOriginContinueNumbers(lotteryLines, ii, fullContainer, itFull);

				++itFull;
			}
			else
			{
				AnalyzeResult::ResultCounterMap &rsm = result.stepRecords[ii];
				auto itFound = std::find_if_not(itFull, std::end(fullContainer), [](uint32 vv) { return vv == 1; });				
				BOOST_ASSERT(itFull != itFound);

				uint32 numCounter = uint32(std::distance(itFull, itFound));

				uint32 extraNum = 0;
				auto numbers = std::move(BuildOriginStepNumbers(lotteryLines, ii, fullContainer, itFull, itFound, extraNum));

				// the calculate method is : the number of 1 in the array
				// and the begin and end count, but the step is need two number to generate, so we need to minus 1.
				// ex : 9, 8, 6, 1, 7, 1, 2 ==> 3, 1, 1, 2 ==> 3,1 has one step, 1, 1 has one step, 1, 2 has one step, 4 number(6, 1, 7, 1) but 3 steps.
				const uint32 finalNumCounter = numCounter + extraNum - 1;	
				auto &resultValue = rsm[finalNumCounter];

				resultValue.numCounter = finalNumCounter;

				resultValue.info.push_back(AnalyzeResult::ResultCounter::OriginInfo());
				resultValue.info.back().type = AnalyzeResult::ResultCounter::Unknown;				

				resultValue.info.back().numbers = std::move(numbers);

				itFull = itFound;
			}
		}

	}
}

std::vector<uint32> SimpleDataAnalyzer::BuildOriginContinueNumbers(const LotteryLineDataArray &lotteryLines, uint32 colIdx, const CounterContainer& container, CounterContainer::const_iterator itData)
{
	const uint32 index = sum_container(container.begin(), itData);

	BOOST_ASSERT(index + *itData <= lotteryLines.size());

	std::vector<uint32>	numbers;

	for (uint32 iOrg = 0; iOrg < *itData; ++iOrg)
	{
		numbers.push_back(lotteryLines[index + iOrg].data[colIdx].num);
	}

	return std::move(numbers);
}

std::vector<uint32> SimpleDataAnalyzer::BuildOriginStepNumbers(const LotteryLineDataArray &lotteryLines, uint32 colIdx, const CounterContainer& container, 
	CounterContainer::const_iterator itBeg, CounterContainer::const_iterator itEnd, uint32 &extraNum)
{
	extraNum = 0;

	std::vector<uint32>	numbers;
	if (itBeg != std::begin(container))
	{
		++extraNum;

		const uint32 previousSumNum = sum_container(std::begin(container), itBeg);
		BOOST_ASSERT(*(itBeg - 1) > 1);

		BOOST_ASSERT(previousSumNum < lotteryLines.size());

		numbers.push_back(lotteryLines[previousSumNum - 1].data[colIdx].num);
	}

	for (auto it = itBeg; it != itEnd; ++it)
	{
		const uint32 previousSumNum = sum_container(std::begin(container), it);

		BOOST_ASSERT(*it == 1);
		numbers.push_back(lotteryLines[previousSumNum].data[colIdx].num);
	}

	if (itEnd != std::end(container))
	{
		++extraNum;
		const uint32 previousSumNum = sum_container(std::begin(container), itEnd);

		BOOST_ASSERT(*itEnd > 1);
		numbers.push_back(lotteryLines[previousSumNum].data[colIdx].num);
	}

	return std::move(numbers);
}


std::vector<uint32> NumDataAnalyzer::BuildOriginContinueNumbers(const LotteryLineDataArray &lotteryLines, uint32 colIdx, const CounterContainer& container, CounterContainer::const_iterator itData)
{
	const uint32 index = sum_container(container.begin(), itData);

	BOOST_ASSERT(index + *itData <= (lotteryLines.size() - 1));

	std::vector<uint32>	numbers;

	for (uint32 iOrg = 0; iOrg < *itData; ++iOrg)
	{
		// +1, because the data is begin from line 2. see DataFilter::Filter function 
		auto& line = lotteryLines[index + iOrg + 1];
		
		numbers.push_back(line.data[line.indices[colIdx]].num);
	}

	return std::move(numbers);
}

std::vector<uint32> NumDataAnalyzer::BuildOriginStepNumbers(const LotteryLineDataArray &lotteryLines, uint32 colIdx, const CounterContainer& container, CounterContainer::const_iterator itBeg, CounterContainer::const_iterator itEnd, uint32 &extraNum)
{
	extraNum = 0;

	auto getLotteryLineValueOp = [&lotteryLines, &container, colIdx](CounterContainer::const_iterator itData)
	{
		const uint32 previousSumNum = sum_container(std::begin(container), itData);
		BOOST_ASSERT((previousSumNum + 1) < lotteryLines.size());

		auto& numLine = lotteryLines[previousSumNum];
		auto& valueLine = lotteryLines[previousSumNum + 1];
		
		return valueLine.data[numLine.indices[colIdx]].num;
	};

	std::vector<uint32>	numbers;
	if (itBeg != std::begin(container))
	{
		++extraNum;
		BOOST_ASSERT(*(itBeg - 1) > 1);

		numbers.push_back(getLotteryLineValueOp(itBeg));
	}

	for (auto it = itBeg; it != itEnd; ++it)
	{
		BOOST_ASSERT(*it == 1);
		numbers.push_back(getLotteryLineValueOp(it));
	}

	if (itEnd != std::end(container))
	{
		++extraNum;
		BOOST_ASSERT(*itEnd > 1);

		numbers.push_back(getLotteryLineValueOp(itEnd));
	}

	return std::move(numbers);
}

static ColumnContainers create_bigsmall_column_containers(const DataFilter::ColumnCounters &columnCounters)
{
	ColumnContainers containers;

	for (uint32 ii = 0; ii < containers.size(); ++ii)
	{
		containers[ii] = &(columnCounters[ii].bigCounterContainer);
	}

	return std::move(containers);
}

static ColumnContainers create_oddeven_column_containers(const DataFilter::ColumnCounters &columnCounters)
{
	ColumnContainers containers;

	for (uint32 ii = 0; ii < containers.size(); ++ii)
	{
		containers[ii] = &(columnCounters[ii].oddCounterContainer);
	}

	return std::move(containers);
}

void BigSmallAnalyzer::Analyze(const LotteryLineDataArray &lotteryDataArray, const DataFilter &filter, AnalyzeResult &result)
{
	IDataAnalyzer::Analyze(lotteryDataArray, create_bigsmall_column_containers(filter.mCounters), std::make_tuple(AnalyzeResult::ResultCounter::Big, AnalyzeResult::ResultCounter::Small), result);
}

void OddEvenAnalyzer::Analyze(const LotteryLineDataArray &lotteryDataArray, const DataFilter &filter, AnalyzeResult &result)
{
	IDataAnalyzer::Analyze(lotteryDataArray, create_oddeven_column_containers(filter.mCounters), std::make_tuple(AnalyzeResult::ResultCounter::Odd, AnalyzeResult::ResultCounter::Even), result);
}

void NumberBigSmallAnalyzer::Analyze(const LotteryLineDataArray &lotteryDataArray, const DataFilter &filter, AnalyzeResult &result)
{
	IDataAnalyzer::Analyze(lotteryDataArray, create_bigsmall_column_containers(filter.mNumCounters), std::make_tuple(AnalyzeResult::ResultCounter::Big, AnalyzeResult::ResultCounter::Small), result);
}

void NumberOddEvenAnalyzer::Analyze(const LotteryLineDataArray &lotteryDataArray, const DataFilter &filter, AnalyzeResult &result)
{
	IDataAnalyzer::Analyze(lotteryDataArray, create_oddeven_column_containers(filter.mNumCounters), std::make_tuple(AnalyzeResult::ResultCounter::Odd, AnalyzeResult::ResultCounter::Even), result);
}