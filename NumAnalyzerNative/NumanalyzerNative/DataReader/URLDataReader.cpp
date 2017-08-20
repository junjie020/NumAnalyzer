#include "stdafx.h"

#include "URLDataReader.h"
#include "LogSystem.h"

#include "curl/include/curl/curl.h"

#define CHECK_URL_CALL(_CALL, ...)	{CURLcode result = _CALL(...); BOOST_ASSERT(result == CURLE_OK);}

URLDataReader::URLDataReader(const std::wstring &URL)
	:mURL(URL)
{

}

//static size_t
//WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
//{
//	size_t realsize = size * nmemb;
//	struct MemoryStruct *mem = (struct MemoryStruct *)userp;
//
//	mem->memory = realloc(mem->memory, mem->size + realsize + 1);
//	if (mem->memory == NULL) {
//		/* out of memory! */
//		printf("not enough memory (realloc returned NULL)\n");
//		return 0;
//	}
//
//	memcpy(&(mem->memory[mem->size]), contents, realsize);
//	mem->size += realsize;
//	mem->memory[mem->size] = 0;
//
//	return realsize;
//}


static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
	URLDataReader *reader = reinterpret_cast<URLDataReader*>(stream);

	auto &content = reader->GetContent();

	const size_t realsize = size * nmemb;
	content.append((const char*)ptr, ((const char*)ptr) + realsize);

	return realsize;
}
//
//int my_progress_func(GtkWidget *bar,
//	double t, /* dltotal */
//	double d, /* dlnow */
//	double ultotal,
//	double ulnow)
//{
//
//}

ErrorType URLDataReader::ConstructData(LotteryLineDataArray &lotterys)
{
	CURL* handle = curl_easy_init();

	curl_easy_setopt(handle, CURLOPT_URL);
	curl_easy_setopt(handle, CURLOPT_TIMEOUT, 20L);
	curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, 10L);

	curl_easy_setopt(handle, CURLOPT_NOPROGRESS, false);

	//curl_easy_setopt(handle, CURLOPT_PROGRESSFUNCTION, );

	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, this);

	CURLcode result = curl_easy_perform(handle);

	curl_easy_cleanup(handle);
	handle = nullptr;

	if (result != CURLE_OK)
	{
		std::ostringstream oss;
		oss << "url perform failed code : " << result << std::endl;
		LogSystem::Get()->Log(oss.str());

		return ErrorType::ET_URLPreformError;
	}

	return ErrorType::ET_NoError;
}


