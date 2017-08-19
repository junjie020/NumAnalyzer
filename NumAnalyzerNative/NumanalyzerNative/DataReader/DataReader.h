#pragma once

#include "ErrorType.h"
#include "LotteryDataType.h"

class IDataReader
{
public:
	virtual ErrorType ConstructData(LotteryLineDataArray &lotterys) = 0;

public:
	static IDataReader* CreateDataReader(bool isURL, const std::wstring &content);
};
