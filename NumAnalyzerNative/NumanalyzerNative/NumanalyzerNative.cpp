// NumanalyzerNative.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "NumanalyzerNative.h"

#include "LotteryDataAnalyzer.h"

#include "StringUtils.h"


// This is an example of an exported variable
NUMANALYZERNATIVE_API int nNumanalyzerNative=0;

// This is an example of an exported function.
NUMANALYZERNATIVE_API int fnNumanalyzerNative(const char* path)
{
	CNumanalyzerNative analyer;
	const std::wstring wPath = is_empty_c_str(path) ? L"" : utf8_to_utf16(std::string(path));
	return int(analyer.Run(wPath));
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

ErrorType CNumanalyzerNative::Run(const std::wstring &path)
{
	ReleaseLotteryAnalyzer();

	mLotteryAnalyzer = new LotteryDataAnalyzer(path);

	auto result = mLotteryAnalyzer->ConstructData();
	if (ErrorType::ET_NoError != result)
		return result;

	LotteryAnalyzeOutputData output;
	return mLotteryAnalyzer->Analyze(output);
}

void CNumanalyzerNative::ReleaseLotteryAnalyzer()
{
	if (mLotteryAnalyzer)
	{
		delete mLotteryAnalyzer;
		mLotteryAnalyzer = nullptr;
	}
}
