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
	void Log(const std::wstring &content);
	void Log(const std::string &content);
private:
	std::wofstream	mFS;

	static LogSystem* mSinglton;
};