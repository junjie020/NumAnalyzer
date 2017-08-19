#include "stdafx.h"
#include "DataAnalyzer.h"
#include "DataFilter.h"

static CounterContainer rebuild_full_container(const CounterContainerPair &pair)
{
	//{@	merge in one vector
	CounterContainer fullContainer;
	fullContainer.reserve(pair.cc0.size() + pair.cc1.size());

	const CounterContainer *first = &pair.cc0;
	const CounterContainer *second = &pair.cc1;
	if (pair.orderIdx != 0)
		std::swap(first, second);


	BOOST_ASSERT(std::abs(int32(first->size() - second->size())) <= 1);

	const uint32 minSize = std::min(first->size(), second->size());

	for (uint32 jj = 0; jj < minSize; ++jj)
	{
		fullContainer.push_back((*first)[jj]);
		fullContainer.push_back((*second)[jj]);
	}

	if (first->size() > second->size())
		fullContainer.push_back(first->back());
	else if (second->size() > first->size())
		fullContainer.push_back(second->back());
	//@}

	return std::move(fullContainer);
}

static uint32 sum_container(CounterContainer::const_iterator itBeg, CounterContainer::const_iterator itEnd)
{
	uint32 index = 0;
	std::for_each(itBeg, itEnd, [&index](uint32 num) {index += num; });

	return index;
}

void IDataAnalyzer::Analyze(const LotteryLineDataArray &lotteryLines, const ColumnContainers &containers,
	const std::tuple<NumType, NumType> &types,
	AnalyzeResult &result)
{
	BOOST_ASSERT(!(*containers[0]).cc0.empty() || !(*containers[0]).cc1.empty());
	BOOST_ASSERT(containers.size() == result.continueRecords.size() && result.continueRecords.size() == result.stepRecords.size());

	for (uint32 ii = 0; ii < containers.size(); ++ii)
	{
		const NumType firstType = (containers[ii]->orderIdx == 0) ? std::get<0>(types) : std::get<1>(types);
		const NumType secondType = (containers[ii]->orderIdx == 0) ? std::get<1>(types) : std::get<0>(types);

		CounterContainer fullContainer = std::move(rebuild_full_container(*containers[ii]));

		CounterContainer::iterator itFull = std::begin(fullContainer);
		while (itFull != std::end(fullContainer))
		{
			auto dis = std::distance(std::begin(fullContainer), itFull);

			const uint32 value = *itFull;
			if (value != 1)
			{
				const NumType type = (dis % 2 == 0) ? firstType : secondType;
				AnalyzeResult::ResultCounterMap &rsm = result.continueRecords[ii];

				auto &resultValue = rsm[value];

				resultValue.info.push_back(AnalyzeResult::ResultCounter::OriginInfo());
				resultValue.info.back().type = type;
				resultValue.numCounter = value;

				resultValue.info.back().numbers = BuildOriginContinueNumbers(lotteryLines, ii, fullContainer, itFull);

				++itFull;
			}
			else
			{
				AnalyzeResult::ResultCounterMap &rsm = result.stepRecords[ii];
				auto itFound = std::find_if_not(itFull, std::end(fullContainer), [](uint32 vv) { return vv == 1; });
				BOOST_ASSERT(itFull != itFound);

				uint32 numCounter = uint32(std::distance(itFull, itFound));

				uint32 extraNum = 0;
				auto numbers = std::move(BuildOriginStepNumbers(lotteryLines, ii, fullContainer, itFull, itFound, extraNum));

				// the calculate method is : the number of 1 in the array
				// and the begin and end count, but the step is need two number to generate, so we need to minus 1.
				// ex : 9, 8, 6, 1, 7, 1, 2 ==> 3, 1, 1, 2 ==> 3,1 has one step, 1, 1 has one step, 1, 2 has one step, 4 number(6, 1, 7, 1) but 3 steps.
				const uint32 finalNumCounter = numCounter + extraNum - 1;
				auto &resultValue = rsm[finalNumCounter];

				resultValue.numCounter = finalNumCounter;

				resultValue.info.push_back(AnalyzeResult::ResultCounter::OriginInfo());
				resultValue.info.back().type = AnalyzeResult::ResultCounter::Unknown;

				resultValue.info.back().numbers = std::move(numbers);

				itFull = itFound;
			}
		}

	}
}

