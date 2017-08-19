#pragma once
#include "DataReader/DataReader.h"

class FileDataReader : public IDataReader
{
public:
	FileDataReader(const std::wstring &path);


	virtual ErrorType ConstructData(LotteryLineDataArray &lotterys) override;

private:
	std::wstring mPath;
};

