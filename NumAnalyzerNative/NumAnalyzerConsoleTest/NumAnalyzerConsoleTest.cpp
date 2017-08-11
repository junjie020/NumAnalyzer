// NumAnalyzerConsoleTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "NumanalyzerNative.h"

int main()
{
	const int buffersize = 1024 * 1024;
	char *buffer = new char[buffersize];
	fnNumanalyzerNative("../TestResource/TestData.txt", buffer, buffersize);

	delete[]buffer;
    return 0;
}

