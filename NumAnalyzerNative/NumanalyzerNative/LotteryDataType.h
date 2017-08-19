#pragma once
#include "RemoveWindowDefMacro.h"
extern bool IsOddNum(uint32 num);
extern bool IsBigNum(uint32 num);

using CounterContainer = std::vector<uint32>;


struct CounterContainerPair
{
	CounterContainer cc0, cc1;
	uint8 orderIdx;
};

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

	std::vector<LotteryData>	data;
	std::vector<uint32>			indices;
	uint32 lotteryNum;
	std::string date;
};

using LotteryLineDataArray = std::vector<LotteryLineData>;

using ColumnContainers = std::array<const CounterContainerPair*, DATA_COLUMN_NUM>;

struct AnalyzeResult
{
	struct ResultCounter
	{
		enum NumType : uint8
		{
			Big = 0,
			Small,
			Odd,
			Even,
			Unknown,
		};


		ResultCounter()
			: numCounter(0)
		{}


		struct OriginInfo
		{
			OriginInfo() : type(Unknown) {}

			std::vector<uint32>	numbers;
			NumType				type;
		};

		std::vector<OriginInfo>	info;
		uint32	numCounter;
	};

	using TheCounter = uint32;
	using ResultCounterMap = std::map<TheCounter, ResultCounter>;
	using ColumnRecords = std::array<ResultCounterMap, DATA_COLUMN_NUM>;

	ColumnRecords	continueRecords;
	ColumnRecords	stepRecords;
};

using NumType = AnalyzeResult::ResultCounter::NumType;

struct AnalyzeResultAll
{
	AnalyzeResult bigSmall;
	AnalyzeResult oddEven;
	AnalyzeResult numBigSmall;
	AnalyzeResult numOddEven;
};
