#pragma once

#include "LotteryDataType.h"

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

struct BigSmallNumChecker {
	static bool IsPositive(uint32 num) { return IsBigNum(num); }
};


struct OddNumChecker {
	static bool IsPositive(uint32 num) { return IsOddNum(num); }
};


class DataFilter
{
public:
	DataFilter(){}
	void Filter(const LotteryLineDataArray &lotteryDataArray);

	struct DataCounter
	{
		ReciprocalCounter<BigSmallNumChecker>	bigNumChecker;
		ReciprocalCounter<OddNumChecker>	oddNumChecker;

		CounterContainerPair				bigCounterContainer;
		CounterContainerPair				oddCounterContainer;
	};

	using ColumnCounters = std::array<DataCounter, DATA_COLUMN_NUM>;
	ColumnCounters mCounters;

	ColumnCounters mNumCounters;
};