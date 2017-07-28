#pragma once

#include "ErrorType.h"
#include "RemoveWindowDefMacro.h"

extern bool IsOddNum(uint32 num);
extern bool IsBigNum(uint32 num);

using CounterContainer = std::vector<uint32>;

using CounterContainerArray = std::tuple<CounterContainer, CounterContainer>;



struct LotteryLineData
{
	struct LotteryData
	{
		LotteryData(uint32 n = 0, uint32 idx = std::numeric_limits<uint8>::max())
			: num(n)
			, idxInLine(idx)
		{}

		bool IsOddNum() const {
			return ::IsOddNum(num);
		}

		bool IsBigNum() const {
			return ::IsBigNum(num);
		}
		uint8 num;
		uint8 idxInLine;
	};

	std::vector<LotteryData>	lineData;
	uint32 lotteryNum;
	std::string date;
};

using LotteryLineDataArray = std::vector<LotteryLineData>;

struct LotteryAnalyzeOutputData
{
	struct ContinueCounter
	{
		std::vector<uint32>	timers;
	};

	struct OddCounter
	{
		std::vector<uint32>	timers;
	};

	std::vector<ContinueCounter>	conConuter;
	std::vector<OddCounter>			oddConuter;
};

template<class ReciprocalType>
class ReciprocalCounter
{
public:
	ReciprocalCounter()
		: mPositiveCounter(0)
		, mNegativeCounter(0)
	{}

	bool Calc(uint32 num)
	{
		const bool bPositive = ReciprocalType::IsPositive(num);

		uint32 *firstCounter = bPositive ? &mPositiveCounter : &mNegativeCounter;
		uint32 *secondCounter = bPositive ? &mNegativeCounter : &mPositiveCounter;

		uint32 backup = *firstCounter;

		++(*firstCounter);
		*secondCounter = 0;
		
		return (backup != 0) == (*firstCounter != 0);
	}

	void StoreInContainer(std::tuple<CounterContainer, CounterContainer> &container)
	{
		auto counter = GetCounter();

		const uint32 posCounter = std::get<0>(counter);
		if (posCounter != 0)
			std::get<0>(container).push_back(posCounter);

		const uint32 negCounter = std::get<1>(counter);
		if (negCounter != 0)
			std::get<1>(container).push_back(negCounter);
	}

	std::tuple<uint32, uint32> GetCounter() const
	{
		return std::make_tuple(mPositiveCounter, mNegativeCounter);
	}


private:
	uint32 mPositiveCounter;
	uint32 mNegativeCounter;
};

struct BigNumChecker {
	static bool IsPositive(uint32 num) { return IsBigNum(num); }
};

struct OddNumChecker {
	static bool IsPositive(uint32 num) { return IsOddNum(num); }
};

class NumAnalyzer
{
public:
	NumAnalyzer(const std::string &name) : mName(name){}

	virtual void Analyze(const LotteryLineDataArray &lotteryDataArray) = 0;

public:
	std::string GetName() const {
		return mName;
	}

	const CounterContainerArray& GetBigCounterArray() const {
		return mBigCounterContainer;
	}

	const CounterContainerArray& GetOddCounterArray() const {
		return mOddCounterContainer;
	}

protected:
	ReciprocalCounter<BigNumChecker> mBigNumChecker;
	ReciprocalCounter<OddNumChecker> mOddNumChecker;

	CounterContainerArray mBigCounterContainer;
	CounterContainerArray mOddCounterContainer;

protected:
	std::string mName;
};


class ContinueNumAnalyer : public NumAnalyzer
{
public:
	ContinueNumAnalyer() : NumAnalyzer("ContinueNumAnalyer"){}
	virtual void Analyze(const LotteryLineDataArray &lotteryDataArray) override;
};

class StepNumAnalyer : public NumAnalyzer
{
public:
	StepNumAnalyer() : NumAnalyzer("StepNumAnalyer") {}
	virtual void Analyze(const LotteryLineDataArray &lotteryDataArray) override;

private:	
	void Analyze(const LotteryLineDataArray &lotteryDataArray, uint32 colIdx, uint32 begIdx);
};

using NumanalyerArray = std::vector<NumAnalyzer*>;


class LotteryDataAnalyzer
{
public:
	LotteryDataAnalyzer(const std::wstring &dataPath);

	ErrorType ConstructData();

	ErrorType Analyze(LotteryAnalyzeOutputData &output);

private:
	void AnalyzeContinueData(ContinueNumAnalyer &analyer);
	void AnalyzeStepData(StepNumAnalyer &analyer);

	void FormatOutput(NumanalyerArray &&analyzers, std::string &outputInfo);

private:
	ErrorType ReadDataFromFile();

private:
	std::wstring	mDataPath;
	std::string		mDataContent;

	LotteryLineDataArray	mLotteryData;
};