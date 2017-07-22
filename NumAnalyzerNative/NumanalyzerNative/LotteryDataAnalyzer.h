#pragma once

#include "ErrorType.h"
#include "RemoveWindowDefMacro.h"

extern bool IsOddNum(uint32 num);
extern bool IsBigNum(uint32 num);

using CounterContainer = std::vector<uint32>;

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
	bool Calc(uint32 num)
	{
		const bool bValid = mPositiveCounter != 0 || mNegativeCounter != 0;
		uint32 posCounter = mPositiveCounter;
		if (ReciprocalType::IsPositive(num))
		{
			++mPositiveCounter;
			mNegativeCounter = 0;
		}
		else
		{
			++mNegativeCounter;
			mPositiveCounter = 0;
		}

		return bValid && ((posCounter != 0) == (mPositiveCounter != 0));
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

class LotteryDataAnalyzer
{
public:
	LotteryDataAnalyzer(const std::wstring &dataPath);

	ErrorType ConstructData();

	ErrorType Analyze(LotteryAnalyzeOutputData &output);

private:

	void CalcOddData(LotteryAnalyzeOutputData &output);
	ErrorType ReadDataFromFile();

private:
	std::wstring	mDataPath;
	std::string		mDataContent;

	std::list<LotteryLineData>	mLotteryData;
};