#pragma once

#include <string>
#include <map>

/*
replace the tags in a string to their values.
for example, "the name is:<name>\n"


remarks:
    tagnames can contain any characters except for "<" and ">".
	if the original string contains "<"(s) not belonging to a tag, convert them to "<<" first.


*/




//All the strings should be encoded with utf-8.
//result:
//    receive the result string. 
//    you can call result.reserve(...) before this function to boost the performance.
//src:
//    the string to be processed. The leading and trailing space (' ' and '\t')
//    in the tag names is ommitted.
//    you must ensure that "src" is null-terminated.
//NameAndValuePairs:
//    The tagname-and-value pairs. The leading and trailing space (' ' and '\t')
//    in NameAndValuePairs is NOT ommitted.
//return value:
//    the number of tags which have been replaced.
size_t ReplaceTags(
	__out std::string& result,
	const char *src,
	const std::map<const std::string, const std::string> &NameAndValuePairs
) noexcept;

#ifdef _DEBUG

void ReplaceTags_test(__out std::string &TestResult) noexcept;

#endif





/*

#include "ReplaceTags.h"

int _tmain(int argc, _TCHAR* argv[])
{

	using namespace std;

	string result;
	ReplaceTags_test(result);
	cout << result;

	
	return 0;
}



*/



