// NumanalyzerNative.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "NumanalyzerNative.h"

#include "LotteryDataAnalyzer.h"
#include "LogSystem.h"

#include "DataReader/DataReader.h"

#include "StringUtils.h"

#include "curl/include/curl/curl.h"

extern "C"
{
	NUMANALYZERNATIVE_API int InitNative()
	{
		LogSystem::Init(L"Log.log");

		CURLcode code = curl_global_init(CURL_GLOBAL_ALL);
		if (CURLE_OK == code)
			return 0;

		{
			std::ostringstream oss;
			oss << "Init cURL failed, url code : " << int32(code) << std::endl;
			LogSystem::Get()->Log(oss.str());
		}		
		return 1;
	}

	NUMANALYZERNATIVE_API void URLPageToReadNative(int pages)
	{
		CNumanalyzerNative::Get().SetURLPageToRead(pages);
	}

	NUMANALYZERNATIVE_API int NumAnalyzeNative(const char* path, char* output, int outputBufferSize, bool isURL)
	{
		const std::wstring wPath = StringUtils::is_empty_c_str(path) ? L"" : StringUtils::utf8_to_utf16(std::string(path));

		std::string outputInfo;
		ErrorType result = CNumanalyzerNative::Get().Run(wPath, outputInfo, isURL);

		if (result == ErrorType::ET_NoError && !outputInfo.empty())
		{
			BOOST_ASSERT(outputBufferSize > 0);
			if (outputInfo.size() > uint32(outputBufferSize))
			{
				LogSystem::Get()->Log("output buffer size is not enough to store all the info\n");
			}

			::strncpy_s(output, outputBufferSize, &*outputInfo.begin(), outputInfo.size());
		}
		else
		{
			std::wostringstream woss;
			woss << L"error code : " << std::endl;
			LogSystem::Get()->Log(woss.str());
		}

		return int32(result);
	}
}



// This is the constructor of a class that has been exported.
// see NumanalyzerNative.h for the class definition

CNumanalyzerNative::CNumanalyzerNative()
	: mLotteryAnalyzer(nullptr)
	, mURLPageToRead(1)
{

}

CNumanalyzerNative::~CNumanalyzerNative()
{
	ReleaseLotteryAnalyzer();
}

ErrorType CNumanalyzerNative::Run(const std::wstring &path, std::string &outputInfo, bool isURL)
{
	ReleaseLotteryAnalyzer();

	mLotteryAnalyzer = new LotteryDataAnalyzer(path);

	const CreateDataReaderParam param = { path, mURLPageToRead, isURL };

	auto result = mLotteryAnalyzer->ConstructData(param);
	if (ErrorType::ET_NoError != result)
		return result;

	return mLotteryAnalyzer->Analyze(outputInfo);
}


void CNumanalyzerNative::Clear()
{
	ReleaseLotteryAnalyzer();
}

void CNumanalyzerNative::ReleaseLotteryAnalyzer()
{
	if (mLotteryAnalyzer)
	{
		delete mLotteryAnalyzer;
		mLotteryAnalyzer = nullptr;
	}
}

CNumanalyzerNative CNumanalyzerNative::mSingleton;
