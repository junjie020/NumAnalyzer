#pragma once

#include "ErrorType.h"
#include "RemoveWindowDefMacro.h"
#include "LotteryDataType.h"
#include "Analyzer/DataFilter.h"
#include "Analyzer/DataAnalyzer.h"



class BigSmallAnalyzer : public SimpleDataAnalyzer
{
public:
	BigSmallAnalyzer() : SimpleDataAnalyzer("BigSmallAnalyzer"){}
	virtual void Analyze(const LotteryLineDataArray &lotteryDataArray, const DataFilter &filter, AnalyzeResult &result) override;
};

class OddEvenAnalyzer : public SimpleDataAnalyzer
{
public:
	OddEvenAnalyzer() : SimpleDataAnalyzer("OddEvenAnalyzer") {}
	virtual void Analyze(const LotteryLineDataArray &lotteryDataArray, const DataFilter &filter, AnalyzeResult &result) override;
};




class NumberBigSmallAnalyzer : public NumDataAnalyzer
{
public:
	NumberBigSmallAnalyzer() : NumDataAnalyzer("NumberBigSmallAnalyzer") {}
	virtual void Analyze(const LotteryLineDataArray &lotteryDataArray, const DataFilter &filter, AnalyzeResult &result) override;
};

class NumberOddEvenAnalyzer : public NumDataAnalyzer
{
public:
	NumberOddEvenAnalyzer() : NumDataAnalyzer("NumberOddEvenAnalyzer") {}
	virtual void Analyze(const LotteryLineDataArray &lotteryDataArray, const DataFilter &filter, AnalyzeResult &result) override;
};

using NumanalyerArray = std::vector<IDataAnalyzer*>;


class LotteryDataAnalyzer
{
public:
	LotteryDataAnalyzer(const std::wstring &dataPath);

	ErrorType ConstructData(bool isURL);

	ErrorType Analyze(std::string &outputInfo);
private:
	void FormatOutput(const AnalyzeResultAll &results, std::string &outputInfo);

private:
	ErrorType ReadDataFromFile();
	ErrorType ReadDataFromURL();

private:
	std::wstring			mDataPath;
	std::string				mDataContent;

	LotteryLineDataArray	mLotteryData;

	DataFilter				mFilter;
};