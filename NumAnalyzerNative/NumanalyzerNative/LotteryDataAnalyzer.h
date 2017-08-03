#pragma once

#include "ErrorType.h"
#include "RemoveWindowDefMacro.h"

extern bool IsOddNum(uint32 num);
extern bool IsBigNum(uint32 num);

using CounterContainer = std::vector<uint32>;

using CounterContainerArray = std::tuple<CounterContainer, CounterContainer>;

using ReciprocalCounterPair = std::tuple<uint32, uint32>;

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

	bool Calc(uint32 num, ReciprocalCounterPair &pair)
	{
		const bool bPositive = ReciprocalType::IsPositive(num);

		std::get<0>(pair) = mPositiveCounter;
		std::get<1>(pair) = mNegativeCounter;

		uint32 *firstCounter = bPositive ? &mPositiveCounter : &mNegativeCounter;
		uint32 *secondCounter = bPositive ? &mNegativeCounter : &mPositiveCounter;

		const bool bNotFirstTimeCalc = ((*firstCounter + *secondCounter) > 0);

		uint32 firstBackup = *firstCounter;
		uint32 secondBackup = *secondCounter;

		++(*firstCounter);
		*secondCounter = 0;
		
		return (firstBackup == 0) && (*firstCounter != 0) && bNotFirstTimeCalc;
	}

	ReciprocalCounterPair GetCounter() const
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

class IDataAnalyzer
{
public:
	IDataAnalyzer(const std::string &name) : mName(name){}

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


class ContinueAnalyer : public IDataAnalyzer
{
public:
	ContinueAnalyer() : IDataAnalyzer("ContinueAnalyzer"){}
	virtual void Analyze(const LotteryLineDataArray &lotteryDataArray) override;
};

class StepAnalyer : public IDataAnalyzer
{
public:
	StepAnalyer() : IDataAnalyzer("StepAnalyzer") {}
	virtual void Analyze(const LotteryLineDataArray &lotteryDataArray) override;

private:	
	void Analyze(const LotteryLineDataArray &lotteryDataArray, uint32 colIdx, uint32 begIdx);
};

using NumanalyerArray = std::vector<IDataAnalyzer*>;


class LotteryDataAnalyzer
{
public:
	LotteryDataAnalyzer(const std::wstring &dataPath);

	ErrorType ConstructData();

	ErrorType Analyze(std::string &outputInfo);
private:
	void AnalyzeContinueData(ContinueAnalyer &analyer);
	void AnalyzeStepData(StepAnalyer &analyer);

	void FormatOutput(NumanalyerArray &&analyzers, std::string &outputInfo);

private:
	ErrorType ReadDataFromFile();

private:
	std::wstring	mDataPath;
	std::string		mDataContent;

	LotteryLineDataArray	mLotteryData;
};