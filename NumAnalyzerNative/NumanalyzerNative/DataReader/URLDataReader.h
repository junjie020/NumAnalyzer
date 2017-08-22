#pragma once

#include "DataReader/DataReader.h"

using URLContent = std::vector<char>;

struct URLContentData
{
	URLContent	content;
	uint32		contentSize;
};
using URLContentDataArray = std::vector<URLContentData>;

class URLDataReader : public IDataReader
{
public:

	URLDataReader(const std::wstring &URL, int32 pagesToRead);

	virtual ErrorType ConstructData(LotteryLineDataArray &lotterys) override;

	URLContentDataArray& GetContents() {
		return mURLContents;
	}

private:
	ErrorType DownloadDataFromURL();
	ErrorType ParseURLContent(LotteryLineDataArray &lotterys);

private:
	std::wstring		mURL;
	URLContentDataArray	mURLContents;

	int32				mPagesToRead;
};