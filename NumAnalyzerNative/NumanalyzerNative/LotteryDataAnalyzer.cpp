#include "stdafx.h"

#include "LotteryDataAnalyzer.h"
#include "StringUtils.h"
#include "LogSystem.h"

#include "rapidjson/include/rapidjson/document.h"
#include "rapidjson/include/rapidjson/stringbuffer.h"
#include "rapidjson/include/rapidjson/writer.h"
#include "rapidjson/include/rapidjson/prettywriter.h"

#include "curl/include/curl/curl.h"

#include "DataReader/DataReader.h"

LotteryDataAnalyzer::LotteryDataAnalyzer(const std::wstring &dataPath)
	:mDataPath(dataPath)
{

}

ErrorType LotteryDataAnalyzer::ConstructData(const CreateDataReaderParam &param)
{
	std::unique_ptr<IDataReader> reader(IDataReader::CreateDataReader(param));

	return reader->ConstructData(mLotteryData);
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

	return StringUtils::utf16_to_gbk(StringUtils::get_parent_path(buffer));
}

static std::string read_json_template_content()
{
	//const std::string jsonTemplatePath = "G:/github/NumAnalyzer/NumAnalyzerNative/NumAnalyzerConsoleTest/jsontest1.json";//join_path(find_json_template_folder(), "Resources\\JSONTemplate.json");
	const std::string jsonTemplatePath = StringUtils::join_path(find_json_template_folder(), "Resources\\JSONTemplate.json");

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

static void output_analyze_results(const AnalyzeResultAll &results, rapidjson::Document &doc)
{
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
}

static void output_origin_numbers(LotteryLineDataArray &lotterys, rapidjson::Document &doc)
{
	//{@	save origin data info
	const char* originNumName = "OriginNumbers";
	BOOST_ASSERT(doc.FindMember(originNumName) == doc.MemberEnd());

	rapidjson::Value originNumValue;
	originNumValue.SetArray();

	for (auto &line : lotterys)
	{
		rapidjson::Value lineValue;
		lineValue.SetArray();

		for (auto &data : line.data)
		{
			lineValue.PushBack(data.num, doc.GetAllocator());

		}

		originNumValue.PushBack(lineValue, doc.GetAllocator());
	}

	doc.AddMember(rapidjson::StringRef(originNumName), originNumValue, doc.GetAllocator());
	//@}
}

void LotteryDataAnalyzer::FormatOutput(const AnalyzeResultAll &results, std::string &outputInfo)
{
	const std::string jsonTemplateContent = read_json_template_content();
	
	rapidjson::Document doc;
	if (doc.Parse(jsonTemplateContent).HasParseError())
	{
		std::ostringstream oss;
		oss << "Json template has error, error code is : " << uint32(doc.GetParseError()) << std::endl;
		LogSystem::Get()->Log(oss.str());
		return;
	}

	output_analyze_results(results, doc);
	output_origin_numbers(mLotteryData, doc);

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

bool IsOddNum(uint32 num)
{
	return num & 0x01;
}

bool IsBigNum(uint32 num)
{
	return num > 5;
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