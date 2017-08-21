#pragma once

#include "DataReader/DataReader.h"

class URLDataReader : public IDataReader
{
public:

	URLDataReader(const std::wstring &URL);

	virtual ErrorType ConstructData(LotteryLineDataArray &lotterys) override;

	std::string& GetContent() {
		return mURLContent;
	}

private:
	std::wstring mURL;

	std::string mURLContent;
};