// NumanalyzerNative.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "NumanalyzerNative.h"

#include "LotteryDataAnalyzer.h"

#include "StringUtils.h"


// This is an example of an exported variable
NUMANALYZERNATIVE_API int nNumanalyzerNative=0;

extern "C"
{
	// This is an example of an exported function.
	NUMANALYZERNATIVE_API int fnNumanalyzerNative(const char* path, char* output)
	{
		CNumanalyzerNative::Get().Clear();

		const std::wstring wPath = is_empty_c_str(path) ? L"" : utf8_to_utf16(std::string(path));

		std::string outputInfo;
		ErrorType result = CNumanalyzerNative::Get().Run(wPath, outputInfo);

		if (result == ErrorType::ET_NoError)
		{
			std::strcpy(output, &*outputInfo.begin());
		}

		return int32(result);
	}
}




// This is the constructor of a class that has been exported.
// see NumanalyzerNative.h for the class definition

CNumanalyzerNative::CNumanalyzerNative()
	:mLotteryAnalyzer(nullptr)
{

}

CNumanalyzerNative::~CNumanalyzerNative()
{
	ReleaseLotteryAnalyzer();
}

ErrorType CNumanalyzerNative::Run(const std::wstring &path, std::string &outputInfo)
{
	ReleaseLotteryAnalyzer();

	mLotteryAnalyzer = new LotteryDataAnalyzer(path);

	auto result = mLotteryAnalyzer->ConstructData();
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
