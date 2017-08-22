// NumAnalyzerConsoleTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "NumanalyzerNative.h"

extern void test();
extern int test_beg(int argc, char **argv);

void test_analyzer()
{
	InitNative();
	const int bufferSize = 1024 * 1024;
	char *buffer = new char[bufferSize];
	NumAnalyzeNative("http://www.bwlc.net/bulletin/prevtrax.html", buffer, bufferSize, true);
	delete[] buffer;
}

int main(int argc, char* argv[])
{
	//std::wstring ss(L"<table> abc </table> <table>abc abc</table>");

	//auto pos = ss.find(L"</table>");

	test_analyzer();
	return 0;
}