std::vector<uint32> SimpleDataAnalyzer::BuildOriginContinueNumbers(const LotteryLineDataArray &lotteryLines, uint32 colIdx, const CounterContainer& container, CounterContainer::const_iterator itData)
{
	const uint32 index = sum_container(container.begin(), itData);

	BOOST_ASSERT(index + *itData <= lotteryLines.size());

	std::vector<uint32>	numbers;

	for (uint32 iOrg = 0; iOrg < *itData; ++iOrg)
	{
		numbers.push_back(lotteryLines[index + iOrg].data[colIdx].num);
	}

	return std::move(numbers);
}

std::vector<uint32> SimpleDataAnalyzer::BuildOriginStepNumbers(const LotteryLineDataArray &lotteryLines, uint32 colIdx, const CounterContainer& container,
	CounterContainer::const_iterator itBeg, CounterContainer::const_iterator itEnd, uint32 &extraNum)
{
	extraNum = 0;

	std::vector<uint32>	numbers;
	if (itBeg != std::begin(container))
	{
		++extraNum;

		const uint32 previousSumNum = sum_container(std::begin(container), itBeg);
		BOOST_ASSERT(*(itBeg - 1) > 1);

		BOOST_ASSERT(previousSumNum < lotteryLines.size());

		numbers.push_back(lotteryLines[previousSumNum - 1].data[colIdx].num);
	}

	for (auto it = itBeg; it != itEnd; ++it)
	{
		const uint32 previousSumNum = sum_container(std::begin(container), it);

		BOOST_ASSERT(*it == 1);
		numbers.push_back(lotteryLines[previousSumNum].data[colIdx].num);
	}

	if (itEnd != std::end(container))
	{
		++extraNum;
		const uint32 previousSumNum = sum_container(std::begin(container), itEnd);

		BOOST_ASSERT(*itEnd > 1);
		numbers.push_back(lotteryLines[previousSumNum].data[colIdx].num);
	}

	return std::move(numbers);
}


std::vector<uint32> NumDataAnalyzer::BuildOriginContinueNumbers(const LotteryLineDataArray &lotteryLines, uint32 colIdx, const CounterContainer& container, CounterContainer::const_iterator itData)
{
	const uint32 index = sum_container(container.begin(), itData);

	BOOST_ASSERT(index + *itData <= (lotteryLines.size() - 1));

	std::vector<uint32>	numbers;

	for (uint32 iOrg = 0; iOrg < *itData; ++iOrg)
	{
		// +1, because the data is begin from line 2. see DataFilter::Filter function 
		auto& line = lotteryLines[index + iOrg + 1];

		numbers.push_back(line.data[line.indices[colIdx]].num);
	}

	return std::move(numbers);
}

std::vector<uint32> NumDataAnalyzer::BuildOriginStepNumbers(const LotteryLineDataArray &lotteryLines, uint32 colIdx, const CounterContainer& container, CounterContainer::const_iterator itBeg, CounterContainer::const_iterator itEnd, uint32 &extraNum)
{
	extraNum = 0;

	auto getLotteryLineValueOp = [&lotteryLines, &container, colIdx](CounterContainer::const_iterator itData)
	{
		const uint32 previousSumNum = sum_container(std::begin(container), itData);
		BOOST_ASSERT((previousSumNum + 1) < lotteryLines.size());

		auto& numLine = lotteryLines[previousSumNum];
		auto& valueLine = lotteryLines[previousSumNum + 1];

		return valueLine.data[numLine.indices[colIdx]].num;
	};

	std::vector<uint32>	numbers;
	if (itBeg != std::begin(container))
	{
		++extraNum;
		BOOST_ASSERT(*(itBeg - 1) > 1);

		numbers.push_back(getLotteryLineValueOp(itBeg));
	}

	for (auto it = itBeg; it != itEnd; ++it)
	{
		BOOST_ASSERT(*it == 1);
		numbers.push_back(getLotteryLineValueOp(it));
	}

	if (itEnd != std::end(container))
	{
		++extraNum;
		BOOST_ASSERT(*itEnd > 1);

		numbers.push_back(getLotteryLineValueOp(itEnd));
	}

	return std::move(numbers);
}
