#include "stdafx.h"
#include "LogSystem.h"
#include "StringUtils.h"

bool LogSystem::Init(const std::wstring &filePath)
{
	mSinglton = new LogSystem(filePath);
	return mSinglton != nullptr;
}

void LogSystem::UnInit()
{
	delete mSinglton;
	mSinglton = nullptr;
}

LogSystem::LogSystem(const std::wstring &path)
	: mFS(path.c_str())
{
	BOOST_ASSERT(mSinglton == nullptr);	
}

void LogSystem::Log(const std::wstring &content, bool newLine /*= true*/)
{
	if (mFS)
	{
		if (newLine)
		{
			const std::wstring contentWithLine = content + L"\n";
			mFS.write(contentWithLine.c_str(), contentWithLine.size());
		}
		else
		{
			mFS.write(content.c_str(), content.size());
		}
	}	
}

void LogSystem::Log(const std::string &content, bool newLine /*= true*/)
{
	Log(StringUtils::utf8_to_utf16(content), newLine);
}

LogSystem* LogSystem::mSinglton;
