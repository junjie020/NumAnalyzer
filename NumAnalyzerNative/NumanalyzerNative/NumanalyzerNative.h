// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the NUMANALYZERNATIVE_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// NUMANALYZERNATIVE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef NUMANALYZERNATIVE_EXPORTS
#define NUMANALYZERNATIVE_API __declspec(dllexport)
#define EXPIMP_TEMPLATE
#else
#define NUMANALYZERNATIVE_API __declspec(dllimport)
#define EXPIMP_TEMPLATE extern
#endif

//EXPIMP_TEMPLATE template class NUMANALYZERNATIVE_API std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>;
//EXPIMP_TEMPLATE template class NUMANALYZERNATIVE_API std::basic_string<char, std::char_traits<char>, std::allocator<char>>;
//
//EXPIMP_TEMPLATE template class NUMANALYZERNATIVE_API std::wstring;
//EXPIMP_TEMPLATE template class NUMANALYZERNATIVE_API std::string;

// This class is exported from the NumanalyzerNative.dll
class NUMANALYZERNATIVE_API CNumanalyzerNative 
{
public:
	CNumanalyzerNative() = default;
	// TODO: add your methods here.
};

extern NUMANALYZERNATIVE_API int nNumanalyzerNative;

NUMANALYZERNATIVE_API int fnNumanalyzerNative(void);