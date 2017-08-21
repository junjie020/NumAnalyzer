#pragma once
#include "DataType.h"
enum class ErrorType : uint32
{
	ET_NoError = 0,

	ET_FilePathIsNotValid,
	ET_FileFormatError,
	ET_FileFormatErrorWithWrongData,

	ET_AnalyzeEmptyData,

	ET_URLPreformError,
};