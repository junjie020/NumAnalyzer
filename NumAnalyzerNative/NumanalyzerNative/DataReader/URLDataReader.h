#pragma once

#include "DataReader/DataReader.h"

class URLDataReader : public IDataReader
{
public:

	URLDataReader(const std::wstring &URL);

	virtual ErrorType ConstructData(LotteryLineDataArray &lotterys) override;

	std::wstring& GetContent() {
		return mURLContent;
	}

private:
	ErrorType DownloadDataFromURL();

private:
	std::wstring mURL;

	std::wstring mURLContent;
};