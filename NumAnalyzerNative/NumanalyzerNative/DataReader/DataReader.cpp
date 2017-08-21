#include "stdafx.h"

#include "DataReader.h"
#include "FileDataReader.h"
#include "URLDataReader.h"

IDataReader* IDataReader::CreateDataReader(bool isURL, const std::wstring &content)
{
	if (isURL)
		return new URLDataReader(content);

	return new FileDataReader(content);
}


