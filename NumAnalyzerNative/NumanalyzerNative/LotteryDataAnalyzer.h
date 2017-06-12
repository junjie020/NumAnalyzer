#pragma once

#include "ErrorType.h"

struct LotteryData
{
	uint32 num;
	std::vector<uint32>	data;

	std::string date;
};

struct LotteryAnalyzeOutputData
{

};

class LotteryDataAnalyzer
{
public:
	LotteryDataAnalyzer(const std::wstring &dataPath);

	ErrorType ConstructData();

	ErrorType Analyze(LotteryAnalyzeOutputData &output);

private:
	ErrorType ReadDataFromFile();

private:
	std::wstring	mDataPath;
	std::string		mDataContent;

	std::list<LotteryData>	mLotteryData;
};