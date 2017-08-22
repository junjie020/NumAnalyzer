#pragma once

#include "ErrorType.h"
#include "LotteryDataType.h"

struct CreateDataReaderParam
{
	std::wstring content;	
	int32 URLPagesToRead;
	bool isURL;
};

class IDataReader
{
public:
	virtual ErrorType ConstructData(LotteryLineDataArray &lotterys) = 0;

public:
	static IDataReader* CreateDataReader(const CreateDataReaderParam &param);
};
