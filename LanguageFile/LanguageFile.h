#pragma once

#include <map>


/*
LanguageFile format:

<language name 1>
<string name 1> value </>(or </string name 1>)
<string name 2> line1
line2
line3 </>(or </string name 1>)
</>(or </string name 1>)

if the string value contains "<", you should convert all the "<" to "<<" first.
the name of tags (<...>) can contain any characters except for "<" and ">".

*/










//result:
//    "string name / string value" pairs.
//    all these strings should be encoded with utf-8.
//buffer:
//    the entire content read from a language file. 
//    the language file should be encoded with utf-8.
//    you must ensure that "buffer" is null-terminated.
//LineEndingNew: 
//    convert all the original line endings("\r" or "\n" or "\r\n") to LineEndingNew.
//    pass nullptr to keep the original line endings.
//    pass "\0" to erase all the original line endings.
//greedy:
//    force searching the entire buffer, in case there are duplicate tags.
//return value:
//    return result.size()
//remarks:
//    the leading/trailing space (space and \t) in tags read from file will be ignored.
//    the leading/trailing space (space and \t) in parameter "LanguageName" will be ignored.
size_t ParseLangFile(
	__out std::map<std::string, std::string> &result,
	const char *buffer,
	const char *LanguageName,
	const char *LineEndingNew = nullptr,
	bool greedy = false
) noexcept;



#ifdef _DEBUG
// test
#include <string>

//TestResult is encoded by utf-8
void ParseLangFile_test(__out std::string &TestResult) noexcept;

#endif









/*


#include "LanguageFile.h"
#include <locale>
#include <memory>


static std::unique_ptr<wchar_t[]> UTF8toUTF16(const char *str1) noexcept
{
int size = MultiByteToWideChar(CP_UTF8, 0, str1, -1, nullptr, 0);
wchar_t *wstr = new wchar_t[size + 1];
MultiByteToWideChar(CP_UTF8, 0, str1, -1, wstr, size + 1);
return std::unique_ptr<wchar_t[]>(wstr);
}

int _tmain(int argc, _TCHAR* argv[])
{
using namespace std;

string TestResult;

ParseLangFile_test(TestResult);

//Some Unicode characters cannot be displayed properly in a console window.
//You can display TestResult with MessageBoxW, or write TestResult to a file.
MessageBoxW(nullptr, UTF8toUTF16(TestResult.c_str()).get(), (wchar_t*)u"test", MB_OK);



return 0;
}



*/