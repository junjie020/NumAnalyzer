// NumanalyzerNative.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "NumanalyzerNative.h"


// This is an example of an exported variable
NUMANALYZERNATIVE_API int nNumanalyzerNative=0;

// This is an example of an exported function.
NUMANALYZERNATIVE_API int fnNumanalyzerNative(void)
{
    return 42;
}

// This is the constructor of a class that has been exported.
// see NumanalyzerNative.h for the class definition