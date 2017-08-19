#include "stdafx.h"

#include "URLDataReader.h"

#include "curl/include/curl/curl.h"

URLDataReader::URLDataReader(const std::wstring &URL)
	:mURL(URL)
{

}

ErrorType URLDataReader::ConstructData(LotteryLineDataArray &lotterys)
{
	CURL* handle = curl_easy_init();

	//curl_easy_setopt(handle, )
	

	
	return ErrorType::ET_NoError;
}


