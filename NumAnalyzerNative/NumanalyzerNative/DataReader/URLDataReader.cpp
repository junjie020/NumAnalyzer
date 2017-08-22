#include "stdafx.h"

#include "URLDataReader.h"
#include "LogSystem.h"
#include "StringUtils.h"
#include "ErrorType.h"

#include "curl/include/curl/curl.h"
#include "boost/algorithm/string.hpp"

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
	content.insert(content.end(), (const char*)ptr, ((const char*)ptr) + realsize);

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

static int debug_function(CURL *handle, curl_infotype type,	char *data, size_t size, void *userp)
{
	const char *text;
	switch (type) {
	case CURLINFO_TEXT:
	{
		std::ostringstream oss;
		oss << "url download error : " << "data" << std::endl;
		LogSystem::Get()->Log(oss.str());
	}
	default:
		return 0;
	case CURLINFO_HEADER_OUT:
		text = "=> Send header";
		break;
	case CURLINFO_DATA_OUT:
		text = "=> Send data";
		break;
	case CURLINFO_SSL_DATA_OUT:
		text = "=> Send SSL data";
		break;
	case CURLINFO_HEADER_IN:
		text = "<= Recv header";
		break;
	case CURLINFO_DATA_IN:
		text = "<= Recv data";
		break;
	case CURLINFO_SSL_DATA_IN:
		text = "<= Recv SSL data";
		break;
	}

	return 0;
}


ErrorType URLDataReader::ConstructData(LotteryLineDataArray &lotterys)
{
	ErrorType result = DownloadDataFromURL();
	if (result == ErrorType::ET_NoError)
	{
		return ParseURLContent(lotterys);
	}

	return result;
}

static size_t get_content_length_func(char *buffer, size_t size,
	size_t nitems, void *userdata)
{
	std::string header(buffer);
	StringUtils::trim(header);

	if (header.find("Content-Length") != std::string::npos)
	{
		std::vector<std::string> parts;
		boost::split(parts, header, boost::is_any_of(":"));
		BOOST_ASSERT(parts.size() == 2);

		URLDataReader *reader = reinterpret_cast<URLDataReader*>(userdata);
		reader->SetContentSize(std::atoi(parts[1].c_str()));	
	}

	return size * nitems;
}

ErrorType URLDataReader::DownloadDataFromURL()
{
	CURL* handle = curl_easy_init();

	const std::string url = StringUtils::utf16_to_utf8(mURL);

	const bool isHttps = url.find("https") != std::string::npos;

	auto result = curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
	result = curl_easy_setopt(handle, CURLOPT_TIMEOUT, 100L);
	result = curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, 100L);

	result = curl_easy_setopt(handle, CURLOPT_NOPROGRESS, false);

	//curl_easy_setopt(handle, CURLOPT_PROGRESSFUNCTION, );

	result = curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data);
	result = curl_easy_setopt(handle, CURLOPT_WRITEDATA, this);

	/* please be verbose */
	curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(handle, CURLOPT_DEBUGFUNCTION, debug_function);

	curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, isHttps);

	curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, get_content_length_func);
	curl_easy_setopt(handle, CURLOPT_HEADERDATA, this);

	result = curl_easy_perform(handle);
	curl_easy_cleanup(handle);
	handle = nullptr;

	BOOST_ASSERT(mURLContentSize == mURLContent.size());

	if (result != CURLE_OK)
	{
		std::ostringstream oss;
		oss << "url perform failed code : " << result << std::endl;
		LogSystem::Get()->Log(oss.str());

		return ErrorType::ET_URLPreformError;
	}

	return ErrorType::ET_NoError;
}

std::wregex build_num_reg_obj();
ErrorType convert_string_data_num_to_lottery(const std::wstring &dataNum, LotteryLineData &lottery);

ErrorType URLDataReader::ParseURLContent(LotteryLineDataArray &lotterys)
{
	std::wstring utf16Content;
	try
	{
		std::string ss(mURLContent.begin(), mURLContent.end());
		utf16Content = StringUtils::utf8_to_utf16(ss);
	}
	catch (std::exception& e)
	{
		std::ostringstream oss;
		oss << "convert failed, throw exception, " << e.what();
		LogSystem::Get()->Log(oss.str());
	}

	const std::wstring tabID = L"id=\"lottery_tabs\"";
	auto foundPos = utf16Content.find(tabID);

	if (foundPos == std::wstring::npos)
		return ErrorType::ET_URLContent_NoLotteryTab;

	const std::wstring tableTab = L"<table";
	const auto tablePos = utf16Content.find(tableTab, foundPos);
	if (tablePos == std::wstring::npos)
		return ErrorType::ET_URLContent_NoLotteryTab_NoTable;

	const std::wstring tableTabEnd = L"</table>";


	auto tablePosEnd = utf16Content.find(tableTabEnd, tablePos);
	if (tablePos == std::wstring::npos)
		return ErrorType::ET_URLContent_NoLotteryTab_NoTableEnd;

	const std::wstring tableContent = utf16Content.substr(tablePos, tablePosEnd - tablePos);

	const std::wstring recordNodeName = L"<tr";
	const std::wstring recordNodeNameEnd = L"</tr>";

	std::vector<std::wstring> dataParts;
	boost::split(dataParts, tableContent, boost::is_any_of(L"\n"));

	std::wregex regNum = build_num_reg_obj();

	for (const auto &ss : dataParts)
	{
		std::wsmatch mResult;
		if (std::regex_search(ss, mResult, regNum))
		{
			lotterys.push_back(LotteryLineData());

			std::wstring dataNum = *mResult.begin();

			auto cvtResult = convert_string_data_num_to_lottery(dataNum, lotterys.back());
			if (cvtResult != ErrorType::ET_NoError)
				return cvtResult;
		}
	}

	return ErrorType::ET_NoError;
}

