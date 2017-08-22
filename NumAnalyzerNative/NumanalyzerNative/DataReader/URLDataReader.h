#pragma once

#include "DataReader/DataReader.h"

class URLDataReader : public IDataReader
{
public:

	URLDataReader(const std::wstring &URL);

	virtual ErrorType ConstructData(LotteryLineDataArray &lotterys) override;

	std::vector<char>& GetContent() {
		return mURLContent;
	}

	void SetContentSize(uint32 contentSize) {
		mURLContentSize = contentSize;
	}
	
private:
	ErrorType DownloadDataFromURL();
	ErrorType ParseURLContent(LotteryLineDataArray &lotterys);

private:
	std::wstring		mURL;
	std::vector<char>	mURLContent;
	uint32				mURLContentSize;

};