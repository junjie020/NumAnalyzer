#pragma once
#include "stdafx.h"
#include "DataFilter.h"

static void store_in_container(const ReciprocalCounterPair &pair, CounterContainerPair &container)
{
	auto first = std::get<0>(pair);
	auto second = std::get<1>(pair);

	const bool isNotInit = container.cc0.empty() && container.cc1.empty();

	if (first != 0)
	{
		if (isNotInit)
			container.orderIdx = 0;

		container.cc0.push_back(first);
	}


	if (second != 0)
	{
		if (isNotInit)
			container.orderIdx = 1;

		container.cc1.push_back(second);
	}

}

static void filter_num(uint32 num,
	ReciprocalCounter<BigSmallNumChecker> &bigNumChecker,
	ReciprocalCounter<OddNumChecker> &oddNumChecker,
	CounterContainerPair &bigCounterContainer,
	CounterContainerPair &oddCounterContainer)
{
	ReciprocalCounterPair pairBig;
	if (bigNumChecker.Calc(num, pairBig))
	{
		store_in_container(pairBig, bigCounterContainer);
	}

	ReciprocalCounterPair pairOdd;
	if (oddNumChecker.Calc(num, pairOdd))
	{
		store_in_container(pairOdd, oddCounterContainer);
	}
}

void DataFilter::Filter(const LotteryLineDataArray &lotteryDataArray)
{
	for (uint32 iCol = 0; iCol < DATA_COLUMN_NUM; ++iCol)
	{
		auto &counter = mCounters[iCol];
		for (uint32 iRow = 0; iRow < lotteryDataArray.size(); ++iRow)
		{
			filter_num(lotteryDataArray[iRow].data[iCol].num, counter.bigNumChecker, counter.oddNumChecker, counter.bigCounterContainer, counter.oddCounterContainer);
		}

		auto bigPair = counter.bigNumChecker.GetCounter();
		store_in_container(bigPair, counter.bigCounterContainer);

		auto oddPair = counter.oddNumChecker.GetCounter();
		store_in_container(oddPair, counter.oddCounterContainer);
	}

	for (uint32 iCol = 0; iCol < DATA_COLUMN_NUM; ++iCol)
	{
		auto &counter = mNumCounters[iCol];
		for (uint32 iRow = 0; iRow < lotteryDataArray.size() - 1; ++iRow)
		{
			const auto& line = lotteryDataArray[iRow];
			const uint32 idx = line.indices[iCol];

			BOOST_ASSERT(line.data[idx].num == (iCol + 1));

			const auto& nextLine = lotteryDataArray[iRow + 1];

			filter_num(nextLine.data[idx].num, counter.bigNumChecker, counter.oddNumChecker, counter.bigCounterContainer, counter.oddCounterContainer);
		}

		auto bigPair = counter.bigNumChecker.GetCounter();
		store_in_container(bigPair, counter.bigCounterContainer);

		auto oddPair = counter.oddNumChecker.GetCounter();
		store_in_container(oddPair, counter.oddCounterContainer);
	}
}