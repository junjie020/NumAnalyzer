#include "stdafx.h"

#include "DataReader.h"
#include "FileDataReader.h"
#include "URLDataReader.h"

IDataReader* IDataReader::CreateDataReader(const CreateDataReaderParam &param)
{
	if (param.isURL)
		return new URLDataReader(param.content, param.URLPagesToRead);

	return new FileDataReader(param.content);
}


