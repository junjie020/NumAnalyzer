#pragma once

#include "LotteryDataType.h"
class DataFilter;

class IDataAnalyzer
{
public:
	IDataAnalyzer(const std::string &name) : mName(name) {}

	virtual void Analyze(const LotteryLineDataArray &lotteryDataArray, const DataFilter &filter, AnalyzeResult &result) = 0;

protected:
	virtual	std::vector<uint32> BuildOriginContinueNumbers(const LotteryLineDataArray &lotteryLines, uint32 colIdx, const CounterContainer& container, CounterContainer::const_iterator itData) = 0;
	virtual std::vector<uint32> BuildOriginStepNumbers(const LotteryLineDataArray &lotteryLines, uint32 colIdx, const CounterContainer& container, CounterContainer::const_iterator itBeg, CounterContainer::const_iterator itEnd, uint32 &extraNum) = 0;
protected:
	void Analyze(const LotteryLineDataArray &lotteryLines, const ColumnContainers &containers,
		const std::tuple<AnalyzeResult::ResultCounter::NumType, AnalyzeResult::ResultCounter::NumType> &types,
		AnalyzeResult &result);

public:
	std::string GetName() const {
		return mName;
	}

protected:
	std::string mName;
};

class SimpleDataAnalyzer : public IDataAnalyzer
{
public:
	SimpleDataAnalyzer(const std::string &name) : IDataAnalyzer(name) {}
protected:

	virtual	std::vector<uint32> BuildOriginContinueNumbers(const LotteryLineDataArray &lotteryLines, uint32 colIdx, const CounterContainer& container, CounterContainer::const_iterator itData) override;


	virtual std::vector<uint32> BuildOriginStepNumbers(const LotteryLineDataArray &lotteryLines, uint32 colIdx, const CounterContainer& container, CounterContainer::const_iterator itBeg, CounterContainer::const_iterator itEnd, uint32 &extraNum) override;
};

class NumDataAnalyzer : public IDataAnalyzer
{
public:
	NumDataAnalyzer(const std::string &name) : IDataAnalyzer(name) {}

protected:
	virtual std::vector<uint32> BuildOriginContinueNumbers(const LotteryLineDataArray &lotteryLines, uint32 colIdx, const CounterContainer& container, CounterContainer::const_iterator itData) override;
	virtual std::vector<uint32> BuildOriginStepNumbers(const LotteryLineDataArray &lotteryLines, uint32 colIdx, const CounterContainer& container, CounterContainer::const_iterator itBeg, CounterContainer::const_iterator itEnd, uint32 &extraNum) override;

};