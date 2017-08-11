#include "stdafx.h"

#include "LotteryDataAnalyzer.h"
#include "StringUtils.h"

#include "boost/algorithm/string.hpp"

#include "rapidjson/include/rapidjson/document.h"
#include "rapidjson/include/rapidjson/stringbuffer.h"
#include "rapidjson/include/rapidjson/writer.h"
#include "rapidjson/include/rapidjson/prettywriter.h"

LotteryDataAnalyzer::LotteryDataAnalyzer(const std::wstring &dataPath)
	:mDataPath(dataPath)
{

}

ErrorType LotteryDataAnalyzer::ConstructData()
{
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

	FormatOutput(results, outputInfo);

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

static const char* get_column_name(uint32 iCol)
{
	const char* names[] = { "Col0", "Col1", "Col2", "Col3", "Col4", "Col5" , "Col6", "Col7" , "Col8", "Col9" };

	BOOST_ASSERT(iCol < _countof(names));

	return names[iCol];
}

static const char* get_data_type_name(AnalyzeResult::ResultCounter::NumType type)
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

static CounterContainer&& rebuild_full_container(const CounterContainerArray &containerTuple)
{
	auto *container0 = &std::get<0>(containerTuple);
	auto *container1 = &std::get<1>(containerTuple);

	if (container0->size() < container1->size())
		std::swap(container0, container1);


	//{@	merge in one vector
	CounterContainer fullContainer;
	fullContainer.reserve(container0->size() + container1->size());

	for (uint32 jj = 0; jj < container1->size(); ++jj)
	{
		fullContainer.push_back((*container0)[jj]);
		fullContainer.push_back((*container1)[jj]);
	}

	BOOST_ASSERT(container0->size() + 1 == container1->size());
	fullContainer.push_back(container0->back());
	//@}

	return std::move(fullContainer);
}

void IDataAnalyzer::Analyze(const LotteryLineDataArray &lotteryLines, const ColumnContainers &containers, 
	const std::tuple<AnalyzeResult::ResultCounter::NumType, AnalyzeResult::ResultCounter::NumType> &types, 
	AnalyzeResult &result)
{
	BOOST_ASSERT(!std::get<0>(*containers[0]).empty() || !std::get<1>(*containers[0]).empty());
	BOOST_ASSERT(containers.size() == result.continueRecords.size() && result.continueRecords.size() == result.stepRecords.size());

	for (uint32 ii = 0; ii < containers.size(); ++ii)
	{
		CounterContainer fullContainer = std::move(rebuild_full_container(*containers[ii]));

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

				BOOST_ASSERT(index + *itFull <= lotteryLines.size());

				for (uint32 iOrg = 0; iOrg < *itFull; ++iOrg)
				{
					resultValue.info.back().numbers.push_back(lotteryLines[index + iOrg].data[ii].num);
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

				resultValue.info.push_back(AnalyzeResult::ResultCounter::OriginInfo());
				resultValue.info.back().type = AnalyzeResult::ResultCounter::Unknown;				

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

void NumberBigSmallAnalyzer::Analyze(const LotteryLineDataArray &lotteryDataArray, const DataFilter &filter, AnalyzeResult &result)
{

}

void NumberOddEvenAnalyzer::Analyze(const LotteryLineDataArray &lotteryDataArray, const DataFilter &filter, AnalyzeResult &result)
{

}
