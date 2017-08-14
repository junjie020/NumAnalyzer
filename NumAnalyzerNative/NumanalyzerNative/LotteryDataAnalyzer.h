#pragma once

#include "ErrorType.h"
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

using ColumnContainers = std::array<const CounterContainerPair*, DATA_COLUMN_NUM>;

struct AnalyzeResult
{
	struct ResultCounter
	{
		enum NumType : uint8
		{
			Big		= 0,
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
			OriginInfo() : type(Unknown){}

			std::vector<uint32>	numbers;
			NumType				type;
		};

		std::vector<OriginInfo>	info;
		uint32	numCounter;		
	};

	using TheCounter = uint32;
	using ResultCounterMap	= std::map<TheCounter, ResultCounter>;
	using ColumnRecords		= std::array<ResultCounterMap, DATA_COLUMN_NUM>;

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


class IDataAnalyzer
{
public:
	IDataAnalyzer(const std::string &name) : mName(name){}

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


//class ContinueAnalyzer : public IDataAnalyzer
//{
//public:
//	ContinueAnalyzer() : IDataAnalyzer("ContinueAnalyzer"){}
//	virtual void Analyze(const LotteryLineDataArray &lotteryDataArray) override;
//};
//
//class StepAnalyzer : public IDataAnalyzer
//{
//public:
//	StepAnalyzer() : IDataAnalyzer("StepAnalyzer") {}
//	virtual void Analyze(const LotteryLineDataArray &lotteryDataArray) override;
//
//private:	
//	void Analyze(const LotteryLineDataArray &lotteryDataArray, uint32 colIdx, uint32 begIdx);
//};


class SimpleDataAnalyzer : public IDataAnalyzer
{
public:
	SimpleDataAnalyzer(const std::string &name) : IDataAnalyzer(name) {}
protected:
	
	virtual	std::vector<uint32> BuildOriginContinueNumbers(const LotteryLineDataArray &lotteryLines, uint32 colIdx, const CounterContainer& container, CounterContainer::const_iterator itData) override;
	
	
	virtual std::vector<uint32> BuildOriginStepNumbers(const LotteryLineDataArray &lotteryLines, uint32 colIdx, const CounterContainer& container, CounterContainer::const_iterator itBeg, CounterContainer::const_iterator itEnd, uint32 &extraNum) override;
};

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

class NumDataAnalyzer : public IDataAnalyzer
{
public:
	NumDataAnalyzer(const std::string &name) : IDataAnalyzer(name){}

protected:
	virtual std::vector<uint32> BuildOriginContinueNumbers(const LotteryLineDataArray &lotteryLines, uint32 colIdx, const CounterContainer& container, CounterContainer::const_iterator itData) override;
	virtual std::vector<uint32> BuildOriginStepNumbers(const LotteryLineDataArray &lotteryLines, uint32 colIdx, const CounterContainer& container, CounterContainer::const_iterator itBeg, CounterContainer::const_iterator itEnd, uint32 &extraNum) override;

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

	ErrorType ConstructData();

	ErrorType Analyze(std::string &outputInfo);
private:
	//void AnalyzeContinueData(ContinueAnalyzer &analyer);
	//void AnalyzeStepData(StepAnalyzer &analyer);

	void FormatOutput(const AnalyzeResultAll &results, std::string &outputInfo);

private:
	ErrorType ReadDataFromFile();

private:
	std::wstring	mDataPath;
	std::string		mDataContent;

	LotteryLineDataArray	mLotteryData;

	DataFilter		mFilter;
};