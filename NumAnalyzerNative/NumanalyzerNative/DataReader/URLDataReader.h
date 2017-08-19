#pragma once

#include "DataReader/DataReader.h"

class URLDataReader : public IDataReader
{
public:

	URLDataReader(const std::wstring &URL);

	virtual ErrorType ConstructData(LotteryLineDataArray &lotterys) override;

private:
	std::wstring mURL;
};