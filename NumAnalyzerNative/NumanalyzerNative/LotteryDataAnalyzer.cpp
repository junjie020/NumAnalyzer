#include "stdafx.h"

#include "LotteryDataAnalyzer.h"


LotteryDataAnalyzer::LotteryDataAnalyzer(const std::wstring &dataPath)
	:mDataPath(dataPath)
{

}

ErrorType LotteryDataAnalyzer::ConstructData()
{
	return ErrorType::ET_NoError;
}

ErrorType LotteryDataAnalyzer::ReadDataFromFile()
{


	return ErrorType::ET_NoError;
}
