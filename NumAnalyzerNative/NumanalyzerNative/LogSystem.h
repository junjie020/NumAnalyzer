#pragma once


class LogSystem
{
public:
	static bool Init(const std::wstring &filePath);
	static void UnInit();

	static LogSystem* Get()
	{
		return mSinglton;
	}

public:
	LogSystem(const std::wstring &path);

public:
	void Log(const std::wstring &content, bool newLine = true);
	void Log(const std::string &content, bool newLine = true);

	template<typename StrType, typename Type>
	void LogImpl(const StrType &content, const Type &data);

private:
	std::wofstream	mFS;

	static LogSystem* mSinglton;
};

template<typename StrType, typename Type>
void LogSystem::LogImpl(const StrType &content, const Type &data)
{
	std::basic_ostringstream<typename StrType::traits_type::char_type, typename StrType::traits_type, typename StrType::allocator_type>	oss;
	oss << data;
	Log(oss.str());
}

#define LOG(_INFO)	LogSystem::Get()->Log(_INFO, true)
#define LOGEX(_INFO, _RHS_INFO)	LogSystem::Get()->LogImpl(std::string(_INFO), _RHS_INFO)