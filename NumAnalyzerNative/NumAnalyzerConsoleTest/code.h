//.h文件
#include <string>
#include <codecvt>
/*    解码不同编码的文本文件类*/
typedef enum tagTextCodeType
{
    TextUnkonw = -1,
    TextANSI = 0,
    TextUTF8,
    TextUNICODE,
    TextUNICODE_BIG
}TextCodeType;

class CDecodeTextFile
{
public:
    CDecodeTextFile();
    ~CDecodeTextFile();
//interface
    std::wstring DecodeFileToWstring(const wchar_t *filePath);
    std::string DecodeFileToString(const wchar_t *filePath);

private:
    TextCodeType GetFileEncodeType(const wchar_t *filePath);

    std::wstring ReadString(TextCodeType type);
    std::wstring ReadAnsiString();
    std::wstring ReadUtf8String();
    std::wstring ReadUnicodeString();
    std::wstring ReadUnicodeBigString();

private:
    FILE  *m_fp;
};
